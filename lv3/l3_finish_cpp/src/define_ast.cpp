#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <variant>
#include <stack>
#include <vector>
#include <queue>

//#include "ast.h"

static int reg_count_koopa = 0;
static int op_id = 0;
static int brackets_number = 0;
static int unary_number = 0;
static int unary_id = 0;
static int once_number =1;

static bool unaryop = false;

//static std::stack<std::string> UnaryopStack; // 操作符栈
//static std::stack<int> numberStack; // 数字栈
//static std::queue<int> intQueue;    //数字队列

// 定义一个可以存储 string 和 int 的 variant 类型
    using StringOrInt = std::variant<std::string, int>;

    // 创建一个 vector 来存储 StringOrInt 类型的元素
    static std::vector<StringOrInt> vec;
    static std::vector<StringOrInt> unary_vec;

static void GenerateIRCode(std::ostream &out);
static void PrintVector(const std::vector<StringOrInt>& vec);
static void PrintUnaryVector(const std::vector<StringOrInt>& unary_vec);
static void deal_arithmetic_expressions(std::ostream &out,std::vector<StringOrInt>& vec);
//static void build_child_tree();

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
        std::cout << "number:" << number << std::endl;
        once_number = number;
        if(unaryop)
        unary_vec.emplace_back(number);
        //std::cout << op_id << "NumberAST\n";
        //intQueue.push(number);
        if(!unaryop)
        vec.emplace_back(number);
        else{
        std::string unarynumber = "%" + std::to_string(unary_number);
        unary_number++;
        std::cout << "unarynumber:" << unarynumber << std::endl;
        vec.emplace_back(unarynumber);
        }
        unaryop = false;

    }
};

class UnaryOpAST : public BaseAST {
public:
    std::string op;

    void Dump(std::ostream &out = std::cout) const override {
        unary_vec.emplace_back(op);
        unaryop = true;
        
        
    }
};

class ExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> l_or_exp;

    void Dump(std::ostream &out = std::cout) const override {
        l_or_exp->Dump(out);
    }
};

class PrimaryExpAST : public BaseAST {
public:
    using ExpPtr = std::unique_ptr<ExpAST>;
    using NumPtr = std::unique_ptr<NumberAST>;
    std::variant<ExpPtr, NumPtr> expr; // 使用 unique_ptr

    void Dump(std::ostream &out) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<ExpAST>>) {
                brackets_number = brackets_number + 2;
                vec.emplace_back("(");
                arg->Dump(out);  // 处理 ExpAST
                vec.emplace_back(")");
            } else {
                arg->Dump(out);  // 处理 NumberAST
            }
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

class MulExpAST : public BaseAST {
public:
    using MulExpPtr = std::unique_ptr<MulExpAST>;
    using UnaryExpPtr = std::unique_ptr<UnaryExpAST>;

    // 表示 MulExp 的两种情况：UnaryExp 或 MulExp 和运算符
    std::variant<UnaryExpPtr, std::tuple<MulExpPtr, std::string, UnaryExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, UnaryExpPtr>) {
                arg->Dump(out);  // 处理 UnaryExpAST
            } else {
                auto &[left, op, right] = arg;
                left->Dump(out);  // 处理左边的 MulExp
                //root->AddChild(std::string(op));
                vec.emplace_back(op);
                op_id++;
                //std::cout << op_id << "MulExpAST\n";
                right->Dump(out);  // 处理右边的 UnaryExp
            }
        }, expr);
    }
};

class AddExpAST : public BaseAST {
public:
    using AddExpPtr = std::unique_ptr<AddExpAST>;
    using MulExpPtr = std::unique_ptr<MulExpAST>;

    // 表示 AddExp 的两种情况：MulExp 或 AddExp 和运算符
    std::variant<MulExpPtr, std::tuple<AddExpPtr, std::string, MulExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, MulExpPtr>) {
                arg->Dump(out);  // 处理 MulExpAST
            } else {
                auto &[left, op, right] = arg;
                left->Dump(out);  // 处理左边的 AddExp
                //root->AddChild(std::string(op));  // 输出操作符
                vec.emplace_back(op);
                op_id++;
                //std::cout << op_id << "AddExpAST\n";
                right->Dump(out);  // 处理右边的 MulExp
            }
        }, expr);
    }
};

class RelExpAST : public BaseAST {
public:
    using RelExpPtr = std::unique_ptr<RelExpAST>;
    using AddExpPtr = std::unique_ptr<AddExpAST>;

    // 表示 RelExp 的两种情况：AddExp 或 RelExp 和运算符
    std::variant<AddExpPtr, std::tuple<RelExpPtr, std::string, AddExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, AddExpPtr>) {
                arg->Dump(out);  // 处理 AddExpAST
            } else {
                auto &[left, op, right] = arg;
                left->Dump(out);  // 处理左边的 RelExp
                //root->AddChild(std::string(op));  // 输出操作符
                vec.emplace_back(op);
                op_id++;
                //std::cout << op_id << "RelExpAST\n";
                right->Dump(out);  // 处理右边的 AddExp
            }
        }, expr);
    }
};

class EqExpAST : public BaseAST {
public:
    using EqExpPtr = std::unique_ptr<EqExpAST>;
    using RelExpPtr = std::unique_ptr<RelExpAST>;

    // 表示 EqExp 的两种情况：RelExp 或 EqExp 和运算符
    std::variant<RelExpPtr, std::tuple<EqExpPtr, std::string, RelExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, RelExpPtr>) {
                arg->Dump(out);  // 处理 RelExpAST
            } else {
                auto &[left, op, right] = arg;
                left->Dump(out);  // 处理左边的 EqExp
                //root->AddChild(std::string(op));  // 输出操作符
                vec.emplace_back(op);
                op_id++;
                //std::cout << op_id << "EqExpAST\n";
                right->Dump(out);  // 处理右边的 RelExp
            }
        }, expr);
    }
};

class LAndExpAST : public BaseAST {
public:
    using LAndExpPtr = std::unique_ptr<LAndExpAST>;
    using EqExpPtr = std::unique_ptr<EqExpAST>;

    // 表示 LAndExp 的两种情况：EqExp 或 LAndExp 和运算符
    std::variant<EqExpPtr, std::tuple<LAndExpPtr, std::string, EqExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, EqExpPtr>) {
                arg->Dump(out);  // 处理 EqExpAST
            } else {
                auto &[left, op, right] = arg;
                left->Dump(out);  // 处理左边的 LAndExp
                //root->AddChild(std::string(op));  // 输出操作符
                vec.emplace_back(op);
                op_id++;
                //std::cout << op_id << "LAndExpAST\n";
                right->Dump(out);  // 处理右边的 EqExp
            }
        }, expr);
    }
};

class LOrExpAST : public BaseAST {
public:
    using LOrExpPtr = std::unique_ptr<LOrExpAST>;
    using LAndExpPtr = std::unique_ptr<LAndExpAST>;

    // 表示 LOrExp 的两种情况：LAndExp 或 LOrExp 和运算符
    std::variant<LAndExpPtr, std::tuple<LOrExpPtr, std::string, LAndExpPtr>> expr;

    void Dump(std::ostream &out = std::cout) const override {
        std::visit([&out](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, LAndExpPtr>) {
                arg->Dump(out);  // 处理 LAndExpAST
            } else {
                auto &[left, op, right] = arg;
                left->Dump(out);  // 处理左边的 LOrExp
                //root->AddChild(std::string(op));  // 输出操作符
                vec.emplace_back(op);
                op_id++;
                //std::cout << op_id << "LOrExpAST\n";
                right->Dump(out);  // 处理右边的 LAndExp
            }
        }, expr);
    }
};


class StmtAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> exp;

    void Dump(std::ostream &out = std::cout) const override {
        exp->Dump(out);
        
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
    //root = std::make_unique<CustomTreeNode>(std::string("root"));
    func_def->Dump(out);
    //build_child_tree();
    //root->PrintTree();
    PrintVector(vec);std::cout << "\n";

    GenerateIRCode(out);
    PrintVector(vec);std::cout << "\n";
    if(reg_count_koopa >= 1){
        out <<"\t" << "ret %" << reg_count_koopa-1;
        out << "\n";
        out << "}";
    }
    else{
        out <<"\t" << "ret " << once_number;
        out << "\n";
        out << "}";
    }
    
    }
};

void GenerateIRCode(std::ostream &out) {
    int number;
    std::string op;
    bool flag_once = true;
    const std::string add = "+";
    const std::string sub = "-";
    const std::string no = "!";
    PrintUnaryVector(unary_vec);
    while (!unary_vec.empty()) {
        for (size_t j = 0; j < unary_vec.size(); ++j) {
        // 找到 number
        if (std::holds_alternative<int>(unary_vec[j])) {
            number = std::get<int>(unary_vec[j]);
            //std::cout << "number:" << number << std::endl;
            bool foundPreviousNumber = false;
            flag_once = true;
            for (int k = j - 1; k >= 0; --k)
            {
                if (std::holds_alternative<std::string>(unary_vec[k])) {
                    foundPreviousNumber = true;
                    op = std::get<std::string>(unary_vec[k]);
                    if (flag_once)
                    {

                        if (op == add){
                            //std::cout << "op==add" <<std::endl;
                            once_number = number; 
                        }
                        
                        else if (op == sub)
                        {   
                            //std::cout << "op==sub" <<std::endl;
                            out <<"\t%" << unary_id << " = sub 0 , " << number << "\n";
                            reg_count_koopa++;
                            unary_id++;
                        }
                        else if (op == no)
                        {   
                            //std::cout << "op==!" <<std::endl;
                            out <<"\t%" << unary_id << " = eq " << number << ", 0" << "\n";
                            reg_count_koopa++;
                            unary_id++;
                        }
                        else{
                            std::cerr << "Error: No found operate." << std::endl;
                        }
                    }
                    else 
                    {
                        if (op == add){
                            //std::cout << "op==add" <<std::endl;
                            once_number = number;   
                        }
                        else if (op == sub)
                        {
                            //std::cout << "op==sub" <<std::endl;   
                            out <<"\t%" << reg_count_koopa << " = sub 0 , %" << unary_id-1 << "\n";
                            reg_count_koopa++;
                            unary_id++;
                        }
                        else if (op == no)
                        {
                            //std::cout << "op==!" <<std::endl;   
                            out <<"\t%" << reg_count_koopa << " = eq %" << unary_id-1 << ", 0" << "\n";
                            reg_count_koopa++;
                            unary_id++;
                        }
                        else{
                            std::cerr << "Error: No found operate." << std::endl;
                        }
                    }
                    flag_once = false;
                
                }
                else if (std::holds_alternative<int>(unary_vec[k])) {
                    break; 
                }
                if (foundPreviousNumber) {
                //std::cerr << "Error: Two consecutive numbers found." << std::endl;
                }
            }
            // 移除从 0 到找到的第一个数字的所有元素
            unary_vec.erase(unary_vec.begin(), unary_vec.begin() + j + 1);
            j = 0; // 重置索引以重新检查 unary_vec
            PrintUnaryVector(unary_vec);
        }
    }
    }

    deal_arithmetic_expressions(out,vec);
}

    

void deal_arithmetic_expressions(std::ostream &out,std::vector<StringOrInt>& vec){
    int l_value;
    int r_value;
    std::string op;
    std::string reg;
    int max_op = (vec.size() - brackets_number) / 2;
    //std::cout << "vec.size()" << "\t" << vec.size() << "\n";
    std::cout << "op_id:" << op_id << "\t" << "max_op:" <<max_op << "\n";
    for (size_t i = 0; i < max_op;)
{
    bool found_operator = false;
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 ()
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "(") {
            std::vector<StringOrInt> newVec;
            size_t openParenIndex = -1; // 用于记录左括号的索引

        // 找到左括号的索引
        for (size_t j = 0; j < vec.size(); ++j) {
            if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "(") {
                openParenIndex = j;
                break;
            }
        }
        // 如果找到了左括号
        if (openParenIndex != -1) {
            // 从左括号开始遍历，直到找到右括号
            for (size_t j = openParenIndex + 1; j < vec.size(); ++j) {
                if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == ")") {
                    // 找到右括号，结束循环
                    break;
                }
                // 添加操作数到新向量
            newVec.push_back(vec[j]);
            }
        }   
            brackets_number = brackets_number - 2;
            std::cout << "newvec:\n";
            PrintVector(newVec);
            // 删除原 vector 中的 () 和其包含的数
            vec.erase(vec.begin() + openParenIndex, vec.begin() + openParenIndex + newVec.size() + 2);
            deal_arithmetic_expressions(out,newVec);
            std::string unarynumber = "%" + std::to_string(reg_count_koopa - 1);
            std::cout << "unarynumber:" << unarynumber << std::endl;
            vec.insert(vec.begin() + j, unarynumber);
            PrintVector(vec);
            int op_number = (newVec.size() + 2) / 3 ;
            std::cout << "op_number:" << op_number << "\n";
            i = i + op_number;
            //PrintVector(vec);std::cout << "\n";
            //found_operator = true;
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 /
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "/") {
            op = "/";
            out << "\t%" << reg_count_koopa << " = div ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 *
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "*") {
            op = "*";
            out << "\t%" << reg_count_koopa << " = mul ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }

            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 %
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "%") {
            op = "%";
            out << "\t%" << reg_count_koopa << " = mod ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 +
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "+") {
            op = "+";
            out << "\t%" << reg_count_koopa << " = add ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 -
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "-") {
            op = "-";
            out << "\t%" << reg_count_koopa << " = sub ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 >
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == ">") {
            op = ">";
            out << "\t%" << reg_count_koopa << " = gt ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 >=
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == ">=") {
            op = ">=";
            out << "\t%" << reg_count_koopa << " = ge ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 <=
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "<=") {
            op = "<=";
            out << "\t%" << reg_count_koopa << " = le ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 <
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "<") {
            op = "<";
            out << "\t%" << reg_count_koopa << " = lt ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 ==
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "==") {
            op = "==";
            out << "\t%" << reg_count_koopa << " = eq ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 !=
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "!=") {
            op = "!=";
            out << "\t%" << reg_count_koopa << " = ne ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 ||
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "||") {
            op = "||";
            out << "\t%" << reg_count_koopa << " = le ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            reg_count_koopa++;

            out << "\t%" << reg_count_koopa << " = ne ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << "\n";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << "\n";
            }
            reg_count_koopa++;

            out << "\t%" << reg_count_koopa << " = or %";
            out << reg_count_koopa-2 << ", %" << reg_count_koopa-1 << "\n";

            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    for (size_t j = 0; j < vec.size(); ++j) {
        // 找到 &&
        if (std::holds_alternative<std::string>(vec[j]) && std::get<std::string>(vec[j]) == "&&") {
            op = "&&";
            out << "\t%" << reg_count_koopa << " = ne ";
            if (j > 0 && std::holds_alternative<int>(vec[j - 1])) {
                l_value = std::get<int>(vec[j - 1]);
                out << l_value << " , ";
            }else if (j > 0 && std::holds_alternative<std::string>(vec[j - 1])) {
                // 处理左值为字符串的情况
                std::string left_reg = std::get<std::string>(vec[j - 1]);
                out << left_reg << " , ";
            }
            out << "0\n";
            reg_count_koopa++;

            out << "\t%" << reg_count_koopa << " = ne ";
            if (j + 1 < vec.size() && std::holds_alternative<int>(vec[j + 1])) {
                r_value = std::get<int>(vec[j + 1]);
                out << r_value << " , ";
            }else if (j + 1 < vec.size() && std::holds_alternative<std::string>(vec[j + 1])) {
                // 处理右值为字符串的情况
                std::string right_reg = std::get<std::string>(vec[j + 1]);
                out << right_reg << " , ";
            }
            out << "0\n";
            reg_count_koopa++;

            out << "\t%" << reg_count_koopa << " = and %";
            out << reg_count_koopa-2 << ", %" << reg_count_koopa-1 << "\n";

            vec.erase(vec.begin() + j - 1, vec.begin() + j + 2);
            reg = "%"  + std::to_string(reg_count_koopa);
            reg_count_koopa++;
            vec.insert(vec.begin() + j - 1, reg);
            i++;
            found_operator = true; 
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
   // 如果在这一轮中没有找到任何有效的操作符，打印错误信息并退出循环
    if (!found_operator) {
        //std::cerr << "Error: No valid operator (*, +, or <=) found in the values." << std::endl;
        break;  // 跳出循环，避免程序继续执行
    }
    }
    std::cout << "finish vec\n";
}


void PrintVector(const std::vector<StringOrInt>& vec) {
    std::cout << "vec内容: \n";
    for (const auto& element : vec) {
        std::visit([](const auto& value) {
            std::cout << value << std::endl;
        }, element);
    }
}
void PrintUnaryVector(const std::vector<StringOrInt>& unary_vec) {
    std::cout << "unary_vec内容: \n";
    for (const auto& element : unary_vec) {
        std::visit([](const auto& value) {
            std::cout << value << std::endl;
        }, element);
    }
}
