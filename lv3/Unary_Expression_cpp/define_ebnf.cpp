#include <iostream>
#include <string>
#include <variant>
#include <memory>

struct UnaryOp{};

struct Number
{
    int int_const;
};

struct Exp
{
    UnaryExp unary_exp;
};


struct PrimaryExp
{
    using ExpPtr = std::unique_ptr<Exp>;
    std::variant<ExpPtr, Number> expr;
};


struct UnaryExp
{   
    using UnaryExpPtr = std::unique_ptr<UnaryExp>;
    std::variant<PrimaryExp, std::pair<UnaryOp, UnaryExpPtr>> expr;
};

struct Stmt {
    Exp exp; 
};

struct Block {
    Stmt stmt; 
};

enum class FuncType {
    Int
};

struct FuncDef {
    FuncType func_type;
    std::string ident;
    Block block;
};

struct CompUnit {
    FuncDef func_def;
};







