#include "TypeExp.h"
#include <stdlib.h>
#include <stdio.h>

void DeleteType(Type *type)
{
    if(type->type == 2)
        DeleteType((Type *)type->structure);
    free(type);
}

int SameField(Field *field1, Field *field2)
{
    if(field1 == NULL && field1 == NULL)
        return 1;
    if(field1 == NULL || field1 == NULL)
        return 0;
    if(SameType(field1->type, field2->type))
        return SameField(field1->next_field, field2->next_field);
    return 0;
}

int SameType(Type *type1, Type *type2)
{
    if(type1 == type2)
        return 1;
    if(type1 == NULL || type2 == NULL)
        return 0;
    if(type1->type != type2->type || type1->size != type2->size)
        return 0;
    if(type1->type == 0 || type1->type == 1)
        return 1;
    if(type1->type == 2)
    {
        if(type1->value.array_length != type2->value.array_length)
            return 0;
        return SameType((Type *)type1->structure, (Type *)type1->structure);
    }
    return SameField((Field *)type1->structure, (Field *)type2->structure);
}

int SameParam(Parameter *p1, Parameter *p2)
{
    if(p1 == p2)
        return 1;
    if(p1 == NULL || p2 == NULL)
        return 0;
    if(!SameType(p1->type, p2->type))
        return 0;
    return SameParam(p1->next, p2->next);
}

int SameFunc(Function *f1, Function *f2)
{
    if(strcmp(f1->name, f2->name))
        return 0;
    if(!SameType(f1->type, f2->type))
        return 0;
    return SameParam(f1->param_structure, f2->param_structure);
}

Field *SearchField(Field *type, char *field_name)
{
    Field *now = type;
    while (now != NULL)
    {
        if(!strcmp(now->type->name, field_name))
            break;
        now = now->next_field;
    }
    return now;
}

void DeleteParam(Parameter *param)
{
    Parameter *temp;
    while (param != NULL)
    {
        temp = param->next;
        if(param->type->type != 3)
            free(param->type);
        free(param);
        param = temp;
    }
}

void DeleteFunc(Function *fun)
{
    if(fun == NULL)
        return;
    if(fun->type != NULL && fun->type->type != 3)
        free(fun->type);
    DeleteParam(fun->param_structure);
    free(fun);
}

void PrintType(Type *type, int deep)
{
    if(type == NULL)
        return;
    for(int i = 0; i < deep; i++)
        printf("  ");
    printf("name:%s, size:%d", type->name, type->size);
    if(type->type == 0)
        printf(", num:%d\n", type->value.type_int);
    else if(type->type == 1)
        printf(", value:%f\n", type->value.type_float);
    else if(type->type == 2)
    {
        printf(", array length:%d\n", type->value.array_length);
        PrintType((Type *)type->structure, deep + 1);
    }
    else
    {
        printf(", fields:\n");
        PrintField((Field *)type->structure, deep + 1);
    }

}

void PrintField(Field *field, int deep)
{
    while (field != NULL)
    {
        PrintType(field->type, deep);
        field = field->next_field;
    }
    
}
