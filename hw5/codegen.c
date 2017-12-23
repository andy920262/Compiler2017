#include "header.h"
#include "codegen.h"
#include "symbolTable.h"
#include <stdio.h>
#include <string.h>

FILE *fp;
int while_count = 0;
int if_count = 0;
int g_const_cnt = 0;

char *g_current_func;

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
    fp = fopen("output.s", "w");
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
    char *r1, *r2, *r3, *type;
    switch (node_type(expr_node)) {
        case EXPR_NODE:
            if (expr_const_eval(expr_node)) {
                if (data_type(expr_node) == INT_TYPE) {
                    r1 = get_int_reg();
                    fprintf(fp, "mov %s #%d\n", r1, const_ival(expr_node));
                } else if (data_type(expr_node) == FLOAT_TYPE) {
                    r1 = get_float_reg();
                    fprintf(fp, ".data\n");
                    fprintf(fp, "_CONST_%d: .float %f\n", g_const_cnt, const_fval(expr_node));
                    fprintf(fp, ".align 3\n");
                    fprintf(fp, ".text\n");
                    fprintf(fp, "ldr %s, =_CONST_%d\n", r1, g_const_cnt++);
                }
            } else if (expr_kind(expr_node) == BINARY_OPERATION) {
                if (data_type(expr_node) == INT_TYPE) {
                    r1 = get_int_reg();
                    type = "";
                } else if (data_type(expr_node) == FLOAT_TYPE) {
                    r1 = get_float_reg();
                    type = "f";
                }
                r2 = gen_expr(expr_node->child);
                r3 = gen_expr(expr_node->child->rightSibling);
                switch (expr_bin_op(expr_node)) {
                    case BINARY_OP_ADD: fprintf(fp, "%sadd %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_SUB: fprintf(fp, "%ssub %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_MUL: fprintf(fp, "%smul %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_DIV: fprintf(fp, "%sdiv %s, %s, %s\n", type[0] == 'f' ? "f" : "s", r1, r2, r3); break;
                    case BINARY_OP_AND: fprintf(fp, "%sand %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_OR: fprintf(fp, "%sorr %s, %s, %s\n", type, r1, r2, r3); break;
                    case BINARY_OP_EQ: case BINARY_OP_GE: case BINARY_OP_LE:
                    case BINARY_OP_NE: case BINARY_OP_GT: case BINARY_OP_LT:
                        fprintf(fp, "%scmp %s, %s\n", type, r2, r3);
                        switch (expr_bin_op(expr_node)) {
                            case BINARY_OP_EQ: fprintf(fp, "cset %s, eq\n", r1); break;
                            case BINARY_OP_GE: fprintf(fp, "cset %s, ge\n", r1); break;
                            case BINARY_OP_LE: fprintf(fp, "cset %s, le\n", r1); break;
                            case BINARY_OP_NE: fprintf(fp, "cset %s, ne\n", r1); break;
                            case BINARY_OP_GT: fprintf(fp, "cset %s, gt\n", r1); break;
                            case BINARY_OP_LT: fprintf(fp, "cset %s, lt\n", r1); break;
                            default: break;
                        }
                    default: break;
                }
            } else if (expr_kind(expr_node) == UNARY_OPERATION) {
                r1 = gen_expr(expr_node->child);
                type = data_type(expr_node) == FLOAT_TYPE ? "f" : "";
                switch (expr_uni_op(expr_node)) {
                    case UNARY_OP_POSITIVE: break;
                    case UNARY_OP_NEGATIVE: fprintf(fp, "%sneg %s, %s\n", type, r1, r1); break;
                    case UNARY_OP_LOGICAL_NEGATION: fprintf(fp, "ands %s, #0\n", r1); break;
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
            r1 = get_int_reg();
            if (id_sym(expr_node)->nestingLevel == 0) {
                r2 = get_addr_reg();
                fprintf(fp, "ldr %s, =_g_%s\n", r2, id_name(expr_node));
                fprintf(fp, "ldr %s, [%s, #0]\n", r1, r2);
            } else {
                fprintf(fp, "ldr %s, [x29, #%d]\n", r1, id_sym(expr_node)->offset);
            }
            break;

        case CONST_VALUE_NODE:
            if (const_type(expr_node) == INTEGERC) {
                r1 = get_int_reg();
                fprintf(fp, "mov %s, #%d\n", r1, const_ival(expr_node));
            } else if (const_type(expr_node) == FLOATC) {
                r1 = get_float_reg();
                fprintf(fp, ".data\n");
                fprintf(fp, "_CONST_%d: .float %f\n", g_const_cnt, const_fval(expr_node));
                fprintf(fp, ".align 3\n");
                fprintf(fp, ".text\n");
                fprintf(fp, "ldr %s, =_CONST_%d\n", r1, g_const_cnt++);
            } else if (const_type(expr_node) == STRINGC) {
                r1 = get_addr_reg();
                fprintf(fp, ".data\n");
                fprintf(fp, "_CONST_%d: .ascii %s\n", g_const_cnt, const_sval(expr_node));
                fprintf(fp, ".align 3\n");
                fprintf(fp, ".text\n");
                fprintf(fp, "ldr %s, =_CONST_%d\n", r1, g_const_cnt++);
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
        fprintf(fp, "mov w0, %s\n", reg);
    } else {
        fprintf(fp, "fmov s0, %s\n", reg);
    }
    fprintf(fp, "bl _end_%s\n", g_current_func);
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

    fprintf(fp, "%s:\n", test_label);
    test_r = gen_expr(expr_node);

    if (data_type(expr_node) == INT_TYPE) {
        fprintf(fp, "cmp %s, #0\n", test_r);
    } else if (data_type(expr_node) == FLOAT_TYPE) {
        fprintf(fp, "fcmp %s, #0\n", test_r);
    }

    fprintf(fp, "beq %s\n", exit_label);
    gen_block(block_node);
    fprintf(fp, "b %s\n", test_label);
    fprintf(fp, "%s:\n", exit_label);
}

void gen_assign(AST_NODE *stmt_node) {
    AST_NODE *lhs = stmt_node->child;
    char *rhs = gen_expr(lhs->rightSibling);

    if (id_kind(lhs) == NORMAL_ID) {
        if (id_sym(lhs)->nestingLevel == 0) {
            char *lhs_id = id_sym(lhs)->name;
            char *lhs_reg = get_addr_reg();
            fprintf(fp, "ldr %s, =_g_%s\n", lhs_reg, lhs_id);
            fprintf(fp, "str %s, [%s, #0]\n", rhs, lhs_reg);
        } else {
            fprintf(fp, "str %s, [x29, #%d]\n", rhs, id_sym(lhs)->offset);
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
        fprintf(fp, "cmp %s, #0\n", test_r);
    } else if (data_type(expr_node) == FLOAT_TYPE) {
        fprintf(fp, "fcmp %s, #0\n", test_r);
    }

    if (node_type(else_node) == NUL_NODE) { /* simple if*/
        fprintf(fp, "beq %s\n", exit_label);
        gen_block(block_node);
        fprintf(fp, "%s:\n", exit_label);
    } else if (node_type(else_node) == BLOCK_NODE) { /* if then else*/
        fprintf(fp, "beq %s\n", else_label);
        gen_block(block_node);
        fprintf(fp, "b %s\n", exit_label);
        fprintf(fp, "%s:\n", else_label);
        gen_block(else_node);
        fprintf(fp, "%s:\n", exit_label);
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
                fprintf(fp, "mov w0, %s\n", reg);
                fprintf(fp, "bl _write_int\n");
                break;
            case FLOAT_TYPE:
                fprintf(fp, "fmov s0, %s\n", reg);
                fprintf(fp, "bl _write_float\n");
                break;
            case CONST_STRING_TYPE:
                fprintf(fp, "mov x0, %s\n", reg);
                fprintf(fp, "bl _write_str\n");
                break;
            default: break;
        }
    } else if (strcmp(func_name, "read") == 0) {
        fprintf(fp, "bl _read_int\n");
    } else if (strcmp(func_name, "fread") == 0) {
        fprintf(fp, "bl _read_float\n");
    } else {
        fprintf(fp, "bl _start_%s\n", func_name);
    }
}

void gen_global_var(AST_NODE *decl_list_node) {
    fprintf(fp, ".data\n");
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
                            fprintf(fp, "_g_%s: .word 0\n", id_name(id_node));
                        } else {
                            fprintf(fp, "_g_%s: .float 0.0\n", id_name(id_node));
                        }
                        break;
                    case ARRAY_ID:
                        /* TODO: global array */
                        break;
                    case WITH_INIT_ID:
                        value = const_type(id_node->child) == FLOATC ? const_fval(id_node->child) : const_ival(id_node->child);
                        if (data_type(type_node) == INT_TYPE) {
                            fprintf(fp, "_g_%s: .word %d\n", id_name(id_node), (int)value);
                        } else {
                            fprintf(fp, "_g_%s: .float %f\n", id_name(id_node), value);
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
    fprintf(fp, ".text\n");
    fprintf(fp, "_start_%s:\n", name);

    /* Gen prologue */
    fprintf(fp, "str x30, [sp, #0]\n");
    fprintf(fp, "str x29, [sp, #-8]\n");
    fprintf(fp, "add x29, sp, #-8\n");
    fprintf(fp, "add sp, sp, #-16\n");
    fprintf(fp, "ldr x30, =_frameSize_%s\n", name);
    fprintf(fp, "ldr w30, [x30, #0]\n");
    fprintf(fp, "sub sp, sp, w30\n");

    /* Gen callee save */
    fprintf(fp, "str x9, [sp, #8]\n");
    fprintf(fp, "str x10, [sp, #16]\n");
    fprintf(fp, "str x11, [sp, #24]\n");
    fprintf(fp, "str x12, [sp, #32]\n");
    fprintf(fp, "str x13, [sp, #40]\n");
    fprintf(fp, "str x14, [sp, #48]\n");
    fprintf(fp, "str x15, [sp, #56]\n");
    fprintf(fp, "str s16, [sp, #64]\n");
    fprintf(fp, "str s17, [sp, #68]\n");
    fprintf(fp, "str s18, [sp, #72]\n");
    fprintf(fp, "str s19, [sp, #76]\n");
    fprintf(fp, "str s20, [sp, #80]\n");
    fprintf(fp, "str s21, [sp, #84]\n");
    fprintf(fp, "str s22, [sp, #88]\n");
    fprintf(fp, "str s23, [sp, #92]\n");


    gen_block(block_node);

    /* Gen callee restore */
    fprintf(fp, "_end_%s:\n", name);
    fprintf(fp, "ldr x9, [sp, #8]\n");
    fprintf(fp, "ldr x10, [sp, #16]\n");
    fprintf(fp, "ldr x11, [sp, #24]\n");
    fprintf(fp, "ldr x12, [sp, #32]\n");
    fprintf(fp, "ldr x13, [sp, #40]\n");
    fprintf(fp, "ldr x14, [sp, #48]\n");
    fprintf(fp, "ldr x15, [sp, #56]\n");
    fprintf(fp, "ldr s16, [sp, #64]\n");
    fprintf(fp, "ldr s17, [sp, #68]\n");
    fprintf(fp, "ldr s18, [sp, #72]\n");
    fprintf(fp, "ldr s19, [sp, #76]\n");
    fprintf(fp, "ldr s20, [sp, #80]\n");
    fprintf(fp, "ldr s21, [sp, #84]\n");
    fprintf(fp, "ldr s22, [sp, #88]\n");
    fprintf(fp, "ldr s23, [sp, #92]\n");

    /* Gen epilogue */
    fprintf(fp, "ldr x30, [x29, #8]\n");
    fprintf(fp, "add sp, x29, #8\n");
    fprintf(fp, "ldr x29, [x29, #0]\n");
    fprintf(fp, "RET x30\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "_frameSize_%s: .word %d\n", name, 92 - id_sym(id_node)->offset);

}

void gen_decl(AST_NODE *decl_node) {
    AST_NODE *id_node = decl_node->child->rightSibling;
    while (id_node != NULL) {
        if (id_kind(id_node) == WITH_INIT_ID) {
            fprintf(fp, "str %s, [sp, %d]\n", gen_expr(id_node->child), id_sym(id_node)->offset);
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
