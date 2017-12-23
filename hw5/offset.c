#include "header.h"
#include "offset.h"
#include "symbolTable.h"
#include <stdio.h>

int block_offset(AST_NODE *block_node, int offset)
{
    if (block_node->child == NULL || block_node->child->nodeType != VARIABLE_DECL_LIST_NODE) return offset;

    AST_NODE *id_node = block_node->child->child->child->rightSibling;
    while (id_node != NULL) {
        SymbolTableEntry *ste = id_node->semantic_value.identifierSemanticValue.symbolTableEntry;
        if (ste->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
            ArrayProperties properties = ste->attribute->attr.typeDescriptor->properties.arrayProperties;
            int array_size = 1;
            for (int i = 0; i < properties.dimension; i++) {
                array_size *= properties.sizeInEachDimension[i];
            }
            offset -= array_size * 4;
        } else {
            offset -= 4;
        }
        ste->offset = offset;
        id_node = id_node->rightSibling;
    }
    return offset;
}

void param_offset(AST_NODE *param_list_node, int offset)
{
    AST_NODE *child = param_list_node->child;
    offset += 16;
    while (child != NULL) {
        child->semantic_value.identifierSemanticValue.symbolTableEntry->offset = offset;
        offset += 4;
        child = child->rightSibling;
    }
}

int calc_offset(AST_NODE *node, int offset)
{
    AST_NODE *child = node->child;
    if (node->nodeType == DECLARATION_NODE && node->semantic_value.declSemanticValue.kind == FUNCTION_DECL) {
        offset = 0;
    } else if (node->nodeType == BLOCK_NODE) {
        offset = block_offset(node, offset);
    } else if (node->nodeType == PARAM_LIST_NODE) {
        param_offset(node, offset);
        return offset;
    }
    while (child != NULL) {
        int v = calc_offset(child, offset);
        offset = v < offset ? v : offset;
        child = child->rightSibling;
    }
    
    if (node->nodeType == DECLARATION_NODE && node->semantic_value.declSemanticValue.kind == FUNCTION_DECL) {
        AST_NODE *id_node = node->child->rightSibling;
        id_node->semantic_value.identifierSemanticValue.symbolTableEntry = retrieveSymbol(id_node->semantic_value.identifierSemanticValue.identifierName);
        id_node->semantic_value.identifierSemanticValue.symbolTableEntry->offset = offset;
        //printf("%s:%d\n", id_node->semantic_value.identifierSemanticValue.identifierName, offset);
    }
    
    return offset;
}

void offset_analysis(AST_NODE* program)
{
    calc_offset(program, 0);
}
