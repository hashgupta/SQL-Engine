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

void Parser::skip(int num_tokens) {
    m_index += num_tokens;
}

Token Parser::try_consume(TokenType type, const std::string& err_msg){
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
    auto expr_lhs = this->m_allocator->emplace<NodeExpr>(term_lhs.value());
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
        
        auto bin_expr = m_allocator->emplace<NodeBinExpr>();
        auto bool_expr = m_allocator->emplace<NodeBoolExpr>();


        auto expr_lhs2 = m_allocator->emplace<NodeExpr>(expr_lhs->expr);

        bool is_bool = false;
        
        if (op.type == TokenType::plus) {
            auto add = m_allocator->emplace<NodeBinary>();
            add->type = NodeBinary::op::ADD;
            add->lhs = expr_lhs2;
            add->rhs = expr_rhs.value();
            
            bin_expr->var = add;
        }
        else if (op.type == TokenType::star) {
            auto multi = m_allocator->emplace<NodeBinary>();
            multi->type = NodeBinary::op::MUL;
            multi->lhs = expr_lhs2;
            multi->rhs = expr_rhs.value();

            bin_expr->var = multi;
        }
        else if (op.type == TokenType::minus) {
            auto sub = m_allocator->emplace<NodeBinary>();
            sub->type = NodeBinary::op::SUB;
            sub->lhs = expr_lhs2;
            sub->rhs = expr_rhs.value();
            
            bin_expr->var = sub;
        }
        else if (op.type == TokenType::fslash) {
            auto div = m_allocator->emplace<NodeBinary>();
            div->type = NodeBinary::op::DIV;
            div->lhs = expr_lhs2;
            div->rhs = expr_rhs.value();
            
            bin_expr->var = div;
        } else {
            is_bool = true;
        }

        
        
        if (op.type == TokenType::gt) {
            auto gt = m_allocator->emplace<NodeBoolComparator>();
            gt->type = NodeBoolComparator::op::GT;
            gt->lhs = expr_lhs2;
            gt->rhs = expr_rhs.value();
            bool_expr->var = gt;
        } else if (op.type == TokenType::gte) {
            auto gte = m_allocator->emplace<NodeBoolComparator>();
            gte->type = NodeBoolComparator::op::GTE;
            gte->lhs = expr_lhs2;
            gte->rhs = expr_rhs.value();
            bool_expr->var = gte;
        } else if (op.type == TokenType::lt) {
            auto lt = m_allocator->emplace<NodeBoolComparator>();
            lt->type = NodeBoolComparator::op::LT;
            lt->lhs = expr_lhs2;
            lt->rhs = expr_rhs.value();
            bool_expr->var = lt;
        } else if (op.type == TokenType::lte) {
            auto lte = m_allocator->emplace<NodeBoolComparator>();
            lte->type = NodeBoolComparator::op::LTE;
            lte->lhs = expr_lhs2;
            lte->rhs = expr_rhs.value();
            bool_expr->var = lte;
        
        } else if (op.type == TokenType::and_) {
            auto and_ = m_allocator->emplace<NodeBoolLogicalBin>();
            and_->lhs = expr_lhs2;
            and_->rhs = expr_rhs.value();
            bool_expr->var = and_;
        } else if (op.type == TokenType::or_) {
            auto or_ = m_allocator->emplace<NodeBoolLogicalBin>();
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
        //     auto not_ = m_allocator->emplace<NodeBoolExprLTE>();
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
        auto term_int_lit = m_allocator->emplace<NodeTermIntLit>();
        term_int_lit->int_lit = int_lit.value();
        auto term = m_allocator->emplace<NodeTerm>();
        term->var = term_int_lit;
        return term;
    }
    else if (auto ident = try_consume(TokenType::ident)) {
        auto expr_ident = m_allocator->emplace<NodeTermIdent>();
        expr_ident->ident = ident.value();
        auto term = m_allocator->emplace<NodeTerm>();
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
        auto term_paren = m_allocator->emplace<NodeTermParen>();
        term_paren->expr = expr.value();
        auto term = m_allocator->emplace<NodeTerm>();
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
        skip();
        auto stmt_select = m_allocator->emplace<NodeStmtSelect>();
        stmt_select->view_columns = std::vector<NodeExpr*>();

        while (true) {
            if (auto expression = parse_expr()) {

                auto expr_ident = m_allocator->emplace<NodeExpr>();
                expr_ident = expression.value();
                stmt_select->view_columns.push_back(expr_ident);
            }
            if (peek().has_value() && peek().value().type == TokenType::comma) {
                skip();
            } else {
                break;
            }

        }
        
        try_consume(TokenType::from, "Expected `from`");
        
        Token ident_token = try_consume(TokenType::ident, "Expected Table Name");
        
        stmt_select->table = ident_token.force_value();

        if (peek().has_value() && peek().value().type == TokenType::where) {
            skip();
            std::optional<NodeExpr *> expression = parse_expr();
            if (expression.has_value()) {
                stmt_select->where = expression;
            }
        }

        
        try_consume(TokenType::semi, "Expected `;`");
        
        auto stmt = m_allocator->emplace<NodeStmt>();
        stmt->var = stmt_select;
        return stmt;
    } else if (peek().has_value() && peek().value().type == TokenType::delete_) {
        skip();
        auto stmt_delete = m_allocator->emplace<NodeStmtDelete>();
        try_consume(TokenType::from, "Expected `from`");
        
        auto ident_token = try_consume(TokenType::ident, "Expected Table Name");
        
        stmt_delete->table = ident_token.force_value();
        
        if (peek().has_value() && peek().value().type == TokenType::where) {
            skip();
            std::optional<NodeExpr *> expression = parse_expr();
            if (expression.has_value()) {
                stmt_delete->where = expression;
            }
        }
        try_consume(TokenType::semi, "Expected `;`");
        auto stmt = m_allocator->emplace<NodeStmt>();
        stmt->var = stmt_delete;
        return stmt;
    
    } else if (peek().has_value() && peek().value().type == TokenType::update_) {
        skip();
        auto stmt_update = m_allocator->emplace<NodeStmtUpdate>();
        auto ident_token = try_consume(TokenType::ident, "Expected Table Name");
        
        stmt_update->table = ident_token.force_value();

        while (true) {
            Token col_name = try_consume(TokenType::ident, "Expected Column Name");
            
            try_consume(TokenType::equal, "Expected `=`");
            
            Token new_value = try_consume(TokenType::ident, "Expected Column Value");
            
            stmt_update->new_column_values[col_name.force_value()] = new_value.force_value();
            
            if (peek().has_value() && peek().value().type == TokenType::comma) {
                skip();
            } else {
                break;
            }
        }



        if (peek().has_value() && peek().value().type == TokenType::where) {
            skip();
            std::optional<NodeExpr *> expression = parse_expr();
            if (expression.has_value()) {
                stmt_update->where = expression.value();
            }
        }
        try_consume(TokenType::semi, "Expected `;`");


        
        return {};
    } else {
        return {};
    }
}



