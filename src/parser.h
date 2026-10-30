#pragma once
#include <vector>
#include "lexer.h"
#include "ast.h"

// ============================================================
// Recursive Descent Parser
// ============================================================
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::unique_ptr<ProgramNode> parse();

private:
    std::vector<Token> tokens_;
    size_t pos_;
    int nesting_ = 0;  // ic ice blok ve !...! sayisi

    // Derleyici, AST'yi ozyinelemeli dolasir; sinirsiz derinlik yigini
    // tasirip programi cokertirdi. Bu sinirlar gercek programlarin cok ustunde.
    static constexpr int MAX_NESTING = 200;
    static constexpr int MAX_DEPTH   = 1000;
    void enterNesting(int line);
    ASTNodePtr makeBinary(const std::string& op, ASTNodePtr l, ASTNodePtr r, int line);

    // Token yonetimi
    const Token& current() const;
    const Token& peek() const;
    void advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token expect(TokenType type, const std::string& message);

    // Gramer kurallari
    std::unique_ptr<ProgramNode> parseProgram();
    ASTNodePtr parseStatement();
    ASTNodePtr parseVarDecl();
    ASTNodePtr parseAssignmentOrExprStmt();
    ASTNodePtr parsePrintStmt();
    ASTNodePtr parseIfStmt();
    ASTNodePtr parseWhileStmt();
    std::vector<ASTNodePtr> parseBlock();

    // Ifade ayristirma (operator onceligi ile)
    ASTNodePtr parseExpression();
    ASTNodePtr parseComparison();
    ASTNodePtr parseAddSub();
    ASTNodePtr parseTerm();
    ASTNodePtr parseFactor();

    // Satir sonu tüketme
    void expectLineEnd();
};
