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
use std::io::Result;
use std::fs::File;
use std::io:: Write;
use std::fmt;


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

fn main() -> Result<()> {
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
  let mut file = File::create(output)?;
  writeln!(file, "{}", ast)?;
  
  Ok(())
}
