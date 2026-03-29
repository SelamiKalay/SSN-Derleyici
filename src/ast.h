#pragma once
#include <string>
#include <vector>
#include <memory>
#include "lexer.h"

// ============================================================
// AST Node Taban Sinifi
// ============================================================
struct ASTNode {
    virtual ~ASTNode() = default;
    int line = 0; // Hata ayiklama icin satir bilgisi
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

// ============================================================
// Expression Node'lari
// ============================================================

// Sayisal literal: 42, 3.14
struct NumberNode : ASTNode {
    double value;
    explicit NumberNode(double v, int l = 0) : value(v) { line = l; }
};

// String literal (interpolation parçalari ile)
struct StringNode : ASTNode {
    std::vector<StringPart> parts;
    explicit StringNode(const std::vector<StringPart>& p, int l = 0) : parts(p) { line = l; }
};

// Degisken referansi: x, toplam
struct IdentifierNode : ASTNode {
    std::string name;
    explicit IdentifierNode(const std::string& n, int l = 0) : name(n) { line = l; }
};

// Ikili operator: +, -, *, /, <, >, ==, !=
// NOT: Grouping (!) icin ayri bir node YOKTUR.
// Parser !expr! gordugunde parseExpression() cagirip sonucu dogrudan dondurur.
// Bu, C'deki (expr) parantezinin AST'de ayri node olmamasi ile ayni mantiktir.
struct BinaryOpNode : ASTNode {
    std::string op;     // "+", "-", "*", "/", "<", ">", "==", "!="
    ASTNodePtr left;
    ASTNodePtr right;
    BinaryOpNode(const std::string& o, ASTNodePtr l, ASTNodePtr r, int ln = 0)
        : op(o), left(std::move(l)), right(std::move(r)) { line = ln; }
};

// ============================================================
// Statement Node'lari
// ============================================================

// degisken x = expr
struct VarDeclNode : ASTNode {
    std::string name;
    ASTNodePtr expr;
    VarDeclNode(const std::string& n, ASTNodePtr e, int l = 0)
        : name(n), expr(std::move(e)) { line = l; }
};

// x = expr (atama)
struct AssignNode : ASTNode {
    std::string name;
    ASTNodePtr expr;
    AssignNode(const std::string& n, ASTNodePtr e, int l = 0)
        : name(n), expr(std::move(e)) { line = l; }
};

// yaz expr
struct PrintNode : ASTNode {
    ASTNodePtr expr;
    explicit PrintNode(ASTNodePtr e, int l = 0) : expr(std::move(e)) { line = l; }
};

// eger condition ise ( body... )
struct IfNode : ASTNode {
    ASTNodePtr condition;
    std::vector<ASTNodePtr> body;
    IfNode(ASTNodePtr cond, std::vector<ASTNodePtr> b, int l = 0)
        : condition(std::move(cond)), body(std::move(b)) { line = l; }
};

// iken condition ise ( body... )
struct WhileNode : ASTNode {
    ASTNodePtr condition;
    std::vector<ASTNodePtr> body;
    WhileNode(ASTNodePtr cond, std::vector<ASTNodePtr> b, int l = 0)
        : condition(std::move(cond)), body(std::move(b)) { line = l; }
};

// ============================================================
// Program Koku
// ============================================================
struct ProgramNode : ASTNode {
    std::vector<ASTNodePtr> statements;
};
