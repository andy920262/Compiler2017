#include "header.h"
#include "codegen.h"
#include "symbolTable.h"
#include <stdio.h>

FILE fp;
int while_count = 0;

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


REG gen_expr(AST_NODE *expr_node) {
    REG r1, r2, r3;
    if (data_type(expr_node) == INT_TYPE) {
        r1 = w9, r2 = w10, r3 = w11;
    } else {
        r1 = s16, r2 = s17, r3 = s18;
    }
    switch (node_type(expr_rel_node)) {
        case EXPR_NODE:
            gen_expr(expr_rel_node);
            break;
        case STMT_NODE:
            gen_func_call(expr_rel_node);
            return X0;
        case IDENTIFIER_NODE:
            break;
        case CONST_VALUE_NODE:
            break;
        default:
            break;
    }

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

void gen_while(AST_NODE* stmt_node) {
    while_count++;
    REG test_r;
    AST_NODE* block_node = stmt_node->child->rightSibling;
    char test_label[15];
    char exit_label[15];
    sprintf(test_label, "_while_test%d", while_count);
    sprintf(exit_label, "_while_exit%d", while_count);

    printf("%s:\n", test_label);
    test_r = gen_expr(stmt_node->child);
    if (test_r < 7) {
        printf("cmp w%d, #0\n", test_r);
    } else {
        printf("fcmp s%d, #0\n", test_r);
    }
    printf("beq %s\n", exit_label);
    gen_block(block_node);
    printf("b %s\n", test_label);
    printf("%s:\n", exit_label);
}

void gen_assign(AST_NODE* stmt_node) {
}

void gen_if(AST_NODE* stmt_node) {
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
                        value = const_type(id_node->child) == FLOATC ? id_ival(id_node) : id_fval(id_node);
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
