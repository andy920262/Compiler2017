#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
#define DEBUG puts("aaa");
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 assignment document. //
int g_anyErrorOccur = 0;

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void processProgramNode(AST_NODE *programNode);
void processDeclarationNode(AST_NODE* declarationNode);
void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize);
void declareFunction(AST_NODE* returnTypeNode);
void processDeclDimList(AST_NODE* variableDeclDimList, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize);
void processTypeNode(AST_NODE* typeNode);
void processBlockNode(AST_NODE* blockNode);
void processStmtNode(AST_NODE* stmtNode);
void processGeneralNode(AST_NODE *node);
void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void checkWhileStmt(AST_NODE* whileNode);
void checkForStmt(AST_NODE* forNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void checkIfStmt(AST_NODE* ifNode);
void checkWriteFunction(AST_NODE* functionCallNode);
void checkFunctionCall(AST_NODE* functionCallNode);
void processExprRelatedNode(AST_NODE* exprRelatedNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter);
void checkReturnStmt(AST_NODE* returnNode);
void processExprNode(AST_NODE* exprNode);
void processVariableLValue(AST_NODE* idNode);
void processVariableRValue(AST_NODE* idNode);
void processConstValueNode(AST_NODE* constValueNode);
void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue);
void evaluateExprValue(AST_NODE* exprNode);

char* getIdNodeName(AST_NODE *node);

typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_UNDECLARED,
    NOT_FUNCTION_NAME,
    TRY_TO_INIT_ARRAY,
    EXCESSIVE_ARRAY_DIM_DECLARATION,
    RETURN_ARRAY,
    VOID_VARIABLE,
    TYPEDEF_VOID_ARRAY,
    PARAMETER_TYPE_UNMATCH,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    RETURN_TYPE_UNMATCH,
    INCOMPATIBLE_ARRAY_DIMENSION,
    NOT_ASSIGNABLE,
    NOT_ARRAY,
    IS_TYPE_NOT_VARIABLE,
    IS_FUNCTION_NOT_VARIABLE,
    STRING_OPERATION,
    ARRAY_SIZE_NOT_INT,
    ARRAY_SIZE_NEGATIVE,
    ARRAY_SUBSCRIPT_NOT_INT,
    PASS_ARRAY_TO_SCALAR,
    PASS_SCALAR_TO_ARRAY
} ErrorMsgKind;

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);

    switch(errorMsgKind)
    {
    case PASS_ARRAY_TO_SCALAR:
        printf("Array \'%s\' passed to scalar parameter \'%s\'.\n", getIdNodeName(node1), name2);
        break;
    case PASS_SCALAR_TO_ARRAY:
        printf("Scalar \'%s\' passed to array parameter \'%s\'.\n", getIdNodeName(node1), name2);
        break;
    default:
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);

	switch(errorMsgKind)
    {
        case SYMBOL_IS_NOT_TYPE:
            printf("ID %s is not type.\n", getIdNodeName(node));
            break;
		case SYMBOL_REDECLARE:
			printf("ID %s redeclared.\n", getIdNodeName(node));
			break;
		case SYMBOL_UNDECLARED:
			printf("ID %s undeclared.\n", getIdNodeName(node));
			break;
        case NOT_FUNCTION_NAME:
            printf("ID %s is not a function.\n", getIdNodeName(node));
            break;
        case TRY_TO_INIT_ARRAY:
            printf("Try to init a array %s.\n", getIdNodeName(node));
            break;
        case EXCESSIVE_ARRAY_DIM_DECLARATION:
            printf("Excessive array dimension declaration.\n");
            break;
        case RETURN_ARRAY:
            printf("Return a array.\n");
            break;
        case VOID_VARIABLE:
            printf("Void variable %s.\n", getIdNodeName(node));
            break;
        case TYPEDEF_VOID_ARRAY:
            printf("Define void array %s.\n", getIdNodeName(node));
            break;
        case PARAMETER_TYPE_UNMATCH:
            printf("Parameter %s type unmatch.\n", getIdNodeName(node));
            break;
		case TOO_FEW_ARGUMENTS:
			printf("too few arguments to function %s.\n", getIdNodeName(node));
			break;
		case TOO_MANY_ARGUMENTS:
			printf("too many arguments to function %s.\n", getIdNodeName(node));
			break;
		case RETURN_TYPE_UNMATCH:
			printf("Incompatible return type.\n");
			break;
        case INCOMPATIBLE_ARRAY_DIMENSION:
            printf("Incompatible array dim in %s.\n", getIdNodeName(node));
            break;
        case NOT_ASSIGNABLE:
            printf("%s is not assignable.\n", getIdNodeName(node));
            break;
        case NOT_ARRAY:
            printf("%s is not a array.\n", getIdNodeName(node));
            break;
        case IS_TYPE_NOT_VARIABLE:
            printf("%s is type not a variable.\n", getIdNodeName(node));
            break;
        case IS_FUNCTION_NOT_VARIABLE:
            printf("%s is a function not a variable.\n", getIdNodeName(node));
            break;
        case STRING_OPERATION:
            printf("String operation.\n");
            break;
        case ARRAY_SIZE_NOT_INT:
            printf("Array size is not int in %s.\n", getIdNodeName(node));
            break;
        case ARRAY_SIZE_NEGATIVE:
            printf("Array size is negative in %s.\n", getIdNodeName(node));
            break;
		case ARRAY_SUBSCRIPT_NOT_INT:
			printf("Array subscript is not an integer.\n");
			break;
		default:
			printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
			break;
    }
}

char* getIdNodeName(AST_NODE *node)
{
    if (node->nodeType != IDENTIFIER_NODE) {
		return NULL;
	}
	return node->semantic_value.identifierSemanticValue.identifierName;
	
}

void semanticAnalysis(AST_NODE *root)
{
    processProgramNode(root);
}


DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2)
{
    if(dataType1 == FLOAT_TYPE || dataType2 == FLOAT_TYPE) {
        return FLOAT_TYPE;
    } else {
        return INT_TYPE;
    }
}


void processProgramNode(AST_NODE *programNode)
{
	AST_NODE *node = programNode->child;
	while (node != NULL) {
        switch (node->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                processGeneralNode(node);
                break;
            case DECLARATION_NODE:
                processDeclarationNode(node);
                break;
            default:
                printf("Wrong node type in void processProgramNode().\n");
                break;
        }
		node = node->rightSibling;
	}
}

void processDeclarationNode(AST_NODE* declarationNode)
{
	AST_NODE *typeNode = declarationNode->child;
	processTypeNode(typeNode);
    if(typeNode->dataType == ERROR_TYPE) {
        declarationNode->dataType = ERROR_TYPE;
        return;
    }
	switch (declarationNode->semantic_value.declSemanticValue.kind) {
		case VARIABLE_DECL:
			declareIdList(declarationNode, VARIABLE_ATTRIBUTE, 0);
			break;
        case TYPE_DECL:
            declareIdList(declarationNode, TYPE_ATTRIBUTE, 0);
            break;
        case FUNCTION_DECL:
            declareFunction(declarationNode);
            break;
        case FUNCTION_PARAMETER_DECL:
            declareIdList(declarationNode, VARIABLE_ATTRIBUTE, 1);
            break;
		default:
			break;
	}
}


void processTypeNode(AST_NODE* idNodeAsType)
{
	char *typename = getIdNodeName(idNodeAsType);
	SymbolTableEntry *sym = retrieveSymbol(typename);
	if (sym == NULL || sym->attribute->attributeKind != TYPE_ATTRIBUTE) {
		printErrorMsg(idNodeAsType, SYMBOL_IS_NOT_TYPE);
        idNodeAsType->dataType = ERROR_TYPE;
	} else {
		idNodeAsType->semantic_value.identifierSemanticValue.symbolTableEntry = sym;
        switch (sym->attribute->attr.typeDescriptor->kind) {
            case SCALAR_TYPE_DESCRIPTOR:
                idNodeAsType->dataType = sym->attribute->attr.typeDescriptor->properties.dataType;
                break;
            case ARRAY_TYPE_DESCRIPTOR:
                idNodeAsType->dataType = sym->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
                break;
        }
	}
}


void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
	AST_NODE *typeNode = declarationNode->child;
	AST_NODE *idNode = typeNode->rightSibling;
	TypeDescriptor *typeDesc = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;

	while (idNode != NULL) {
		char *name = getIdNodeName(idNode);
		if (declaredLocally(name)) {
			printErrorMsg(idNode, SYMBOL_REDECLARE);
            declarationNode->dataType = ERROR_TYPE;
            idNode = idNode->rightSibling;
			continue;
		}

		SymbolAttribute *attr = malloc(sizeof(SymbolAttribute));
		attr->attributeKind = isVariableOrTypeAttribute;

		switch (idNode->semantic_value.identifierSemanticValue.kind) {
            case NORMAL_ID:
                attr->attr.typeDescriptor = typeDesc;
                break;
            case ARRAY_ID:
                attr->attr.typeDescriptor = malloc(sizeof(TypeDescriptor));
                attr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                processDeclDimList(idNode, attr->attr.typeDescriptor, ignoreArrayFirstDimSize);
                if (idNode->dataType == ERROR_TYPE) break;
                if (typeDesc->kind == SCALAR_TYPE_DESCRIPTOR) {
                    attr->attr.typeDescriptor->properties.arrayProperties.elementType = typeDesc->properties.dataType;
                } else if (typeDesc->kind == ARRAY_TYPE_DESCRIPTOR) {
                    int typeDim = typeDesc->properties.arrayProperties.dimension;
                    int dim = attr->attr.typeDescriptor->properties.arrayProperties.dimension;
                    if ((typeDim + dim) > MAX_ARRAY_DIMENSION) {
                        printErrorMsg(idNode, EXCESSIVE_ARRAY_DIM_DECLARATION);
                        idNode->dataType = ERROR_TYPE;
                        break;
                    }
                    attr->attr.typeDescriptor->properties.arrayProperties.elementType = typeDesc->properties.arrayProperties.elementType;
                    attr->attr.typeDescriptor->properties.arrayProperties.dimension = typeDim + dim;
                    for(int i = 0, j = dim; j < dim + typeDim; ++i, ++j) {
                        attr->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension[j] = typeDesc->properties.arrayProperties.sizeInEachDimension[i];
                    }
                }
                break;
            case WITH_INIT_ID:
                if (typeDesc->kind == ARRAY_TYPE_DESCRIPTOR) {
                    printErrorMsg(idNode, TRY_TO_INIT_ARRAY);
                    idNode->dataType = ERROR_TYPE;
                } else {
                    attr->attr.typeDescriptor = typeDesc;
                    processExprRelatedNode(idNode->child);
                }
                break;
		}
        if (idNode->dataType != ERROR_TYPE) {
	        idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(name, attr);
        }
        idNode = idNode->rightSibling;
	}
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
    if (assignOrExprRelatedNode->nodeType == STMT_NODE) { 
        if (assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT) {
            checkAssignmentStmt(assignOrExprRelatedNode);
        } else if (assignOrExprRelatedNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT) {
            checkFunctionCall(assignOrExprRelatedNode);
        }
    } else if (assignOrExprRelatedNode->nodeType == EXPR_NODE) {
        processExprRelatedNode(assignOrExprRelatedNode);
    }
}

void checkWhileStmt(AST_NODE* whileNode)
{
    checkAssignOrExpr(whileNode->child);
    processStmtNode(whileNode->child->rightSibling);
}


void checkForStmt(AST_NODE* forNode)
{
    processGeneralNode(forNode->child);
    processGeneralNode(forNode->child->rightSibling);
    processGeneralNode(forNode->child->rightSibling->rightSibling);
    processStmtNode(forNode->child->rightSibling->rightSibling->rightSibling);
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE *idNode = assignmentNode->child;
    AST_NODE *exprNode = idNode->rightSibling;
    processVariableLValue(idNode);
    processExprRelatedNode(exprNode);
    if (idNode->dataType == ERROR_TYPE || exprNode->dataType == ERROR_TYPE) {
        assignmentNode->dataType = ERROR_TYPE;
        return;
    }
    if (exprNode->dataType == INT_PTR_TYPE || exprNode->dataType == FLOAT_PTR_TYPE) {
        printErrorMsg(exprNode, INCOMPATIBLE_ARRAY_DIMENSION);
        assignmentNode->dataType = ERROR_TYPE;
        return;
    }
    if (exprNode->dataType == CONST_STRING_TYPE) {
        printErrorMsg(exprNode, STRING_OPERATION);
        assignmentNode->dataType = ERROR_TYPE;
    } else {
        assignmentNode->dataType = getBiggerType(idNode->dataType, exprNode->dataType);
    }
}


void checkIfStmt(AST_NODE* ifNode)
{
    checkAssignOrExpr(ifNode->child);
    processStmtNode(ifNode->child->rightSibling);
    processStmtNode(ifNode->child->rightSibling->rightSibling);
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
    AST_NODE *idNode = functionCallNode->child;
    AST_NODE *params = idNode->rightSibling;

    processGeneralNode(params);

    AST_NODE *node = params->child;
    if (node == NULL) {
        printErrorMsg(idNode, TOO_FEW_ARGUMENTS);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    if (node->dataType != INT_TYPE && node->dataType != FLOAT_TYPE && node->dataType != CONST_STRING_TYPE) {
        printErrorMsg(node, PARAMETER_TYPE_UNMATCH);
        functionCallNode->dataType = ERROR_TYPE;
    }
    if (node->rightSibling != NULL) {
        printErrorMsg(idNode, TOO_MANY_ARGUMENTS);
        functionCallNode->dataType = ERROR_TYPE;
    }
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
    AST_NODE *idNode = functionCallNode->child;
    char *funcName = getIdNodeName(idNode);
    if (strcmp(funcName, "write") == 0) {
        checkWriteFunction(functionCallNode);
        return;
    }
    SymbolTableEntry* sym = retrieveSymbol(funcName);
    if (sym == NULL) {
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    if(sym->attribute->attributeKind != FUNCTION_SIGNATURE) {
        printErrorMsg(idNode, NOT_FUNCTION_NAME);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    
    AST_NODE* actualParameterList = idNode->rightSibling;
    processGeneralNode(actualParameterList);

    AST_NODE* actualParameter = actualParameterList->child;
    Parameter* formalParameter = sym->attribute->attr.functionSignature->parameterList;
    
    while (actualParameter != NULL && formalParameter != NULL) {
        checkParameterPassing(formalParameter, actualParameter);
        if (actualParameter->dataType == ERROR_TYPE) {
            functionCallNode->dataType = ERROR_TYPE;
            return;
        }
        actualParameter = actualParameter->rightSibling;
        formalParameter = formalParameter->next;
    }
    if (actualParameter == NULL && formalParameter != NULL) {
        printErrorMsg(idNode, TOO_FEW_ARGUMENTS);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    if (actualParameter != NULL && formalParameter == NULL) {
        printErrorMsg(idNode, TOO_MANY_ARGUMENTS);
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    functionCallNode->dataType = sym->attribute->attr.functionSignature->returnType;
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
    DATA_TYPE actualParameterDataType = actualParameter->dataType;
    int isArray = 0;
    if(actualParameter->dataType == INT_PTR_TYPE || actualParameter->dataType == FLOAT_PTR_TYPE) {
        isArray = 1;
    }
    if (formalParameter->type->kind == SCALAR_TYPE_DESCRIPTOR && isArray) {
        printErrorMsgSpecial(actualParameter, formalParameter->parameterName, PASS_ARRAY_TO_SCALAR);
        actualParameter->dataType = ERROR_TYPE;
        return;
    }
    if (formalParameter->type->kind == ARRAY_TYPE_DESCRIPTOR && !isArray) {
        printErrorMsgSpecial(actualParameter, formalParameter->parameterName, PASS_SCALAR_TO_ARRAY);
        actualParameter->dataType = ERROR_TYPE;
        return;
    }
    if (actualParameter->dataType == CONST_STRING_TYPE) {
        printErrorMsg(actualParameter, PARAMETER_TYPE_UNMATCH);
        actualParameter->dataType = ERROR_TYPE;
    }
}


void processExprRelatedNode(AST_NODE* exprRelatedNode)
{
    switch (exprRelatedNode->nodeType) {
        case EXPR_NODE:
            processExprNode(exprRelatedNode);
            break;
        case STMT_NODE:
            checkFunctionCall(exprRelatedNode);
            break;
        case IDENTIFIER_NODE:
            processVariableRValue(exprRelatedNode);
            break;
        case CONST_VALUE_NODE:
            processConstValueNode(exprRelatedNode);
            break;
        default:
            break;
    }
}

void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
    float v;
    if (exprOrConstNode->nodeType == CONST_VALUE_NODE) {
        if (exprOrConstNode->dataType == INT_TYPE) {
            v = exprOrConstNode->semantic_value.const1->const_u.intval;
        } else {
            v = exprOrConstNode->semantic_value.const1->const_u.fval;
        }
    } else {
        if (exprOrConstNode->dataType == INT_TYPE) {
            v = exprOrConstNode->semantic_value.exprSemanticValue.constEvalValue.iValue;
        } else {
            v = exprOrConstNode->semantic_value.exprSemanticValue.constEvalValue.fValue;
        }
    }
    /*
    if (iValue != NULL) {
        *iValue = v;
    */
    if (fValue != NULL) {
        *fValue = v;
    }
}

void evaluateExprValue(AST_NODE* exprNode)
{
    float ret;
    if (exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
        AST_NODE *l = exprNode->child;
        AST_NODE *r = l->rightSibling;
        float rv, lv;
        getExprOrConstValue(l, NULL, &lv);
        getExprOrConstValue(r, NULL, &rv);
        switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
            case BINARY_OP_ADD:
                ret = lv + rv;
                break;
            case BINARY_OP_SUB:
                ret = lv - rv;
                break;
            case BINARY_OP_MUL:
                ret = lv * rv;
                break;
            case BINARY_OP_DIV:
                ret = lv / rv;
                break;
            case BINARY_OP_EQ:
                ret = lv == rv;
                break;
            case BINARY_OP_GE:
                ret = lv >= rv;
                break;
            case BINARY_OP_LE:
                ret = lv <= rv;
                break;
            case BINARY_OP_NE:
                ret = lv != rv;
                break;
            case BINARY_OP_GT:
                ret = lv > rv;
                break;
            case BINARY_OP_LT:
                ret = lv < rv;
                break;
            case BINARY_OP_AND:
                ret = lv && rv;
                break;
            case BINARY_OP_OR:
                ret = lv || rv;
                break;
        }
        if (l->dataType == INT_TYPE && r->dataType == INT_TYPE) {
            exprNode->dataType = INT_TYPE;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = ret;
        } else {
            exprNode->dataType = FLOAT_TYPE;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = ret;
        }
    } else {
        AST_NODE *node = exprNode->child;
        float v;
        getExprOrConstValue(node, NULL, &v);
        switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
            case UNARY_OP_POSITIVE:
                ret = v;
                break;
            case UNARY_OP_NEGATIVE:
                ret = -v;
                break;
            case UNARY_OP_LOGICAL_NEGATION:
                ret = !v;
                break;
        }
        if (node->dataType == INT_TYPE) {
            exprNode->dataType = INT_TYPE;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = ret;
        } else {
            exprNode->dataType = FLOAT_TYPE;
            exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = ret;
        }
    }
}


void processExprNode(AST_NODE* exprNode)
{
    if (exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION) {
        AST_NODE *l = exprNode->child;
        AST_NODE *r = l->rightSibling;
        processExprRelatedNode(l);
        processExprRelatedNode(r);
        if (l->dataType == ERROR_TYPE || r->dataType == ERROR_TYPE) {
            exprNode->dataType = ERROR_TYPE;
            return;
        }
        if(l->dataType == INT_PTR_TYPE || l->dataType == FLOAT_PTR_TYPE) {
            printErrorMsg(l, INCOMPATIBLE_ARRAY_DIMENSION);
            exprNode->dataType = ERROR_TYPE;
        }
        if(r->dataType == INT_PTR_TYPE || r->dataType == FLOAT_PTR_TYPE) {
            printErrorMsg(r, INCOMPATIBLE_ARRAY_DIMENSION);
            exprNode->dataType = ERROR_TYPE;
        }
        if (l->dataType == CONST_STRING_TYPE || r->dataType == CONST_STRING_TYPE) {
            printErrorMsg(exprNode, STRING_OPERATION);
            exprNode->dataType = ERROR_TYPE;
        }
        if (exprNode->dataType == ERROR_TYPE) {
            return;
        }
        if ((l->nodeType == CONST_VALUE_NODE || (l->nodeType == EXPR_NODE && l->semantic_value.exprSemanticValue.isConstEval)) &&
            (r->nodeType == CONST_VALUE_NODE || (r->nodeType == EXPR_NODE && r->semantic_value.exprSemanticValue.isConstEval))) {
            evaluateExprValue(exprNode);
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
        }
        exprNode->dataType = getBiggerType(l->dataType, r->dataType);
    } else {
        AST_NODE *node = exprNode->child;
        processExprRelatedNode(node);
        if (node->dataType == ERROR_TYPE) {
            exprNode->dataType = ERROR_TYPE;
            return;
        }
        if(node->dataType == INT_PTR_TYPE || node->dataType == FLOAT_PTR_TYPE) {
            printErrorMsg(node, INCOMPATIBLE_ARRAY_DIMENSION);
            exprNode->dataType = ERROR_TYPE;
        }
        if (node->nodeType == CONST_STRING_TYPE) {
            printErrorMsg(exprNode, STRING_OPERATION);
            exprNode->dataType = ERROR_TYPE;
        }
        if (exprNode->dataType == ERROR_TYPE) {
            return;
        }
        if (node->nodeType == CONST_VALUE_NODE || (node->nodeType == EXPR_NODE && node->semantic_value.exprSemanticValue.isConstEval)) {
            evaluateExprValue(exprNode);
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
        }
        exprNode->dataType = node->dataType;
    }
}


void processVariableLValue(AST_NODE* idNode)
{
    SymbolTableEntry *sym = retrieveSymbol(getIdNodeName(idNode));
    if (sym == NULL) {
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = sym;
    if (sym->attribute->attributeKind == TYPE_ATTRIBUTE) {
        printErrorMsg(idNode, IS_TYPE_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    if (sym->attribute->attributeKind == FUNCTION_SIGNATURE) {
        printErrorMsg(idNode, IS_FUNCTION_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    
    TypeDescriptor *typeDesc = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;

    if (idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID) {
        if (typeDesc->kind == ARRAY_TYPE_DESCRIPTOR) {
            printErrorMsg(idNode, NOT_ASSIGNABLE); // can't assign array type.
            idNode->dataType = ERROR_TYPE;
            return;
        }
        idNode->dataType = typeDesc->properties.dataType;
    } else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID) {
        if (typeDesc->kind != ARRAY_TYPE_DESCRIPTOR) {
            printErrorMsg(idNode, NOT_ARRAY);
            idNode->dataType = ERROR_TYPE;
            return;
        }
        AST_NODE *dimNode = idNode->child;
        int dim = 0;
        while (dimNode != NULL) {
           dim++;
           processExprRelatedNode(dimNode);
           if (dimNode->dataType != INT_TYPE) {
                printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
                idNode->dataType = ERROR_TYPE;
                return;
           }
           dimNode = dimNode->rightSibling;
        }
        if (dim != typeDesc->properties.arrayProperties.dimension) {
            printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
            idNode->dataType = ERROR_TYPE;
        } else {
            idNode->dataType = typeDesc->properties.arrayProperties.elementType;
        }
    }
    
}

void processVariableRValue(AST_NODE* idNode)
{
    SymbolTableEntry *sym = retrieveSymbol(getIdNodeName(idNode));
    if (sym == NULL) {
        printErrorMsg(idNode, SYMBOL_UNDECLARED);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    idNode->semantic_value.identifierSemanticValue.symbolTableEntry = sym;
    if (sym->attribute->attributeKind == TYPE_ATTRIBUTE) {
        printErrorMsg(idNode, IS_TYPE_NOT_VARIABLE);
        idNode->dataType = ERROR_TYPE;
        return;
    }
    TypeDescriptor *typeDesc = idNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;

    if (idNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID) {
        if (typeDesc->kind == ARRAY_TYPE_DESCRIPTOR) {
            if (typeDesc->properties.arrayProperties.elementType == INT_TYPE) {
                idNode->dataType = INT_PTR_TYPE;
            } else {
                idNode->dataType = FLOAT_PTR_TYPE;
            }
        } else {
            idNode->dataType = typeDesc->properties.dataType;
        }
    } else if(idNode->semantic_value.identifierSemanticValue.kind == ARRAY_ID) {
        if (typeDesc->kind != ARRAY_TYPE_DESCRIPTOR) {
            printErrorMsg(idNode, NOT_ARRAY);
            idNode->dataType = ERROR_TYPE;
            return;
        }
        AST_NODE *dimNode = idNode->child;
        int dim = 0;
        while (dimNode != NULL) {
           dim++;
           processExprRelatedNode(dimNode);
           if (dimNode->dataType != INT_TYPE) {
                printErrorMsg(idNode, ARRAY_SUBSCRIPT_NOT_INT);
                idNode->dataType = ERROR_TYPE;
                return;
           }
           dimNode = dimNode->rightSibling;
        }
        if (dim > typeDesc->properties.arrayProperties.dimension) {
            printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
            idNode->dataType = ERROR_TYPE;
        } else if (dim < typeDesc->properties.arrayProperties.dimension) {
            if (typeDesc->properties.arrayProperties.elementType == INT_TYPE) {
                idNode->dataType = INT_PTR_TYPE;
            } else {
                idNode->dataType = FLOAT_PTR_TYPE;
            }
        } else {
            idNode->dataType = typeDesc->properties.arrayProperties.elementType;
        }
    }
    
}


void processConstValueNode(AST_NODE* constValueNode)
{
    switch (constValueNode->semantic_value.const1->const_type) {
        case INTEGERC:
            constValueNode->dataType = INT_TYPE;
            constValueNode->semantic_value.exprSemanticValue.constEvalValue.iValue = constValueNode->semantic_value.const1->const_u.intval;
            break;
        case FLOATC:
            constValueNode->dataType = FLOAT_TYPE;
            constValueNode->semantic_value.exprSemanticValue.constEvalValue.fValue = constValueNode->semantic_value.const1->const_u.fval;
            break;
        case STRINGC:
            constValueNode->dataType = CONST_STRING_TYPE;
            break;
        default:
            constValueNode->dataType = ERROR_TYPE;
    }
}


void checkReturnStmt(AST_NODE* returnNode)
{
    AST_NODE *funcTypeNode = returnNode->parent;
    AST_NODE *returnValNode = returnNode->child;
    while (funcTypeNode->parent->nodeType != DECLARATION_NODE) {
        funcTypeNode = funcTypeNode->parent->leftmostSibling;
    }
    processExprRelatedNode(returnValNode);
    if ((returnValNode->nodeType == NUL_NODE && funcTypeNode->dataType == VOID_TYPE) ||
        returnValNode->dataType != funcTypeNode->dataType) {
        printErrorMsg(returnNode, RETURN_TYPE_UNMATCH);
        returnNode->dataType = ERROR_TYPE;
    }
}


void processBlockNode(AST_NODE* blockNode)
{
    openScope();
    AST_NODE *node = blockNode->child;
    while (node != NULL) {
        processGeneralNode(node);
        node = node->rightSibling;
    }
    closeScope();
}


void processStmtNode(AST_NODE* stmtNode)
{
    if (stmtNode->nodeType == NUL_NODE) {
        return;
    }
    if (stmtNode->nodeType == BLOCK_NODE) {
        processBlockNode(stmtNode);
        return;
    }
    switch (stmtNode->semantic_value.stmtSemanticValue.kind) {
        case WHILE_STMT:
            checkWhileStmt(stmtNode);
            break;
        case FOR_STMT:
            checkForStmt(stmtNode);
            break;
        case ASSIGN_STMT:
            checkAssignmentStmt(stmtNode);
            break;
        case IF_STMT:
            checkIfStmt(stmtNode);
            break;
        case FUNCTION_CALL_STMT:
            checkFunctionCall(stmtNode);
            break;
        case RETURN_STMT:
            checkReturnStmt(stmtNode);
            break;
    }
}


void processGeneralNode(AST_NODE *node)
{
    switch(node->nodeType) {
        case VARIABLE_DECL_LIST_NODE:
            node = node->child;
            while (node != NULL) {
                processDeclarationNode(node);
                node = node->rightSibling;
            }
            break;
        case STMT_LIST_NODE:
            node = node->child;
            while (node != NULL) {
                processStmtNode(node);
                node = node->rightSibling;
            }
            break;
        case NONEMPTY_ASSIGN_EXPR_LIST_NODE:
            node = node->child;
            while (node != NULL) {
                checkAssignOrExpr(node);
                node = node->rightSibling;
            }
            break;
        case NONEMPTY_RELOP_EXPR_LIST_NODE:
            node = node->child;
            while (node != NULL) {
                processExprRelatedNode(node);
                node = node->rightSibling;
            }
            break;
        default:
            break;
    }
}

void processDeclDimList(AST_NODE* idNode, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize)
{
	AST_NODE *dimNode = idNode->child;
	int dim = 0;
	if (ignoreFirstDimSize) {
		typeDescriptor->properties.arrayProperties.sizeInEachDimension[dim] = 0;
		dim++;
		dimNode = dimNode->rightSibling;
	}
	while (dimNode != NULL) {
        if (++dim > MAX_ARRAY_DIMENSION) {
            printErrorMsg(idNode, EXCESSIVE_ARRAY_DIM_DECLARATION);
            idNode->dataType = ERROR_TYPE;
            break;
        }
        processExprRelatedNode(dimNode);
		if (dimNode->dataType != INT_TYPE) {
			printErrorMsg(idNode, ARRAY_SIZE_NOT_INT);
            idNode->dataType = ERROR_TYPE;
		} else if (dimNode->semantic_value.exprSemanticValue.isConstEval && dimNode->semantic_value.exprSemanticValue.constEvalValue.iValue < 0) {
            printErrorMsg(idNode, ARRAY_SIZE_NEGATIVE);
            idNode->dataType = ERROR_TYPE;
        }
        if (idNode->dataType == ERROR_TYPE) {
            return;
        }
               
		int dimSize = dimNode->semantic_value.exprSemanticValue.constEvalValue.iValue;
		typeDescriptor->properties.arrayProperties.sizeInEachDimension[dim] = dimSize;
		dimNode = dimNode->rightSibling;
	}
	typeDescriptor->properties.arrayProperties.dimension = dim;
}


void declareFunction(AST_NODE* declarationNode)
{
    AST_NODE *typeNode = declarationNode->child;
    if(typeNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR) {
        printErrorMsg(typeNode, RETURN_ARRAY);
        declarationNode->dataType = ERROR_TYPE;
        return;
    }
    AST_NODE* idNode = typeNode->rightSibling;
    if (declaredLocally(getIdNodeName(idNode))) {
        printErrorMsg(idNode, SYMBOL_REDECLARE);
        declarationNode->dataType = ERROR_TYPE;
        return;
    }
    
    SymbolAttribute *attribute = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
    attribute->attributeKind = FUNCTION_SIGNATURE;
    attribute->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    attribute->attr.functionSignature->returnType = typeNode->dataType;
    attribute->attr.functionSignature->parameterList = NULL;

    enterSymbol(getIdNodeName(idNode), attribute);
    
    openScope();
    AST_NODE *paramNode = idNode->rightSibling->child;
    Parameter *lastParam = NULL;
    int paramCnt = 0;
    while (paramNode != NULL) {
        paramCnt++;
        processDeclarationNode(paramNode);
        if (paramNode->dataType == ERROR_TYPE) {
            declarationNode->dataType = ERROR_TYPE;
            return;
        }
        AST_NODE *paramId = paramNode->child->rightSibling;
        Parameter *param = (Parameter*)malloc(sizeof(Parameter));
        param->next = NULL;
        param->parameterName = getIdNodeName(paramId);
        param->type = paramId->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
        if (lastParam == NULL) {
            attribute->attr.functionSignature->parameterList = param;
        }
        if (lastParam != NULL) {
            lastParam->next = param;
        }
        lastParam = param;
        paramNode = paramNode->rightSibling;
    }
    attribute->attr.functionSignature->parametersCount = paramCnt;
    AST_NODE *blockNode = idNode->rightSibling->rightSibling;
    AST_NODE *node = blockNode->child;
    while (node != NULL) {
        processGeneralNode(node);
        node = node->rightSibling;
    }
    closeScope();
}
