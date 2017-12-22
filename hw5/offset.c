#include "header.h"
#include "offset.h"
#include "symbolTable.h"

void visit(AST_NODE *node, int offset)
{
    AST_NODE *child = node->child;
    if (node->nodeType == DECLARATION_NODE && node->semantic_value.declSemanticValue.kind == FUNCTION_DECL) {
        while (child != NULL) {
            visit(child, offset);
        }
    } else if () {
        while (child != NULL) {
            visit(child, offset);
        }
    }
}

void offset_analysis(AST_NODE* program)
{
    visit(program, 0);
}
