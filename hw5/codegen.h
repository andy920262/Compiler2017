#ifndef __CODEGEN_H__
#define __CODEGEN_H__

#include "header.h"


#define node_type(node) node->nodeType
#define data_type(node) node->dataType
#define id_name(node) node->semantic_value.identifierSemanticValue.identifierName

void codegen(AST_NODE *program);
void gen_function(AST_NODE *decl_node);
void gen_block(AST_NODE *block_node);
void gen_statement(AST_NODE *stmt_node);
#endif
