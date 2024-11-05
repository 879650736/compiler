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
%token '<' '>' LEQ GEQ
%token EQ NEQ AND OR
%token <int_val> INT_CONST


%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_val> Exp PrimaryExp UnaryExp UnaryOp
%type <ast_val> AddExp MulExp 
%type <ast_val> RelExp EqExp LAndExp LOrExp
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
  : LOrExp {
    auto ast = new ExpAST();
    ast->l_or_exp = unique_ptr<BaseAST>($1);
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

RelExp
  : AddExp {
      auto ast = new RelExpAST();
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($1));
      ast->expr = std::move(addExp);  // 存储 AddExp
      $$ = ast;
  }
  | RelExp '<' AddExp {
      auto ast = new RelExpAST();
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1));
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($3));
      ast->expr = std::make_tuple(std::move(relExp), "<", std::move(addExp));  // 存储 RelExp, 操作符, AddExp
      $$ = ast;
  }
  | RelExp '>' AddExp {
      auto ast = new RelExpAST();
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1));
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($3));
      ast->expr = std::make_tuple(std::move(relExp), ">", std::move(addExp));  // 存储 RelExp, 操作符, AddExp
      $$ = ast;
  }
  | RelExp LEQ AddExp {
      auto ast = new RelExpAST();
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1));
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($3));
      ast->expr = std::make_tuple(std::move(relExp), "<=", std::move(addExp));  // 存储 RelExp, 操作符, AddExp
      $$ = ast;
  }
  | RelExp GEQ AddExp {
      auto ast = new RelExpAST();
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1));
      auto addExp = std::unique_ptr<AddExpAST>(static_cast<AddExpAST*>($3));
      ast->expr = std::make_tuple(std::move(relExp), ">=", std::move(addExp));  // 存储 RelExp, 操作符, AddExp
      $$ = ast;
  }
  ;

EqExp
  : RelExp {
      auto ast = new EqExpAST();
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($1));
      ast->expr = std::move(relExp);  // 存储 RelExp
      $$ = ast;
  }
  | EqExp EQ RelExp {
      auto ast = new EqExpAST();
      auto eqExp = std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($1));
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($3));
      ast->expr = std::make_tuple(std::move(eqExp), "==", std::move(relExp));  // 存储 EqExp, 操作符, RelExp
      $$ = ast;
  }
  | EqExp NEQ RelExp {
      auto ast = new EqExpAST();
      auto eqExp = std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($1));
      auto relExp = std::unique_ptr<RelExpAST>(static_cast<RelExpAST*>($3));
      ast->expr = std::make_tuple(std::move(eqExp), "!=", std::move(relExp));  // 存储 EqExp, 操作符, RelExp
      $$ = ast;
  }
  ;

LAndExp
  : EqExp {
      auto ast = new LAndExpAST();
      auto eqExp = std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($1));
      ast->expr = std::move(eqExp);  // 存储 EqExp
      $$ = ast;
  }
  | LAndExp AND EqExp {
      auto ast = new LAndExpAST();
      auto landExp = std::unique_ptr<LAndExpAST>(static_cast<LAndExpAST*>($1));
      auto eqExp = std::unique_ptr<EqExpAST>(static_cast<EqExpAST*>($3));
      ast->expr = std::make_tuple(std::move(landExp), "&&", std::move(eqExp));  // 存储 LAndExp, 操作符, EqExp
      $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
      auto ast = new LOrExpAST();
      auto landExp = std::unique_ptr<LAndExpAST>(static_cast<LAndExpAST*>($1));
      ast->expr = std::move(landExp);  // 存储 LAndExp
      $$ = ast;
  }
  | LOrExp OR LAndExp {
      auto ast = new LOrExpAST();
      auto lorExp = std::unique_ptr<LOrExpAST>(static_cast<LOrExpAST*>($1));
      auto landExp = std::unique_ptr<LAndExpAST>(static_cast<LAndExpAST*>($3));
      ast->expr = std::make_tuple(std::move(lorExp), "||", std::move(landExp));  // 存储 LOrExp, 操作符, LAndExp
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
