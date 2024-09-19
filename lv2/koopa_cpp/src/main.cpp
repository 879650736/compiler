#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <cassert>

#include "koopa.h"
#include "ast.h"

using namespace std;

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

// 处理 raw program
// 使用 for 循环遍历函数列表
for (size_t i = 0; i < raw.funcs.len; ++i) {
    // 确认列表中的元素是函数
    assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
    
    // 获取当前函数
    koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
    
    for (size_t j = 0; j < func->bbs.len; ++j) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
        for (size_t k = 0; k < bb->insts.len; ++k) {
            koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];
            // 示例程序中, 你得到的 value 一定是一条 return 指令
            assert(value->kind.tag == KOOPA_RVT_RETURN);
            // 于是我们可以按照处理 return 指令的方式处理这个 value
            // return 指令中, value 代表返回值
            koopa_raw_value_t ret_value = value->kind.data.ret.value;
            // 示例程序中, ret_value 一定是一个 integer
            assert(ret_value->kind.tag == KOOPA_RVT_INTEGER);
            // 于是我们可以按照处理 integer 的方式处理 ret_value
            // integer 中, value 代表整数的数值
            int32_t int_val = ret_value->kind.data.integer.value;
            // 示例程序中, 这个数值一定是 0
            assert(int_val == 0);
        }

    }

    // 进一步处理当前函数，比如输出函数名
    if (func->name != nullptr) {
        std::cout << "Function name: " << func->name << std::endl;
    }
    
    // 你还可以进一步遍历函数内的基本块、指令等
    
}


// 处理完成, 释放 raw program builder 占用的内存
// 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
// 所以不要在 raw program 处理完毕之前释放 builder
koopa_delete_raw_program_builder(builder);

}
