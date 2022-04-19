#include "IntermediaCode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 256

void Intermediaitem2String(char *buffer, IntermediaCodeItem *item);

void Operate2String(char *buffer, Operand *operand);

void Relop2String(char *buffer, int relop_type);

void itoa(char *buffer, int num);

IntermediaCodeItem *NewIntermediaCodeItem(int code_type, int relop, Operand *dest, Operand *operand1, Operand *operand2)
{
    IntermediaCodeItem *item = (IntermediaCodeItem *)malloc(sizeof(IntermediaCodeItem));
    item->last = NULL;
    item->next = NULL;
    item->code_type = code_type;
    item->relop = relop;
    item->dest = dest;
    item->operand1 = operand1;
    item->operand2 = operand2;
    return item;
}

void InsertIntermediaCode(IntermediaCodes *codes, IntermediaCodeItem *item)
{
    item->last = codes->tail;
    if(codes->tail != NULL)
        codes->tail->next = item;
    codes->tail = item;
    if(codes->head == NULL)
        codes->head = item;
}

Operand *NewOperand(int type, int operand_temp_num, int offset, int immediate, char *name)
{
    Operand *operand = (Operand *)malloc((sizeof(Operand)));
    operand->type = type;
    operand->operand_temp_num = operand_temp_num;
    operand->offset = offset;
    operand->immediate = immediate;
    operand->name = name;
    return operand;
}

void WriteIntermediaCode(IntermediaCodes *codes, char *filename)
{
    FILE *fp = fopen(filename, "w");
    if(fp == NULL)
        return;
    
    char buffer[BUFFER_SIZE];

    for(IntermediaCodeItem *item = codes->head; item != NULL; item = item->next)
    {
        Intermediaitem2String(buffer, item);
        fwrite(buffer, strlen(buffer), 1, fp);
    }

    fclose(fp);
}

void Intermediaitem2String(char *buffer, IntermediaCodeItem *item)
{
    memset((void *)buffer, 0, BUFFER_SIZE);
    char operand_buffer[BUFFER_SIZE];
    char relop_buffer[3];
    char num_buffer[BUFFER_SIZE];
    int code_type = item->code_type;
    
    if(code_type == LABEL_)
    {
        strcat(buffer, "LABEL ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " :");
    }
    else if(code_type == FUNCTION_)
    {
        strcat(buffer, "FUNCTION ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " :");
    }
    else if(code_type == ASSIGNOP_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == ADD_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
        strcat(buffer, " + ");
        Operate2String(operand_buffer, item->operand2);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == MINUS_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
        strcat(buffer, " - ");
        Operate2String(operand_buffer, item->operand2);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == STAR_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
        strcat(buffer, " * ");
        Operate2String(operand_buffer, item->operand2);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == DIV_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
        strcat(buffer, " / ");
        Operate2String(operand_buffer, item->operand2);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == GET_ADDRESS_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := &");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == ADDRESS_TO_VALUE_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := *");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == VALUE_TO_ADDRESS_)
    {
        strcat(buffer, "*");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == GOTO_)
    {
        strcat(buffer, "GOTO ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == IF_)
    {
        strcat(buffer, "IF ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
        strcat(buffer, " ");
        Relop2String(relop_buffer, item->relop);
        strcat(buffer, relop_buffer);
        strcat(buffer, " ");
        Operate2String(operand_buffer, item->operand2);
        strcat(buffer, operand_buffer);
        strcat(buffer, " GOTO ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == RETURN_)
    {
        strcat(buffer, "RETURN ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == DEC_)
    {
        strcat(buffer, "DEC ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " ");
        itoa(num_buffer, item->dest->offset);
        strcat(buffer, num_buffer);
    }
    else if(code_type == ARG_)
    {
        strcat(buffer, "ARG ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == CALL_)
    {
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
        strcat(buffer, " := CALL ");
        Operate2String(operand_buffer, item->operand1);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == PARAM_)
    {
        strcat(buffer, "PARAM ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == READ_)
    {
        strcat(buffer, "READ ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }
    else if(code_type == WRITE_)
    {
        strcat(buffer, "WRITE ");
        Operate2String(operand_buffer, item->dest);
        strcat(buffer, operand_buffer);
    }

    strcat(buffer, "\n");
}

void Operate2String(char *buffer, Operand *operand)
{
    memset(buffer, 0, sizeof(buffer));
    char num_buffer[BUFFER_SIZE];

    int type = operand->type;
    if(type == IMMEDIATE)
    {
        strcat(buffer, "#");
        itoa(num_buffer, operand->immediate);
        strcat(buffer, num_buffer);
    }
    else if(type == VARIABLE || type == POINTER)
    {
        int operand_temp_num = operand->operand_temp_num;
        strcat(buffer, operand_temp_num < 0 ? "v" : "t");
        itoa(num_buffer, operand_temp_num);
        strcat(buffer, num_buffer + (operand_temp_num < 0));
    }
    else if(type == FUNCTION_NAME)
        strcat(buffer, operand->name);
    else if(type == LABEL_NUM)
    {
        strcat(buffer, "label");
        itoa(num_buffer, operand->operand_temp_num);
        strcat(buffer, num_buffer);
    }
}

void Relop2String(char *buffer, int relop_type)
{
    memset(buffer, 0, sizeof(buffer));

    if(relop_type == GREATER)
        strcpy(buffer, ">");
    else if(relop_type == GREATER_EQ)
        strcpy(buffer, ">=");
    else if(relop_type == LESS)
        strcpy(buffer, "<");
    else if(relop_type == LESS_EQ)
        strcpy(buffer, "<=");
    else if(relop_type == EQ)
        strcpy(buffer, "==");
    else if(relop_type == NEQ)
        strcpy(buffer, "!=");
}

void itoa(char *buffer, int num)
{
    memset(buffer, 0, sizeof(buffer));
    if(num < 0)
    {
        num = -num;
        *buffer = '-';
        buffer++;   
    }
    char *now = buffer;
    if(num == 0)
    {
        *now = '0';
        *(now + 1) = '\0';
        return;
    }
    for(; num != 0; num /= 10, now++)
        *now = ('0' + (num % 10));
    *(now--) = '\0';
    for(; now > buffer; now--, buffer++)
    {
        *now = *now ^ *buffer;
        *buffer = *now ^ *buffer;
        *now = *now ^ *buffer;
    }
}