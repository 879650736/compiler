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


  for &func in program.func_layout() {
    let func_data = program.func(func);
    // 访问函数
    // ...
    // 遍历基本块列表
  for (&bb, node) in func_data.layout().bbs() {
    // 一些必要的处理
    // ...
      // 遍历指令列表
      for &inst in node.insts().keys() {
        let value_data = func_data.dfg().value(inst);
        // 访问指令
        // ...
        use koopa::ir::ValueKind;
        match value_data.kind() {
          ValueKind::Integer(int) => {
            // 处理 integer 指令
            // ...
          }
          ValueKind::Return(ret) => {
            // 处理 ret 指令
            // ...
          }
          // 其他种类暂时遇不到
          _ => unreachable!(),
        }
      }
    }
  
    
  
  }
  
  

  println!("Successfully parsed Koopa IR program");
  
  Ok(())
}
