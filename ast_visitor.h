#pragma once

struct VISITOR
{
  virtual bool visitID(std::shared_ptr<ID> e)
  {
    return false;
  }
  virtual bool visitSTR(std::shared_ptr<STR> s)
  {
    return false;
  }
  virtual bool visitSEXPR(std::shared_ptr<SEXPR> se)
  {
    return false;
  }
};

inline bool expr_visit(std::shared_ptr<EXPR> e, std::shared_ptr<VISITOR> v)
{
  bool changed = false;
  if(auto ee = std::dynamic_pointer_cast<SEXPR>(e))
  {
    changed = changed || v->visitSEXPR(ee);
    for(auto const se : ee->exprs)
      changed = changed || expr_visit(se, v);
  }
  else if (auto dv = std::dynamic_pointer_cast<BIDEFVAR>(e))
  {
    changed = changed || expr_visit(dv->v, v);
  }
  else if (auto f = std::dynamic_pointer_cast<USERFUNC>(e))
  {
    for (auto b : f->body)
      changed = changed || v->visitSEXPR(b);
  }
  else if(auto ee = std::dynamic_pointer_cast<ID>(e))
  {
    changed = changed || v->visitID(ee);
  }
  else if(auto ee = std::dynamic_pointer_cast<STR>(e))
  {
    changed = changed || v->visitSTR(ee);
  }
  else if(auto ee = std::dynamic_pointer_cast<BISUM>(e))
  {
    changed = changed || expr_visit(ee->lhs, v);
    changed = changed || expr_visit(ee->rhs, v);
  }
  else if(auto ee = std::dynamic_pointer_cast<BIMUL>(e))
  {
    changed = changed || expr_visit(ee->lhs, v);
    changed = changed || expr_visit(ee->rhs, v);
  }
  else if (auto ce = std::dynamic_pointer_cast<CALLEXPR>(e))
  {
    for (auto arg : ce->args)
      changed = changed || expr_visit(arg, v);
  }
  return changed;
}
