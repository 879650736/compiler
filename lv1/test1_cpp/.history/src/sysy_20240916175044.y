%code requires {
  #include <memory>
  #include <string>
}

%{

#include <iostream>
#include <memory>
#include <string>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<std::string> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<std::string> &ast }


%union {
  std::string *str_val;
  int int_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <str_val> FuncDef FuncType Block Stmt Number

%%


CompUnit
  : FuncDef {
    ast = unique_ptr<string>($1);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto type = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($2);
    auto block = unique_ptr<string>($5);
    $$ = new string(*type + " " + *ident + "() " + *block);
  }
  ;


FuncType
  : INT {
    $$ = new string("int");
  }
  ;

Block
  : '{' Stmt '}' {
    auto stmt = unique_ptr<string>($2);
    $$ = new string("{ " + *stmt + " }");
  }
  ;

Stmt
  : RETURN Number ';' {
    auto number = unique_ptr<string>($2);
    $$ = new string("return " + *number + ";");
  }
  ;

Number
  : INT_CONST {
    $$ = new string(to_string($1));
  }
  ;

%%


void yyerror(unique_ptr<string> &ast, const char *s) {
  cerr << "error: " << s << endl;
}