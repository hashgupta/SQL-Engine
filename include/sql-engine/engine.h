#pragma once

#include <string>

#include "allocator.h"
#include "map"
#include "parser.h"
#include "reader.h"

// template<class T, typename... Args>
// struct BaseVisitor {
//     std::vector<DataItem> row;
//     std::map<std::string, int> map;
//     Engine* engine;


// }
class Engine {
  ArenaAllocator* alloc;
  std::map<std::string, int> map;

public:
  Engine();

  std::string execute(std::string query);

  void execute_select_statement(NodeStmtSelect* stmt);

  std::vector<std::vector<DataItem>> evaluate_filter(Data data, NodeExpr* expr,
                                                     std::map<std::string, int> map);

  DataItem evaluate_expr(NodeExpr* expr, std::vector<DataItem> row, std::map<std::string, int> map);
};


struct BinExprVisitor {
  std::vector<DataItem> row;
  std::map<std::string, int> map;
  Engine* engine;
  int operator()(const NodeBinary* bin_node) const;
};

struct NodeExprVisitor {
  std::vector<DataItem> row;
  std::map<std::string, int> map;
  Engine* engine;

  DataItem operator()(const NodeBinExpr* bin_expr_node) const;
  DataItem operator()(const NodeTerm* node_term) const;
  DataItem operator()(const NodeBoolExpr* paren_term) const;
};

struct NodeTermExprVisitor {
  std::vector<DataItem> row;
  std::map<std::string, int> map;
  Engine* engine;

  DataItem operator()(const NodeTermIntLit* int_lit_node) const;
  DataItem operator()(const NodeTermIdent* ident_term) const;
  DataItem operator()(const NodeTermParen* paren_term) const;
};

struct BoolNodeVisitor {
    std::vector<DataItem> row;
    std::map<std::string, int> map;
    Engine* engine;
    bool operator()(const NodeBoolComparator* comp_node) const;
    bool operator()(const NodeBoolLogicalBin* log_node) const;
    bool operator()(const NodeBoolLogicalUnary* unary_node) const;
};