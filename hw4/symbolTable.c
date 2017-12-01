#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

int HASH(char * str) {
    int idx = 0;
    while (*str) {
        idx = idx << 1;
        idx += *str;
        str++;
    }
    return (idx & (HASH_TABLE_SIZE - 1));
}

SymbolTable symbolTable;

void print_stack(char* s)
{
    printf("%s", s);
    SymbolTableEntry* head = symbolTable.scopeStack;
    while (head) {
        printf("\n|\nV\n");
        SymbolTableEntry* listHead = head;
        while (listHead) {
            printf("%s |", listHead->name);
            listHead = listHead->nextInStackChain;
        }
        head = head->nextInStack;
    }
    printf("\n-------------------------------------------------\n");

}

SymbolTableEntry* newSymbolTableEntry(int nestingLevel)
{
    SymbolTableEntry* symbolTableEntry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    symbolTableEntry->nextInHashChain = NULL;
    symbolTableEntry->prevInHashChain = NULL;
    symbolTableEntry->nextInStack = NULL;
    symbolTableEntry->nextInStackChain = NULL;
    symbolTableEntry->sameNameInOuterLevel = NULL;
    symbolTableEntry->attribute = NULL;
    symbolTableEntry->name = NULL;
    symbolTableEntry->nestingLevel = nestingLevel;
    return symbolTableEntry;
}

void removeFromHashTrain(int hashIndex, SymbolTableEntry* entry)    /*DONE*/
{
    if (entry->prevInHashChain) {
        entry->prevInHashChain->nextInHashChain = entry->nextInHashChain;
    }
    else {
        symbolTable.hashTable[hashIndex] = entry->nextInHashChain;
    }

    if (entry->nextInHashChain) {
        entry->nextInHashChain->prevInHashChain = entry->prevInHashChain;
    }

    entry->prevInHashChain = NULL;
    entry->nextInHashChain = NULL;
}

void enterIntoHashTrain(int hashIndex, SymbolTableEntry* entry) /*DONE*/
{
    SymbolTableEntry* head = symbolTable.hashTable[hashIndex];
    if (head) {
        head->prevInHashChain = entry;
        entry->nextInHashChain = head;
    }
    symbolTable.hashTable[hashIndex] = entry;
}

void initializeSymbolTable()    /*DONE*/
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        symbolTable.hashTable[i] = NULL;
    }
    symbolTable.currentLevel = 0;
    symbolTable.scopeStack = NULL;


    SymbolAttribute* Attr = (SymbolAttribute*)malloc(sizeof(SymbolAttribute));
    Attr->attributeKind = TYPE_ATTRIBUTE;
    Attr->attr.typeDescriptor = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
    Attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
    Attr->attr.typeDescriptor->properties.dataType = INT_TYPE;
    enterSymbol(SYMBOL_TABLE_INT_NAME, Attr);

    Attr->attributeKind = TYPE_ATTRIBUTE;
    Attr->attr.typeDescriptor = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
    Attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
    Attr->attr.typeDescriptor->properties.dataType = FLOAT_TYPE;
    enterSymbol(SYMBOL_TABLE_FLOAT_NAME, Attr);

    Attr->attributeKind = TYPE_ATTRIBUTE;
    Attr->attr.typeDescriptor = (TypeDescriptor*)malloc(sizeof(TypeDescriptor));
    Attr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
    Attr->attr.typeDescriptor->properties.dataType = VOID_TYPE;
    enterSymbol(SYMBOL_TABLE_VOID_NAME, Attr);

    Attr->attributeKind = FUNCTION_SIGNATURE;
    Attr->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    Attr->attr.functionSignature->parametersCount = 0;
    Attr->attr.functionSignature->parameterList = NULL;
    Attr->attr.functionSignature->returnType = INT_TYPE;
    enterSymbol(SYMBOL_TABLE_SYS_LIB_READ, Attr);

    Attr->attributeKind = FUNCTION_SIGNATURE;
    Attr->attr.functionSignature = (FunctionSignature*)malloc(sizeof(FunctionSignature));
    Attr->attr.functionSignature->parametersCount = 0;
    Attr->attr.functionSignature->parameterList = NULL;
    Attr->attr.functionSignature->returnType = FLOAT_TYPE;
    enterSymbol(SYMBOL_TABLE_SYS_LIB_FREAD, Attr);

    free(Attr);
}

void symbolTableEnd()   /*DONE*/
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        SymbolTableEntry* entry = symbolTable.hashTable[i];
        while (entry) {
            SymbolTableEntry* next = entry->nextInHashChain;
            free(entry);
            entry = next;
        }
    }
}

SymbolTableEntry* retrieveSymbol(char* symbolName)  /*DONE*/
{
    int hash = HASH(symbolName);
    SymbolTableEntry* entry = symbolTable.hashTable[hash];
    while (entry != NULL) {
        if (strcmp(symbolName, entry->name) == 0) {
            return entry;
        }
        entry = entry->nextInHashChain;
    }
    return NULL;
}

SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute) /*DONE*/
{
    int hash = HASH(symbolName);
    SymbolTableEntry* head = symbolTable.hashTable[hash];
    SymbolTableEntry* new = newSymbolTableEntry(symbolTable.currentLevel);
    new->name = symbolName;
    new->attribute = attribute;
    while (head) {
        if (strcmp(symbolName, head->name) == 0) {
            if (symbolTable.currentLevel == head->nestingLevel) {
                free(new);
                return NULL;
            }
            else {
                removeFromHashTrain(hash, head);
                new->sameNameInOuterLevel = head;
                break;
            }
        }
        head = head->nextInHashChain;
    }

    enterIntoHashTrain(hash, new);

    if (symbolTable.scopeStack) {
        if (symbolTable.scopeStack->name) {
            new->nextInStack = symbolTable.scopeStack->nextInStack;
            new->nextInStackChain = symbolTable.scopeStack;
        }
        else {
            new->nextInStack = symbolTable.scopeStack->nextInStack;
            new->nextInStackChain = NULL;
        }
    }
    else {
        new->nextInStack = NULL;
        new->nextInStackChain = NULL;
    }
    symbolTable.scopeStack = new;

    return new;
}

//remove the symbol from the current scope
void removeSymbol(char* symbolName) /*DONE*/
{
    int hash = HASH(symbolName);

    SymbolTableEntry* head = symbolTable.hashTable[hash];
    while (head) {
        if (strcmp(symbolName, head->name) == 0) {
            if (symbolTable.currentLevel != head->nestingLevel) {
                return;
            }
            removeFromHashTrain(hash, head);
            if (head->sameNameInOuterLevel) {
                enterIntoHashTrain(hash, head->sameNameInOuterLevel);
                break;
            }
        }
        head = head->nextInHashChain;
    }

    SymbolTableEntry* tmpPrev = NULL;
    head = symbolTable.scopeStack;
    while (head) {
        if (strcmp(symbolName, head->name) == 0) {
            if (tmpPrev) {
                tmpPrev->nextInStackChain = head->nextInStackChain;
            }
            else {
                head->nextInStackChain->nextInStack = head->nextInStack;
                symbolTable.scopeStack = head->nextInStackChain;
            }
            free(head);
            break;
        }
        tmpPrev = head;
        head = head->nextInStackChain;
    }
}

int declaredLocally(char* symbolName)   /*DONE*/
{
    int hash = HASH(symbolName);
    SymbolTableEntry* head = symbolTable.hashTable[hash];
    while (head) {
        if (strcmp(symbolName, head->name) == 0) {
            if (symbolTable.currentLevel == head->nestingLevel) {
                return 1;
            }
            return 0;
        }
        head = head->nextInHashChain;
    }

    return 0;
}

void openScope()    /*DONE*/
{
    symbolTable.currentLevel++;

    SymbolTableEntry* newhead = newSymbolTableEntry(symbolTable.currentLevel);
    newhead->nextInStack = symbolTable.scopeStack;
    symbolTable.scopeStack = newhead;
}

void closeScope()   /*DONE*/
{
    if (symbolTable.currentLevel < 0) {
        return;
    }

    SymbolTableEntry* head = symbolTable.scopeStack;
    while (head) {
        int hash = HASH(head->name);
        removeFromHashTrain(hash, head);
        if (head->sameNameInOuterLevel) {
            enterIntoHashTrain(hash, head->sameNameInOuterLevel);
        }
        head = head->nextInStackChain;
    }

    head = symbolTable.scopeStack;
    symbolTable.scopeStack = symbolTable.scopeStack->nextInStack;

    symbolTable.currentLevel--;
}
