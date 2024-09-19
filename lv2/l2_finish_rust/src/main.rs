#[derive(Debug)]
pub struct CompUnit {
    pub func_def: FuncDef,
  }
  
  #[derive(Debug)]
  pub struct FuncDef {
    pub func_type: FuncType,
    pub ident: String,
    pub block: Block,
  }
  
  #[derive(Debug)]
  pub struct FuncType {
  
  }
  
  #[derive(Debug)]
  pub struct Block {
    pub stmt: Stmt,
  }

  #[derive(Debug)]
  pub struct Stmt {
    pub number: Number,
}

#[derive(Debug)]
pub struct Number {
  pub int_const: i32,
}

use lalrpop_util::lalrpop_mod;
use std::env::args;
use std::fs::read_to_string;
use std::fs::File;
use std::io::{self, Read, Write};
use std::fmt;
use koopa::front::Driver;
use koopa::ir;


// 实现 Display trait
impl fmt::Display for CompUnit {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
      write!(f, "{} ", self.func_def)?;
      Ok(())
  }
}

impl fmt::Display for FuncDef {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
      write!(f, "fun @{}(): ", self.ident)?;
      write!(f, "{}", self.func_type)?;
      write!(f, "{{\n")?;
      write!(f, "{}", self.block)?;
      Ok(())
  }
}

impl fmt::Display for FuncType {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
      write!(f, "i32")?;
      Ok(())
  }
}

impl fmt::Display for Block {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
      write!(f, "%")?;
      write!(f, "entry:")?;
      write!(f, "\n")?;
      write!(f, "{}", self.stmt)?;
      Ok(())
  }
}

impl fmt::Display for Stmt {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
      write!(f, "\tret ")?;
      write!(f, "{}", self.number)?;
      write!(f, "\n}} ")?;
      Ok(())
  }
}

impl fmt::Display for Number {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
      write!(f, "{}", self.int_const)?;
      Ok(())
  }
}

// 根据内存形式 Koopa IR 生成汇编
trait GenerateAsm {
  fn generate(&self, out: &mut dyn fmt::Write);
}

impl GenerateAsm for koopa::ir::Program {
  fn generate(&self, out: &mut dyn fmt::Write) {
    writeln!(out, "\t.text").unwrap();
    for &func in self.func_layout() {
      self.func(func).generate(out);
    }
  }
}

impl GenerateAsm for koopa::ir::FunctionData {
  fn generate(&self, out: &mut dyn fmt::Write) {
    // 输出函数名
    let func_name = self.name().replace('@', "");
    writeln!(out, "\t.globl {}", func_name).unwrap();
    writeln!(out, "{}:\n", func_name).unwrap();
    
    // 访问基本块
    for (&bb, node) in self.layout().bbs() {
        // 遍历指令
        for &inst in node.insts().keys() {
          let value_data = self.dfg().value(inst);
          // 访问指令
          use koopa::ir::ValueKind;
          //writeln!(out, "访问指令").unwrap();
            match value_data.kind() {
                ValueKind::Integer(int) => {
                    // 处理 integer 指令
                    writeln!(out, "\tli a0, {}\n", int.value()).unwrap();
                    // 可以根据需要生成更多汇编代码
                }
                ValueKind::Return(ret) => {
                    // 处理 return 指令
                    
                  let return_value = ret.value();
                  match return_value {
                  Some(value) => {
                    // 假设你有某种方法来确定 value 的类型
                    // 例如，可能需要用其他方式获取 Value 的类型
                            let r_value = self.dfg().value(value);
                            if let ValueKind::Integer(int_value) = r_value.kind(){
                              writeln!(out, "\tli a0, {}", int_value.value()).unwrap();
                            }
                            
                        }
                  None => {
                    // 如果没有返回值
                    unreachable!("Return without value");
                }
            }
                    writeln!(out, "\tret\n").unwrap();
                }
                // 其他种类暂时遇不到
                _ => unreachable!(),
            }
        }
    }
  }
}


// 引用 lalrpop 生成的解析器
// 因为我们刚刚创建了 sysy.lalrpop, 所以模块名是 sysy
lalrpop_mod!(sysy);

fn main() -> Result<(), Box<dyn std::error::Error>> {
  // 解析命令行参数
  let mut args = args();
  args.next();
  let mode = args.next().unwrap();
  let input = args.next().unwrap();
  args.next();
  let output = args.next().unwrap();

  // 读取输入文件
  let input = read_to_string(input)?;

  // 调用 lalrpop 生成的 parser 解析输入文件
  let ast = sysy::CompUnitParser::new().parse(&input).unwrap();

  // 输出解析得到的 AST
  println!("{}", ast);

  // 将 AST 输出到文件
  let mut file = File::create("output.txt")?;
  writeln!(file, "{}", ast)?;
  

  
  // 读取文件内容到字符串
  let mut s = String::new();
  {
    let mut file = File::open("output.txt")?;
    file.read_to_string(&mut s)?;
  }

  let driver = koopa::front::Driver::from(s);
  let program = driver.generate_program().unwrap();


  // 创建一个 String 来暂存生成的汇编代码
  let mut asm_output = String::new();

  // 生成汇编并写入到 String 中
  program.generate(&mut asm_output);

  // 将生成的汇编代码写入到输出文件
  let mut asm_file = File::create(output)?;
  asm_file.write_all(asm_output.as_bytes())?;

  
  

  println!("Successfully parsed Koopa IR program");
  
  Ok(())
}
