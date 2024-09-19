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
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
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
    switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
        Visit(kind.data.ret,out);
        break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
        Visit(kind.data.integer,out);
        break;
    default:
      // 其他类型暂时遇不到
        assert(false);
    }
}

// 处理 return 指令
void Visit(const koopa_raw_return_t &ret,std::ostream &out) {
    // 访问 return 指令中包含的返回值
    auto ret_value = ret.value;
    // 示例程序中, ret_value 一定是一个 integer
    assert(ret_value->kind.tag == KOOPA_RVT_INTEGER);
    // 处理 integer 指令
    Visit(ret_value->kind.data.integer,out);
    out << "\tret\n";
}

// 处理 integer 指令
void Visit(const koopa_raw_integer_t &integer,std::ostream &out) {
    int32_t int_val = integer.value;
    out << "\tli a0, " << int_val << "\n";
}

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {

assert(argc == 5);
auto mode = argv[1];
auto input = argv[2];
auto output = argv[4];

yyin = fopen(input, "r");
assert(yyin);

unique_ptr<BaseAST> ast;
auto ret = yyparse(ast);
assert(!ret);


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

cout << "finish" <<endl;
}
