#include "lexer.h"
#include <iostream>
#include <sstream>

// ============================================================
// Constructor
// ============================================================
Lexer::Lexer(const std::string& source)
    : source_(source), pos_(0), line_(1) {}

// ============================================================
// Yardimci metodlar
// ============================================================
char Lexer::current() const {
    if (isAtEnd()) return '\0';
    return source_[pos_];
}

char Lexer::peek() const {
    if (pos_ + 1 >= source_.size()) return '\0';
    return source_[pos_ + 1];
}

void Lexer::advance() {
    if (current() == '\n') line_++;
    pos_++;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = current();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

bool Lexer::isAtEnd() const {
    return pos_ >= source_.size();
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

// ============================================================
// é (e-acute) UTF-8 eslesmesi: 0xC3 0xA9
// ============================================================
bool Lexer::matchUTF8_e_acute() {
    if (pos_ + 1 < source_.size()) {
        unsigned char c1 = static_cast<unsigned char>(source_[pos_]);
        unsigned char c2 = static_cast<unsigned char>(source_[pos_ + 1]);
        if (c1 == 0xC3 && c2 == 0xA9) {
            // é bulundu — satir sonu isareti
            if (source_[pos_] == '\n') line_++;
            pos_ += 2;
            return true;
        }
    }
    return false;
}

// ============================================================
// Sayi Tokenize
// ============================================================
Token Lexer::makeNumber() {
    int startLine = line_;
    std::string num;
    while (!isAtEnd() && isDigit(current())) {
        num += current();
        advance();
    }
    // Ondalikli sayi destegi
    if (!isAtEnd() && current() == '.' && isDigit(peek())) {
        num += current();
        advance();
        while (!isAtEnd() && isDigit(current())) {
            num += current();
            advance();
        }
    }
    return Token(TokenType::TOKEN_NUMBER, num, startLine);
}

// ============================================================
// Identifier veya Keyword Tokenize
// ============================================================
Token Lexer::makeIdentifierOrKeyword() {
    int startLine = line_;
    std::string ident;
    while (!isAtEnd() && isAlphaNumeric(current())) {
        ident += current();
        advance();
    }

    // Anahtar kelime kontrolu
    if (ident == "degisken") return Token(TokenType::TOKEN_DEGISKEN, ident, startLine);
    if (ident == "eger")     return Token(TokenType::TOKEN_EGER, ident, startLine);
    if (ident == "iken")     return Token(TokenType::TOKEN_IKEN, ident, startLine);
    if (ident == "ise")      return Token(TokenType::TOKEN_ISE, ident, startLine);
    if (ident == "yaz")      return Token(TokenType::TOKEN_YAZ, ident, startLine);

    return Token(TokenType::TOKEN_IDENT, ident, startLine);
}

// ============================================================
// String Tokenize — String Interpolation ile
// String icinde ! karakteri NORMAL karakterdir.
// &degiskenAdi formatinda interpolation yapilir.
// ============================================================
Token Lexer::makeString() {
    int startLine = line_;
    advance(); // Acilis " tüket

    std::vector<StringPart> parts;
    std::string currentLiteral;

    while (!isAtEnd() && current() != '"') {
        if (current() == '&') {
            // String interpolation: &degiskenAdi
            // Onceki literal parcayi kaydet
            if (!currentLiteral.empty()) {
                parts.push_back({false, currentLiteral});
                currentLiteral.clear();
            }

            advance(); // & tüket

            // Degisken adini oku
            std::string varName;
            while (!isAtEnd() && current() != '"' && isAlphaNumeric(current())) {
                varName += current();
                advance();
            }

            if (!varName.empty()) {
                parts.push_back({true, varName});
            }
        } else if (current() == '\\') {
            // Escape sequence
            advance();
            if (!isAtEnd()) {
                switch (current()) {
                    case 'n':  currentLiteral += '\n'; break;
                    case 't':  currentLiteral += '\t'; break;
                    case '"':  currentLiteral += '"';  break;
                    case '\\': currentLiteral += '\\'; break;
                    case '&':  currentLiteral += '&';  break;
                    default:   currentLiteral += current(); break;
                }
                advance();
            }
        } else {
            // Normal karakter (! dahil — string icinde ! ozel anlam tasimaz)
            currentLiteral += current();
            advance();
        }
    }

    if (isAtEnd()) {
        throw std::runtime_error("Satir " + std::to_string(startLine) +
            ": Kapanmamis string literali");
    }

    advance(); // Kapanis " tüket

    // Kalan literal parcayi ekle
    if (!currentLiteral.empty()) {
        parts.push_back({false, currentLiteral});
    }

    // Eger sadece tek bir literal parca varsa, basit string
    std::string fullValue;
    for (auto& p : parts) {
        if (p.isVariable) fullValue += "&" + p.value;
        else fullValue += p.value;
    }

    return Token(TokenType::TOKEN_STRING, fullValue, startLine, parts);
}

// ============================================================
// Ana Tokenize Fonksiyonu
// ============================================================
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        // --- é (UTF-8: 0xC3 0xA9) kontrolu ---
        if (matchUTF8_e_acute()) {
            tokens.push_back(Token(TokenType::TOKEN_SEMICOLON, "é", line_));
            continue;
        }

        char c = current();

        // --- Tek karakterli tokenlar ---
        switch (c) {
            case ';':
                tokens.push_back(Token(TokenType::TOKEN_SEMICOLON, ";", line_));
                advance();
                continue;
            case '+':
                tokens.push_back(Token(TokenType::TOKEN_PLUS, "+", line_));
                advance();
                continue;
            case '-':
                tokens.push_back(Token(TokenType::TOKEN_MINUS, "-", line_));
                advance();
                continue;
            case '*':
                tokens.push_back(Token(TokenType::TOKEN_STAR, "*", line_));
                advance();
                continue;
            case '/':
                tokens.push_back(Token(TokenType::TOKEN_SLASH, "/", line_));
                advance();
                continue;
            case '(':
                tokens.push_back(Token(TokenType::TOKEN_LPAREN, "(", line_));
                advance();
                continue;
            case ')':
                tokens.push_back(Token(TokenType::TOKEN_RPAREN, ")", line_));
                advance();
                continue;
            case '<':
                tokens.push_back(Token(TokenType::TOKEN_LT, "<", line_));
                advance();
                continue;
            case '>':
                tokens.push_back(Token(TokenType::TOKEN_GT, ">", line_));
                advance();
                continue;
            default:
                break;
        }

        // --- Cift karakterli operatorler ---
        if (c == '=' && peek() == '=') {
            tokens.push_back(Token(TokenType::TOKEN_EQ, "==", line_));
            advance(); advance();
            continue;
        }
        if (c == '=') {
            tokens.push_back(Token(TokenType::TOKEN_ASSIGN, "=", line_));
            advance();
            continue;
        }

        // --- ! karakteri: != veya grouping ---
        if (c == '!') {
            if (peek() == '=') {
                // != operatoru
                tokens.push_back(Token(TokenType::TOKEN_NEQ, "!=", line_));
                advance(); advance();
                continue;
            } else {
                // Grouping operatoru
                tokens.push_back(Token(TokenType::TOKEN_BANG, "!", line_));
                advance();
                continue;
            }
        }

        // --- String literal ---
        if (c == '"') {
            tokens.push_back(makeString());
            continue;
        }

        // --- Sayi ---
        if (isDigit(c)) {
            tokens.push_back(makeNumber());
            continue;
        }

        // --- Identifier veya Keyword ---
        if (isAlpha(c)) {
            tokens.push_back(makeIdentifierOrKeyword());
            continue;
        }

        // --- Bilinmeyen karakter ---
        throw std::runtime_error("Satir " + std::to_string(line_) +
            ": Bilinmeyen karakter '" + std::string(1, c) + "' (0x" +
            std::to_string(static_cast<int>(static_cast<unsigned char>(c))) + ")");
    }

    tokens.push_back(Token(TokenType::TOKEN_EOF, "", line_));
    return tokens;
}
