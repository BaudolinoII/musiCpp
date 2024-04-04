%{
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
FILE *yyin;
}%

%option c
%option outfile="scanner.c"
%option noyywrap
%option yylinend

DIGIT   [0-9]
CHARA   [a-zA-ZñÑ]
DASH	[-_]
WASTE   [\t\r\f" "]

IDENT	{CHARA}({CHARA}|{DIGIT}|{DASH})*
INTEGER (-?[1-9]{DIGIT})|0
REAL_N  {INTEGER}+"."{DIGIT}+

MUSIC	\"[^\n"]*\"	
ARGS	\'[^\n']*\'	/*"*/
%%

%%

int main(int argc, char** argv){
	if((yyin = fopen(argv[1], "rt")) == NULL){
		printf("No se pudo abrir el archivo %s\n", argv[1]);
		return -1;
	}
	printf("El archivo %s se abrió correctamente\n", argv[1]);
	yylex();
	printf("Analisis lexico concluido\n");
	fclose(yyin);
	return 0;
}