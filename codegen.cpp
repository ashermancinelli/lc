#include <cstdlib>
#include <cstring>
#include "parse.h"
#include "opc.h"
#include "err.h"
#include "lower.h"
#include "dbg.h"
#include "config.h"

Value* SEXPR::codegen()
{
  if(dump("lower"))
    puts("lowering SEXPR");
  if(auto f = dynamic_pointer_cast<BIFUNC>(exprs[0]))
  {
    if(dump("lower"))
      puts("lowering BIFUNC from SEXPR");
    return f->codegen();
  }
  else
    return exprs[0]->codegen();
}

Value* NUM::codegen()
{
  if(dump("lower"))
    puts("lowering NUM");
  return ConstantFP::get(context(), APFloat((double)this->v));
}

Value* ID::codegen()
{
  if(dump("lower"))
    puts("lowering ID");
  Value* v = get_value(this->n);
  return v;
}

PROTOTYPE::PROTOTYPE(std::shared_ptr<SEXPR> se)
{
  auto _n = std::dynamic_pointer_cast<ID>(se->exprs[0]);
  LCASSERT_P("sema", "prototype sexpr must have all ID element types", _n);
  n = _n->n;
  for(int i = 1; i < se->exprs.size(); i++)
  {
    auto arg = std::dynamic_pointer_cast<ID>(se->exprs[i]);
    LCASSERT_P("sema", "prototype sexpr must have all ID element types", arg);
    args.push_back(arg->n);
  }
}
Function* PROTOTYPE::codegen()
{
  if(dump("lower"))
    puts("lowering PROTOTYPE");
  std::vector<Type*> argtypes(args.size(), Type::getDoubleTy(context()));
  auto*              ft = FunctionType::get(Type::getDoubleTy(context()), argtypes, false);
  auto*              f  = Function::Create(ft, Function::ExternalLinkage, this->n, get_module());

  unsigned i = 0;
  for(auto& arg : f->args())
    arg.setName(this->args[i++]);

  return f;
}

std::map<std::string, Value*> USERFUNC::local_values;
Value*                        USERFUNC::codegen()
{
  if(dump("lower"))
    puts("lowering USERFUNC");
  Function* f = get_module().getFunction(proto->n);

  if(!f)
    f = proto->codegen();

  if(!f)
    return nullptr;   // uh oh!

  auto* bb = BasicBlock::Create(context(), "entrypoint", f);
  get_builder().SetInsertPoint(bb);

  local_values.clear();
  for(auto& a : f->args())
    local_values[std::string(a.getName())] = &a;

  Value* r = this->body->codegen();
  get_builder().CreateRet(r);
  return f;
}

Value* CALLEXPR::codegen()
{
  if(dump("lower"))
    puts("lowering CALLEXPR");
  Function* f = get_module().getFunction(n);
  if(!f)
  {
    std::string msg = "could not find function '" + n + "' at time of reference";
    reg_msg(LC_MSG{"lower", msg, MSG_ERROR});
  }

  if(f->arg_size() != args.size())
  {
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    sprintf(msg, "argument mismatch for '%s'. got %zu arguments, expected %zu.", n.c_str(),
      args.size(), f->arg_size());
    reg_msg(LC_MSG{"lower", msg, MSG_ERROR});
  }

  std::vector<Value*> vargs;
  for(int i = 0; i < args.size(); i++)
  {
    vargs.push_back(args[i]->codegen());
    if(!vargs.back())
    {
      char msg[1024];
      memset(msg, 0, sizeof(msg));
      sprintf(msg, "failed to codgen for argument %zu of call to function %s", vargs.size(),
        n.c_str());
      reg_msg(LC_MSG{"lower", msg, MSG_ERROR});
      return nullptr;
    }
  }

  return get_builder().CreateCall(f, vargs, "call" + lower_id());
}

Value* MODULE::codegen_funcs()
{
  Value* last = nullptr;
  for(auto se : sexprs)
  {
    if(auto defun = std::dynamic_pointer_cast<USERFUNC>(se->exprs[0]))
      last = defun->codegen();
  }
  return last;
}

Value* MODULE::codegen()
{
  Value* last;
  for(auto se : sexprs)
  {
    // If we're at a defun, just skip it... we've already done codegen for
    // defuns at this point.
    if(std::dynamic_pointer_cast<USERFUNC>(se->exprs[0]))
      continue;
    last = se->codegen();
  }
  return last;
}

Value* BISUM::codegen()
{
  auto* l = lhs->codegen();
  auto* r = rhs->codegen();
  return get_builder().CreateFAdd(l, r, "sum" + lower_id());
}

Value* BIMUL::codegen()
{
  auto* l = lhs->codegen();
  auto* r = rhs->codegen();
  return get_builder().CreateFMul(l, r, "mul" + lower_id());
}

Value* BIDEFVAR::codegen()
{
  if(dump("lower"))
    printf("lowering defvar '%s'\n", id.c_str());
  add_value(id, v->codegen());
  return get_value(id);
}
