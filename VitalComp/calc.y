%{ 
    #include <stdio.h> 
    int yylex(void);
    void yyerror(char *s);
    FILE *yyin;
%} 
  
%token NUMBER EVALUAR
  
%left '+' '-'
%left '*' '/'
%left NEG

/* Rule Section */
%% 
  
Init    : Lista { return 0;}
;

Lista   : Lista EVALUAR '(' Expr ')' ';' { printf("Resultado = %d\n", $4);}
        | EVALUAR '(' Expr ')' ';' { printf("Resultado = %d\n", $3); }
;
Expr    : Expr '+' Expr { $$ = $1 + $3;} 
        | Expr '-' Expr { $$ = $1 - $3;}
        | Expr '*' Expr { $$ = $1 * $3;} 
        | Expr '/' Expr { $$ = $1 / $3;}
        | '-' Expr %prec NEG { $$ = -$2;} 
        |'(' Expr ')' { $$ = $2;} 
        | NUMBER { $$ = $1;} 
; 
  
%% 
  
int main(int argc, char** argv){
    if((yyin = fopen(argv[1], "rt")) == NULL){
        printf("No se pudo abrir el archivo %s\n", argv[1]);
        return -1;
    }
    printf("El archivo %s se abrio correctamente\n", argv[1]);
    yyparse();
    yylex();
    printf("Analisis concluido\n");
    fclose(yyin);
    return 0;
}

void yyerror(char *s) { 
   printf("%s\n", s); 
}

int yywrap(){
    return 1;
}