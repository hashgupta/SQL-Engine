#pragma once

#include <string>
#include "allocator.h"
#include "reader.h"
#include "parser.h"
#include "map"

struct BinExprVisitor;

struct BoolExprVisitor;

class Engine {
    ArenaAllocator* alloc;
    std::map<std::string, int> map;
    
    public:

    Engine();

    std::string execute(std::string query);

    void execute_select_statement(NodeStmtSelect* stmt);
    
    std::vector<std::vector<DataItem>> evaluate_filter(Data data, NodeExpr* expr, std::map<std::string, int> map);

    DataItem evaluate_expr(NodeExpr* expr, std::vector<DataItem> row, std::map<std::string, int> map); 
};