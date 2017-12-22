#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include "header.h"


#define node_type(node) node->nodeType
#define data_type(node) node->dataType
#define id_sym(node) node->semantic_value.identifierSemanticValue.symbolTableEntry
#define id_name(node) node->semantic_value.identifierSemanticValue.identifierName
#define id_kind(node) node->semantic_value.identifierSemanticValue.kind
#define decl_kind(node) node->semantic_value.declSemanticValue.kind
#define stmt_kind(node) node->semantic_value.stmtSemanticValue.kind
#define expr_kind(node) node->semantic_value.exprSemanticValue.kind
#define sym_typedesc(sym) sym->attribute->attr.typeDescriptor
#define id_ival(node) node->child->semantic_value.const1->const_u.intval
#define id_fval(node) node->child->semantic_value.const1->const_u.fval
#define const_type(node) node->semantic_value.const1->const_type

typedef enum REG {
    w9, w10, w11, w12, w13, w14, w15,
    s16, s17, s18, s19, s20, s21, s22, s23
} REG;


char* int_reg = {
    "w9", "w10", "w11", "w12", "w13", "w14", "w15"
};

char* float_reg = {
    "s16", "s17", "s18", "s19", "s20", "s21", "s22", "s23"
};

void codegen(AST_NODE *program);
void gen_global_var(AST_NODE *decl_list_node);
void gen_func_decl(AST_NODE *decl_node);
void gen_block(AST_NODE *block_node);
void gen_decl(AST_NODE *decl_node);
void gen_stmt(AST_NODE *stmt_node);
void gen_while(AST_NODE *stmt_node);
void gen_assign(AST_NODE *stmt_node);
void gen_if(AST_NODE *stmt_node);
REG gen_expr(AST_NODE *expr_node);
void gen_func_call(AST_NODE *stmt_node);
#endif
