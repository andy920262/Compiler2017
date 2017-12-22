#include "header.h"
#include "codegen.h"
#include <stdio.h>

void codegen(AST_NODE *program) {
    AST_NODE *decl_node = program->child;
    while (decl_node != NULL) {
        if (node_type(decl_node) == VARIABLE_DECL_LIST_NODE) {
            /* TODO: variable declare */
        } else if (node_type(decl_node) == DECLARATION_NODE) {
            gen_function(decl_node);
        }
    }
}

void gen_function(AST_NODE *decl_node) {
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

    gen_block(block_node);

    /* TODO: Gen epilogue */
    printf("_end_%s:\n", name);
    printf("ldr x30, [x29, #8]\n");
    printf("add sp, x29, #8\n");
    printf("ldr x29, [x29, #0]\n");
    printf("RET x30\n");
    printf(".data\n");
    printf("_frameSize_%s: .word\n", name); // TODO: size after '.word'

}

void gen_block(AST_NODE *block_node) {

}
