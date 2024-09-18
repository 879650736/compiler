#include <iostream>
#include <string>
#include <memory>


// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump() const override {
    func_def->Dump();
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump() const override {
    std::cout << "fun ";
    std::cout << "@" << ident;
    std::cout << "(): ";
    func_type->Dump();
    std::cout << " {" << "\n";
    block->Dump();
    }
};

class FuncTypeAST : public BaseAST{
    

    void Dump() const override {
        std::cout <<"i32";
    }
};


class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override {
        std::cout << "%" << "entry:" << "\n";
        stmt->Dump();
    }
};


class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> number;

    void Dump() const override {
        std::cout <<"\t"<< "ret ";
        number->Dump();
        std::cout << "\n";
        std::cout << "}";
    }
};

class NumberAST : public BaseAST {
public:
    int number;

    NumberAST(int val) : number(val) {}
    
    void Dump() const override {
        std::cout << number;
    }
};




