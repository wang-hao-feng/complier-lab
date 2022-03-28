.PHONY:parser

parser:
	@bison -d syntax.y
	@flex lexical.l
	@gcc NodeStruct.c syntax.tab.c -lfl -w -o $@