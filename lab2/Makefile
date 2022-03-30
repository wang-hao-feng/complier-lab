.PHONY:parser

parser:
	@bison -d syntax.y
	@flex lexical.l
	@gcc syntax.tab.c -lfl -w -o $@