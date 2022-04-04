#include "stack.h"
#include <stdlib.h>

Stack *NewStack()
{
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->length = 0;
    stack->max_length = MIN_STACK_MAX_LENGTH;
    stack->values = (void **)malloc(sizeof(void *) * stack->max_length);
}

void DeleteStack(Stack *stack)
{
    free(stack->values);
    free(stack);
}

void UpdateStack(Stack *stack)
{
    void **temp_values = (void **)malloc(sizeof(void *) * stack->max_length);
    memcpy(temp_values, stack->values, sizeof(void *) * stack->length);
    free(stack->values);
    stack->values = temp_values;
}

void Push(Stack *stack, void *value)
{
    stack->values[stack->length] = value;
    stack->length++;

    if(stack->length == stack->max_length)
    {
        stack->max_length <<= 1;
        UpdateStack(stack);
    }
}

void *Pop(Stack *stack)
{
    if(stack->length == 0)
        return NULL;
    stack->length--;
    void *ans = stack->values[stack->length];

    if(stack->max_length > MIN_STACK_MAX_LENGTH && stack->length < (stack->max_length >> 2))
    {
        stack->max_length <<= 1;
        UpdateStack(stack);
    }

    return ans;
}