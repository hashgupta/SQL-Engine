#include <sql-engine/parser.h>
#include <sql-engine/allocator.h>

Parser::Parser(std::vector<Token> tokens, ArenaAllocator* alloc) : m_tokens(std::move(tokens)) // 4 mb
{
    this->m_allocator = alloc;
};

std::optional<Token> Parser::peek(int offset)
{
    if (m_index + offset >= m_tokens.size()) {
        return {};
    }
    else {
        return m_tokens.at(m_index + offset);
    }
}


Token Parser::consume(){
    return m_tokens.at(m_index++);
};

std::optional<Token> Parser::try_consume(TokenType type, const std::string& err_msg){
    if (peek().has_value() && peek().value().type == type) {
        return consume();
    }
    else {
        std::cerr << err_msg << std::endl;
        exit(EXIT_FAILURE);
    }
};

std::optional<Token> Parser::try_consume(TokenType type) {
    if (peek().has_value() && peek().value().type == type) {
        return consume();
    }
    else {
        return {};
    }
};

std::optional<NodeExpr*> Parser::parse_expr(int min_prec = 0, bool expect_bool = false) {
    std::optional<NodeTerm*> term_lhs = this->parse_term();

    if (!term_lhs.has_value()) {
        return {};
    }
    auto expr_lhs = this->m_allocator->alloc<NodeExpr>();
    expr_lhs->expr = term_lhs.value();

    while (true) {
        std::optional<Token> curr_tok = peek();
        std::optional<int> prec;
        if (curr_tok.has_value()) {
            prec = bin_prec(curr_tok->type);
            if (!prec.has_value() || prec < min_prec) {
                break;
            }
        }
        else {
            break;
        }
        
        Token op = consume();
        int next_min_prec = prec.value() + 1;
        auto expr_rhs = parse_expr(next_min_prec);
        
        if (!expr_rhs.has_value()) {
            std::cerr << "Unable to parse expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        auto bin_expr = m_allocator->alloc<NodeBinExpr>();
        auto bool_expr = m_allocator->alloc<NodeBoolExpr>();

        auto expr_lhs2 = m_allocator->alloc<NodeExpr>();
        expr_lhs2->expr = expr_lhs->expr;

        bool is_bool = false;
        
        if (op.type == TokenType::plus) {
            auto add = m_allocator->alloc<NodeBinExprAdd>();
            add->lhs = expr_lhs2;
            add->rhs = expr_rhs.value();
            bin_expr->var = add;
        }
        else if (op.type == TokenType::star) {
            auto multi = m_allocator->alloc<NodeBinExprMulti>();
            multi->lhs = expr_lhs2;
            multi->rhs = expr_rhs.value();
            bin_expr->var = multi;
        }
        else if (op.type == TokenType::minus) {
            auto sub = m_allocator->alloc<NodeBinExprSub>();
            sub->lhs = expr_lhs2;
            sub->rhs = expr_rhs.value();
            bin_expr->var = sub;
        }
        else if (op.type == TokenType::fslash) {
            auto div = m_allocator->alloc<NodeBinExprDiv>();
            div->lhs = expr_lhs2;
            div->rhs = expr_rhs.value();
            bin_expr->var = div;
        } else {
            is_bool = true;
        }

        
        
        if (op.type == TokenType::gt) {
            auto gt = m_allocator->alloc<NodeBoolExprGT>();
            gt->lhs = expr_lhs2;
            gt->rhs = expr_rhs.value();
            bool_expr->var = gt;
        } else if (op.type == TokenType::gte) {
            auto gte = m_allocator->alloc<NodeBoolExprGTE>();
            gte->lhs = expr_lhs2;
            gte->rhs = expr_rhs.value();
            bool_expr->var = gte;
        } else if (op.type == TokenType::lt) {
            auto lt = m_allocator->alloc<NodeBoolExprLT>();
            lt->lhs = expr_lhs2;
            lt->rhs = expr_rhs.value();
            bool_expr->var = lt;
        } else if (op.type == TokenType::lte) {
            auto lte = m_allocator->alloc<NodeBoolExprLTE>();
            lte->lhs = expr_lhs2;
            lte->rhs = expr_rhs.value();
            bool_expr->var = lte;
        
        } else if (op.type == TokenType::and_) {
            auto and_ = m_allocator->alloc<NodeBoolExprAND>();
            and_->lhs = expr_lhs2;
            and_->rhs = expr_rhs.value();
            bool_expr->var = and_;
        } else if (op.type == TokenType::or_) {
            auto or_ = m_allocator->alloc<NodeBoolExprOR>();
            or_->lhs = expr_lhs2;
            or_->rhs = expr_rhs.value();
            bool_expr->var = or_;
        }
        else {
            if (is_bool) {
                assert(false); // Unreachable;
            }
        }
        // } else if (op.type == TokenType::not_) {
        //     auto not_ = m_allocator->alloc<NodeBoolExprLTE>();
        //     not_->lhs = expr_lhs2;
        //     not_->rhs = expr_rhs.value();
        //     bool_expr->var = not_;
        // }

        if (is_bool) {
            expr_lhs->expr = bool_expr;
        } else {
            expr_lhs->expr = bin_expr;
        }
    }
    return expr_lhs;
}

std::optional<NodeTerm*> Parser::parse_term()
{
    if (auto int_lit = try_consume(TokenType::int_lit)) {
        auto term_int_lit = m_allocator->alloc<NodeTermIntLit>();
        term_int_lit->int_lit = int_lit.value();
        auto term = m_allocator->alloc<NodeTerm>();
        term->var = term_int_lit;
        return term;
    }
    else if (auto ident = try_consume(TokenType::ident)) {
        auto expr_ident = m_allocator->alloc<NodeTermIdent>();
        expr_ident->ident = ident.value();
        auto term = m_allocator->alloc<NodeTerm>();
        term->var = expr_ident;
        return term;
    }
    else if (auto open_paren = try_consume(TokenType::open_paren)) {
        auto expr = parse_expr();
        if (!expr.has_value()) {
            std::cerr << "Expected expression" << std::endl;
            exit(EXIT_FAILURE);
        }
        try_consume(TokenType::close_paren, "Expected `)`");
        auto term_paren = m_allocator->alloc<NodeTermParen>();
        term_paren->expr = expr.value();
        auto term = m_allocator->alloc<NodeTerm>();
        term->var = term_paren;
        return term;
    }
    else {
        return {};
    }
}

std::optional<NodeStmt*> Parser::parse_stmt()
{

    if (peek().has_value() && peek().value().type == TokenType::select) {
        consume();
        auto stmt_select = m_allocator->alloc<NodeStmtSelect>();
        stmt_select->view_columns = std::vector<NodeExpr*>();

        while (true) {
            if (auto expression = parse_expr()) {

                auto expr_ident = m_allocator->alloc<NodeExpr>();
                expr_ident = expression.value();
                stmt_select->view_columns.push_back(expr_ident);
            }
            if (peek().has_value() && peek().value().type == TokenType::comma) {
                consume();
            } else {
                break;
            }

        }
        
        try_consume(TokenType::from, "Expected `from`");
        
        auto ident_token = try_consume(TokenType::ident, "Expected Table Name");
        auto table = m_allocator->alloc<NodeTermIdent>();
        table->ident = ident_token.value();
        
        stmt_select->table = table;

        if (peek().has_value() && peek().value().type == TokenType::where) {
            consume();
            std::optional<NodeExpr *> expression = parse_expr();
            if (expression.has_value()) {
                stmt_select->where = expression;
            }
        }

        
        try_consume(TokenType::semi, "Expected `;`");
        
        auto stmt = m_allocator->alloc<NodeStmt>();
        stmt->var = stmt_select;
        return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::delete_) {
        consume();
        auto stmt_delete = m_allocator->alloc<NodeStmtDelete>();
        try_consume(TokenType::from, "Expected `from`");
        auto ident_token = try_consume(TokenType::ident, "Expected Table Name");
        auto table = m_allocator->alloc<NodeTermIdent>();
        table->ident = ident_token.value();
        stmt_delete->table = table;
        if (peek().has_value() && peek().value().type == TokenType::where) {
            consume();
            std::optional<NodeExpr *> expression = parse_expr();
            if (expression.has_value()) {
                stmt_delete->condition = expression;
            }
        }
        try_consume(TokenType::semi, "Expected `;`");
        auto stmt = m_allocator->alloc<NodeStmt>();
        stmt->var = stmt_delete;
        return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::update_) {
        return {};
    } else {
        return {};
    }
}



