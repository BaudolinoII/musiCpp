@echo off
bison -dy calc.y
flex calc.l 
gcc lex.yy.c y.tab.c