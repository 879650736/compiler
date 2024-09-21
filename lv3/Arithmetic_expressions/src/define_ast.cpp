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

static std::stack<std::string> opStack; // 操作符栈
static std::stack<int> numberStack; // 数字栈
//static std::queue<int> intQueue;    //数字队列

// 定义一个可以存储 string 和 int 的 variant 类型
    using StringOrInt = std::variant<std::string, int>;

    // 创建一个 vector 来存储 StringOrInt 类型的元素
    static std::vector<StringOrInt> vec;

// 定义可能的子节点值类型
//using NodeValue = std::variant<int, std::string>;
/*
// 树节点的结构
struct CustomTreeNode {
   NodeValue value;  // 节点值
    std::vector<std::unique_ptr<CustomTreeNode>> children;  // 子节点列表

    // 构造函数
    CustomTreeNode(int val) : value(val) {}
    CustomTreeNode(const std::string& val) : value(val) {}

    // 添加子节点的方法
    void AddChild(int childValue) {
        children.push_back(std::make_unique<CustomTreeNode>(childValue));
    }

    void AddChild(const std::string& childValue) {
        children.push_back(std::make_unique<CustomTreeNode>(childValue));
    }

    // 输出节点值和其子节点
    void PrintTree(int depth = 0) const {
        // 打印当前节点和层级深度
        for (int k = 0; k < depth; ++k) {
            std::cout << "--";  // 用于表示层级
        }
        // 打印节点值
        std::visit([](const auto& val) { std::cout << val; }, value);
        std::cout << std::endl;
        
        // 打印子节点
        for (const auto& child : children) {
            child->PrintTree(depth + 1);
        }
    }

    // 广度优先遍历，返回所有节点的值
    std::vector<NodeValue> BreadthFirstTraversal() const {
        std::vector<NodeValue> values;  // 用于存储所有节点的值
        std::queue<const CustomTreeNode*> nodeQueue;  // 队列用于BFS遍历

        nodeQueue.push(this);  // 将根节点加入队列

        while (!nodeQueue.empty()) {
            const CustomTreeNode* currentNode = nodeQueue.front();
            nodeQueue.pop();

            // 将当前节点的值存入结果中
            values.push_back(currentNode->value);

            // 将所有子节点加入队列
            for (const auto& child : currentNode->children) {
                nodeQueue.push(child.get());
            }
        }

        return values;
    }
};
*/
//static std::unique_ptr<CustomTreeNode> root;


static void GenerateIRCode(std::ostream &out); // 前向声明
static void PrintVector(const std::vector<StringOrInt>& vec);
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
        numberStack.push(number);
        //std::cout << op_id << "NumberAST\n";
        //intQueue.push(number);
        vec.emplace_back(number);

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
    std::unique_ptr<BaseAST> add_exp;

    void Dump(std::ostream &out = std::cout) const override {
        add_exp->Dump(out);
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
    //PrintVector(vec);std::cout << "\n";
    GenerateIRCode(out);
    //PrintVector(vec);std::cout << "\n";
    out <<"\t" << "ret %" << reg_count_koopa-1;
    out << "\n";
    out << "}";
    }
};

void GenerateIRCode(std::ostream &out = std::cout) {
    while (!opStack.empty()) {

        std::string op = opStack.top();
        opStack.pop();
        if (reg_count_koopa == 0)
        {
            int number = numberStack.top();
            numberStack.pop();
            if (op == "+")
            ;
            else if (op == "-")
            {
                out <<"\t%0" << " = sub 0 , " << number << "\n";
                reg_count_koopa++;
            }
            else if (op == "!")
            {
                out <<"\t%0" << " = eq " << number << ", 0" << "\n";
                reg_count_koopa++;
            }
        }
        else 
        {
            if (op == "+")
            ;
            else if (op == "-")
            {
                out <<"\t%" << reg_count_koopa << " = sub 0 , %" << reg_count_koopa-1 << "\n";
                reg_count_koopa++;
            }
            
            else if (op == "!")
            {
                out <<"\t%" << reg_count_koopa << " = eq %" << reg_count_koopa-1 << ", 0" << "\n";
                reg_count_koopa++;
            }
        }
    }


    int l_value;
    int r_value;
    std::string op;
    std::string reg;
    int max_op = vec.size() / 2;
    //std::cout << "vec.size()" << "\t" << vec.size() << "\n";
    //std::cout << op_id << "\t" <<max_op << "\n";


for (size_t i = 0; i < max_op;)
{
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
            //PrintVector(vec);std::cout << "\n";
            break;
        }
    }
    //throw std::runtime_error("Error: No valid operator (* or +) found in the values.");
}



    }


/*

void build_child_tree()
{

    // 临时队列来处理出队的数字
    std::queue<int> tempQueue = intQueue;

     // 首先处理队头元素
    if (!tempQueue.empty()) {
        int frontNumber = tempQueue.front();
        tempQueue.pop();
        root->children[0]->AddChild(frontNumber); 
    }

    for (size_t k = 0; k < op_id; k++)
    {
        if (k == op_id - 1 && !tempQueue.empty()) {
            int backNumber = tempQueue.front();
            tempQueue.pop();
            root->children[k]->AddChild(backNumber);  // 将队尾数字添加为最后一个操作符节点的子节点
        }else if (!tempQueue.empty())
        {
            int number = tempQueue.front();
            tempQueue.pop();
            root->children[k]->AddChild(number);
            root->children[k+1]->AddChild(number);
        }
        
    }
}
*/

void PrintVector(const std::vector<StringOrInt>& vec) {
    for (const auto& element : vec) {
        std::visit([](const auto& value) {
            std::cout << value << std::endl;
        }, element);
    }
}