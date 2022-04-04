.PHONY:parser

parser:
	@bison -d syntax.y
	@flex lexical.l
	@gcc NodeStruct.h error.c TypeExp.c stack.c map.c trie.c SymbolTable.c analyse.c syntax.tab.c -lfl -w -g -o $@


test:parser
	@for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23; do \
		echo test$$i.cmm;\
		./parser test/test$$i.cmm;\
		echo -----------------------------;\
	done