#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <variant>
#include <stack>
//#include "ast.h"

static int i = 0;
static std::stack<std::string> opStack; // 操作符栈
static std::stack<int> numberStack; // 数字栈



static void GenerateIRCode(std::ostream &out); // 前向声明

// 所有 AST 的基类
class BaseAST {
public:
    virtual ~BaseAST() = default;

    virtual void Dump(std::ostream &out = std::cout) const = 0;
};

class NumberAST : public BaseAST {
public:
    int number;
    void Dump(std::ostream &out = std::cout) const override {
        numberStack.push(number);
        GenerateIRCode(out);
    }
};

class UnaryOpAST : public BaseAST {
public:
    std::string op;

    void Dump(std::ostream &out = std::cout) const override {
        opStack.push(op); 
        
        
    }
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> unary_exp;

    void Dump(std::ostream &out = std::cout) const override {
        unary_exp->Dump(out);
    }
};

class PrimaryExpAST : public BaseAST {
public:
    using ExpPtr = std::unique_ptr<ExpAST>;
    using NumPtr = std::unique_ptr<NumberAST>;
    std::variant<ExpPtr, NumPtr> expr; // 使用 unique_ptr

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            arg->Dump(out);
        }, expr);
    }
};


class UnaryExpAST : public BaseAST {
public:
    using UnaryExpPtr = std::unique_ptr<UnaryExpAST>;
    std::variant<std::unique_ptr<PrimaryExpAST>, std::pair<std::unique_ptr<UnaryOpAST>, UnaryExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<PrimaryExpAST>>) {
                arg->Dump(out);  // 处理 PrimaryExpAST
            } else {
                arg.first->Dump(out);  // 处理 UnaryOpAST
                arg.second->Dump(out);  // 处理 UnaryExpPtr
            }
        }, expr);
    }
};




class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump(std::ostream &out = std::cout) const override {
        exp->Dump(out);
        out <<"\t" << "ret %" << i-1;
        out << "\n";
        out << "}";
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

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    void Dump(std::ostream &out = std::cout) const override {
    func_def->Dump(out);
    }
};

void GenerateIRCode(std::ostream &out = std::cout) {
    while (!opStack.empty()) {
        std::string op = opStack.top();
        opStack.pop();
        if (i == 0)
        {
            int number = numberStack.top();
            numberStack.pop();
            if (op == "+")
            ;
            else if (op == "-")
            {
                out <<"\t%0" << " = sub 0 , " << number << "\n";
                i++;
            }
            else if (op == "!")
            {
                out <<"\t%0" << " = eq " << number << ", 0" << "\n";
                i++;
            }
        }
        else 
        {
            if (op == "+")
            ;
            else if (op == "-")
            {
                out <<"\t%" << i << " = sub 0 , %" << i-1 << "\n";
                i++;
            }
            
            else if (op == "!")
            {
                out <<"\t%" << i << " = eq %" << i-1 << ", 0" << "\n";
                i++;
            }
        }
        
    }
}


