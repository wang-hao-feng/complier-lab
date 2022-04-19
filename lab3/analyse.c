#include "analyse.h"
#include <stdlib.h>
#include "SymbolTable.h"
#include "trie.h"
#include "error.h"

SymbolTable *variables;
SymbolTable *functions;
TrieTree *structures;
Type *return_type;
Map *undef_fun;
Stack *fun_symbol_stack;

int temp_num = 1;
int v_num = -1;
int label_num = 1;

SymbolItem *SearchSymbolGlobal(char *name);
void InsertReadAndWrite();

void analyseProgram(Node *program)
{
    temp_num = 0;
    variables = NewSymbolTable();
    functions = NewSymbolTable();
    structures = NewTrieTree();
    undef_fun = NewMap();
    fun_symbol_stack = NewStack();
    codes = (IntermediaCodes *)malloc(sizeof(IntermediaCodes));

    //插入read和write函数
    InsertReadAndWrite();

    Node *extdeflist = program->left;
    while (extdeflist->val_type != 0)
    {
        analyseExtDef(extdeflist->left);
        extdeflist = extdeflist->left->right;
    }

    CheckFunction();
}

void analyseExtDef(Node *extdef)
{
    Node *specifier = extdef->left;
    Type *base_type = analyseSpecifier(specifier);
    if(!strcmp(specifier->right->type, "ExtDecList"))
    {
        Node *vardec = specifier->right->left;
        while (1)
        {
            Type *temp = analyseVarDec(base_type, vardec, 0);
            if(temp != NULL)
            {
                translateVarDec(temp);
                InsertSymbolItem(variables, temp->name, NewSymbolItem((void *)temp));
            }
            if(vardec->right == NULL)
                break;
            vardec = vardec->right->right->left;
        }
        free(base_type);
    }
    else if(!strcmp(specifier->right->type, "FunDec"))
    {
        Node *fundec = specifier->right;
        Function *fun = analyseFunDec(fundec);
        SymbolItem *item = SearchSymbol(functions, fun->name);
        fun->type = base_type;
        if(item != NULL)
        {
            if(!SameFunc(fun, (Function *)item->structure))
            {
                if(!strcmp(fundec->right->type, "SEMI"))
                    PrintErrorMsg(CONFLICT_FUNCTION_DEFINATION, fundec->line, 3, "Inconsistent declaration of function \"", fun->name, "\".");
                else
                    PrintErrorMsg(CONFLICT_FUNCTION_DEFINATION, fundec->line, 3, "Definition does not match declaration of function \"", fun->name, "\".");
            }
            else if(!strcmp(fundec->right->type, "CompSt"))
            {
                ((Function *)item->structure)->def = 1;
                return_type = ((Function *)item->structure)->type;
                //插入函数定义中间代码
                translateFunDec((Function *)item->structure);
                analyseCompSt(fundec->right);
            }
            DeleteFunc(fun);
            return;
        }
        InsertSymbolItem(functions, fun->name, NewSymbolItem((void *)fun));
        if(!strcmp(fundec->right->type, "SEMI"))
        {
            fun->def = 0;
            InsertPair(undef_fun, fundec->line, (void *)fun);
        }
        else
        {
            fun->def = 1;
            return_type = fun->type;
            //插入函数定义中间代码
            translateFunDec(fun);
            analyseCompSt(fundec->right);
        }
    }
}

//返回表示的类型，如果是结构体，则同时将其定义放到structures中
Type *analyseSpecifier(Node *specifier)
{
     Type *temp = (Type *)malloc(sizeof(Type));
     if(!strcmp(specifier->left->type, "TYPE"))
     {
         if(!strcmp(specifier->left->val.type_string, "int"))
         {
             temp->size = sizeof(int);
             temp->type = 0;
         }
         else if(!strcmp(specifier->left->val.type_string, "float"))
         {
             temp->size = sizeof(float);
             temp->type = 1;
         }
     }
     else
     {
        Node *tag = specifier->left->left->right;
        if(!strcmp(tag->type, "Tag"))
        {
            free(temp);
            Node *id = tag->left;
            temp = (Type *)SearchTrieTreeValue(structures, id->val.type_string);
            if(!temp)
                PrintErrorMsg(UNDEFINED_STRUCT, tag->line, 3, "Undefined structure \"", id->val.type_string, "\".");
        }
        else
        {
            if(tag->val_type != 0)
            {
                Node *id = tag->left;
                if(SearchTrieTreeValue(structures, id->val.type_string))
                {
                    PrintErrorMsg(REDEFINED_STRUCT, tag->line, 3, "Redefined structure \"", id->val.type_string, "\".");
                    free(temp);
                    return NULL;
                }
                temp->name = id->val.type_string;
                InsertTrieNode(structures, temp->name, temp);
            }
            temp->type = 3;
            temp->size = 0;
            Node *deflist = tag->right->right;
            while(deflist->val_type != 0)
            {
                Field *head = (Field *)analyseDef(deflist->left, 0);
                while(head != NULL)
                {
                    if(SearchField((Field *)temp->structure, head->type->name))
                    {
                        PrintErrorMsg(REDIFINED_OR_INIT_FILED, deflist->line, 3, "Redefined field \"", head->type->name, "\".");
                        Field *next = head->next_field;
                        if(next != NULL)
                        {
                            memcpy(head, head->next_field, sizeof(Field));
                            head->next_field = next->next_field;
                            free(next);
                        }
                        head = head->next_field;
                    }
                    else
                    {
                        Field *next = head->next_field;
                        temp->size += head->type->size;
                        head->next_field = (Field *)temp->structure;
                        head->offset = head->next_field == NULL ? 0 : head->next_field->offset + head->type->size;
                        temp->structure = (void *)head;
                        head = next;
                    }
                }
                deflist = deflist->left->right;
            }
        }
     }
     return temp;
}

Type *analyseVarDec(Type *base_type, Node *vardec, int is_def_func)
{
    Type *temp = (Type *)malloc(sizeof(Type));
    vardec = vardec->left;
    if(vardec->val_type == 3)
    {
        memcpy(temp, base_type, sizeof(Type));
        temp->name = vardec->val.type_string;
        if(!is_def_func && SearchSymbol(variables, temp->name) != NULL)
        {
            PrintErrorMsg(REDEFINED_VARIABLE, vardec->line, 3, "Redefined variable \"", temp->name, "\".");
            free(temp);
            temp = NULL;
        }
    }
    else
    {
        temp->type = 2;
        temp->value.array_length = vardec->right->right->val.type_int;
        Type *son = analyseVarDec(base_type, vardec, is_def_func);
        if(son)
        {
            temp->size = temp->value.array_length * son->size;
            temp->structure = son;
            temp->name = son->name;
        }
        else
        {
            free(temp);
            temp = NULL;
        }
    }
    return temp;
}

void *analyseDef(Node *def, int is_func)
{
    Node *specifier = def->left;
    Type *base_type = analyseSpecifier(specifier);
    if(base_type == NULL)
        return NULL;
    Node *dec = specifier->right->left;
    Field *field = NULL;
    while (1)
    {
        Type *temp = analyseVarDec(base_type, dec->left, 0);
        if(temp != NULL)
        {
            if(is_func)
            {
                if(SearchSymbol(variables, temp->name))
                {
                    PrintErrorMsg(REDEFINED_VARIABLE, dec->left->line, 3, "Redefined variable \"", temp->name, "\".");
                    free(temp);
                }
                else
                {
                    translateVarDec(temp);
                    if(dec->left->right != NULL)
                    {
                        Node *exp = dec->left->right->right;
                        Operand *operand1 = NewOperand(IMMEDIATE, 0, -1, 0, NULL);
                        if(!SameType(temp, analyseExp(exp, operand1)))
                            PrintErrorMsg(ASSIGNMENT_TYPE_MISMATCH, dec->left->line, 1, "Type mismatched for assignment.");
                        else
                        {
                            Operand *dest = NewOperand(temp->temp_num < 0 ? VARIABLE : POINTER, temp->temp_num, -1, 0, NULL);
                            translateASSIGNOP(dest, GetValue(operand1));
                        }
                    }
                    InsertSymbolItem(variables, temp->name, NewSymbolItem((void *)temp));
                }
            }
            else
            {
                Field *temp_field = (Field *)malloc(sizeof(Field));
                temp_field->type = temp;
                temp_field->offset = 0;
                temp_field->next_field = field;
                field = temp_field;
                if(dec->left->right != NULL)
                    PrintErrorMsg(REDIFINED_OR_INIT_FILED, dec->left->line, 1, "Illegal initialization.");
            }
        }
        if(dec->right == NULL)
            break;
        dec = dec->right->right->left;
    }
    free(base_type);
    return (void *)field;
}

Type *analyseExp(Node *exp, Operand *dest)
{
    Type *exp_type = NULL;
    Node *temp = exp->left;
    if(!strcmp(temp->type, "Exp"))
    {
        Node *operator = temp->right;
        if(!strcmp(operator->type, "AND") || !strcmp(operator->type, "OR") || !strcmp(operator->type, "RELOP"))
            exp_type = AnalyseExp2TranslateCond(exp, dest);
        else
        {
            Operand *op1 = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
            Type *operand1 = analyseExp(temp, op1);
            if(operand1 != NULL)
            {
                Operand *op2 = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
                if(!strcmp(operator->type, "ASSIGNOP"))
                {
                    Type *operand2 = analyseExp(operator->right, op2);
                    if(!SameType(operand1, operand2))
                        PrintErrorMsg(ASSIGNMENT_TYPE_MISMATCH, temp->line, 1, "Type mismatched for assignment.");
                    else if(operand1->name == NULL)
                        PrintErrorMsg(INVALID_LEFT_HAND, temp->line, 1, "The left-hand side of an assignment must be a variable.");
                    else
                    {
                        translateASSIGNOP(op1, GetValue(op2));
                        if(dest != NULL)
                            translateASSIGNOP(dest, GetValue(op1));
                    }
                }
                else if(!strcmp(operator->type, "PLUS"))
                {
                    exp_type = operate(operand1, analyseExp(operator->right, op2), ADD_, temp->line);
                    translateCalculate(dest, op1, op2, 0);
                }
                else if(!strcmp(operator->type, "MINUS"))
                {
                    exp_type = operate(operand1, analyseExp(operator->right, op2), MINUS_, temp->line);
                    translateCalculate(dest, op1, op2, 1);
                }
                else if(!strcmp(operator->type, "STAR"))
                {
                    exp_type = operate(operand1, analyseExp(operator->right, op2), STAR_, temp->line);
                    translateCalculate(dest, op1, op2, 2);
                }
                else if(!strcmp(operator->type, "DIV"))
                {
                    exp_type = operate(operand1, analyseExp(operator->right, op2), DIV_, temp->line);
                    translateCalculate(dest, op1, op2, 3);
                }
                else if(!strcmp(operator->type, "LB"))
                {
                    if(operand1->type != 2)
                        PrintErrorMsg(NOT_ARRAY, temp->line, 3, "\"", operand1->name, "\" is not an array");
                    else
                    {
                        Operand *num = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
                        Type *operand2 = analyseExp(operator->right, num);
                        if(operand2->type == 1)
                            PrintErrorMsg(NOT_INTEGER_INDEX, temp->line, 1, "\"float\" is not an integer");
                        else if(operand2->type > 1)
                            PrintErrorMsg(NOT_INTEGER_INDEX, temp->line, 3, "\"", operand2->name, "\" is not an integer");
                        else
                        {
                            exp_type = (Type *)operand1->structure;
                            if(dest != NULL)
                            {
                                dest->type = POINTER;
                                dest->offset = 0;
                                dest->immediate = 0;
                                dest->name = NULL;
                                Operand *size = NewOperand(IMMEDIATE, temp_num++, -1, exp_type->size, NULL);
                                Operand *offset = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
                                IntermediaCodeItem *item = NewIntermediaCodeItem(STAR_, -1, offset, size, num);
                                InsertIntermediaCode(codes, item);
                                item = NewIntermediaCodeItem(ADD_, -1, dest, op1, offset);
                                InsertIntermediaCode(codes, item);
                            }
                        }
                    }
                }
                else if(!strcmp(operator->type, "DOT"))
                {
                    if(operand1->type != 3)
                        PrintErrorMsg(NOT_STRUCT, temp->line, 1, "Illegal use of \".\"");
                    else
                    {
                        Field *field = SearchField((Field *)operand1->structure, operator->right->val.type_string);
                        if(field == NULL)
                            PrintErrorMsg(NOT_EXIST_FILED, temp->line, 3, "Non-existentent field \"", operator->right->val.type_string, "\".");
                        else
                        {
                            exp_type = field->type;
                            if(dest != NULL)
                            {
                                dest->type = POINTER;
                                dest->offset = 0;
                                Operand *offset = NewOperand(IMMEDIATE, 0, -1, field->offset, NULL);
                                IntermediaCodeItem *item = NewIntermediaCodeItem(ADD_, -1, dest, op1, offset);
                                InsertIntermediaCode(codes, item);
                            }
                        }
                    }
                }
            }
        }
    }
    else if(!strcmp(temp->type, "LP"))
        exp_type = analyseExp(temp->right, dest);
    else if(!strcmp(temp->type, "MINUS"))
    {
        Operand *op = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
        Type *operand = analyseExp(temp->right, op);
        if(operand->type != 0 && operand->type != 1)
            PrintErrorMsg(RETURN_TYPE_MISMATCH, temp->line, 1, "Type mismatched for minus, the operand should be int or float.");
        else
        {
            exp_type = (Type *)malloc(sizeof(Type));
            memcpy(exp_type, operand, sizeof(Type));
            if(operand->type == 0)
            {
                exp_type->value.type_int = -exp_type->value.type_int;
                if(dest != NULL)
                {
                    Operand *zero = NewOperand(IMMEDIATE, 0, -1, 0, NULL);
                    IntermediaCodeItem *item = NewIntermediaCodeItem(MINUS_, -1, dest, zero, GetValue(op));
                    InsertIntermediaCode(codes, item);
                }
            }
            else
                exp_type->value.type_float = -exp_type->value.type_float;
        }
    }
    else if(!strcmp(temp->type, "NOT"))
        AnalyseExp2TranslateCond(exp, dest);
    else if(!strcmp(temp->type, "ID"))
    {
        if(temp->right == NULL)
        {
            SymbolItem *symbolitem = SearchSymbolGlobal(temp->val.type_string);
            if(symbolitem == NULL)
                PrintErrorMsg(UNDIFINED_VARIABLE, temp->line, 3, "Undefined variable \"", temp->val.type_string, "\".");
            else
            {
                exp_type = (Type *)symbolitem->structure;
                if(dest != NULL)
                {
                    dest->type = ((Type *)symbolitem->structure)->type < 3 ? VARIABLE : POINTER;
                    dest->operand_temp_num = ((Type *)symbolitem->structure)->temp_num;
                    dest->offset = -1;
                    dest->immediate = 0;
                    dest->name = NULL;
                }
            }
        }
        else
        {
            SymbolItem *symbolitem = SearchSymbol(functions, temp->val.type_string);
            if(symbolitem == NULL)
            {
                if(SearchSymbol(variables, temp->val.type_string))
                    PrintErrorMsg(NOT_FUNCTION, temp->line, 3, "\"", temp->val.type_string, "\" is not a function");
                else
                    PrintErrorMsg(UNDIFINED_FUNCTION, temp->line, 3, "Undefined function \"", temp->val.type_string, "\".");
            }
            else
            {
                Function *fun = (Function *)symbolitem->structure;
                int is_write = !strcmp(fun->name, "write");
                int is_read = !strcmp(fun->name, "read");
                Parameter *args = NULL;
                if(temp->right->right->right != NULL)
                    args = analyseArgs(temp->right->right, is_write);
                
                if(is_read && dest != NULL)
                {
                    IntermediaCodeItem *item = NewIntermediaCodeItem(READ_, -1, dest, NULL, NULL);
                    InsertIntermediaCode(codes, item);
                }
                else if(!is_write && !is_read)
                {
                    if(dest == NULL)
                        dest = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
                    Operand *fun_name = NewOperand(FUNCTION_NAME, 0, -1, 0, fun->name);
                    IntermediaCodeItem *item = NewIntermediaCodeItem(CALL_, -1, dest, fun_name, NULL);
                    InsertIntermediaCode(codes, item);
                }

                if(!SameParam(args, fun->param_structure))
                    PrintErrorMsg(PARAMETER_MISMATCH, temp->line, 3, "Invalid arguments for function \"", temp->val.type_string, "\".");
                else
                {
                    exp_type = (Type *)malloc(sizeof(Type));
                    memcpy(exp_type, fun->type, sizeof(Type));
                }
            }
        }
    }
    else if(!strcmp(temp->type, "INT"))
    {
        exp_type = (Type *)malloc(sizeof(Type));
        exp_type->size = sizeof(int);
        exp_type->structure = NULL;
        exp_type->type = 0;
        exp_type->value.type_int = temp->val.type_int;
        if(dest != NULL)
        {
            dest->type = IMMEDIATE;
            dest->operand_temp_num = 0;
            dest->offset = -1;
            dest->immediate = temp->val.type_int;
            dest->name = NULL;
        }
    }
    else if(!strcmp(temp->type, "FLOAT"))
    {
        exp_type = (Type *)malloc(sizeof(Type));
        exp_type->size = sizeof(float);
        exp_type->structure = NULL;
        exp_type->type = 1;
        exp_type->value.type_float = temp->val.type_float;
    }
    return exp_type;
}

Function *analyseFunDec(Node *fundec)
{
    Function *fun = (Function *)malloc(sizeof(Function));
    fun->name = fundec->left->val.type_string;
    SymbolItem *temp = SearchSymbol(functions, fun->name);
    int is_def = -1;
    if(temp != NULL)
        is_def = ((Function *)temp->structure)->def;
    if(is_def == 1)
    {
        PrintErrorMsg(REDEFINED_FUNCTION, fundec->line, 3, "Redefined function \"", fun->name, "\".");
        free(fun);
        return (Function *)temp->structure;
    }
    fun->param_structure = NULL;
    Node *paramdec = fundec->left->right->right->left;
    if(paramdec != NULL)
    {
        while(1)
        {
            Parameter *p = (Parameter *)malloc(sizeof(Parameter));
            Type *base_type = analyseSpecifier(paramdec->left);
            p->type = analyseVarDec(base_type, paramdec->left->right, is_def == 0);
            if(p->type == NULL)
            {
                DeleteFunc(fun);
                fun = NULL;
                break;
            }
            p->next = fun->param_structure;
            p->offset = p->next == NULL ? 0 : p->next->offset + p->next->type->size;
            fun->param_structure = p;
            if(is_def == -1)
            {
                p->type->temp_num = v_num--;
                InsertSymbolItem(variables, p->type->name, NewSymbolItem(p->type));
            }
            if(paramdec->right == NULL)
                break;
            paramdec = paramdec->right->right->left;
        }
    }

    return fun;
}

void analyseCompSt(Node *compst)
{
    Push(fun_symbol_stack, (void *)variables);
    variables = NewSymbolTable();
    Node *deflist = compst->left->right;
    Node *stmtlist = deflist->right;
    while (deflist->val_type != 0)
    {
        analyseDef(deflist->left, 1);
        deflist = deflist->left->right;
    }
    while (stmtlist->val_type != 0)
    {
        analyseStmt(stmtlist->left);
        stmtlist = stmtlist->left->right;
    }
    DeleteSymbalTable(variables);
    variables = (SymbolTable *)Pop(fun_symbol_stack);
}

void analyseStmt(Node *stmt)
{
    Node *temp = stmt->left;
    if(!strcmp(temp->type, "Exp"))
        analyseExp(temp, NULL);
    else if(!strcmp(temp->type, "CompSt"))
        analyseCompSt(temp);
    else if(!strcmp(temp->type, "RETURN"))
    {
        Operand *dest = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
        Type *re = analyseExp(temp->right, dest);
        if(!SameType(re, return_type))
            PrintErrorMsg(RETURN_TYPE_MISMATCH, temp->line, 1, "Type mismatched for return.");
        IntermediaCodeItem *item = NewIntermediaCodeItem(RETURN_, -1, dest, NULL, NULL);
        InsertIntermediaCode(codes, item);
    }
    else if(!strcmp(temp->type, "IF"))
    {
        Operand *label1 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
        Operand *label2 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
        Node *exp = temp->right->right;
        Type *exp_type = translateCond(exp, NULL, label1, label2);
        if(exp_type != NULL)
        {
            if(exp_type->type != 0)
                PrintErrorMsg(OPERANDS_TYPE_MISMATCH, temp->line, 1, "Type mismatched for if");
        }
        IntermediaCodeItem *item = NewIntermediaCodeItem(LABEL_, -1, label1, NULL, NULL);
        InsertIntermediaCode(codes, item);
        Node *stmt2 = exp->right->right;
        analyseStmt(stmt2);
        if(stmt2->right != NULL)
        {
            Operand *label3 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
            item = NewIntermediaCodeItem(GOTO_, -1, label3, NULL, NULL);
            InsertIntermediaCode(codes, item);
            item = NewIntermediaCodeItem(LABEL_, -1, label2, NULL, NULL);
            InsertIntermediaCode(codes, item);
            analyseStmt(stmt2->right->right);
            item = NewIntermediaCodeItem(LABEL_, -1, label3, NULL, NULL);
            InsertIntermediaCode(codes, item);
        }
        else
        {
            item = NewIntermediaCodeItem(LABEL_, -1, label2, NULL, NULL);
            InsertIntermediaCode(codes, item);
        }
    }
    else if(!strcmp(temp->type, "WHILE"))
    {
        Operand *label1 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
        Operand *label2 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
        Operand *label3 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
        IntermediaCodeItem *item = NewIntermediaCodeItem(LABEL_, -1, label1, NULL, NULL);
        InsertIntermediaCode(codes, item);
        Node *exp = temp->right->right;
        Type *exp_type = translateCond(exp, NULL, label2, label3);
        if(exp_type->type != 0)
            PrintErrorMsg(OPERANDS_TYPE_MISMATCH, temp->line, 1, "Type mismatched for while");
        item = NewIntermediaCodeItem(LABEL_, -1, label2, NULL, NULL);
        InsertIntermediaCode(codes, item);
        analyseStmt(exp->right->right);
        item = NewIntermediaCodeItem(GOTO_, -1, label1, NULL, NULL);
        InsertIntermediaCode(codes, item);
        item = NewIntermediaCodeItem(LABEL_, -1, label3, NULL, NULL);
        InsertIntermediaCode(codes, item);
    }
}

Field *analyseArgs(Node *args, int is_write)
{
    Parameter *params = NULL;
    Node *exp = args->left;
    Stack *params_stack = NewStack();
    while(1)
    {
        Operand *dest = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
        Parameter *temp = (Parameter *)malloc(sizeof(Parameter));
        temp->type = analyseExp(exp, dest);
        temp->next = params;
        params = temp;
        Push(params_stack, dest);
        if(exp->right == NULL)
            break;
        exp = exp->right->right->left;
    }
    IntermediaCodeItem *item = NULL;
    for(int i = 0; i < params_stack->length; i++)
    {
        if(!is_write)
            item = NewIntermediaCodeItem(ARG_, -1, (Operand *)params_stack->values[i], NULL, NULL);
        else
            item = NewIntermediaCodeItem(WRITE_, -1, GetValue((Operand *)params_stack->values[i]), NULL, NULL);
        InsertIntermediaCode(codes, item);
    }
    return params;
}

void translateFunDec(Function *fun)
{
    //插入FUNCTION fun.name :
    Operand *dest = NewOperand(FUNCTION_NAME, 0, -1, 0, fun->name);
    IntermediaCodeItem *new_function = NewIntermediaCodeItem(FUNCTION_, -1, dest, NULL, NULL);
    InsertIntermediaCode(codes, new_function);

    //插入PARAM x;
    Parameter *now = fun->param_structure;
    Stack *params = NewStack();
    for(; now != NULL; now = now->next)
        Push(params, (void *)now);
    
    while (params->length > 0)
    {
        now = (Parameter *)Pop(params);
        Operand *dest = NewOperand(now->type->type < 2 ? VARIABLE : POINTER, now->type->temp_num, now->type->size, 0, NULL);
        IntermediaCodeItem *new_param = NewIntermediaCodeItem(PARAM_, -1, dest, NULL, NULL);
        InsertIntermediaCode(codes, new_param);
    }
}

void translateVarDec(Type *var)
{
    var->temp_num = v_num--;
    if(var->type > 1)
    {
        Operand *dest = NewOperand(POINTER, var->temp_num, -1, 0, NULL);
        Operand *space = NewOperand(VARIABLE, v_num--, var->size, 0, NULL);
        IntermediaCodeItem *item = NewIntermediaCodeItem(DEC_, -1, space, NULL, NULL);
        InsertIntermediaCode(codes, item);
        item = NewIntermediaCodeItem(GET_ADDRESS_, -1, dest, space, NULL);
        InsertIntermediaCode(codes, item);
    }
}

void translateASSIGNOP(Operand *dest, Operand *operand)
{
    if(dest->type == 1)
    {
        IntermediaCodeItem *item = NewIntermediaCodeItem(ASSIGNOP_, -1, dest, operand, NULL);
        InsertIntermediaCode(codes, item);
        return;
    }
    //获取被赋值对象地址
    //Operand *offset = NewOperand(IMMEDIATE, 0, 0, dest->offset, NULL);
    //Operand *temp_addr = NewOperand(POINTER, temp_num++, 0, 0, NULL);
    //IntermediaCodeItem *item = NewIntermediaCodeItem(ADD_, -1, temp_addr, dest, offset);
    //InsertIntermediaCode(codes, item);
    //赋值
    IntermediaCodeItem *item = NewIntermediaCodeItem(VALUE_TO_ADDRESS_, -1, dest, operand, NULL);
    InsertIntermediaCode(codes, item);
}

Type *translateCond(Node *exp, Operand *dest, Operand *true_label, Operand *false_label)
{
    Type *exp_type = NULL;
    IntermediaCodeItem *item = NULL;
    if(!strcmp(exp->left->type, "NOT"))
    {
        Type *operand = translateCond(exp->left->right, dest, false_label, true_label);
        if(operand->type != 0)
            PrintErrorMsg(RETURN_TYPE_MISMATCH, exp->left->line, 1, "Type mismatched for not, the operand should be int.");
        else
        {
            exp_type = (Type *)malloc(sizeof(Type));
            memcpy(exp_type, operand, sizeof(Type));
            exp_type->value.type_int = !exp_type->value.type_int;
        }
        return exp_type;
    }
    else if(!strcmp(exp->left->type, "Exp"))
    {
        Node *exp1 = exp->left;
        Node *exp2 = exp1->right->right;
        Node *operator = exp1->right;
        Operand *op1 = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
        Type *operand1 = analyseExp(exp1, op1);
        Operand *op2 = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
        if(operand1 != NULL)
        {
            if(!strcmp(exp1->right->type, "RELOP"))
            {
                Node *relop = exp1->right;
                int relop_type = -1;
                if(!strcmp(relop->val.type_string, ">"))
                    relop_type = GREATER;
                else if(!strcmp(relop->val.type_string, ">="))
                    relop_type = GREATER_EQ;
                else if(!strcmp(relop->val.type_string, "<"))
                    relop_type = LESS;
                else if(!strcmp(relop->val.type_string, "<="))
                    relop_type = LESS_EQ;
                else if(!strcmp(relop->val.type_string, "=="))
                    relop_type = EQ;
                else if(!strcmp(relop->val.type_string, "!="))
                    relop_type = NEQ;
                exp_type = operate(operand1, analyseExp(operator->right, op2), relop_type, exp1->line);
                if(exp_type != NULL)
                {
                    item = NewIntermediaCodeItem(IF_, relop_type, true_label, op1, op2);
                    InsertIntermediaCode(codes, item);
                    item = NewIntermediaCodeItem(GOTO_, -1, false_label, NULL, NULL);
                    InsertIntermediaCode(codes, item);
                }
                return exp_type;
            }
            int is_and = !strcmp(exp1->right->type, "AND");
            if(is_and || !strcmp(exp1->right->type, "OR"))
            {
                exp_type = operate(operand1, analyseExp(operator->right, op2), is_and ? AND_ : OR_, exp1->line);
                if(exp_type != NULL)
                {
                    Operand *label1 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
                    translateCond(exp1, NULL, is_and ? label1 : true_label, is_and ? false_label : label1);
                    item = NewIntermediaCodeItem(LABEL_, -1, label1, NULL, NULL);
                    InsertIntermediaCode(codes, item);
                    translateCond(exp2, NULL, true_label, false_label);
                }
                return exp_type;
            }
        }
        else
            return exp_type;
    }
    Operand *t1 = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
    Operand *zero = NewOperand(IMMEDIATE, temp_num++, -1, 0, NULL);
    exp_type = analyseExp(exp, t1);
    item = NewIntermediaCodeItem(IF_, NEQ, true_label, t1, zero);
    InsertIntermediaCode(codes, item);
    return exp_type;
}

void translateCalculate(Operand *dest, Operand *operand1, Operand *operand2, int operator)
{
    if(dest == NULL)
        return;
    IntermediaCodeItem *item = NewIntermediaCodeItem(ADD_ + operator, -1, dest, GetValue(operand1), GetValue(operand2));
    InsertIntermediaCode(codes, item);
}

Type *operate(Type *operand1, Type *operand2, int operator, int line)
{
    if((operator > 3 && (operand1->type != 0 || operand2->type != 0)) || (!SameType(operand1, operand2) || (operand1->type > 1)))
    {
        PrintErrorMsg(OPERANDS_TYPE_MISMATCH, line, 1, "Type mismatched for operands.");
        return NULL;
    }
    Type *ans = (Type *)malloc(sizeof(Type));
    memcpy(ans, operand1, sizeof(Type));
    if(operand1->type == 0)
    {
        switch (operator)
        {
        case ADD_:
            ans->value.type_int += operand2->value.type_int;
            break;
        case MINUS_:
            ans->value.type_int -= operand2->value.type_int;
            break;
        case STAR_:
            ans->value.type_int *= operand2->value.type_int;
            break;
        case DIV_:
            ans->value.type_int /= operand2->value.type_int;
            break;
        case AND_:
            ans->value.type_int = ans->value.type_int && operand2->value.type_int;
            break;
        case OR_:
            ans->value.type_int = ans->value.type_int || operand2->value.type_int;
            break;
        case GREATER:
            ans->value.type_int = ans->value.type_int > operand2->value.type_int;
            break;
        case LESS:
            ans->value.type_int = ans->value.type_int < operand2->value.type_int;
            break;
        case GREATER_EQ:
            ans->value.type_int = ans->value.type_int >= operand2->value.type_int;
            break;
        case LESS_EQ:
            ans->value.type_int = ans->value.type_int <= operand2->value.type_int;
            break;
        case EQ:
            ans->value.type_int = ans->value.type_int == operand2->value.type_int;
            break;
        case NEQ:
            ans->value.type_int = ans->value.type_int != operand2->value.type_int;
            break;
        default:
            free(ans);
            ans = NULL;
            break;
        }
    }
    else if(operand1->type == 1)
    {
        switch (operator)
        {
        case 0:
            ans->value.type_float += operand2->value.type_float;
            break;
        case 1:
            ans->value.type_float -= operand2->value.type_float;
            break;
        case 2:
            ans->value.type_float *= operand2->value.type_float;
            break;
        case 3:
            ans->value.type_float /= operand2->value.type_float;
            break;
        default:
            free(ans);
            ans = NULL;
            break;
        }
    }
    return ans;
}

void CheckFunction()
{
    for(int i = 0; i < undef_fun->length; i++)
    {
        Function *fun = (Function *)undef_fun->values[i];
        SymbolItem *item = SearchSymbol(functions, fun->name);
        if(item == NULL)
            continue;
        if(((Function *)item->structure)->def != 1)
            PrintErrorMsg(UNDEFINED_FUNCTION_STATEMENT, undef_fun->keys[i], 3, "Undefined function \"", ((Function *)undef_fun->values[i])->name, "\".");
    }
}

void InsertReadAndWrite()
{
    Function *read = (Function *)malloc(sizeof(Function));
    read->def = 1;
    read->name = "read";
    read->type = (Type *)malloc((sizeof(Type)));
    read->type->size = sizeof(int);
    read->type->type = 0;
    read->type->structure = NULL;
    read->param_structure = NULL;
    InsertSymbolItem(functions, "read", NewSymbolItem((void *)read));

    Function *write = (Function *)malloc(sizeof(Function));
    write->def = 1;
    write->name = "write";
    write->type = (Type *)malloc((sizeof(Type)));
    write->type->size = sizeof(int);
    write->type->type = 0;
    write->type->structure = NULL;
    write->param_structure = (Parameter *)malloc(sizeof(Parameter));
    write->param_structure->offset = 0;
    write->param_structure->next = NULL;
    write->param_structure->type = (Type *)malloc(sizeof(Type));
    write->param_structure->type->name = NULL;
    write->param_structure->type->size = sizeof(int);
    write->param_structure->type->structure = NULL;
    write->param_structure->type->type = 0;
    InsertSymbolItem(functions, "write", NewSymbolItem((void *)write));
}

SymbolItem *SearchSymbolGlobal(char *name)
{
    SymbolItem *item = SearchSymbol(variables, name);
    if(item != NULL)
        return item;
    for(int i = fun_symbol_stack->length - 1; i >= 0; i--)
    {
        SymbolTable *table = (SymbolTable *)fun_symbol_stack->values[i];
        item = SearchSymbol(table, name);
        if(item != NULL)
            return item;
    }
    return NULL;
}

Operand *GetValue(Operand *operand)
{
    if(operand->type != POINTER)
        return operand;
    Operand *value = NewOperand(VARIABLE, temp_num++, -1, 0, NULL);
    IntermediaCodeItem *item = NULL;
    if(operand->offset != 0)
    {
        Operand *offset = NewOperand(IMMEDIATE, 0, -1, operand->offset, NULL);
        Operand *temp_addr = NewOperand(POINTER, temp_num++, 0, 0, NULL);
        item = NewIntermediaCodeItem(ADD_, -1, temp_addr, operand, offset);
        InsertIntermediaCode(codes, item);
        item = NewIntermediaCodeItem(ADDRESS_TO_VALUE_, -1, value, temp_addr, NULL);
    }
    else
        item = NewIntermediaCodeItem(ADDRESS_TO_VALUE_, -1, value, operand, NULL);
    InsertIntermediaCode(codes, item);
    return value;
}

Type *AnalyseExp2TranslateCond(Node *exp, Operand *dest)
{
    Operand *label1 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
    Operand *label2 = NewOperand(LABEL_NUM, label_num++, -1, 0, NULL);
    Operand *false = NewOperand(IMMEDIATE, 0, -1, 0, NULL);
    Operand *true = NewOperand(IMMEDIATE, 0, -1, 1, NULL);
    IntermediaCodeItem *item = NULL;
    if(dest != NULL)
    {
        item = NewIntermediaCodeItem(ASSIGNOP_, -1, dest, false, NULL);
        InsertIntermediaCode(codes, item);
    }
    Type *exp_type = translateCond(exp, dest, label1, label2);
    item = NewIntermediaCodeItem(LABEL_, -1, label1, NULL, NULL);
    InsertIntermediaCode(codes, item);
    if(dest != NULL)
    {
        item = NewIntermediaCodeItem(ASSIGNOP_, -1, dest, true, NULL);
        InsertIntermediaCode(codes, item);
    }
    item = NewIntermediaCodeItem(LABEL_, -1, label2, NULL, NULL);
    InsertIntermediaCode(codes, item);
    return exp_type;
}