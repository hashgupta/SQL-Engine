#include <fmt/format.h>
#include <sql-engine/engine.h>

#include <iostream>
#include <map>
#include <numeric>

int BinExprVisitor::operator()(const NodeBinary* bin_node) const {
  auto lhs = engine->evaluate_expr(bin_node->lhs, row, map);
  auto rhs = engine->evaluate_expr(bin_node->rhs, row, map);
  int left = std::get<int>(lhs.item);
  int right = std::get<int>(rhs.item);
  switch (bin_node->type) {
    case NodeBinary::ADD:
      return left + right;
    case NodeBinary::SUB:
      return left - right;
    case NodeBinary::MUL:
      return left * right;
    case NodeBinary::DIV:
      return left / right;
  }
};

DataItem NodeTermExprVisitor::operator()(const NodeTermIntLit* int_lit_node) const {
  DataItem output;
  Token literal_token = int_lit_node->int_lit;
  output.item = stoi(literal_token.force_value());
  return output;
};

DataItem NodeTermExprVisitor::operator()(const NodeTermIdent* ident_term) const {
  Token term = ident_term->ident;
  DataItem col_value = row.at(map.at(term.force_value()));
  return col_value;
};
DataItem NodeTermExprVisitor::operator()(const NodeTermParen* paren_term) const {
  auto paren_expr = paren_term->expr;
  return engine->evaluate_expr(paren_expr, row, map);
}


DataItem NodeExprVisitor::operator()(const NodeBinExpr* bin_expr_node) const {
  DataItem output;
  BinExprVisitor visitor{.engine = engine, .row = row, .map = map};
  output.item = std::visit(visitor, bin_expr_node->var);
  return output;
}
DataItem NodeExprVisitor::operator()(const NodeTerm* node_term) const {
  NodeTermExprVisitor visitor{.engine = engine, .row = row, .map = map};
  return std::visit(visitor, node_term->var);
}
DataItem NodeExprVisitor::operator()(const NodeBoolExpr* paren_term) const {
  DataItem output;
  BoolNodeVisitor visitor{.engine = engine, .row = row, .map = map};
  output.item = std::visit(visitor, paren_term->var);
  return output;
}



bool BoolNodeVisitor::operator()(const NodeBoolComparator* comp_node) const {
  auto lhs = engine->evaluate_expr(comp_node->lhs, row, map);
  auto rhs = engine->evaluate_expr(comp_node->rhs, row, map);
  int left = std::get<int>(lhs.item);
  int right = std::get<int>(rhs.item);
  switch (comp_node->type) {
    case NodeBoolComparator::LT:
      return left < right;
    case NodeBoolComparator::LTE:
      return left <= right;
    case NodeBoolComparator::GT:
      return left > right;
    case NodeBoolComparator::GTE:
      return left >= right;
  }
};

bool BoolNodeVisitor::operator()(const NodeBoolLogicalBin* log_node) const {
  DataItem lhs = engine->evaluate_expr(log_node->lhs, row, map);
  DataItem rhs = engine->evaluate_expr(log_node->rhs, row, map);
  bool lhs_bool = std::get<bool>(lhs.item);
  bool rhs_bool = std::get<bool>(rhs.item);
  if (log_node->type == NodeBoolLogicalBin::op::AND) {
    return lhs_bool && rhs_bool;
  } else {
    return lhs_bool || rhs_bool;
  }
}

bool BoolNodeVisitor::operator()(const NodeBoolLogicalUnary* unary_node) const {
  DataItem lhs = engine->evaluate_expr(unary_node->expr, row, map);
  bool lhs_bool = std::get<bool>(lhs.item);
  switch (unary_node->type) {
    case NodeBoolLogicalUnary::op::NOT:
      return !lhs_bool;
    default:
      std::cerr << "PLEASE UPDATE THIS";
      exit(EXIT_FAILURE);
  }
}

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
  Reader reader(stmt->table + ".csv");
  Data data = reader.read_data();

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
  for (std::vector<DataItem> row : data.rows) {
    std::vector<DataItem> tuple;

    for (NodeExpr* column_expr : select_cols) {
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

std::vector<std::vector<DataItem>> Engine::evaluate_filter(Data data, NodeExpr* expr,
                                                           std::map<std::string, int> map) {
  std::vector<std::vector<DataItem>> result;

  for (std::vector<DataItem> row : data.rows) {
    DataItem item = this->evaluate_expr(expr, row, map);

    if (bool a = std::get<bool>(item.item); a) {
      result.push_back(row);
    }
  }
  return result;
};

DataItem Engine::evaluate_expr(NodeExpr* expr, std::vector<DataItem> row,
                               std::map<std::string, int> map) {
  NodeExprVisitor visitor{.engine = this, .row = row, .map = map};
  return std::visit(visitor, expr->expr);
}