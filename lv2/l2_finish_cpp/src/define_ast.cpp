#include <iostream>
#include <string>
#include <memory>
#include <fstream>


// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump(std::ostream &out = std::cout) const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump(std::ostream &out = std::cout) const override {
    func_def->Dump(out);
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump(std::ostream &out = std::cout) const override {
    out << "fun ";
    out << "@" << ident;
    out << "(): ";
    func_type->Dump(out);
    out << " {" << "\n";
    block->Dump(out);
    }
};

class FuncTypeAST : public BaseAST{
public:
    void Dump(std::ostream &out = std::cout) const override {
        out <<"i32";
    }
};


class BlockAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> stmt;

    void Dump(std::ostream &out = std::cout) const override {
        out << "%" << "entry:" << "\n";
        stmt->Dump(out);
    }
};


class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> number;

    void Dump(std::ostream &out = std::cout) const override {
        out <<"\t"<< "ret ";
        number->Dump(out);
        out << "\n";
        out << "}";
    }
};

class NumberAST : public BaseAST {
public:
    int number;

    NumberAST(int val) : number(val) {}
    
    void Dump(std::ostream &out = std::cout) const override {
        out << number;
    }
};




