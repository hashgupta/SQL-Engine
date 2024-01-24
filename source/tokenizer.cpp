#include <sql-engine/tokenizer.h>
#include <iostream>
#include <vector>

std::optional<int> bin_prec(TokenType type) {
    switch (type) {
    case TokenType::minus:
    case TokenType::plus:
    case TokenType::gt:
    case TokenType::lt:
    case TokenType::gte:
    case TokenType::lte:
    case TokenType::and_:
    case TokenType::or_:
        return 0;
    case TokenType::fslash:
    case TokenType::star:
        return 1;
    default:
        return {};
    }
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    std::string buf;
    while (peek().has_value()) {
        if (std::isalpha(peek().value())) {
            buf.push_back(consume());
            while (peek().has_value() && std::isalnum(peek().value())) {
                buf.push_back(consume());
            }
            if (buf == "from") {
                tokens.push_back({ .type = TokenType::from });
                buf.clear();
            }
            else if (buf == "order") {
                tokens.push_back({ .type = TokenType::order });
                buf.clear();
            }
            else if (buf == "by") {
                tokens.push_back({ .type = TokenType::by });
                buf.clear();
            }
            else if (buf == "where") {
                tokens.push_back({ .type = TokenType::where });
                buf.clear();
            }
            else if (buf == "select") {
                tokens.push_back({ .type = TokenType::select });
                buf.clear();
            }
            else if (buf == "and") {
                tokens.push_back({ .type = TokenType::and_ });
                buf.clear();
            }
            else if (buf == "or") {
                tokens.push_back({ .type = TokenType::or_ });
                buf.clear();
            }
            else if (buf == "delete") {
                tokens.push_back({ .type = TokenType::delete_ });
                buf.clear();
            }
            else if (buf == "having") {
                tokens.push_back({ .type = TokenType::having });
                buf.clear();
            }
            else if (buf == "update") {
                tokens.push_back({ .type = TokenType::update_ });
                buf.clear();
            }
            else {
                tokens.push_back({ .type = TokenType::ident, .value = buf });
                buf.clear();
            }
        }
        else if (std::isdigit(peek().value())) {
            buf.push_back(consume());
            while (peek().has_value() && std::isdigit(peek().value())) {
                buf.push_back(consume());
            }
            tokens.push_back({ .type = TokenType::int_lit, .value = buf });
            buf.clear();
        }
        else if (peek().value() == '(') {
            consume();
            tokens.push_back({ .type = TokenType::open_paren });
        }
        else if (peek().value() == ')') {
            consume();
            tokens.push_back({ .type = TokenType::close_paren });
        }
        else if (peek().value() == '>') {
            if (peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ .type = TokenType::gte, .value = "gte" });
            } else {
                consume();
                tokens.push_back({ .type = TokenType::gt, .value = "gt" });
            }
        }
        else if (peek().value() == '<') {
            if (peek(1).value() == '=') {
                consume();
                consume();
                tokens.push_back({ .type = TokenType::lte, .value = "lte" });
            } else {
                consume();
                tokens.push_back({ .type = TokenType::lt, .value = "lt" });
            }
        }
        else if (peek().value() == ';') {
            consume();
            tokens.push_back({ .type = TokenType::semi, .value = "semi" });
        }
        else if (peek().value() == '+') {
            consume();
            tokens.push_back({ .type = TokenType::plus });
        }
        else if (peek().value() == '*') {
            consume();
            tokens.push_back({ .type = TokenType::star });
        }
        else if (peek().value() == '-') {
            consume();
            tokens.push_back({ .type = TokenType::minus });
        }
        else if (peek().value() == '/') {
            consume();
            tokens.push_back({ .type = TokenType::fslash });
        }
        else if (peek().value() == ',') {
            consume();
            tokens.push_back({ .type = TokenType::comma });
        }
        else if (std::isspace(peek().value())) {
            consume();
        }
        else {
            std::cerr << "You messed up!" << std::endl;
            exit(EXIT_FAILURE);
        }
        }
        m_index = 0;
        return tokens;
};

std::optional<char> Tokenizer::peek(int offset) const
{
    if (m_index + offset >= m_src.length()) {
        return {};
    }
    else {
        return m_src.at(m_index + offset);
    }
};

inline char Tokenizer::consume()
{
    return m_src.at(m_index++);
};


