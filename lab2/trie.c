#include "trie.h"
#include <stdlib.h>

TrieTree *NewTrieTree()
{
    TrieTree *root = (TrieTree *)malloc(sizeof(TrieTree));
    root->next = NewMap();
    root->value = NULL;
    return root;
}

void DeleteTrieTree(TrieTree *root)
{
    for(int i = 0; i < root->next->length; i++)
    {
        void *temp = root->next->values[i];
        if(temp != NULL)
            DeleteTrieTree((TrieTree *)temp);
    }
    DeleteMap(root->next);
    free(root);
}

TrieTree *SearchTrieNode(TrieTree *root, const char *word)
{
    if(word == NULL)
        return root;
    for(char *now = word; *now != '\0'; now++)
    {
        void *temp = SearchMapValue(root->next, (int)*now);
        if(temp == NULL)
            return NULL;
        root = (TrieTree *)temp;
    }
    return root;
}

void *InsertTrieNode(TrieTree *root, const char *word, void *value)
{
    //如果节点已存在，则覆盖并返回当前value
    TrieTree *node = SearchTrieNode(root, word);
    if(node != NULL)
    {
        void *ans = node->value;
        node->value = value;
        return ans;
    }

    for(char *now = word; *now != '\0'; now++)
    {
        void *temp = SearchMapValue(root->next, (int)*now);
        if(temp == NULL)
        {
            TrieTree *new = NewTrieTree();
            InsertPair(root->next, (int)*now, (void *)new);
            root = new;
            continue;
        }
        root = (TrieTree *)temp;
    }
    root->value = value;
    return NULL;
}

void *SearchTrieTreeValue(TrieTree *root, const char *word)
{
    TrieTree *node = SearchTrieNode(root, word);
    if(node == NULL)
        return NULL;
    return node->value;
}