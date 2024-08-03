#pragma once

#include <sql-engine/allocator.h>
#include <vector>
#include <iostream>
#include <sql-engine/tokenizer.h>
#include <sql-engine/reader.h>
#include <optional>
#include <variant>
#include <map>


struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBoolExpr;

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeBinary {
    NodeExpr* lhs;
    NodeExpr* rhs;
    enum op {ADD, SUB, DIV, MUL} type;
};

struct NodeBoolComparator {
    NodeExpr* lhs;
    NodeExpr* rhs;
    enum op {GT, GTE, LT, LTE} type;
};

struct NodeBoolLogicalBin {
    NodeExpr* lhs;
    NodeExpr* rhs;
    enum op {AND, OR} type;
};

struct NodeBoolLogicalUnary {
    NodeExpr* expr;
    enum op {NOT} type;
};

struct NodeBinExpr {
    std::variant<NodeBinary*> var;
};

struct NodeBoolExpr {
    std::variant<NodeBoolComparator*, NodeBoolLogicalBin*, NodeBoolLogicalUnary*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeBinExpr*, NodeTerm*, NodeBoolExpr*> expr;
};

struct NodeStmtSelect {
    std::vector<NodeExpr*> view_columns;
    std::string table;
    std::optional<NodeExpr*> where;
    std::optional<NodeTermIdent*> order_by;
};

struct NodeStmtUpdate {
    NodeExpr* where;
    std::string table;
    //TODO: Change the value of this map to DataItem later
    std::map<std::string, std::string> new_column_values;

};



struct NodeStmtDelete {
    std::string table;
    std::optional<NodeExpr*> where;
};

struct NodeStmt {
    std::variant<NodeStmtSelect*, NodeStmtUpdate*, NodeStmtDelete*> var;
};



class Parser {
    public:

    explicit Parser(std::vector<Token> tokens, ArenaAllocator* alloc);

    std::optional<NodeTerm*> parse_term();

    std::optional<NodeStmt*> parse_stmt();

    std::optional<NodeExpr*> parse_expr(int min_prec, bool expect_bool);

    Token consume();

    inline void skip(int num_tokens = 1);

    inline Token try_consume(TokenType type, const std::string& err_msg);

    inline std::optional<Token> try_consume(TokenType type);


    [[nodiscard]] inline std::optional<Token> peek(int offset = 0);
    
    private:
    
    std::vector<Token> m_tokens;
    
    unsigned int m_index;
    
    ArenaAllocator* m_allocator;
};
