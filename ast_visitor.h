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

inline void expr_visit(std::shared_ptr<EXPR> e, std::shared_ptr<VISITOR> v)
{
  if(auto ee = std::dynamic_pointer_cast<SEXPR>(e))
  {
    v->visitSEXPR(ee);
    for(auto const se : ee->exprs)
      expr_visit(se, v);
  }
  else if(auto ee = std::dynamic_pointer_cast<ID>(e))
  {
    v->visitID(ee);
  }
  else if(auto ee = std::dynamic_pointer_cast<STR>(e))
  {
    v->visitSTR(ee);
  }
}
