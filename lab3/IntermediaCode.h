#ifndef _INTERMEDIACODE_H_
#define _INTERMEDIACODE_H_

#define LABEL_ 0                 //LABEL dest.operand_temp_num :
#define FUNCTION_ 1              //FUNCTION dest.name :
#define ASSIGNOP_ 2              //dest.operand_temp_num := operand1.operand_temp_num
#define ADD_ 3                   //dest.operand_temp_num := operand1.operand_temp_num + operand2.operand_temp_num
#define MINUS_ 4                 //dest.operand_temp_num := operand1.operand_temp_num - operand2.operand_temp_num
#define STAR_ 5                  //dest.operand_temp_num := operand1.operand_temp_num * operand2.operand_temp_num
#define DIV_ 6                   //dest.operand_temp_num := operand1.operand_temp_num / operand2.operand_temp_num
#define GET_ADDRESS_ 7           //dest.operand_temp_num := &operand1.operand_temp_num
#define ADDRESS_TO_VALUE_ 8      //dest.operand_temp_num := *operand1.operand_temp_num
#define VALUE_TO_ADDRESS_ 9      //*dest.operand_temp_num := operand1.operand_temp_num
#define GOTO_ 10                 //GOTO dest.operand_temp_num
#define IF_ 11                   //IF operand1.operand_temp_num relop_type operand2.operand_temp_num GOTO dest.operand_temp_num
#define RETURN_ 12               //RETURN dest.operand_temp_num
#define DEC_ 13                  //DEC dest.operand_temp_num [dest.offset]
#define ARG_ 14                  //ARG dest.operand_temp_num
#define CALL_ 15                 //dest.operand_temp_num := CALL operand1.name
#define PARAM_ 16                //PARAM dest.operand_temp_num
#define READ_ 17                 //READ dest.operand_temp_num
#define WRITE_ 18                //WRITE dest.operand_temp_num

#define AND_ 7
#define OR_ 8
#define GREATER 9
#define GREATER_EQ 10
#define LESS 11
#define LESS_EQ 12
#define EQ 13
#define NEQ 14

/*
    用于区分操作数
    type == 0表示当前数是立即数, == 1表示是一般变量,  == 2表示是指针, == 3表示是函数名
    operand_temp_num表示当前操作数的值存储在哪个临时变量中
    name存储当前操作数的变量名
    如果是数组的某个值，name表示数组名
    如果是结构体，name表示结构体名，而不是域名
    offset表示当前变量的偏移量，如果offset==-1，则表示当前变量不是数组的某个值或结构体的域
    immediate表示当前数是立即数
*/
#define IMMEDIATE 0
#define VARIABLE 1
#define POINTER 2
#define FUNCTION_NAME 3
#define LABEL_NUM 4
typedef struct Operand
{
    int type;
    int operand_temp_num;
    int offset;
    int immediate;
    char *name;
}Operand;

typedef struct IntermediaCodeItem
{
    struct IntermediaCodeItem* last;
    struct IntermediaCodeItem* next;
    int code_type;
    int relop;
    //目的操作数
    Operand *dest;
    //第一个操作数
    Operand *operand1;
    //第二个从操作数
    Operand *operand2;
}IntermediaCodeItem;

typedef struct IntermediaCodes
{
    IntermediaCodeItem *head;
    IntermediaCodeItem *tail;
}IntermediaCodes;

IntermediaCodeItem *NewIntermediaCodeItem(int code_type, int relop, Operand *dest, Operand *operand1, Operand *operand2);
void InsertIntermediaCode(IntermediaCodes *codes, IntermediaCodeItem *item);
void WriteIntermediaCode(IntermediaCodes *codes, char *filename);
Operand *NewOperand(int type, int operand_temp_num, int offset, int immediate, char *name);

#endif