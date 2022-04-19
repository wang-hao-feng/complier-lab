#include "analyse.h"
#include <stdlib.h>
#include "SymbolTable.h"
#include "trie.h"
#include "error.h"
#include "stack.h"

SymbolTable *variables = NULL;
SymbolTable *functions = NULL;
TrieTree *structures = NULL;
Type *return_type = NULL;
Map *undef_fun = NULL;
Stack *fun_symbol_stack = NULL;

SymbolItem *SearchSymbolGlobal(char *name);

void analyseProgram(Node *program)
{
    variables = NewSymbolTable();
    functions = NewSymbolTable();
    structures = NewTrieTree();
    undef_fun = NewMap();
    fun_symbol_stack = NewStack();

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
                InsertSymbolItem(variables, temp->name, NewSymbolItem((void *)temp));
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
            else if(strcmp(fundec->right->type, "SEMI"))
            {
                ((Function *)item->structure)->def = 1;
                return_type = ((Function *)item->structure)->type;
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
                    if(dec->left->right != NULL)
                    {
                        Node *exp = dec->left->right->right;
                        if(!SameType(temp, analyseExp(exp)))
                            PrintErrorMsg(ASSIGNMENT_TYPE_MISMATCH, dec->left->line, 1, "Type mismatched for assignment.");
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

Type *analyseExp(Node *exp)
{
    Type *exp_type = NULL;
    Node *temp = exp->left;
    if(!strcmp(temp->type, "Exp"))
    {
        Type *operand1 = analyseExp(temp);
        Node *operator = temp->right;
        if(operand1 != NULL)
        {
            if(!strcmp(operator->type, "ASSIGNOP"))
            {
                Type *operand2 = analyseExp(operator->right);
                if(!SameType(operand1, operand2))
                    PrintErrorMsg(ASSIGNMENT_TYPE_MISMATCH, temp->line, 1, "Type mismatched for assignment.");
                else if(operand1->name == NULL)
                    PrintErrorMsg(INVALID_LEFT_HAND, temp->line, 1, "The left-hand side of an assignment must be a variable.");
            }
            else if(!strcmp(operator->type, "AND"))
                exp_type = operate(operand1, analyseExp(operator->right), 4, temp->line);
            else if(!strcmp(operator->type, "OR"))
                exp_type = operate(operand1, analyseExp(operator->right), 5, temp->line);
            else if(!strcmp(operator->type, "RELOP"))
            {
                char *c = operator->val.type_string;
                if(*c == '>')
                {
                    if(*(c + 1) == '\0')
                        exp_type = operate(operand1, analyseExp(operator->right), 6, temp->line);
                    else
                        exp_type = operate(operand1, analyseExp(operator->right), 8, temp->line);
                }
                else if(*c == '<')
                {
                    if(*(c + 1) == '\0')
                        exp_type = operate(operand1, analyseExp(operator->right), 7, temp->line);
                    else
                        exp_type = operate(operand1, analyseExp(operator->right), 9, temp->line);
                }
                else if(*c == '=')
                    exp_type = operate(operand1, analyseExp(operator->right), 10, temp->line);
                else if(*c == '!')
                    exp_type = operate(operand1, analyseExp(operator->right), 11, temp->line);
            }
            else if(!strcmp(operator->type, "PLUS"))
                exp_type = operate(operand1, analyseExp(operator->right), 0, temp->line);
            else if(!strcmp(operator->type, "MINUS"))
                exp_type = operate(operand1, analyseExp(operator->right), 1, temp->line);
            else if(!strcmp(operator->type, "STAR"))
                exp_type = operate(operand1, analyseExp(operator->right), 2, temp->line);
            else if(!strcmp(operator->type, "DIV"))
                exp_type = operate(operand1, analyseExp(operator->right), 3, temp->line);
            else if(!strcmp(operator->type, "LB"))
            {
                if(operand1->type != 2)
                    PrintErrorMsg(NOT_ARRAY, temp->line, 3, "\"", operand1->name, "\" is not an array");
                else
                {
                    Type *operand2 = analyseExp(operator->right);
                    if(operand2->type == 1)
                        PrintErrorMsg(NOT_INTEGER_INDEX, temp->line, 1, "\"float\" is not an integer");
                    else if(operand2->type > 1)
                        PrintErrorMsg(NOT_INTEGER_INDEX, temp->line, 3, "\"", operand2->name, "\" is not an integer");
                    else
                        exp_type = (Type *)operand1->structure;
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
                        exp_type = field->type;
                }
            }
        }
    }
    else if(!strcmp(temp->type, "LP"))
        exp_type = analyseExp(temp->right);
    else if(!strcmp(temp->type, "MINUS"))
    {
        Type *operand = analyseExp(temp->right);
        if(operand->type != 0 && operand->type != 1)
            PrintErrorMsg(RETURN_TYPE_MISMATCH, temp->line, 1, "Type mismatched for minus, the operand should be int or float.");
        else
        {
            exp_type = (Type *)malloc(sizeof(Type));
            memcpy(exp_type, operand, sizeof(Type));
            if(operand->type == 0)
                exp_type->value.type_int = -exp_type->value.type_int;
            else
                exp_type->value.type_float = -exp_type->value.type_float;
        }
    }
    else if(!strcmp(temp->type, "NOT"))
    {
        Type *operand = analyseExp(temp->right);
        if(operand->type != 0)
            PrintErrorMsg(RETURN_TYPE_MISMATCH, temp->line, 1, "Type mismatched for not, the operand should be int.");
        else
        {
            exp_type = (Type *)malloc(sizeof(Type));
            memcpy(exp_type, operand, sizeof(Type));
            exp_type->value.type_int = !exp_type->value.type_int;
        }
    }
    else if(!strcmp(temp->type, "ID"))
    {
        if(temp->right == NULL)
        {
            SymbolItem *item = SearchSymbolGlobal(temp->val.type_string);
            if(item == NULL)
                PrintErrorMsg(UNDIFINED_VARIABLE, temp->line, 3, "Undefined variable \"", temp->val.type_string, "\".");
            else
                exp_type = (Type *)item->structure;  
        }
        else
        {
            SymbolItem *item = SearchSymbol(functions, temp->val.type_string);
            if(item == NULL)
            {
                if(SearchSymbol(variables, temp->val.type_string))
                    PrintErrorMsg(NOT_FUNCTION, temp->line, 3, "\"", temp->val.type_string, "\" is not a function");
                else
                    PrintErrorMsg(UNDIFINED_FUNCTION, temp->line, 3, "Undefined function \"", temp->val.type_string, "\".");
            }
            else
            {
                Function *fun = (Function *)item->structure;
                Parameter *args = NULL;
                if(temp->right->right->right != NULL)
                    args = analyseArgs(temp->right->right);
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
                InsertSymbolItem(variables, p->type->name, NewSymbolItem(p->type));
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
        analyseExp(temp);
    else if(!strcmp(temp->type, "CompSt"))
        analyseCompSt(temp);
    else if(!strcmp(temp->type, "RETURN"))
    {
        Type *re = analyseExp(temp->right);
        if(!SameType(re, return_type))
            PrintErrorMsg(RETURN_TYPE_MISMATCH, temp->line, 1, "Type mismatched for return.");
    }
    else if(!strcmp(temp->type, "IF"))
    {
        Node *exp = temp->right->right;
        Type *exp_type = analyseExp(exp);
        if(exp_type != NULL)
        {
            if(exp_type->type != 0)
                PrintErrorMsg(OPERANDS_TYPE_MISMATCH, temp->line, 1, "Type mismatched for if");
        }
        Node *stmt2 = exp->right->right;
        analyseStmt(stmt2);
        if(stmt2->right != NULL)
            analyseStmt(stmt2->right->right);
    }
    else if(!strcmp(temp->type, "WHILE"))
    {
        Node *exp = temp->right->right;
        Type *exp_type = analyseExp(exp);
        if(exp_type->type != 0)
            PrintErrorMsg(OPERANDS_TYPE_MISMATCH, temp->line, 1, "Type mismatched for while");
        analyseStmt(exp->right->right);
    }
}

Field *analyseArgs(Node *args)
{
    Parameter *params = NULL;
    Node *exp = args->left;
    while(1)
    {
        Parameter *temp = (Parameter *)malloc(sizeof(Parameter));
        temp->type = analyseExp(exp);
        temp->next = params;
        params = temp;
        if(exp->right == NULL)
            break;
        exp = exp->right->right->left;
    }
    return params;
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
        case 0:
            ans->value.type_int += operand2->value.type_int;
            break;
        case 1:
            ans->value.type_int -= operand2->value.type_int;
            break;
        case 2:
            ans->value.type_int *= operand2->value.type_int;
            break;
        case 3:
            ans->value.type_int /= operand2->value.type_int;
            break;
        case 4:
            ans->value.type_int = ans->value.type_int && operand2->value.type_int;
            break;
        case 5:
            ans->value.type_int = ans->value.type_int || operand2->value.type_int;
            break;
        case 6:
            ans->value.type_int = ans->value.type_int > operand2->value.type_int;
            break;
        case 7:
            ans->value.type_int = ans->value.type_int < operand2->value.type_int;
            break;
        case 8:
            ans->value.type_int = ans->value.type_int >= operand2->value.type_int;
            break;
        case 9:
            ans->value.type_int = ans->value.type_int <= operand2->value.type_int;
            break;
        case 10:
            ans->value.type_int = ans->value.type_int == operand2->value.type_int;
            break;
        case 11:
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