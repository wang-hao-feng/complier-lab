%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "syntax.tab.h"

    int yycolumn = 1;
    #define YY_USER_ACTION update();

    extern int yylineon;
    extern int lexical_error;

    unsigned int atocmmi(const char *nptr);
    void update();
    struct Node *NewTerminalNode(char *text, const char *type);
%}

digit [0-9]
decimal 0|[1-9]{digit}*
octal 0[0-7]+
hex 0[xX][0-9a-fA-F]+
int {decimal}|{octal}|{hex}
float {digit}+\.{digit}+|({digit}+\.{digit}*|{digit}*\.{digit}+)[eE][\+-]?{digit}+

letter [_a-zA-Z]
id {letter}({letter}|{digit})*

%%

"int"|"float" { yylval.type_node = NewTerminalNode(yytext, "TYPE"); return TYPE; }
"struct" { yylval.type_node = NewTerminalNode(yytext, "STRUCT"); return STRUCT; }
"return" { yylval.type_node = NewTerminalNode(yytext, "RETURN"); return RETURN; }
"if" { yylval.type_node = NewTerminalNode(yytext, "IF"); return IF; }
"else" { yylval.type_node = NewTerminalNode(yytext, "ELSE"); return ELSE; }
"while" { yylval.type_node = NewTerminalNode(yytext, "WHILE"); return WHILE; }

{int} { yylval.type_node = NewTerminalNode(yytext, "INT"); return INT; }
{float} { yylval.type_node = NewTerminalNode(yytext, "FLOAT"); return FLOAT; }
{id} { yylval.type_node = NewTerminalNode(yytext, "ID"); return ID; }

0({letter}|{digit})+ {
    lexical_error = 1;
    if(yytext[1] == 'x')
        printf("Error type A at Line %d: Illegal hexadecimal number \'%s\'\n", yylineno, yytext);
    else
        printf("Error type A at Line %d: Illegal octal number \'%s\'\n", yylineno, yytext);
    return INT;
}

\.{digit}+|{digit}+\.|{digit}*\.{digit}+[eE][\+-]?|[eE][\+-]?{digit}+ {
    lexical_error = 1;
    printf("Error type A at Line %d: Illegal float point number \'%s\'\n", yylineno, yytext);
    return FLOAT;
}

({letter}|{digit})+ {
    lexical_error = 1;
    printf("Error type A at Line %d: Illegal ID \'%s\'\n", yylineno, yytext);
    return ID;
}

";" { yylval.type_node = NewTerminalNode(yytext, "SEMI"); return SEMI; }
"," { yylval.type_node = NewTerminalNode(yytext, "COMMA"); return COMMA; }
"=" { yylval.type_node = NewTerminalNode(yytext, "ASSIGNOP"); return ASSIGNOP; }
">"|"<"|">="|"<="|"=="|"!=" { yylval.type_node = NewTerminalNode(yytext, "RELOP"); return RELOP; }

"+" { yylval.type_node = NewTerminalNode(yytext, "PLUS"); return PLUS; }
"-" { yylval.type_node = NewTerminalNode(yytext, "MINUS"); return MINUS; }
"*" { yylval.type_node = NewTerminalNode(yytext, "STAR"); return STAR; }
"/" { yylval.type_node = NewTerminalNode(yytext, "DIV"); return DIV; }

"&&" { yylval.type_node = NewTerminalNode(yytext, "AND"); return AND; }
"||" { yylval.type_node = NewTerminalNode(yytext, "OR"); return OR; }
"!" { yylval.type_node = NewTerminalNode(yytext, "NOT"); return NOT; }

"." { yylval.type_node = NewTerminalNode(yytext, "DOT"); return DOT; }

"(" { yylval.type_node = NewTerminalNode(yytext, "LP"); return LP; }
")" { yylval.type_node = NewTerminalNode(yytext, "RP"); return RP; }
"[" { yylval.type_node = NewTerminalNode(yytext, "LB"); return LB; }
"]" { yylval.type_node = NewTerminalNode(yytext, "RB"); return RB; }
"{" { yylval.type_node = NewTerminalNode(yytext, "LC"); return LC; }
"}" { yylval.type_node = NewTerminalNode(yytext, "RC"); return RC; }

"//" { 
    char c = input(); 
    while (c != '\n') 
        c = input();
    unput(c);
}

"/*" { 
    char c1; 
    char c2 = input(); 
    int flag = 1;
    while(flag) 
    { 
        c1 = c2; 
        c2 = input(); 
        if(c1 == '\n')
        {
            yylineno++;
            yycolumn = 1;
        }
        if(c1 == '*' && c2 == '/')
            flag = 0;
    } 
}

" "|\t {}
"\n" { yylineno++; yycolumn = 1; }

. { 
    printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);
    lexical_error = 1;
}

%%

unsigned int atocmmi(const char *nptr)
{
    unsigned int val = 0;       //数值
    unsigned int log_base = 3;     //进制数的对数

    //确定进制数，如果为十进制则直接调用atoi并返回
    if(*nptr == '0')
    {
        if(strlen(nptr) <= 1)
            return 0;
        nptr++;

        if(*nptr == 'x' || *nptr == 'X')
        {
            log_base = 4;
            nptr++;
        }
    }
    else
        return (unsigned int)atoi(nptr);

    //字符串转整型数
    while(*nptr != '\0')
    {
        val <<= log_base;

        if(*nptr >= '0' && *nptr <= '9')
            val += *nptr - '0';
        else if(*nptr >= 'a' && *nptr <= 'f')
            val += *nptr - 'a' + 10;
        else if(*nptr >= 'A' && *nptr <= 'F')
            val += *nptr - 'A' + 10;
        
        nptr++;
    }

    return val;
}

void update()
{
    yylloc.first_line = yylloc.last_line = yylineno;
    yylloc.first_column = yycolumn;
    yylloc.last_column = yycolumn + yyleng - 1;
    yycolumn += yyleng;
}

struct Node *NewTerminalNode(char *text, const char *type)
{
    struct Node *val = (struct Node *)malloc(sizeof(struct Node));
    val->left = val->right = NULL;
    val->line = yylineno;
    val->type = (char *)malloc(sizeof(char) * (strlen(type) + 1));
    strcpy(val->type, type);

    if(!strcmp(type, "INT"))
    {
        val->val_type = 1;
        (val->val).type_int = atocmmi(text);
    }
    else if(!strcmp(type, "FLOAT"))
    {
        val->val_type = 2;
        (val->val).type_float = atof(text);
    }
    else
    {
        if(!strcmp(type, "ID") || !strcmp(type, "TYPE"))
            val->val_type = 3;
        else
            val->val_type = 4;
        (val->val).type_string = (char *)malloc(sizeof(char) * (strlen(text) + 1));
        strcpy((val->val).type_string, text);
    }

    return val;
}