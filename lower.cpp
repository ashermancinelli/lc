#include "lower.h"
#include "config.h"

using namespace llvm;

static std::unique_ptr<LLVMContext> ctx;
static std::unique_ptr<Module> module;
static std::unique_ptr<IRBuilder<>> builder;
static std::map<std::string, Value *> named_values;

LLVMContext& context()
{
  return *ctx;
}

std::string lower_id()
{
  static int tmpid = 0;
  std::string tmp(6, '\0');
  sprintf(tmp.data(), "%5d", tmpid++);
  return tmp;
}

IRBuilder<>& get_builder()
{
  return *builder;
}

Value* get_value(std::string n)
{
  auto v = named_values.find(n);
  if (v == named_values.end())
  {
    std::string msg =  "could not find named value '" + n + "'";
    reg_msg(LC_MSG{"sema", msg, MSG_ERROR});
    return nullptr;
  }
  return (*v).second;
}

void add_value(std::string name, Value* v)
{
  named_values[name] = v;
}

void lower(std::shared_ptr<MODULE> m)
{
  ctx = std::make_unique<LLVMContext>();
  module = std::make_unique<Module>("lisp compiler", *ctx);
  builder = std::make_unique<IRBuilder<>>(*ctx);
  std::vector<Type*> entrypoint;
  FunctionType* ft = FunctionType::get(IntegerType::get(*ctx, 8), false);
  Function *f = Function::Create(ft, Function::ExternalLinkage, "main", module.get());
  BasicBlock *bb = BasicBlock::Create(*ctx, "entry", f);
  builder->SetInsertPoint(bb);
  auto* v = m->codegen();
  auto *ret = builder->CreateFPToSI(v, IntegerType::get(*ctx, 8), "return");
  builder->CreateRet(ret);
  if (outfile().size())
  {
    std::error_code ec;
    raw_fd_ostream os{outfile(), ec};
    if (ec)
      reg_msg(LC_MSG{"lower", "could not open output file for writing", MSG_FATAL});
    else
      module->print(os, nullptr);
  }
  else
    module->print(errs(), nullptr);
}
