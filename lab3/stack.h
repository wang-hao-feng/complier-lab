#ifndef _STACK_H_
#define _STACK_H_

#define MIN_STACK_MAX_LENGTH 4

typedef struct Stack
{
    int length;
    int max_length;
    void **values;
}Stack;

Stack *NewStack();

void DeleteStack(Stack *stack);

void Push(Stack *stack, void *value);

void *Pop(Stack *stack);

#endif