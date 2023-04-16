%{
	#include<iostream>
	#include<fstream>
	#include<vector>
	#include<string>
	FILE* yyin;
}%

/*Leer un solo archivo por entrada*/
%option c++
%option outfile"vitalComp.h"
%option noyywrap
%option yylineno

/*Tokens esenciales*/
DIG		[0-9]
CHARA	[a-zA-ZñÑ]|{CHRATM}
SPLIT	[\t\n\b" "]
/*Comentarios*/
COMSL	[!]
COMBB	"!*"
COMBE	"*!"
/*Tokens de Música*/
ATOMIC	[#&*\-+/;:<>,._{}()]
IDENT	{CHARA}({CHARA}|{DIG}|"_")*
DIRECT	\"[^\n]*\"		
/*"Palabras Reservadas/**/
IMPORT	^"IMPORT"{SPLIT}+{DIRECT}
TONCAT 	"Do"|"Re"|"Mi"|"Fa"|"Sol"|"La"|"Si"
TIMESP	"REDO"|"BLAN"|"NEGR"|"CORC"|"SEMC"
CHORDS	"ADD"|"MAY"|"MEN"|"AUM"|"DIS"|"SUS"
%%

%%