#include <stdlib.h>

//#define INT 0
//#define FLOAT 1
//#define ARRAY 2
//#define STRUCT 3

#ifndef _TYPEEXP_H_
#define _TYPEEXP_H_

typedef struct Type
{
    char *name;
    int type;
    int size;
    int temp_num;   //用来存储中间代码中使用哪个临时变量
    union
    {
        int type_int;
        int array_length;
        float type_float;
    }value;
    
    void *structure;
}Type;

typedef struct Field
{
    int offset;
    Type *type;
    struct Field *next_field;
}Field;

typedef struct Parameter
{
    int offset;
    Type *type;
    struct Parameter *next;
}Parameter;

typedef struct Function
{
    int def;
    char *name;
    Type *type;
    Parameter *param_structure;
}Function;

void DeleteType(Type *type);

int SameField(Field *field1, Field *field2);

int SameType(Type *type1, Type *type2);

int SameParam(Parameter *p1, Parameter *p2);

int SameFunc(Function *f1, Function *f2);

Field *SearchField(Field *type, char *field_name);

void DeleteParam(Parameter *param);

void DeleteFunc(Function *fun);

void PrintType(Type *type, int deep);

void PrintField(Field *field, int deep);

#endif