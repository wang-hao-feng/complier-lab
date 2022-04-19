.PHONY:parser

parser:
	@bison -d syntax.y
	@flex lexical.l
	@gcc NodeStruct.h error.c TypeExp.c stack.c map.c trie.c SymbolTable.c IntermediaCode.c analyse.c syntax.tab.c -lfl -w -g -o $@

test:parser
	@for i in 1 2 3 4; do \
		echo test$$i.cmm;\
		./parser test/test$$i.cmm test/test$$i.ir;\
		echo -----------------------------;\
	done