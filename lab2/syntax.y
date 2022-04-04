%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
    #include "NodeStruct.h"
    #include "analyse.h"
    #include "lex.yy.c"

    #define MAX_STACK_SIZE 1024

    Node *root;

    int lexical_error = 0, syntax_error = 0;

    void yyerror(char *msg);
    Node *NewNonTerminalNode(int argc, const char *type, ...);
    //SymbolTable *variables = NewSymbolTable();
    //SymbolTable *functions = NewSymbolTable();
    //TrieTree *structure = NewTrieTree();
%}

%locations

%union {
    Node * type_node;
}

%token <type_node> TYPE STRUCT RETURN IF ELSE WHILE
%token <type_node> INT
%token <type_node> FLOAT
%token <type_node> ID
%token <type_node> RELOP SEMI COMMA ASSIGNOP
%token <type_node> PLUS MINUS STAR DIV
%token <type_node> AND OR NOT
%token <type_node> DOT
%token <type_node> LP RP LB RB LC RC

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEGATIVE
%left LP RP LB RB DOT

%type <type_node> Program ExtDefList ExtDef ExtDecList Specifier
%type <type_node> StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Args

%%

// High-level Definitions
Program : ExtDefList
    {
        $$ = NewNonTerminalNode(1, "Program", &$1);
        root = $$;
    };
ExtDefList : /*empty*/
    {
        $$ = NewNonTerminalNode(0, "ExtDefList");
        $$->line = @$.first_line;
        $$->val_type = 0;
    }
    | ExtDef ExtDefList
    {
        $$ = NewNonTerminalNode(2, "ExtDefList", &$1, &$2);
    };
ExtDef : Specifier ExtDecList SEMI
    {
        $$ = NewNonTerminalNode(3, "ExtDef", &$1, &$2, &$3);
    }
    | Specifier SEMI
    {
        $$ = NewNonTerminalNode(2, "ExtDef", &$1, &$2);
    }
    | Specifier FunDec CompSt
    {
        $$ = NewNonTerminalNode(3, "ExtDef", &$1, &$2, &$3);
    }
    | Specifier FunDec SEMI        //函数声明
    {
        $$ = NewNonTerminalNode(3, "ExtDef", &$1, &$2, &$3);
    }
    | error SEMI
    {
        $$ = NewNonTerminalNode(0, "ExtDef");
    };
ExtDecList : VarDec
    {
        $$ = NewNonTerminalNode(1, "ExtDecList", &$1);
    }
    | VarDec COMMA ExtDecList
    {
        $$ = NewNonTerminalNode(3, "ExtDecList", &$1, &$2, &$3);
    };

//Specifiers
Specifier : TYPE
    {
        $$ = NewNonTerminalNode(1, "Specifier", &$1);
    }
    | StructSpecifier
    {
        $$ = NewNonTerminalNode(1, "Specifier", &$1);
    };
StructSpecifier : STRUCT OptTag LC DefList RC
    {
        $$ = NewNonTerminalNode(5, "StructSpecifier", &$1, &$2, &$3, &$4, &$5);
    }
    | STRUCT Tag
    {
        $$ = NewNonTerminalNode(2, "StructSpecifier", &$1, &$2);
    }
    | error RC
    {
        $$ = NewNonTerminalNode(0, "StructSpecifier");
    };
OptTag : /*empty*/
    {
        $$ = NewNonTerminalNode(0, "OptTag");
        $$->line = @$.first_line;
        $$->val_type = 0;
    }
    | ID
    {
        $$ = NewNonTerminalNode(1, "OptTag", &$1);
    };
Tag : ID
    {
        $$ = NewNonTerminalNode(1, "Tag", &$1);
    };

//Declarators
VarDec : ID
    {
        $$ = NewNonTerminalNode(1, "VarDec", &$1);
    }
    | VarDec LB INT RB
    {
        $$ = NewNonTerminalNode(4, "VarDec", &$1, &$2, &$3, &$4);
    }
    | error RB
    {
        $$ = NewNonTerminalNode(0, "VarDec");
    };
FunDec : ID LP VarList RP
    {
        $$ = NewNonTerminalNode(4, "FunDec", &$1, &$2, &$3, &$4);
    }
    | ID LP RP
    {
        $$ = NewNonTerminalNode(3, "FunDec", &$1, &$2, &$3);
    }
    | error RP
    {
        $$ = NewNonTerminalNode(0, "FunDec");
    };
VarList : ParamDec COMMA VarList
    {
        $$ = NewNonTerminalNode(3, "VarList", &$1, &$2, &$3);
    }
    | ParamDec
    {
        $$ = NewNonTerminalNode(1, "VarList", &$1);
    };
ParamDec : Specifier VarDec
    {
        $$ = NewNonTerminalNode(2, "ParamDec", &$1, &$2);
    };

//Statements
CompSt : LC DefList StmtList RC
    {
        $$ = NewNonTerminalNode(4, "CompSt", &$1, &$2, &$3, &$4);
    }
    | error RC
    {
        $$ = NewNonTerminalNode(0, "CompSt");
    };
StmtList : /*empty*/
    {
        $$ = NewNonTerminalNode(0, "StmtList");
        $$->line = @$.first_line;
        $$->val_type = 0;
    }
    | Stmt StmtList
    {
        $$ = NewNonTerminalNode(2, "StmtList", &$1, &$2);
    };
Stmt : Exp SEMI
    {
        $$ = NewNonTerminalNode(2, "Stmt", &$1, &$2);
    }
    | CompSt
    {
        $$ = NewNonTerminalNode(1, "Stmt", &$1);
    }
    | RETURN Exp SEMI
    {
        $$ = NewNonTerminalNode(3, "Stmt", &$1, &$2, &$3);
    }
    | IF LP Exp RP Stmt
    {
        $$ = NewNonTerminalNode(5, "Stmt", &$1, &$2, &$3, &$4, &$5);
    }
    | IF LP Exp RP Stmt ELSE Stmt
    {
        $$ = NewNonTerminalNode(7, "Stmt", &$1, &$2, &$3, &$4, &$5, &$6, &$7);
    }
    | WHILE LP Exp RP Stmt
    {
        $$ = NewNonTerminalNode(5, "Stmt", &$1, &$2, &$3, &$4, &$5);
    }
    | error SEMI
    {
        $$ = NewNonTerminalNode(0, "Stmt");
    };

//Local Definitions
DefList : /*empty*/
    {
        $$ = NewNonTerminalNode(0, "DefList");
        $$->line = @$.first_line;
        $$->val_type = 0;
    }
    | Def DefList
    {
        $$ = NewNonTerminalNode(2, "DefList", &$1, &$2);
    };
Def : Specifier DecList SEMI
    {
        $$ = NewNonTerminalNode(3, "Def", &$1, &$2, &$3);
    }
    | error SEMI
    {
        $$ = NewNonTerminalNode(0, "Def");
    };
DecList : Dec
    {
        $$ = NewNonTerminalNode(1, "DecList", &$1);
    }
    | Dec COMMA DecList
    {
        $$ = NewNonTerminalNode(3, "DecList", &$1, &$2, &$3);
    };
Dec : VarDec
    {
        $$ = NewNonTerminalNode(1, "Dec", &$1);
    }
    | VarDec ASSIGNOP Exp
    {
        $$ = NewNonTerminalNode(3, "Dec", &$1, &$2, &$3);
    };

//Expressions
Exp : Exp ASSIGNOP Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp AND Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp OR Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp RELOP Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp PLUS Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp MINUS Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp STAR Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp DIV Exp
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | LP Exp LP
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | MINUS Exp %prec NEGATIVE
    {
        $$ = NewNonTerminalNode(2, "Exp", &$1, &$2);
    }
    | NOT Exp
    {
        $$ = NewNonTerminalNode(2, "Exp", &$1, &$2);
    }
    | ID LP Args RP
    {
        $$ = NewNonTerminalNode(4, "Exp", &$1, &$2, &$3, &$4);
    }
    | ID LP RP
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | Exp LB Exp RB
    {
        $$ = NewNonTerminalNode(4, "Exp", &$1, &$2, &$3, &$4);
    }
    | Exp DOT ID
    {
        $$ = NewNonTerminalNode(3, "Exp", &$1, &$2, &$3);
    }
    | ID
    {
        $$ = NewNonTerminalNode(1, "Exp", &$1);
    }
    | INT
    {
        $$ = NewNonTerminalNode(1, "Exp", &$1);
    }
    | FLOAT
    {
        $$ = NewNonTerminalNode(1, "Exp", &$1);
    };
Args : Exp COMMA Args
    {
        $$ = NewNonTerminalNode(3, "Args", &$1, &$2, &$3);
    }
    | Exp
    {
        $$ = NewNonTerminalNode(1, "Args", &$1);
    };

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
    if(!lexical_error && !syntax_error)
        analyseProgram(root);
    return 0;
}

void yyerror(char *msg)
{
    printf("Error type B at Line %d: %s\n", yylloc.first_line, msg);
    syntax_error = 1;
}

Node *NewNonTerminalNode(int argc, const char *type, ...)
{
    Node *parent;
    parent = (Node *)malloc(sizeof(Node));
    va_list list;
    va_start(list, type);
    parent->type = (char *)malloc(sizeof(char) * (strlen(type) + 1));
    strcpy(parent->type, type);
    parent->val_type = -1;
    if(syntax_error)
        return parent;

    YYSTYPE *last;
    YYSTYPE *now;
    for(int i = 0; i < argc; i++)
    {
        now = va_arg(list, YYSTYPE *);
        if(!i)
        {
            parent->left = ((*now).type_node);
            parent->line = ((*now).type_node)->line;
        }
        else
            ((*last).type_node)->right = ((*now).type_node);
        last = now;
    }

    va_end(list);

    return parent;
}

/*
void PrintTree()
{
    Node *stack[MAX_STACK_SIZE];
    stack[0] = root;
    int state[MAX_STACK_SIZE];
    memset(state, 0, sizeof(state));
    state[0] = 6;
    int stack_top = 1;
    Node *now = root;
    int deep = 0;

    PrintNode(root, deep);
    while(1)
    {
        if(state[stack_top-1] & 0x2)
        {
            state[stack_top-1] -= 2;
            if(now->left != NULL)
            {
                now = now->left;
                state[stack_top] = 7;
                stack[stack_top++] = now;
                deep++;
                PrintNode(now, deep);
            }
        }
        else if(state[stack_top-1] & 0x4)
        {
            state[stack_top-1] -= 4;
            if(now->right != NULL)
            {
                now = now->right;
                state[stack_top] = 6;
                stack[stack_top++] = now;
                PrintNode(now, deep);
            }
        }
        else
        {
            stack_top--;
            if(!stack_top)
                break;
            now = stack[stack_top-1];
            if(state[stack_top] % 2)
                deep--;
        }
    }
}
*/