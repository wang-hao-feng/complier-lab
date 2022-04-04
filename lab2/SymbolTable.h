#include "trie.h"
#include "stack.h"
#include "TypeExp.h"

#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

typedef struct SymbolItem
{
    void *structure;
}SymbolItem;

typedef struct SymbolTable
{
    TrieTree *names;
}SymbolTable;

SymbolItem *NewSymbolItem(void *sturcture);

SymbolTable *NewSymbolTable();

SymbolItem *SearchSymbol(SymbolTable *table, char *name);

int InsertSymbolItem(SymbolTable *table, char *name, SymbolItem *item);

void DeleteSymbalItem(SymbolItem *item);

void DeleteSymbalTable(SymbolTable *table);

#endif