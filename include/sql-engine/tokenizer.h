#pragma once
#include <optional>
#include <string>

enum class TokenType {
    int_lit,
    semi,
    open_paren,
    close_paren,
    ident,
    as_,
    fslash,
    plus,
    star,
    minus,
    select,
    from,
    where,
    order,
    by,
    and_,
    or_,
    delete_,
    having,
    update_,
    comma,
    gt,
    lt,
    gte,
    lte,
};


struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

std::optional<int> bin_prec(TokenType type);

class Tokenizer {
public:
    inline explicit Tokenizer(std::string src)
        : m_src(std::move(src))
    {
    }

    std::vector<Token> tokenize();

    private:
    [[nodiscard]] inline std::optional<char> peek(int offset = 0) const;

    inline char consume();

    const std::string m_src;
    size_t m_index = 0;
};