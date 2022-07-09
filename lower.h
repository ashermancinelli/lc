#pragma once
#include "ll.h"
#include "parse.h"
struct MODULE;
void lower(std::shared_ptr<MODULE> m);
void add_value(std::string name, Value* v);
Value* get_value(std::string n);
LLVMContext& context();
IRBuilder<>& get_builder();
std::string lower_id();
