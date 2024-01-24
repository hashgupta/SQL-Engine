#include <fmt/format.h>
#include <sql-engine/engine.h>
#include <iostream>
#include <numeric>
#include <map>

struct BinExprVisitor {
    std::vector<DataItem> row;
    std::map<std::string, int> map;
    Engine *engine;

    int operator()(const NodeBinExprSub* sub) const
    {
        auto lhs = engine->evaluate_expr(sub->lhs, row, map);
        auto rhs = engine->evaluate_expr(sub->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left - right;
        
    }
    int operator()(const NodeBinExprAdd* add) const
    {
        auto lhs = engine->evaluate_expr(add->lhs, row, map);
        auto rhs = engine->evaluate_expr(add->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left + right;
        
    }
    int operator()(const NodeBinExprMulti* multi) const
    {
        auto lhs = engine->evaluate_expr(multi->lhs, row, map);
        auto rhs = engine->evaluate_expr(multi->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left * right;
    }
    int operator()(const NodeBinExprDiv* div) const
    {
        
        auto lhs = engine->evaluate_expr(div->lhs, row, map);
        auto rhs = engine->evaluate_expr(div->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left / right;
    }
};



struct BoolExprVisitor {
    std::vector<DataItem> row;
    std::map<std::string, int> map;
    Engine *engine;

    bool operator()(const NodeBoolExprGT* gt) const
    {
        auto lhs = engine->evaluate_expr(gt->lhs, row, map);
        auto rhs = engine->evaluate_expr(gt->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left > right;
        
    }
    bool operator()(const NodeBoolExprGTE* gte) const
    {
        auto lhs = engine->evaluate_expr(gte->lhs, row, map);
        auto rhs = engine->evaluate_expr(gte->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left >= right;
        
    }
    bool operator()(const NodeBoolExprLT* lt) const
    {
        auto lhs = engine->evaluate_expr(lt->lhs, row, map);
        auto rhs = engine->evaluate_expr(lt->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left < right;
    }
    bool operator()(const NodeBoolExprLTE* lte) const
    {
        
        auto lhs = engine->evaluate_expr(lte->lhs, row, map);
        auto rhs = engine->evaluate_expr(lte->rhs, row, map);
        int left = std::get<int>(lhs.item);
        int right = std::get<int>(rhs.item);
        return left <= right;
    }
    bool operator()(const NodeBoolExprAND* and_) const
    {
        DataItem lhs = engine->evaluate_expr(and_->lhs, row, map);
        DataItem rhs = engine->evaluate_expr(and_->rhs, row, map);
        bool lhs_bool = std::get<bool>(lhs.item);
        bool rhs_bool = std::get<bool>(rhs.item);
        return lhs_bool && rhs_bool;
    }
    bool operator()(const NodeBoolExprOR* or_) const
    {

        DataItem lhs = engine->evaluate_expr(or_->lhs, row, map);
        DataItem rhs = engine->evaluate_expr(or_->rhs, row, map);
        bool lhs_bool = std::get<bool>(lhs.item);
        bool rhs_bool = std::get<bool>(rhs.item);
        return lhs_bool || rhs_bool;
    }

    bool operator()(const NodeBoolExprNOT* not_) const
    {
        DataItem lhs = engine->evaluate_expr(not_->expr, row, map);
        bool lhs_bool = std::get<bool>(lhs.item);
        return !lhs_bool;
    }
};


Engine::Engine() {
  this->alloc = new ArenaAllocator(8 * 1024 * 1024);
  std::map<std::string, int> new_map;
  this->map = new_map;
};

std::string Engine::execute(std::string query) {
  Tokenizer* tokenizer = new Tokenizer(query);
  std::vector<Token> tokens = tokenizer->tokenize();

  Parser* parser = new Parser(tokens, this->alloc);
  std::optional<NodeStmt*> stmt = parser->parse_stmt();
  
  if (stmt.has_value()) {
    NodeStmt* stmt_pointer = stmt.value();
    NodeStmtSelect* select_stmt = std::get<NodeStmtSelect*>(stmt_pointer->var);
    this->execute_select_statement(select_stmt);
  }
  
  return "done";

};



void Engine::execute_select_statement(NodeStmtSelect* stmt) {

  Reader reader(stmt->table->ident.value.value() + ".csv");
  Data data = reader.get_data();
  
  auto select_cols = stmt->view_columns;
  std::vector<std::vector<DataItem>> result;
  
  for (int i = 0; i < data.column_names.size(); i++) {
    map.insert_or_assign(data.column_names[i], i);
  }


  // filter rows based on where clause
  if (stmt->where.has_value()) {
    auto array_passed = this->evaluate_filter(data, stmt->where.value(), map);
    data.rows = array_passed;
  }

  // Evaluate Query expressions 
  for (std::vector<DataItem> row: data.rows) {
    std::vector<DataItem> tuple;
    
    for (NodeExpr* column_expr: select_cols) {
      auto output = this->evaluate_expr(column_expr, row, map);
      tuple.push_back(output);
    }
    
    result.push_back(tuple);
  }
  
  
  // Print results to console
  for (int i = 0; i < result.size(); i++) {
    
    for (int j = 0; j < result[i].size(); j++) {
      DataItem row_item = result[i][j];
      
      if (int* integer = std::get_if<int>(&row_item.item)) {
        std::cout << *integer << "\t";
      } else {
        std::cout << std::get<std::string>(row_item.item) << "\t";
      }
    }
    
    std::cout << "\n";
  }
}



std::vector<std::vector<DataItem>> Engine::evaluate_filter(Data data, NodeExpr* expr, std::map<std::string, int> map) {
  std::vector<std::vector<DataItem>> result;

  for (std::vector<DataItem> row: data.rows) {
    DataItem item = this->evaluate_expr(expr, row, map);

    if (bool a = std::get<bool>(item.item); a) {
      result.push_back(row);
    }
  }
  return result;
};


DataItem Engine::evaluate_expr(NodeExpr* expr, std::vector<DataItem> row, std::map<std::string, int> map) {
  
  DataItem output;
  
  
  if (auto ident_pointer = std::get_if<NodeTerm *>(&expr->expr)) {
    NodeTerm* ident = *ident_pointer;
    if (auto integer_pointer = std::get_if<NodeTermIntLit *>(&ident->var)) {
      NodeTermIntLit * integer = *integer_pointer;
      output.item = stoi(integer->int_lit.value.value());
      return output;
    } else if (auto term_pointer = std::get_if<NodeTermIdent *>(&ident->var)) {
      auto term = *term_pointer;
      DataItem col_value = row.at(map.at(term->ident.value.value()));
      return col_value;
    } else if (auto paren_pointer = std::get_if<NodeTermParen *>(&ident->var)){
      auto paren = *paren_pointer;
      return evaluate_expr(paren->expr, row, map);
    } else {
      exit(EXIT_FAILURE);
    }
  }

  if (auto bin_expr = std::get_if<NodeBinExpr *>(&expr->expr)) {
    auto loaded_bin_expr = *bin_expr;
    BinExprVisitor visitor { .engine = this, .row = row, .map = map};
    output.item = std::visit(visitor, loaded_bin_expr->var);
    return output;
  } 
  
  
  if (auto bool_expr = std::get_if<NodeBoolExpr *>(&expr->expr)) {
    auto loaded_bool_expr = *bool_expr;
    BoolExprVisitor visitor { .engine = this, .row = row, .map = map};
    output.item = std::visit(visitor, loaded_bool_expr->var);
    return output;
  }
  std::cerr << "SHIT" << std::endl;
  exit(EXIT_FAILURE);
}