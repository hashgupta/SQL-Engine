#pragma once

#include <sql-engine/allocator.h>
#include <vector>
#include <iostream>
#include <sql-engine/tokenizer.h>
#include <optional>
#include <variant>

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

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprGT {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprLT {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprGTE {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprLTE {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprAND {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprOR {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBoolExprNOT {
    NodeExpr* expr;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprMulti*, NodeBinExprSub*, NodeBinExprDiv*> var;
};

struct NodeBoolExpr {
    std::variant<NodeBoolExprGT*, NodeBoolExprLT*, NodeBoolExprGTE*, NodeBoolExprLTE*, NodeBoolExprAND*, NodeBoolExprOR*, NodeBoolExprNOT*> var;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeBinExpr*, NodeTerm*, NodeBoolExpr*> expr;
};

struct NodeStmtSelect {
    //std::vector<NodeExpr*> columns;
    std::vector<NodeExpr*> view_columns;
    NodeTermIdent* table;
    std::optional<NodeExpr*> where;
    std::optional<NodeTermIdent*> order_by;
};

struct NodeStmtUpdate {
    std::vector<NodeExpr*> columns;
    NodeTermIdent* table;

};



struct NodeStmtDelete {
    NodeTermIdent* table;
    std::optional<NodeExpr*> condition;
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

    inline std::optional<Token> try_consume(TokenType type, const std::string& err_msg);

    inline std::optional<Token> try_consume(TokenType type);


    [[nodiscard]] inline std::optional<Token> peek(int offset = 0);
    
    private:
    
    std::vector<Token> m_tokens;
    
    unsigned int m_index;
    
    ArenaAllocator* m_allocator;
};
