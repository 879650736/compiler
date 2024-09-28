%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"

using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*

/* 块注释的处理 */
BlockComment   "/*"(([^\*]*(\*[^\/])?)*)"*/"


/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%%

{WhiteSpace}    ;
{LineComment}   ;
{BlockComment}  ;

"int"           { return INT; }
"return"        { return RETURN; }
"+"             { return '+'; }
"-"             { return '-'; }
"!"             { return '!'; }
"<"             { return '<'; }
">"             { return '>'; }
"<="             { return LEQ; }
">="             { return GEQ; }
"=="             { return EQ; }
"!="             { return NEQ; }
"&&"             { return AND; }
"||"             { return OR; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }

.               { return yytext[0]; }

%%