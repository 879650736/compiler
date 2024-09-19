#include <iostream>
#include <string>


struct Number
{
    int int_const;
};

struct Stmt {
    Number number; 
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







