#include "map.h"

#ifndef _TRIE_H_
#define _TRIE_H_

typedef struct TrieTree
{
    Map *next;
    void *value;
}TrieTree;

TrieTree *NewTrieTree();

void DeleteTrieTree(TrieTree *root);

TrieTree *SearchTrieNode(TrieTree *root, const char *word);

void *InsertTrieNode(TrieTree *root, const char *word, void *value);

void *SearchTrieTreeValue(TrieTree *root, const char *word);

#endif