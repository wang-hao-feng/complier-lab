#include "SymbolTable.h"
#include <stdlib.h>
#include "map.h"

SymbolItem *NewSymbolItem(void *structure)
{
    SymbolItem *item = (SymbolItem *)malloc(sizeof(SymbolItem));
    item->structure = structure;
    return item;
}

SymbolTable *NewSymbolTable()
{
    SymbolTable *table = (SymbolTable *)malloc(sizeof(SymbolTable));
    table->names = NewTrieTree();
    return table;
}

SymbolItem *SearchSymbol(SymbolTable *table, char *name)
{
    return (SymbolItem *)SearchTrieTreeValue(table->names, name);
}

int InsertSymbolItem(SymbolTable *table, char *name, SymbolItem *item)
{
    if(SearchTrieNode(table->names, name))
        return 0;
    InsertTrieNode(table->names, name, (void *)item);
    return 1;
}

void DeleteSymbalItem(SymbolItem *item)
{
    free(item);
}

void DeleteSymbalTable(SymbolTable *table)
{
    DeleteTrieTree(table->names);
    free(table);
}