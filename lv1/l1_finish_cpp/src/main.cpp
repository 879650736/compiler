#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
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
