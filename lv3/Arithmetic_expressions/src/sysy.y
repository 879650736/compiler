%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}


%token INT RETURN
%token <str_val> IDENT 
%token '+' '-' '!'
%token '*' '/' '%'
%token <int_val> INT_CONST


%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp
%type <ast_val> AddExp MulExp 
%type <int_val> Number

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    $$ =ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Exp
  : AddExp {
    auto ast = new ExpAST();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
      auto ast = new MulExpAST();
      auto unaryExp = std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($1));
      ast->expr = std::move(unaryExp);  // 存储 UnaryExp
      $$ = ast;
  }
  | MulExp '*' UnaryExp {
      auto ast = new MulExpAST();
      auto mulExp = std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($1));
      auto unaryExp = std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($3));
      ast->expr = std::make_tuple(std::move(mulExp), "*", std::move(unaryExp));  // 存储 MulExp, 操作符, UnaryExp
      $$ = ast;
  }
  | MulExp '/' UnaryExp {
      auto ast = new MulExpAST();
      auto mulExp = std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($1));
      auto unaryExp = std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($3));
      ast->expr = std::make_tuple(std::move(mulExp), "/", std::move(unaryExp));  // 存储 MulExp, 操作符, UnaryExp
      $$ = ast;
  }
  | MulExp '%' UnaryExp {
      auto ast = new MulExpAST();
      auto mulExp = std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($1));
      auto unaryExp = std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST*>($3));
      ast->expr = std::make_tuple(std::move(mulExp), "%", std::move(unaryExp));  // 存储 MulExp, 操作符, UnaryExp
      $$ = ast;
  }
  ;


PrimaryExp
  : '(' Exp ')' {
      auto ast = new PrimaryExpAST();
      auto expPtr = dynamic_cast<ExpAST*>(static_cast<BaseAST*>($2)); 
      ast->expr = std::make_unique<ExpAST>(std::move(*expPtr)); 
      $$ = ast;
  }
  | Number {
      auto ast = new PrimaryExpAST();
      auto numberAst = new NumberAST();
      numberAst->number = $1;  // 假设 $1 是一个 int
      ast->expr = std::make_unique<NumberAST>(*numberAst);  // 将 NumberAST 移入 variant
      $$ = ast;
  }
  ;

AddExp
  : MulExp {
      auto ast = new AddExpAST();
      auto mulExp = std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($1));
      ast->expr = std::move(mulExp);  // 存储 MulExp
      $$ = ast;
  }
  | AddExp '+' MulExp {
      auto ast = new AddExpAST();
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($1));
      auto mulExp = std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($3));
      ast->expr = std::make_tuple(std::move(addExp), "+", std::move(mulExp));  // 存储 AddExp, 操作符, MulExp
      $$ = ast;
  }
  | AddExp '-' MulExp {
      auto ast = new AddExpAST();
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($1));
      auto mulExp = std::unique_ptr<MulExpAST>(static_cast<MulExpAST*>($3));
      ast->expr = std::make_tuple(std::move(addExp), "-", std::move(mulExp));  // 存储 AddExp, 操作符, MulExp
      $$ = ast;
  }
  ;


UnaryExp
  : PrimaryExp {
      auto ast = new UnaryExpAST();
      auto primaryExp = dynamic_cast<PrimaryExpAST*>(static_cast<BaseAST*>($1)); 
      ast->expr = std::make_unique<PrimaryExpAST>(std::move(*primaryExp));
      $$ = ast;
  }
  | UnaryOp UnaryExp {
      auto ast = new UnaryExpAST();
      auto unaryOp = std::unique_ptr<UnaryOpAST>(static_cast<UnaryOpAST *>($1));  // 从 ast_val 中提取 UnaryOp
      // 假设 unaryOp 中的 op 是 string 类型
      auto unaryExp = std::unique_ptr<UnaryExpAST>(static_cast<UnaryExpAST *>($2));  // 确保类型匹配
      ast->expr = std::make_pair(std::move(unaryOp), std::move(unaryExp));  // 存储 UnaryOp 和 UnaryExp
      $$ = ast;
  }
  ;





UnaryOp
  : '+' {
      auto ast = new UnaryOpAST();
      ast->op = "+";  
      $$ = ast;
  }
  | '-' {
      auto ast = new UnaryOpAST();
      ast->op = "-";  
      $$ = ast;
  }
  | '!' {
      auto ast = new UnaryOpAST();
      ast->op = "!";  
      $$ = ast;
  }
  ;


Number
  : INT_CONST {
    $$ = $1;  // Assuming $1 is an int
  }
  ;
%%

void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  std::cerr << "Error: " << s << std::endl;
}
