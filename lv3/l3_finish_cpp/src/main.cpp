#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <algorithm> 


#include "koopa.h"
#include "ast.h"

using namespace std;
static int reg_count_rv = 0;
static int increment_reg_count_rv = 0;
static bool l_flag_int = false;
static bool r_flag_int = false;
static int l_binary_reg = 0;
static int r_binary_reg = 0;
static bool l_binary_flag = false;
static bool r_binary_flag = false;

struct BufferInfo {
    const void** buffer;  // 存储 buffer 数组
    int reg_count_rv;     // 存储 reg_count_rv
    uint32_t len;         // 存储 buffer 的长度
};

static std::vector<BufferInfo> l_find_reg_info;
static std::vector<BufferInfo> r_find_reg_info;

static int incrementRegCount(int reg_count_rv);
static void print_slice(const koopa_raw_slice_t &slice);
static void insertToGlobal(const void** buffer, uint32_t len, int reg_count_rv,std::vector<BufferInfo>& find_reg_pair);
static void printGlobalContents(std::vector<BufferInfo>& find_reg_pair);
static int findRegCountRv(const void* target, const std::vector<BufferInfo>& find_reg_info);

// 定义日志启用标志
#define ENABLE_LOGGING 1 // 设置为 1 启用日志，设置为 0 禁用日志

// 定义日志宏
#if ENABLE_LOGGING
    #define LOG(msg) std::cout << msg << std::endl
#else
    #define LOG(msg) // 如果禁用日志，什么都不做
#endif

// 访问 raw program
void Visit(const koopa_raw_program_t &program,std::ostream &out);

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice,std::ostream &out);

// 访问函数
void Visit(const koopa_raw_function_t &func,std::ostream &out);

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb,std::ostream &out);

// 访问指令
void Visit(const koopa_raw_value_t &value,std::ostream &out);

// 处理 return 指令
void Visit(const koopa_raw_return_t &ret,std::ostream &out);

// 处理 integer 指令
void Visit(const koopa_raw_integer_t &integer,std::ostream &out);

//处理 binary 指令
void Visit(const koopa_raw_binary_t &binary,std::ostream &out);

// 访问 raw program
void Visit(const koopa_raw_program_t &program,std::ostream &out = std::cout) {
    out << "\t.text\n";
    // 访问所有全局变量
    Visit(program.values,out);
    // 访问所有函数
    Visit(program.funcs,out);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice,std::ostream &out = std::cout) {
    for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
        case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr),out);
        break;
        case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr),out);
        break;
        case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr),out);
        break;
        default:
        
        assert(false);
    }
    }
}

// 访问函数
void Visit(const koopa_raw_function_t &func,std::ostream &out) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
    std::string func_name(func->name);
    func_name.erase(std::remove(func_name.begin(), func_name.end(), '@'), func_name.end());
    out << "\t.globl " << func_name << "\n";
    out << func_name << ":\n";
    Visit(func->bbs,out);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb,std::ostream &out) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
    Visit(bb->insts,out);
}

// 访问指令
void Visit(const koopa_raw_value_t &value,std::ostream &out) {
  // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    cout <<"kind.tag:" << kind.tag << "\n";//2个KOOPA_RVT_BINARY，一个KOOPA_RVT_RETURN
    switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
        Visit(kind.data.ret,out);
        break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
        Visit(kind.data.integer,out);
        break;
    case KOOPA_RVT_BINARY:
      // 访问 binary 指令
        Visit(kind.data.binary,out);
        break;
    default:

        ;
    }
}

// 处理 return 指令
void Visit(const koopa_raw_return_t &ret,std::ostream &out) {
    // 访问 return 指令中包含的返回值
    auto ret_value = ret.value;
    LOG("return.tag:" << ret_value->kind.tag);
    switch (ret_value->kind.tag){
        case KOOPA_RVT_INTEGER:
        Visit(ret_value->kind.data.integer,out);
        out << "\tret\n";
        break;
        case KOOPA_RVT_BINARY:
        out << "\tmv a0, t" << incrementRegCount(reg_count_rv -1) << "\n";
        out << "\tret\n";
        break;
        default:
        LOG("undefine return action");
        break;
    }
    
    
}

// 处理 integer 指令
void Visit(const koopa_raw_integer_t &integer,std::ostream &out) {
    int32_t int_val = integer.value;
    out << "\tli a0, " << int_val << "\n";
}

//处理 binary 指令
void Visit(const koopa_raw_binary_t &binary,std::ostream &out) {
    auto op = binary.op;
    LOG("op:" << op);
    auto lhs = binary.lhs;
    auto rhs = binary.rhs;
    auto l_kind = lhs->kind.tag;
    auto r_kind = rhs->kind.tag;
    auto l_name = lhs->name;
    auto r_name = rhs->name;
    auto l_used_by = lhs->used_by;
    auto r_used_by = rhs->used_by;
    //LOG("l_used_by:");
    //print_slice(l_used_by);
    //LOG("r_used_by:");
    //print_slice(r_used_by);
    insertToGlobal(l_used_by.buffer,l_used_by.len,reg_count_rv,l_find_reg_info);
    insertToGlobal(r_used_by.buffer,r_used_by.len,reg_count_rv,r_find_reg_info);
    printGlobalContents(l_find_reg_info);
    printGlobalContents(r_find_reg_info);
    int32_t l_value;
    int32_t r_value;
    if (l_kind == KOOPA_RVT_INTEGER)
    {
        l_flag_int = true;
        l_binary_flag = false;
        l_value = lhs->kind.data.integer.value;
    }
    else if (l_kind == KOOPA_RVT_BINARY)
    {
        l_binary_reg = findRegCountRv(lhs,l_find_reg_info);
        LOG("l_binary_reg:" << l_binary_reg);
        l_flag_int = false;
        l_binary_flag = true;
    }else{
        l_flag_int = false;
        l_binary_flag = false;
        l_value = 0;
    }
    
    if (r_kind == KOOPA_RVT_INTEGER)
    {
        r_flag_int = true;
        r_value = rhs->kind.data.integer.value;
    }
    else if (r_kind == KOOPA_RVT_BINARY)
    {
        r_binary_reg = findRegCountRv(rhs,r_find_reg_info);
        LOG("r_binary_reg:" << r_binary_reg);
        r_flag_int = false;
        r_binary_flag = true;
    }else{
        l_flag_int = false;
        l_binary_flag = false;
        l_value = 0;
    }


switch (l_kind) {
    case KOOPA_RVT_INTEGER:  // lhs 的指令类型为 INTEGER
        LOG("lhs is INTEGER");
        break;
    case KOOPA_RVT_RETURN:   // lhs 的指令类型为 RETURN
        LOG("lhs is RETURN");
        break;
    case KOOPA_RVT_BINARY:
        LOG("lhs is BINARY");
        break;
}

switch (r_kind) { 
    case KOOPA_RVT_INTEGER:  // rhs 的指令类型为 INTEGER
        LOG("rhs is INTEGER");
        break;
    case KOOPA_RVT_RETURN:   // rhs 的指令类型为 RETURN
        LOG("rhs is RETURN");
        break;
    case KOOPA_RVT_BINARY:
        LOG("rhs is BINARY");
        break;
}

    switch (op)
    {
    case KOOPA_RBO_NOT_EQ:
        
        out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
        if (l_flag_int)
        {
            out << l_value << "\n";
        }else
        {
            out << "0\n";
        }
        out << "\txor t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv) << ", ";
        if (r_flag_int)
        {
            out << r_value << "\n";
        }else
        {
            out << "0\n";
        }
        out << "\tsnez t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv) << "\n";
        reg_count_rv++;
        break;
    case KOOPA_RBO_EQ:
        out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
        if (l_flag_int)
        {
            out << l_value << "\n";
        }else
        {
            out << "0\n";
        }
        out << "\txor t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv) << ", ";
        if (r_flag_int)
        {
            out << r_value << "\n";
        }else
        {
            out << "0\n";
        }
        out << "\tseqz t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv) << "\n";
        reg_count_rv++;
        break;
    case KOOPA_RBO_SUB:
        if (l_flag_int){
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int)
        {
            out << "\tsub t" << incrementRegCount(reg_count_rv) << ", ";
            out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
            out << "t" << incrementRegCount(reg_count_rv)-1 <<"\n";    //r_value
        }else{
            if (l_flag_int)
            {
                out << "\tsub t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-1) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-2) <<"\n";    //r_value
            };
            if (r_flag_int)
            {
                out << "\tsub t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
            if (!r_flag_int && !l_flag_int){
                out << "\tsub t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
        }
        reg_count_rv++;
        break;
    case KOOPA_RBO_MUL:
        if (l_flag_int){
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int)
        {
            out << "\tmul t" << incrementRegCount(reg_count_rv) << ", ";
            out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
            out << "t" << incrementRegCount(reg_count_rv)-1 <<"\n";    //r_value
        }else{
            if (l_flag_int)
            {
                out << "\tmul t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-1) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-2) <<"\n";    //r_value
            };
            if (r_flag_int)
            {
                out << "\tmul t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
            if (!r_flag_int && !l_flag_int){
                out << "\tmul t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
        }
        reg_count_rv++;
        break;
    case KOOPA_RBO_ADD:
        if (l_flag_int){
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int)
        {
            out << "\tadd t" << incrementRegCount(reg_count_rv) << ", ";
            out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
            out << "t" << incrementRegCount(reg_count_rv)-1 <<"\n";    //r_value
        }else{
            if (l_flag_int)
            {
                out << "\tadd t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-1) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-2) <<"\n";    //r_value
            };
            if (r_flag_int)
            {
                out << "\tadd t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
            if (!r_flag_int && !l_flag_int){
                out << "\tadd t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
        }
        reg_count_rv++;
        break;
    case KOOPA_RBO_LE:
        if (l_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int){
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
            reg_count_rv++;
        }else if (l_binary_flag)
        {
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(l_binary_reg) << "\n";
        }else if (r_binary_flag)
        {
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(r_binary_reg);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
        }else if (r_binary_flag && l_binary_flag){
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(l_binary_reg);
            out << ", t" << incrementRegCount(r_binary_reg) << "\n";
        }
        out << "\txori t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv);
            out << ", 1" << "\n";
        break;
    case KOOPA_RBO_GE:
        if (l_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int){
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
            reg_count_rv++;
        }else if (l_binary_flag)
        {
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(l_binary_reg) << "\n";
        }else if (r_binary_flag)
        {
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(r_binary_reg);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
        }else if (r_binary_flag && l_binary_flag){
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(l_binary_reg);
            out << ", t" << incrementRegCount(r_binary_reg) << "\n";
        }
        out << "\txori t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv);
        out << ", 1" << "\n";
        reg_count_rv++;
        break;
    case KOOPA_RBO_LT:
        if (l_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int){
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
            reg_count_rv++;
        }else if (l_binary_flag)
        {
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(l_binary_reg) << "\n";
        }else if (r_binary_flag)
        {
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(r_binary_reg);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
        }else if (r_binary_flag && l_binary_flag){
            out << "\tslt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(l_binary_reg);
            out << ", t" << incrementRegCount(r_binary_reg) << "\n";
        }
        reg_count_rv++;
        break;
    case KOOPA_RBO_GT:
        if (l_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int){
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
            reg_count_rv++;
        }else if (l_binary_flag)
        {
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(reg_count_rv-1);
            out << ", t" << incrementRegCount(l_binary_reg) << "\n";
        }else if (r_binary_flag)
        {
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(r_binary_reg);
            out << ", t" << incrementRegCount(reg_count_rv-2) << "\n";
        }else if (r_binary_flag && l_binary_flag){
            out << "\tsgt t" << incrementRegCount(reg_count_rv) << ", t" << incrementRegCount(l_binary_reg);
            out << ", t" << incrementRegCount(r_binary_reg) << "\n";
        }
        reg_count_rv++;
        break;
    case KOOPA_RBO_DIV:
        if (l_flag_int){
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int)
        {
            out << "\tdiv t" << incrementRegCount(reg_count_rv) << ", ";
            out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
            out << "t" << incrementRegCount(reg_count_rv)-1 <<"\n";    //r_value
        }else{
            if (l_flag_int)
            {
                out << "\tdiv t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-1) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-2) <<"\n";    //r_value
            };
            if (r_flag_int)
            {
                out << "\tdiv t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
            if (!r_flag_int && !l_flag_int){
                out << "\tdiv t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
        }
        reg_count_rv++;
        break;
    case KOOPA_RBO_MOD:
        if (l_flag_int){
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << l_value << "\n";
            reg_count_rv++;
        }
        if (r_flag_int)
        {
            out << "\tli t" << incrementRegCount(reg_count_rv) << ", ";
            out << r_value << "\n";
            reg_count_rv++;
        }
        if (l_flag_int && r_flag_int)
        {
            out << "\trem t" << incrementRegCount(reg_count_rv) << ", ";
            out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
            out << "t" << incrementRegCount(reg_count_rv)-1 <<"\n";    //r_value
        }else{
            if (l_flag_int)
            {
                out << "\trem t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-1) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-2) <<"\n";    //r_value
            };
            if (r_flag_int)
            {
                out << "\trem t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
            if (!r_flag_int && !l_flag_int){
                out << "\trem t" << incrementRegCount(reg_count_rv) << ", ";
                out << "t" << incrementRegCount(reg_count_rv-2) << ", " ;  //l_value
                out << "t" << incrementRegCount(reg_count_rv-1) <<"\n";    //r_value
            }
        }
        reg_count_rv++;
        break;
    default:
        break;
    }
    
    LOG("l_value:" << l_value );
    LOG("r_value:" << r_value);
    LOG("l_flag:" << l_flag_int);
    LOG("r_flag:" << r_flag_int);
    LOG("lhs:" << lhs);
    LOG("rhs:" << rhs);
    if (l_name)
    {
        LOG("l_name:" << l_name);
    }
    if (r_name)
    {
        LOG("r_name:" << r_name);
    }

    
}

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {

assert(argc == 5);
std::string mode = argv[1];
auto input = argv[2];
auto output = argv[4];

yyin = fopen(input, "r");
assert(yyin);

unique_ptr<BaseAST> ast;
auto ret = yyparse(ast);
assert(!ret);

cout << "mode:" << mode << "\n";

std::string mode_koopa = "-koopa";
std::string mode_rv = "-riscv";

if (mode == mode_koopa)
{
// dump AST
std::ofstream koopa_ir_file(output);
if (koopa_ir_file.is_open()) {
        ast->Dump(koopa_ir_file);
        koopa_ir_file.close();
        std::cout << "Koopa IR successfully written to output.koopa" << std::endl;
    } else {
        std::cerr << "Failed to open file for writing Koopa IR." << std::endl;
    }
cout << endl;
}

if (mode == mode_rv)
{
// dump AST
std::ofstream koopa_ir_file("output.koopa");
if (koopa_ir_file.is_open()) {
        ast->Dump(koopa_ir_file);
        koopa_ir_file.close();
        std::cout << "Koopa IR successfully written to output.koopa" << std::endl;
    } else {
        std::cerr << "Failed to open file for writing Koopa IR." << std::endl;
        return 1; 
    }
cout << endl;



std::ifstream file("output.koopa"); 
std::stringstream buffer;
if (file.is_open()) {
        buffer << file.rdbuf();  // 读取文件内容到 buffer
        file.close();
    } else {
        std::cerr << "Failed to open file for reading Koopa IR." << std::endl;
        return 1;  // 失败时退出
    }

    std::string str = buffer.str(); 
    const char* c_str = str.c_str();

// 解析字符串 str, 得到 Koopa IR 程序
koopa_program_t program;
koopa_error_code_t ret_koopa = koopa_parse_from_string(c_str, &program);
assert(ret_koopa == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
// 创建一个 raw program builder, 用来构建 raw program
koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
// 将 Koopa IR 程序转换为 raw program
koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
// 释放 Koopa IR 程序占用的内存
koopa_delete_program(program);

    
    
std::ofstream koopa_rv_file(output);
if (koopa_rv_file.is_open()) {
    // 访问 raw program
    Visit(raw,koopa_rv_file);
    koopa_rv_file.close();
    std::cout << "Koopa rv successfully written to output.koopa" << std::endl;
} else {
    std::cerr << "Failed to open file for writing Koopa rv." << std::endl;
    return 1; 
}
cout << endl;

// 处理完成, 释放 raw program builder 占用的内存
// 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
// 所以不要在 raw program 处理完毕之前释放 builder
koopa_delete_raw_program_builder(builder);
}

cout << "finish" <<endl;
}

static int incrementRegCount(int reg_count_rv) {
    // 循环在 0 到 6 之间
    increment_reg_count_rv = reg_count_rv % 7; 
    //LOG("Current increment_reg_count_rv: " << increment_reg_count_rv);
    //LOG("Current reg_count_rv: " << reg_count_rv);
    return increment_reg_count_rv;
}

static void print_slice(const koopa_raw_slice_t &slice) {
    for (uint32_t i = 0; i < slice.len; ++i) {
        // 直接打印 buffer 中的 void* 指针
        LOG("buffer[" << i << "] = " << slice.buffer[i]);
    }
}

static void insertToGlobal(const void** buffer, uint32_t len, int reg_count_rv,std::vector<BufferInfo>& find_reg_info) {
    BufferInfo new_info = {buffer, reg_count_rv, len};
    find_reg_info.push_back(new_info); // 添加到 vector 中
}

// 打印 myGlobal 的内容
static void printGlobalContents(std::vector<BufferInfo>& find_reg_info) {
    LOG("Global Pair contents:");
    for (const auto& info : find_reg_info) {
        LOG("reg_count_rv = " << info.reg_count_rv);
        for (size_t i = 0; i < info.len; ++i) {
            LOG("buffer[" << i << "] = " << info.buffer[i]);
        }
    }
}

static int findRegCountRv(const void* target, const std::vector<BufferInfo>& find_reg_info) {
    for (const auto& info : find_reg_info) {
        for (size_t i = 0; i < info.len; ++i) {
            // 比较当前 BufferInfo 中的 buffer[i] 和目标指针
            if (info.buffer[i] == target) {
                return info.reg_count_rv; // 找到匹配，返回 reg_count_rv
            }
        }
    }
    return -1; // 如果未找到，返回一个标识值（例如 -1）
}