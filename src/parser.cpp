#include "parser.h"
#include <stdexcept>
#include <sstream>

// ============================================================
// Constructor
// ============================================================
Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), pos_(0) {}

// ============================================================
// Token Yonetimi
// ============================================================
const Token& Parser::current() const {
    return tokens_[pos_];
}

const Token& Parser::peek() const {
    if (pos_ + 1 < tokens_.size()) return tokens_[pos_ + 1];
    return tokens_.back(); // EOF
}

void Parser::advance() {
    if (pos_ < tokens_.size() - 1) pos_++;
}

bool Parser::check(TokenType type) const {
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        Token tok = current();
        advance();
        return tok;
    }
    throw std::runtime_error("Satir " + std::to_string(current().line) +
        ": " + message + " ('" + current().value + "' bulundu)");
}

// ============================================================
// Satir Sonu Tüketme
// ============================================================
void Parser::expectLineEnd() {
    expect(TokenType::TOKEN_SEMICOLON, "Satir sonu (é veya ;) bekleniyor");
}

// ============================================================
// Ana Parse Fonksiyonu
// ============================================================
std::unique_ptr<ProgramNode> Parser::parse() {
    return parseProgram();
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();

    while (!check(TokenType::TOKEN_EOF)) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

// ============================================================
// Statement Ayristirma
// ============================================================
ASTNodePtr Parser::parseStatement() {
    // degisken x = expr é
    if (check(TokenType::TOKEN_DEGISKEN)) {
        return parseVarDecl();
    }

    // yaz expr é
    if (check(TokenType::TOKEN_YAZ)) {
        return parsePrintStmt();
    }

    // eger condition ise ( ... )
    if (check(TokenType::TOKEN_EGER)) {
        return parseIfStmt();
    }

    // iken condition ise ( ... )
    if (check(TokenType::TOKEN_IKEN)) {
        return parseWhileStmt();
    }

    // identifier = expr é  (atama)
    if (check(TokenType::TOKEN_IDENT)) {
        return parseAssignmentOrExprStmt();
    }

    throw std::runtime_error("Satir " + std::to_string(current().line) +
        ": Beklenmeyen token '" + current().value + "'");
}

// ============================================================
// degisken x = expr é
// ============================================================
ASTNodePtr Parser::parseVarDecl() {
    int ln = current().line;
    advance(); // 'degisken' tüket

    Token name = expect(TokenType::TOKEN_IDENT, "Degisken adi bekleniyor");
    expect(TokenType::TOKEN_ASSIGN, "'=' bekleniyor");

    auto expr = parseExpression();
    expectLineEnd();

    return std::make_unique<VarDeclNode>(name.value, std::move(expr), ln);
}

// ============================================================
// x = expr é  (atama)
// ============================================================
ASTNodePtr Parser::parseAssignmentOrExprStmt() {
    int ln = current().line;
    Token name = current();
    advance(); // identifier tüket

    // Atama
    if (check(TokenType::TOKEN_ASSIGN)) {
        advance(); // '=' tüket
        auto expr = parseExpression();
        expectLineEnd();
        return std::make_unique<AssignNode>(name.value, std::move(expr), ln);
    }

    // Eger = degilse, hata
    throw std::runtime_error("Satir " + std::to_string(ln) +
        ": '=' bekleniyor '" + name.value + "' sonrasinda");
}

// ============================================================
// yaz expr é
// ============================================================
ASTNodePtr Parser::parsePrintStmt() {
    int ln = current().line;
    advance(); // 'yaz' tüket

    auto expr = parseExpression();
    expectLineEnd();

    return std::make_unique<PrintNode>(std::move(expr), ln);
}

// ============================================================
// eger condition ise ( body... )
// ============================================================
ASTNodePtr Parser::parseIfStmt() {
    int ln = current().line;
    advance(); // 'eger' tüket

    auto condition = parseComparison();

    expect(TokenType::TOKEN_ISE, "'ise' bekleniyor");

    auto body = parseBlock();

    return std::make_unique<IfNode>(std::move(condition), std::move(body), ln);
}

// ============================================================
// iken condition ise ( body... )
// ============================================================
ASTNodePtr Parser::parseWhileStmt() {
    int ln = current().line;
    advance(); // 'iken' tüket

    auto condition = parseComparison();

    expect(TokenType::TOKEN_ISE, "'ise' bekleniyor");

    auto body = parseBlock();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body), ln);
}

// ============================================================
// Blok Ayristirma: ( statement* )
// ============================================================
std::vector<ASTNodePtr> Parser::parseBlock() {
    expect(TokenType::TOKEN_LPAREN, "'(' bekleniyor (blok baslangici)");

    std::vector<ASTNodePtr> statements;

    while (!check(TokenType::TOKEN_RPAREN) && !check(TokenType::TOKEN_EOF)) {
        statements.push_back(parseStatement());
    }

    expect(TokenType::TOKEN_RPAREN, "')' bekleniyor (blok sonu)");

    return statements;
}

// ============================================================
// Expression Ayristirma (Operator Onceligi)
// ============================================================

// En dusuk oncelik: karsilastirma (<, >, ==, !=)
ASTNodePtr Parser::parseExpression() {
    return parseComparison();
}

// Karsilastirma: expr (<|>|==|!=) expr
ASTNodePtr Parser::parseComparison() {
    auto left = parseAddSub();

    if (check(TokenType::TOKEN_LT) || check(TokenType::TOKEN_GT) ||
        check(TokenType::TOKEN_EQ) || check(TokenType::TOKEN_NEQ)) {
        int ln = current().line;
        std::string op = current().value;
        advance();
        auto right = parseAddSub();
        return std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), ln);
    }

    return left;
}

// Toplama/Cikarma: term ((+|-) term)*
ASTNodePtr Parser::parseAddSub() {
    auto left = parseTerm();

    while (check(TokenType::TOKEN_PLUS) || check(TokenType::TOKEN_MINUS)) {
        int ln = current().line;
        std::string op = current().value;
        advance();
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), ln);
    }

    return left;
}

// Carpma/Bolme: factor ((*|/) factor)*
ASTNodePtr Parser::parseTerm() {
    auto left = parseFactor();

    while (check(TokenType::TOKEN_STAR) || check(TokenType::TOKEN_SLASH)) {
        int ln = current().line;
        std::string op = current().value;
        advance();
        auto right = parseFactor();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), ln);
    }

    return left;
}

// ============================================================
// Factor: NUMBER | STRING | IDENTIFIER | !expression!
//
// KRITIK TASARIM KARARI:
// ! expression ! yapisi icin ayri bir AST node OLUSTURULMAZ.
// parseFactor() ! gordugunde parseExpression() cagirip
// dogrudan sonuc node'unu dondurur.
// Tıpkı C'deki (expr) parantezinin AST'de ayri node olmamasi gibi.
// ============================================================
ASTNodePtr Parser::parseFactor() {
    int ln = current().line;

    // --- Sayisal literal ---
    if (check(TokenType::TOKEN_NUMBER)) {
        double val = std::stod(current().value);
        advance();
        return std::make_unique<NumberNode>(val, ln);
    }

    // --- String literal ---
    if (check(TokenType::TOKEN_STRING)) {
        auto parts = current().stringParts;
        advance();
        return std::make_unique<StringNode>(parts, ln);
    }

    // --- Identifier ---
    if (check(TokenType::TOKEN_IDENT)) {
        std::string name = current().value;
        advance();
        return std::make_unique<IdentifierNode>(name, ln);
    }

    // --- Grouping: ! expression ! ---
    // ! gordugumuzde:
    // 1. Acilis ! tüket
    // 2. parseExpression() cagir (tam ifadeyi ayristir)
    // 3. Kapanis ! bekle ve tüket
    // 4. Sonucu dogrudan dondur (ayri GroupingNode yok)
    if (check(TokenType::TOKEN_BANG)) {
        advance(); // Acilis ! tüket
        auto expr = parseExpression(); // Tam ifadeyi ayristir
        expect(TokenType::TOKEN_BANG, "Kapanis '!' bekleniyor (grouping)");
        return expr; // Dogrudan expression node'unu dondur
    }

    throw std::runtime_error("Satir " + std::to_string(ln) +
        ": Ifade bekleniyor, '" + current().value + "' bulundu");
}
