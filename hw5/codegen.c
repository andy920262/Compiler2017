#include "header.h"
#include "codegen.h"
#include "symbolTable.h"
#include <stdio.h>

FILE fp;
int while_count = 0;
int g_const_cnt = 0;

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

    char *r1, *r2, *r3;
    char type;
    switch (node_type(expr_node)) {
        case EXPR_NODE:
            if (expr_const_eval(expr_node)) {
                if (data_type(expr_node) == INT_TYPE) {
                    r1 = get_int_reg();
                    printf("mov %s #%d\n", r1, const_ival(expr_node));
                } else if (data_type(expr_node) == FLOAT_TYPE) {
                    r1 = get_float_reg();
                    printf(".data\n");
                    printf("__CONST_%d: .float %f", g_const_cnt, const_fval(expr_node));
                    printf(".text");
                    printf("ldr %s, =_CONST_%d", r1, g_const_cnt++);
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
                    case BINARY_OP_ADD: printf("%cadd %s, %s, %s", type, r1, r2, r3); break;
                    case BINARY_OP_SUB: printf("%csub %s, %s, %s", type, r1, r2, r3); break;
                    case BINARY_OP_MUL: printf("%cmul %s, %s, %s", type, r1, r2, r3); break;
                    case BINARY_OP_DIV: printf("%cdiv %s, %s, %s", type == 'f' ? 's' : 'f', r1, r2, r3); break;
                    case BINARY_OP_AND: printf("%cand %s, %s, %s", type, r1, r2, r3); break;
                    case BINARY_OP_OR: printf("%corr %s, %s, %s", type, r1, r2, r3); break;
                    case BINARY_OP_EQ: case BINARY_OP_GE: case BINARY_OP_LE:
                    case BINARY_OP_NE: case BINARY_OP_GT: case BINARY_OP_LT:
                        printf("%ccmp %s, %s", type, r2, r3);
                        switch (expr_bin_op(expr_node)) {
                            case BINARY_OP_EQ: printf("cset %s, eq", r1); break;
                            case BINARY_OP_GE: printf("cset %s, ge", r1); break;
                            case BINARY_OP_LE: printf("cset %s, le", r1); break;
                            case BINARY_OP_NE: printf("cset %s, ne", r1); break;
                            case BINARY_OP_GT: printf("cset %s, gt", r1); break;
                            case BINARY_OP_LT: printf("cset %s, lt", r1); break;
                            default: break;
                        }
                    default: break;
                }
            } else if (expr_kind(expr_node) == UNARY_OPERATION) {
                r1 = gen_expr(expr_node->child);
                switch (expr_uni_op(expr_node)) {
                    case UNARY_OP_POSITIVE: break;
                    case UNARY_OP_NEGATIVE: printf("%csub %s, , %s", r1, REG[lhs]); break;
                    case UNARY_OP_LOGICAL_NEGATION:
                        emit("vcmp.f32 %s, #0", REG[lhs]);
                        emit("vmrs apsr_nzcv, fpscr");
                        emit("moveq %s, #1", REG[dst]);
                        emit("movne %s, #0", REG[dst]);
                        break;
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
            } else if (data_type(expr_node) == CONST_STRING_TYPE) {
                r1 = x0;
            }
            break;

        case IDENTIFIER_NODE:
            if (data_type(expr_node) == INT_TYPE) {
                r1 = get_int_reg();
            } else if (data_type(expr_node) == FLOAT_TYPE) {
                r1 = get_float_reg();
            }
            printf("ldr %s [fp #%d]\n", r1, id_sym(expr_node)->offset);
            break;

        case CONST_VALUE_NODE:
            if (const_type(expr_node) == INTEGERC) {
                r1 = get_int_reg();
                printf("mov %s #%d\n", r1, const_ival(expr_node));
            } else if (const_type(expr_node) == FLOATC) {
                r1 = get_float_reg();
                printf(".data\n");
                printf("__CONST_%d: .float %f", g_const_cnt, const_fval(expr_node));
                printf(".text");
                printf("ldr %s, =_CONST_%d", r1, g_const_cnt++);
            } else if (const_type(expr_node) == STRINGC) {
                r1 = get_addr_reg();
                printf(".data\n");
                printf("__CONST_%d: .ascii %s", g_const_cnt, const_sval(expr_node));
                printf(".text");
                printf("ldr %s, =_CONST_%d", r1, g_const_cnt++);
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

            break;
        case FUNCTION_CALL_STMT:
            gen_func_call(stmt_node);
            break;
defualt:
            break;
    }

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

void gen_assign(AST_NODE *stmt_node) {  /*TODO*/
    AST_NODE *lhs = stmt_node->child;
    AST_NODE *rhs = lhs->rightSibling;

    if (id_kind(lhs) == NORMAL_ID) {

    } else {

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
    } else {continue;} /* if then else if*/
}

void gen_func_call(AST_NODE *stmt_node) {
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
                        value = const_type(id_node->child) == FLOATC ? const_ival(id_node->child) : const_fval(id_node->child);
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
    printf("_end_%s:\n", name);
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
            /*  TODO: simple constant initialization */
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
        } else if (node_type(child) == STMT_NODE) {
            AST_NODE *stmt_node = child->child;
            while (stmt_node != NULL) {
                gen_stmt(child);
                stmt_node = stmt_node->rightSibling;
            }
        } else {
            /* NULL node */
        }
        child = child->rightSibling;
    }
}
