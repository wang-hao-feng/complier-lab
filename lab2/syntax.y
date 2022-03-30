%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
    #include "error.h"
    #include "lex.yy.c"

    int lexical_error = 0;
    int syntax_error = 0;

    void yyerror(char *msg);
%}

%locations

%token TYPE STRUCT RETURN IF ELSE WHILE
%token INT
%token FLOAT
%token ID
%token RELOP SEMI COMMA ASSIGNOP
%token PLUS MINUS STAR DIV
%token AND OR NOT
%token DOT
%token LP RP LB RB LC RC

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEGATIVE
%left LP RP LB RB DOT

%%

// High-level Definitions
Program : ExtDefList;
ExtDefList : /*empty*/
    | ExtDef ExtDefList;
ExtDef : Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    | error SEMI;
ExtDecList : VarDec
    | VarDec COMMA ExtDecList;

//Specifiers
Specifier : TYPE
    | StructSpecifier;
StructSpecifier : STRUCT OptTag LC DefList RC
    | STRUCT Tag
    | error RC;
OptTag : /*empty*/
    | ID;
Tag : ID;

//Declarators
VarDec : ID
    | VarDec LB INT RB
    | error RB;
FunDec : ID LP VarList RP
    | ID LP RP
    | error RP;
VarList : ParamDec COMMA VarList
    | ParamDec;
ParamDec : Specifier VarDec;

//Statements
CompSt : LC DefList StmtList RC
    | error RC;
StmtList : /*empty*/
    | Stmt StmtList;
Stmt : Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt
    | error SEMI;

//Local Definitions
DefList : /*empty*/
    | Def DefList;
Def : Specifier DecList SEMI
    | error SEMI;
DecList : Dec
    | Dec COMMA DecList;
Dec : VarDec
    | VarDec ASSIGNOP Exp;

//Expressions
Exp : Exp ASSIGNOP Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp RELOP Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp STAR Exp
    | Exp DIV Exp
    | LP Exp LP
    | MINUS Exp %prec NEGATIVE
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT
    | FLOAT;
Args : Exp COMMA Args
    | Exp;

%%

int main(int argc, char **argv)
{
    if(argc <= 1)
        return 1;
    FILE *f = fopen(argv[1], "r");
    if(!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    return 0;
}

void yyerror(char *msg)
{
    printf("Error type B at Line %d: %s\n", yylloc.first_line, msg);
    syntax_error = 1;
}