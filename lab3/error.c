#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

int error_line = 0;

void PrintErrorMsg(int error_type, int line, int n, ...)
{
    semantic_error = 1;
    if(line == error_line)
        return;
    error_line = line;
    int total = 0;
    va_list list;
    va_start(list, n);
    for(int i = 0; i < n; i++)
        total += strlen(va_arg(list, char *));
    va_end(list);

    char *msg = (char *)malloc(sizeof(char) * (total + 1));
    total = 0;
    va_start(list, n);
    for(int i = 0; i < n; i++)
    {
        char *next = va_arg(list, char *);
        strcpy(msg + total, next);
        total += strlen(next);
    }
    va_end(list);
    printf("Error type %d at Line %d: %s\n", error_type, line, msg);
    free(msg);
}