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
#define const_ival(node) node->semantic_value.const1->const_u.intval
#define const_fval(node) node->semantic_value.const1->const_u.fval
#define const_sval(node) node->semantic_value.const1->const_u.sc
#define expr_const_eval(node) node->semantic_value.exprSemanticValue.isConstEval
#define const_type(node) node->semantic_value.const1->const_type
#define expr_bin_op(node) node->semantic_value.exprSemanticValue.op.binaryOp
#define expr_uni_op(node) node->semantic_value.exprSemanticValue.op.unaryOp


#define get_int_reg() int_reg[(g_int_reg_cnt++) % 7]
#define get_float_reg() float_reg[(g_float_reg_cnt++) % 8]
#define get_addr_reg() addr_reg[(g_addr_reg_cnt++) % 7]


void codegen(AST_NODE *program);
void gen_global_var(AST_NODE *decl_list_node);
void gen_func_decl(AST_NODE *decl_node);
void gen_block(AST_NODE *block_node);
void gen_decl(AST_NODE *decl_node);
void gen_stmt(AST_NODE *stmt_node);
void gen_while(AST_NODE *stmt_node);
void gen_assign(AST_NODE *stmt_node);
void gen_if(AST_NODE *stmt_node);
char *gen_expr(AST_NODE *expr_node);
void gen_func_call(AST_NODE *stmt_node);
#endif
