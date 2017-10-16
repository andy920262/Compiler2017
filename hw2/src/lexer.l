%option noyywrap
%{
#include <stdio.h>
#include "header.h"
int linenumber;
symtab * lookup();
symtab * ptr;
void     insertID();
void     printSymTab();
/* You need to define for all tokens in C--, here are some examples */
#define INT         10
#define FLOAT       11
#define OP_ASSIGN   12
#define OP_OR       13
#define MK_LPAREN   14
#define MK_RPAREN   15
#define ERROR       100

%}

letter   [A-Za-z]
digit    [0-9]
ID   {letter}({letter}|{digit}|"_")*
WS   [ \t\v\n\f]+
Int_constant {digit}+
exponent [eE][+-]?{digit}+

/* You need to define the following RE's */
Float_constant ({digit}*"."{digit}+{exponent}?)|({digit}+"."{digit}*{exponent}?)|({digit}+{exponent})
String_constant \"[^\n\"]*\"
comment "/*"((\*+[^/*])|([^*]))*\**"*/"


/* Other operators appear here */
OP_assign       "="
OP_or           "||"
OP_and          "&&"
OP_not          "!"
OP_plus         "+"
OP_minus        "-"
OP_mul          "*"
OP_div          "/"
OP_lt           "<"
OP_gt           ">"
OP_le           "<="
OP_ge           ">="
OP_equal        "=="
OP_noteq        "!="


/* Other separators appear here */
DL_lparen  "("
DL_rparen  ")"
DL_lbrckt  "["
DL_rbrckt  "]"
DL_lbrace  "{"
DL_rbrace  "}"
DL_comma   ","
DL_semicol ";"
DL_dot     "."


newline  "\n"
error    .

%%

{WS}            {}
"return"        {}
"typedef"       {}
"if"            {}
"else"          {}
"int"           {/* return INT; */ }
"float"         {/* return FLOAT; */ }
"for"           {}
"void"          {}
"while"         {}
{Int_constant}  {}
{Float_constant}    {}
{String_constant} {}
{comment}       {puts(yytext);}
{ID}            {
            ptr = lookup(yytext);
            if (ptr == NULL)
                insertID(yytext);
            else
                ptr->counter++;
        }

{OP_assign}     {/* return OP_ASSIGN; */}
{OP_or}         {/* return OP_OR; */}
{OP_and}        {}
{OP_not}        {}
{OP_plus}       {}
{OP_minus}      {}
{OP_mul}        {}
{OP_div}        {}
{OP_lt}         {}
{OP_gt}         {}
{OP_le}         {}
{OP_ge}         {}
{OP_equal}      {}
{OP_noteq}      {}
{DL_lparen}     {/* return MK_LPAREN; */}
{DL_rparen}     {/* return MK_RPAREN; */}
{DL_lbrckt}     {}
{DL_rbrckt}     {}
{DL_lbrace}     {/* return MK_LBRACE; */}
{DL_rbrace}     {/* return MK_RBRACE; */}
{DL_comma}      {}
{DL_semicol}    {}
{DL_dot}        {}
{newline}       {linenumber += 1;}
{error}         {
            printf("ERR %s\n", yytext);
            return ERROR;
            /*puts(yytext);*/
        }


%%

int main(int argc, char **argv)
{
 argc--; ++argv;
   if (argc > 0)
      yyin = fopen(argv[0], "r");
   else
      yyin = stdin;
   yylex();
   //printSymTab();
   printSymCount();
}

