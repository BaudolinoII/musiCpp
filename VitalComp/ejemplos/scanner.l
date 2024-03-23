%option c++
%option outfile="scanner.cpp"
%option noyywrap

%{
    #include <iostream>
    #include <string>
    #include <unordered_map>
%}

DIGIT   [0-9]
CHARA   [a-zA-ZñÑ]

COM_L   "//"[^\n]*\n
COM_B   "/*"*"*/"

INTEGER {DIGIT}+
REAL_N  {DIGIT}+"."{DIGIT}+
IDENT   {CHARA}({CHARA}|{DIGIT}|"_")*
MUSIC   \"[^\"]*\"  //"
ARGS    \'[^\']*\'  //'

WASTE   [\t\r\f" "]

%%

{DIGIT1}{DIGIT}*  {cout << "Lexer: " << yytext << endl;
                    return Parser::NUMBER;
                  }

.                 {
                    return Parser::UNKNOWN;
                  }

<<EOF>>           {
                   yyterminate();
                  }
%%