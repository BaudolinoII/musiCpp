%{ 
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
FILE *yyin;
%} 

%option outfile="scanner.c"
%option noyywrap
  
DIGIT   [0-9]
CHARA   [a-zA-Z]
DASH	[-_]
OPS		[-*+/"%"]
BLOCK 	[()"[""]"{}]
LOGIC	["==""!=""|""&""#"!"<<"">>""<="">="]
SPLIT	[,.;:]

MUSIC	\"[^"]*\"	
ARGS	\'[^']*\'
COMMS 	["IMPORT""PACKAGE""INIT_SCRIPT""SCRIPT"]

IDENT	{CHARA}({CHARA}|{DIGIT}|{DASH})*
INTEGER (-?[1-9]{DIGIT})|0
REAL_N  {INTEGER}+"."{DIGIT}+
ASIGN	{OPS}?=

COM_L   "//"[^\n]*\n
COM_B   "/*"[^"*/"]*"*/"
WASTE   [\t\r\f" "]

/* Rule Section */
%% 
{OPS}			{ printf("Op Aritmetica Detectada %c\n", yytext);}
{LOGIC}			{ printf("Op Logica Detectada %c\n", yytext);}
{ASIGN}			{ printf("Asignacion Detectada %c\n", yytext);}
{COMMS}			{ printf("Comando Detectado %s\n", yytext);}
{IDENT}			{ printf("Identificador Detectado %s\n", yytext);}
{INTEGER}		{ printf("Entero Detectado %s\n", yytext);}
{REAL_N}		{ printf("Real Detectado %s\n", yytext);}
{MUSIC}			{ printf("Melodia Detectada %s\n", yytext);}
{ARGS}			{ printf("Argumento Detectado %s\n", yytext);}
{WASTE}			{ ; }
{COM_L}			{ ; }
{COM_B}			{ ; }
.				{ printf("Caracter invalido %s\n", yytext);}
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