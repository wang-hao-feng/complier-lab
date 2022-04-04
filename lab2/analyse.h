#include "NodeStruct.h"
#include "TypeExp.h"

#ifndef _ANALYSE_H_
#define _ANALYSE_H_

void analyseProgram(Node *program);

void analyseExtDef(Node *extdef);

Type *analyseSpecifier(Node *specifier);

Type *analyseVarDec(Type *base_type, Node *vardec,  int is_def_func);

void *analyseDef(Node *def, int is_func);

Type *analyseExp(Node *exp);

Function *analyseFunDec(Node *fundec);

void analyseCompSt(Node *compst);

void analyseStmt(Node *stmt);

Field *analyseArgs(Node *args);

Type *operate(Type *operand1, Type *operand2, int operator, int line);

void CheckFunction();

#endif