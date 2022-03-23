.PHONY:parser

parser:
	@bison -d syntax.y
	@flex lexical.l
	@gcc function.c syntax.tab.c -lfl -w -o $@