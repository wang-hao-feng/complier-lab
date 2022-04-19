#include "NodeStruct.h"
#include "TypeExp.h"
#include "IntermediaCode.h"

#ifndef _ANALYSE_H_
#define _ANALYSE_H_

extern IntermediaCodes *codes;

//语义分析
void analyseProgram(Node *program);

void analyseExtDef(Node *extdef);

Type *analyseSpecifier(Node *specifier);

Type *analyseVarDec(Type *base_type, Node *vardec,  int is_def_func);

void *analyseDef(Node *def, int is_func);

Type *analyseExp(Node *exp, Operand *dest);

Function *analyseFunDec(Node *fundec);

void analyseCompSt(Node *compst);

void analyseStmt(Node *stmt);

Field *analyseArgs(Node *args, int is_write);

//中间代码生成
void translateFunDec(Function *fun);

void translateVarDec(Type *var);

void translateASSIGNOP(Operand *dest, Operand *operand);

Type *translateCond(Node *exp, Operand *dest, Operand *true_label, Operand *false_label);

void translateCalculate(Operand *dest, Operand *operand1, Operand *operand2, int operator); //operator == 0表示+, 1表示-, 2表示*, 3表示/

//辅助函数
Type *operate(Type *operand1, Type *operand2, int operator, int line);

void CheckFunction();

Operand *GetValue(Operand *operand);    //将是地址的操作数取出放到一个变量中

Type *AnalyseExp2TranslateCond(Node *exp, Operand *dest);

#endif