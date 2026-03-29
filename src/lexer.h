#pragma once
#include <string>
#include <vector>
#include <stdexcept>

// ============================================================
// Token Tipleri
// ============================================================
enum class TokenType {
    // Anahtar kelimeler
    TOKEN_DEGISKEN,    // degisken (var)
    TOKEN_EGER,        // eger (if)
    TOKEN_IKEN,        // iken (while)
    TOKEN_ISE,         // ise (then / scope baslangici)
    TOKEN_YAZ,         // yaz (print)

    // Literaller
    TOKEN_NUMBER,      // 42, 3.14
    TOKEN_STRING,      // "hello &x world"

    // Tanimlayici
    TOKEN_IDENT,       // degisken adi

    // Operatorler
    TOKEN_PLUS,        // +
    TOKEN_MINUS,       // -
    TOKEN_STAR,        // *
    TOKEN_SLASH,       // /
    TOKEN_ASSIGN,      // =
    TOKEN_EQ,          // ==
    TOKEN_NEQ,         // !=
    TOKEN_LT,          // <
    TOKEN_GT,          // >

    // Grouping & Delimiters
    TOKEN_BANG,        // ! (grouping operatoru)
    TOKEN_LPAREN,      // (
    TOKEN_RPAREN,      // )
    TOKEN_SEMICOLON,   // é veya ;

    // Ozel
    TOKEN_EOF          // Dosya sonu
};

// ============================================================
// String Interpolation icin parca yapisi
// ============================================================
struct StringPart {
    bool isVariable;       // true ise degisken referansi, false ise literal
    std::string value;     // literal metin veya degisken adi
};

// ============================================================
// Token Yapisi
// ============================================================
struct Token {
    TokenType type;
    std::string value;                 // token'in ham degeri
    std::vector<StringPart> stringParts; // sadece TOKEN_STRING icin
    int line;                          // kaynak koddaki satir numarasi

    Token(TokenType t, const std::string& v, int l)
        : type(t), value(v), line(l) {}

    Token(TokenType t, const std::string& v, int l, const std::vector<StringPart>& parts)
        : type(t), value(v), line(l), stringParts(parts) {}
};

// ============================================================
// Lexer Sinifi
// ============================================================
class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t pos_;
    int line_;

    char current() const;
    char peek() const;
    void advance();
    void skipWhitespace();
    bool isAtEnd() const;
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;

    Token makeNumber();
    Token makeIdentifierOrKeyword();
    Token makeString();
    bool matchUTF8_e_acute();  // é karakterini tanir
};
