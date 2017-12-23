#include "header.h"
#include "codegen.h"
#include "symbolTable.h"
#include <stdio.h>
#include <string.h>

FILE fp;
int while_count = 0;
int if_count = 0;
int g_const_cnt = 0;

char *g_current_func;

int g_int_reg_cnt = 0;
int g_float_reg_cnt = 0;
int g_addr_reg_cnt = 0;

char *x0 = "x0";
char *w0 = "w0";
char *s0 = "s0";

char *addr_reg[] = {
    "x9", "x10", "x11", "x12", "x13", "x14", "x15"
};
char *int_reg[] = {
    "w9", "w10", "w11", "w12", "w13", "w14", "w15"
};
char *float_reg[] = {
    "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23"
};

void codegen(AST_NODE *program) {
    AST_NODE *child = program->child;
    while (child != NULL) {
        if (node_type(child) == VARIABLE_DECL_LIST_NODE) {
            gen_global_var(child);
        } else if (node_type(child) == DECLARATION_NODE) {
            gen_func_decl(child);
        }
        child = child->rightSibling;
    }
}


char *gen_expr(AST_NODE *expr_node) {
    char *r1, *r2, *r3, type;
    switch (node_type(expr_node)) {
        case EXPR_NODE:
            if (expr_const_eval(expr_node)) {
                if (data_type(expr_node) == INT_TYPE) {
                    r1 = get_int_reg();
                    printf("mov %s #%d\n", r1, const_ival(expr_node));
                } else if (data_type(expr_node) == FLOAT_TYPE) {
                    r1 = get_float_reg();
                    printf(".data\n");
                    printf("__CONST_%d: .float %f\n", g_const_cnt, const_fval(expr_node));
                    printf(".text\n");
                    printf("ldr %s, =_CONST_%d\n", r1, g_const_cnt++);
                }
            } else if (expr_kind(expr_node) == BINARY_OPERATION) {
                if (data_type(expr_node) == INT_TYPE) {
                    r1 = get_int_reg();
                    type = '\0';
                } else if (data_type(expr_node) == FLOAT_TYPE) {
                    r1 = get_float_reg();
                    type = 'f';
                }
                r2 = gen_expr(expr_node->child);
                r3 = gen_expr(expr_node->child->rightSibling);
                switch (expr_bin_op(expr_node)) {
                    case BINARY_OP_ADD: printf("%cadd %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_SUB: printf("%csub %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_MUL: printf("%cmul %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_DIV: printf("%cdiv %s, %s, %s\n", type == 'f' ? 'f' : 's', r1, r2, r3); break;
                    case BINARY_OP_AND: printf("%cand %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_OR: printf("%corr %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_EQ: case BINARY_OP_GE: case BINARY_OP_LE:
                    case BINARY_OP_NE: case BINARY_OP_GT: case BINARY_OP_LT:
                        printf("%ccmp %s, %s\n", type, r2, r3);
                        switch (expr_bin_op(expr_node)) {
                            case BINARY_OP_EQ: printf("cset %s, eq\n", r1); break;
                            case BINARY_OP_GE: printf("cset %s, ge\n", r1); break;
                            case BINARY_OP_LE: printf("cset %s, le\n", r1); break;
                            case BINARY_OP_NE: printf("cset %s, ne\n", r1); break;
                            case BINARY_OP_GT: printf("cset %s, gt\n", r1); break;
                            case BINARY_OP_LT: printf("cset %s, lt\n", r1); break;
                            default: break;
                        }
                    default: break;
                }
            } else if (expr_kind(expr_node) == UNARY_OPERATION) {
                r1 = gen_expr(expr_node->child);
                type = data_type(expr_node) == FLOAT_TYPE ? 'f' : '\0';
                switch (expr_uni_op(expr_node)) {
                    case UNARY_OP_POSITIVE: break;
                    case UNARY_OP_NEGATIVE: printf("%cneg %s, %s\n", type, r1, r1); break;
                    case UNARY_OP_LOGICAL_NEGATION: printf("ands %s, #0\n", r1); break;
                    default: ;
                }
            }
            break;

        case STMT_NODE:
            gen_func_call(expr_node);
            if (data_type(expr_node) == INT_TYPE) {
                r1 = w0;
            } else if (data_type(expr_node) == FLOAT_TYPE) {
                r1 = s0;
            }
            break;

        case IDENTIFIER_NODE:
            if (data_type(expr_node) == INT_TYPE) {
                r1 = get_int_reg();
            } else if (data_type(expr_node) == FLOAT_TYPE) {
                r1 = get_float_reg();
            }
            if (id_sym(expr_node)->nestingLevel == 0) {
                printf("ldr %s, =_g_%s\n", r1, id_name(expr_node));
            } else {
                printf("ldr %s [fp #%d]\n", r1, id_sym(expr_node)->offset);
            }
            break;

        case CONST_VALUE_NODE:
            if (const_type(expr_node) == INTEGERC) {
                r1 = get_int_reg();
                printf("mov %s #%d\n", r1, const_ival(expr_node));
            } else if (const_type(expr_node) == FLOATC) {
                r1 = get_float_reg();
                printf(".data\n");
                printf("__CONST_%d: .float %f\n", g_const_cnt, const_fval(expr_node));
                printf(".text\n");
                printf("ldr %s, =_CONST_%d\n", r1, g_const_cnt++);
            } else if (const_type(expr_node) == STRINGC) {
                r1 = get_addr_reg();
                printf(".data\n");
                printf("__CONST_%d: .ascii %s\n", g_const_cnt, const_sval(expr_node));
                printf(".text\n");
                printf("ldr %s, =_CONST_%d\n", r1, g_const_cnt++);
            }
            break;
        default: break;
    }
    return r1;
}

void gen_stmt(AST_NODE *stmt_node) {
    if (node_type(stmt_node) == BLOCK_NODE) {
        gen_block(stmt_node);
        return;
    }
    switch (stmt_kind(stmt_node)) {
        case WHILE_STMT:
            gen_while(stmt_node);
            break;
        case FOR_STMT:
            /* TODO: for statement */
            break;
        case IF_STMT:
            gen_if(stmt_node);
            break;
        case ASSIGN_STMT:
            gen_assign(stmt_node);
            break;
        case RETURN_STMT:
            gen_return(stmt_node);
            break;
        case FUNCTION_CALL_STMT:
            gen_func_call(stmt_node);
            break;
defualt:
            break;
    }

}
void gen_return(AST_NODE *stmt_node) {
    AST_NODE *expr_node = stmt_node->child;
    char *reg = gen_expr(expr_node);
    if (data_type(stmt_node) == INT_TYPE) {
        printf("mov w0, %s\n", reg);
    } else {
        printf("fmov s0, %s\n", reg);
    }
    printf("bl _end_%s\n", g_current_func);
}

void gen_while(AST_NODE *stmt_node) {
    while_count++;
    char *test_r;
    AST_NODE *block_node = stmt_node->child->rightSibling;
    AST_NODE *expr_node = stmt_node->child;
    char test_label[15];
    char exit_label[15];
    sprintf(test_label, "_while_test%d", while_count);
    sprintf(exit_label, "_while_exit%d", while_count);

    printf("%s:\n", test_label);
    test_r = gen_expr(expr_node);

    if (data_type(expr_node) == INT_TYPE) {
        printf("cmp %s, #0\n", test_r);
    } else if (data_type(expr_node) == FLOAT_TYPE) {
        printf("fcmp %s, #0\n", test_r);
    }

    printf("beq %s\n", exit_label);
    gen_block(block_node);
    printf("b %s\n", test_label);
    printf("%s:\n", exit_label);
}

void gen_assign(AST_NODE *stmt_node) {
    AST_NODE *lhs = stmt_node->child;
    char *rhs = gen_expr(lhs->rightSibling);

    if (id_kind(lhs) == NORMAL_ID) {
        if (id_sym(lhs)->nestingLevel == 0) {
            char *lhs_id = id_sym(lhs)->name;
            char *lhs_reg = get_addr_reg();
            printf("ldr %s, =%s\n", lhs_reg, lhs_id);
            printf("str %s, [%s, #0]\n", rhs, lhs_reg);
        } else {
            printf("str %s, [x29, #%d]\n", rhs, id_sym(lhs)->offset);
        }
    } else if (id_kind(lhs) == ARRAY_ID) {
        /* TODO: array assignment*/
    }
}

void gen_if(AST_NODE *stmt_node) {
    if_count++;
    char *test_r;
    AST_NODE *expr_node = stmt_node->child;
    AST_NODE *block_node = expr_node->rightSibling;
    AST_NODE *else_node = block_node->rightSibling;
    char exit_label[15];
    char else_label[15];
    sprintf(exit_label, "_exit_label%d", if_count);
    sprintf(else_label, "_else_label%d", if_count);

    test_r = gen_expr(expr_node);

    if (data_type(expr_node) == INT_TYPE) {
        printf("cmp %s, #0\n", test_r);
    } else if (data_type(expr_node) == FLOAT_TYPE) {
        printf("fcmp %s, #0\n", test_r);
    }

    if (node_type(else_node) == NUL_NODE) { /* simple if*/
        printf("beq %s\n", exit_label);
        gen_block(block_node);
        printf("%s:\n", exit_label);
    } else if (node_type(else_node) == BLOCK_NODE) { /* if then else*/
        printf("beq %s\n", else_label);
        gen_block(block_node);
        printf("b %s\n", exit_label);
        printf("%s:\n", else_label);
        gen_block(else_node);
        printf("%s:\n", exit_label);
    } else {
        /* TODO: if then else if*/
    }
}

void gen_func_call(AST_NODE *stmt_node) {
    AST_NODE *id_node = stmt_node->child;
    AST_NODE *param_list_node = id_node->rightSibling;
    char *func_name = id_name(id_node);
    if (strcmp(func_name, "write") == 0) {
        AST_NODE *param_node = param_list_node->child;
        char *reg = gen_expr(param_node);
        switch (data_type(param_node)) {
            case INT_TYPE:
                printf("mov w0, %s\n", reg);
                printf("bl _write_int\n");
                break;
            case FLOAT_TYPE:
                printf("fmov s0, %s\n", reg);
                printf("bl _write_float\n");
                break;
            case CONST_STRING_TYPE:
                printf("mov x0, %s\n", reg);
                printf("bl _write_str\n");
                break;
            default: break;
        }
    } else if (strcmp(func_name, "read") == 0) {
        printf("bl _read_int\n");
    } else if (strcmp(func_name, "fread") == 0) {
        printf("bl _read_float\n");
    } else {
        printf("bl _start_%s\n", func_name);
    }
}

void gen_global_var(AST_NODE *decl_list_node) {
    printf(".data\n");
    AST_NODE *decl_node = decl_list_node->child;
    while (decl_node != NULL) {
        if (decl_kind(decl_node) == VARIABLE_DECL) {
            AST_NODE *type_node = decl_node->child;
            AST_NODE *id_node = type_node->rightSibling;
            float value;
            while (id_node != NULL) {
                switch (id_kind(id_node)) {
                    case NORMAL_ID:
                        if (data_type(type_node) == INT_TYPE) {
                            printf("_g_%s: .word 0\n", id_name(id_node));
                        } else {
                            printf("_g_%s: .float 0.0\n", id_name(id_node));
                        }
                        break;
                    case ARRAY_ID:
                        /* TODO: global array */
                        break;
                    case WITH_INIT_ID:
                        value = const_type(id_node->child) == FLOATC ? const_fval(id_node->child) : const_ival(id_node->child);
                        if (data_type(type_node) == INT_TYPE) {
                            printf("_g_%s: .word %d\n", id_name(id_node), (int)value);
                        } else {
                            printf("_g_%s: .float %f\n", id_name(id_node), value);
                        }
                        break;
                    default:
                        break;
                }
                id_node = id_node->rightSibling;
            }
        }
        decl_node = decl_node->rightSibling;
    }

}

void gen_func_decl(AST_NODE *decl_node) {
    AST_NODE *type_node = decl_node->child;
    AST_NODE *id_node = type_node->rightSibling;
    AST_NODE *block_node = id_node->rightSibling->rightSibling;
    char *name = id_name(id_node);
    g_current_func = name;

    /* Gen head */
    printf(".text\n");
    printf("_start_%s:\n", name);

    /* Gen prologue */
    printf("str x30, [sp, #0]\n");
    printf("str x29, [sp, #-8]\n");
    printf("add x29, sp, #-8\n");
    printf("add sp, sp, #-16\n");
    printf("ldr x30, =_frameSize_%s\n", name);
    printf("ldr w30, [x30, #0]\n");
    printf("sub sp, sp, w30\n");

    /* Gen callee save */
    printf("str x9, [sp, #8]\n");
    printf("str x10, [sp, #16]\n");
    printf("str x11, [sp, #24]\n");
    printf("str x12, [sp, #32]\n");
    printf("str x13, [sp, #40]\n");
    printf("str x14, [sp, #48]\n");
    printf("str x15, [sp, #56]\n");
    printf("str s16, [sp, #64]\n");
    printf("str s17, [sp, #68]\n");
    printf("str s18, [sp, #72]\n");
    printf("str s19, [sp, #76]\n");
    printf("str s20, [sp, #80]\n");
    printf("str s21, [sp, #84]\n");
    printf("str s22, [sp, #88]\n");
    printf("str s23, [sp, #92]\n");


    gen_block(block_node);

    /* Gen callee restore */
    printf("_end_%s:\n", name);
    printf("ldr x9, [sp, #8]\n");
    printf("ldr x10, [sp, #16]\n");
    printf("ldr x11, [sp, #24]\n");
    printf("ldr x12, [sp, #32]\n");
    printf("ldr x13, [sp, #40]\n");
    printf("ldr x14, [sp, #48]\n");
    printf("ldr x15, [sp, #56]\n");
    printf("ldr s16, [sp, #64]\n");
    printf("ldr s17, [sp, #68]\n");
    printf("ldr s18, [sp, #72]\n");
    printf("ldr s19, [sp, #76]\n");
    printf("ldr s20, [sp, #80]\n");
    printf("ldr s21, [sp, #84]\n");
    printf("ldr s22, [sp, #88]\n");
    printf("ldr s23, [sp, #92]\n");

    /* Gen epilogue */
    printf("ldr x30, [x29, #8]\n");
    printf("add sp, x29, #8\n");
    printf("ldr x29, [x29, #0]\n");
    printf("RET x30\n");
    printf(".data\n");
    printf("_frameSize_%s: .word %d\n", name, 92 - id_sym(id_node)->offset);

}

void gen_decl(AST_NODE *decl_node) {
    AST_NODE *id_node = decl_node->child->rightSibling;
    while (id_node != NULL) {
        if (id_kind(id_node) == WITH_INIT_ID) {
            printf("str %s, [sp, %d]\n", gen_expr(id_node->child), id_sym(id_node)->offset);
        }
        id_node = id_node->rightSibling;
    }
}

void gen_block(AST_NODE *block_node) {
    AST_NODE *child = block_node->child;
    int frame_size = 0;
    while (child != NULL) {
        if (node_type(child) == VARIABLE_DECL_LIST_NODE) {
            AST_NODE *decl_node = child->child;
            while (decl_node != NULL) {
                if (decl_kind(decl_node) == VARIABLE_DECL) {
                    gen_decl(decl_node);
                }
                decl_node = decl_node->rightSibling;
            }
        } else if (node_type(child) == STMT_LIST_NODE) {
            AST_NODE *stmt_node = child->child;
            while (stmt_node != NULL) {
                gen_stmt(stmt_node);
                stmt_node = stmt_node->rightSibling;
            }
        } else {
            /* NULL node */
        }
        child = child->rightSibling;
    }
}
