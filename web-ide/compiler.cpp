// ============================================================
// SSN — Türkçe Sözdizimli Bytecode Compiler & Stack-Based VM
// Tek Dosya Versiyonu (Web IDE İçin)
//
// Derleme: cl /EHsc /utf-8 /O2 compiler.cpp /Fe:compiler.exe
//     veya: g++ -std=c++17 -O2 -o compiler compiler.cpp
//
// Kullanım: compiler <dosya.tc>
//
// Pipeline: Kaynak Kod → Lexer → Parser → AST → Compiler → Bytecode → VM
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <cstdint>
#include <iomanip>
#include <cmath>

// ╔══════════════════════════════════════════════════════════════╗
// ║                    BÖLÜM 1: TOKEN & LEXER                  ║
// ╚══════════════════════════════════════════════════════════════╝

enum class TokenType {
    TOKEN_DEGISKEN, TOKEN_EGER, TOKEN_IKEN, TOKEN_ISE, TOKEN_YAZ,
    TOKEN_NUMBER, TOKEN_STRING, TOKEN_IDENT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_ASSIGN, TOKEN_EQ, TOKEN_NEQ, TOKEN_LT, TOKEN_GT,
    TOKEN_BANG, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMICOLON,
    TOKEN_EOF
};

struct StringPart {
    bool isVariable;
    std::string value;
};

struct Token {
    TokenType type;
    std::string value;
    std::vector<StringPart> stringParts;
    int line;
    Token(TokenType t, const std::string& v, int l) : type(t), value(v), line(l) {}
    Token(TokenType t, const std::string& v, int l, const std::vector<StringPart>& parts)
        : type(t), value(v), line(l), stringParts(parts) {}
};

class Lexer {
public:
    explicit Lexer(const std::string& source) : source_(source), pos_(0), line_(1) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (!isAtEnd()) {
            skipWhitespace();
            if (isAtEnd()) break;
            if (matchUTF8_e_acute()) { tokens.push_back(Token(TokenType::TOKEN_SEMICOLON, ";", line_)); continue; }
            char c = current();
            switch (c) {
                case ';': tokens.push_back(Token(TokenType::TOKEN_SEMICOLON, ";", line_)); advance(); continue;
                case '+': tokens.push_back(Token(TokenType::TOKEN_PLUS, "+", line_)); advance(); continue;
                case '-': tokens.push_back(Token(TokenType::TOKEN_MINUS, "-", line_)); advance(); continue;
                case '*': tokens.push_back(Token(TokenType::TOKEN_STAR, "*", line_)); advance(); continue;
                case '/': tokens.push_back(Token(TokenType::TOKEN_SLASH, "/", line_)); advance(); continue;
                case '(': tokens.push_back(Token(TokenType::TOKEN_LPAREN, "(", line_)); advance(); continue;
                case ')': tokens.push_back(Token(TokenType::TOKEN_RPAREN, ")", line_)); advance(); continue;
                case '<': tokens.push_back(Token(TokenType::TOKEN_LT, "<", line_)); advance(); continue;
                case '>': tokens.push_back(Token(TokenType::TOKEN_GT, ">", line_)); advance(); continue;
                default: break;
            }
            if (c == '=' && peek() == '=') { tokens.push_back(Token(TokenType::TOKEN_EQ, "==", line_)); advance(); advance(); continue; }
            if (c == '=') { tokens.push_back(Token(TokenType::TOKEN_ASSIGN, "=", line_)); advance(); continue; }
            if (c == '!') {
                if (peek() == '=') { tokens.push_back(Token(TokenType::TOKEN_NEQ, "!=", line_)); advance(); advance(); continue; }
                else { tokens.push_back(Token(TokenType::TOKEN_BANG, "!", line_)); advance(); continue; }
            }
            if (c == '"') { tokens.push_back(makeString()); continue; }
            if (isDigit(c)) { tokens.push_back(makeNumber()); continue; }
            if (isAlpha(c)) { tokens.push_back(makeIdentifierOrKeyword()); continue; }
            throw std::runtime_error("Satir " + std::to_string(line_) + ": Bilinmeyen karakter '" + std::string(1, c) + "'");
        }
        tokens.push_back(Token(TokenType::TOKEN_EOF, "", line_));
        return tokens;
    }

private:
    std::string source_;
    size_t pos_;
    int line_;

    char current() const { return isAtEnd() ? '\0' : source_[pos_]; }
    char peek() const { return (pos_ + 1 >= source_.size()) ? '\0' : source_[pos_ + 1]; }
    void advance() { if (current() == '\n') line_++; pos_++; }
    bool isAtEnd() const { return pos_ >= source_.size(); }
    bool isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool isDigit(char c) const { return c >= '0' && c <= '9'; }
    bool isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }

    void skipWhitespace() {
        while (!isAtEnd() && (current() == ' ' || current() == '\t' || current() == '\r' || current() == '\n'))
            advance();
    }

    bool matchUTF8_e_acute() {
        if (pos_ + 1 < source_.size()) {
            unsigned char c1 = static_cast<unsigned char>(source_[pos_]);
            unsigned char c2 = static_cast<unsigned char>(source_[pos_ + 1]);
            if (c1 == 0xC3 && c2 == 0xA9) { pos_ += 2; return true; }
        }
        return false;
    }

    Token makeNumber() {
        int startLine = line_;
        std::string num;
        while (!isAtEnd() && isDigit(current())) { num += current(); advance(); }
        if (!isAtEnd() && current() == '.' && isDigit(peek())) {
            num += current(); advance();
            while (!isAtEnd() && isDigit(current())) { num += current(); advance(); }
        }
        return Token(TokenType::TOKEN_NUMBER, num, startLine);
    }

    Token makeIdentifierOrKeyword() {
        int startLine = line_;
        std::string ident;
        while (!isAtEnd() && isAlphaNumeric(current())) { ident += current(); advance(); }
        if (ident == "degisken") return Token(TokenType::TOKEN_DEGISKEN, ident, startLine);
        if (ident == "eger")     return Token(TokenType::TOKEN_EGER, ident, startLine);
        if (ident == "iken")     return Token(TokenType::TOKEN_IKEN, ident, startLine);
        if (ident == "ise")      return Token(TokenType::TOKEN_ISE, ident, startLine);
        if (ident == "yaz")      return Token(TokenType::TOKEN_YAZ, ident, startLine);
        return Token(TokenType::TOKEN_IDENT, ident, startLine);
    }

    Token makeString() {
        int startLine = line_;
        advance();
        std::vector<StringPart> parts;
        std::string currentLiteral;
        while (!isAtEnd() && current() != '"') {
            if (current() == '&') {
                if (!currentLiteral.empty()) { parts.push_back({false, currentLiteral}); currentLiteral.clear(); }
                advance();
                std::string varName;
                while (!isAtEnd() && current() != '"' && isAlphaNumeric(current())) { varName += current(); advance(); }
                if (!varName.empty()) parts.push_back({true, varName});
            } else if (current() == '\\') {
                advance();
                if (!isAtEnd()) {
                    switch (current()) {
                        case 'n': currentLiteral += '\n'; break;
                        case 't': currentLiteral += '\t'; break;
                        case '"': currentLiteral += '"'; break;
                        case '\\': currentLiteral += '\\'; break;
                        case '&': currentLiteral += '&'; break;
                        default: currentLiteral += current(); break;
                    }
                    advance();
                }
            } else {
                currentLiteral += current();
                advance();
            }
        }
        if (isAtEnd()) throw std::runtime_error("Satir " + std::to_string(startLine) + ": Kapanmamis string literali");
        advance();
        if (!currentLiteral.empty()) parts.push_back({false, currentLiteral});
        std::string fullValue;
        for (auto& p : parts) { if (p.isVariable) fullValue += "&" + p.value; else fullValue += p.value; }
        return Token(TokenType::TOKEN_STRING, fullValue, startLine, parts);
    }
};

// ╔══════════════════════════════════════════════════════════════╗
// ║                    BÖLÜM 2: AST NODE'LARI                  ║
// ╚══════════════════════════════════════════════════════════════╝

struct ASTNode { virtual ~ASTNode() = default; int line = 0; };
using ASTNodePtr = std::unique_ptr<ASTNode>;

struct NumberNode : ASTNode {
    double value;
    explicit NumberNode(double v, int l = 0) : value(v) { line = l; }
};

struct StringNode : ASTNode {
    std::vector<StringPart> parts;
    explicit StringNode(const std::vector<StringPart>& p, int l = 0) : parts(p) { line = l; }
};

struct IdentifierNode : ASTNode {
    std::string name;
    explicit IdentifierNode(const std::string& n, int l = 0) : name(n) { line = l; }
};

struct BinaryOpNode : ASTNode {
    std::string op;
    ASTNodePtr left, right;
    BinaryOpNode(const std::string& o, ASTNodePtr l, ASTNodePtr r, int ln = 0)
        : op(o), left(std::move(l)), right(std::move(r)) { line = ln; }
};

struct VarDeclNode : ASTNode {
    std::string name; ASTNodePtr expr;
    VarDeclNode(const std::string& n, ASTNodePtr e, int l = 0) : name(n), expr(std::move(e)) { line = l; }
};

struct AssignNode : ASTNode {
    std::string name; ASTNodePtr expr;
    AssignNode(const std::string& n, ASTNodePtr e, int l = 0) : name(n), expr(std::move(e)) { line = l; }
};

struct PrintNode : ASTNode {
    ASTNodePtr expr;
    explicit PrintNode(ASTNodePtr e, int l = 0) : expr(std::move(e)) { line = l; }
};

struct IfNode : ASTNode {
    ASTNodePtr condition; std::vector<ASTNodePtr> body;
    IfNode(ASTNodePtr cond, std::vector<ASTNodePtr> b, int l = 0)
        : condition(std::move(cond)), body(std::move(b)) { line = l; }
};

struct WhileNode : ASTNode {
    ASTNodePtr condition; std::vector<ASTNodePtr> body;
    WhileNode(ASTNodePtr cond, std::vector<ASTNodePtr> b, int l = 0)
        : condition(std::move(cond)), body(std::move(b)) { line = l; }
};

struct ProgramNode : ASTNode {
    std::vector<ASTNodePtr> statements;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║                  BÖLÜM 3: RECURSIVE DESCENT PARSER          ║
// ╚══════════════════════════════════════════════════════════════╝

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens), pos_(0) {}

    std::unique_ptr<ProgramNode> parse() {
        auto program = std::make_unique<ProgramNode>();
        while (!check(TokenType::TOKEN_EOF))
            program->statements.push_back(parseStatement());
        return program;
    }

private:
    std::vector<Token> tokens_;
    size_t pos_;

    const Token& current() const { return tokens_[pos_]; }
    const Token& peekTok() const { return (pos_ + 1 < tokens_.size()) ? tokens_[pos_ + 1] : tokens_.back(); }
    void advance() { if (pos_ < tokens_.size() - 1) pos_++; }
    bool check(TokenType type) const { return current().type == type; }

    Token expect(TokenType type, const std::string& message) {
        if (check(type)) { Token tok = current(); advance(); return tok; }
        throw std::runtime_error("Satir " + std::to_string(current().line) + ": " + message + " ('" + current().value + "' bulundu)");
    }

    void expectLineEnd() { expect(TokenType::TOKEN_SEMICOLON, "Satir sonu (é veya ;) bekleniyor"); }

    ASTNodePtr parseStatement() {
        if (check(TokenType::TOKEN_DEGISKEN)) return parseVarDecl();
        if (check(TokenType::TOKEN_YAZ))      return parsePrintStmt();
        if (check(TokenType::TOKEN_EGER))      return parseIfStmt();
        if (check(TokenType::TOKEN_IKEN))      return parseWhileStmt();
        if (check(TokenType::TOKEN_IDENT))     return parseAssignment();
        throw std::runtime_error("Satir " + std::to_string(current().line) + ": Beklenmeyen token '" + current().value + "'");
    }

    ASTNodePtr parseVarDecl() {
        int ln = current().line; advance();
        Token name = expect(TokenType::TOKEN_IDENT, "Degisken adi bekleniyor");
        expect(TokenType::TOKEN_ASSIGN, "'=' bekleniyor");
        auto expr = parseExpression(); expectLineEnd();
        return std::make_unique<VarDeclNode>(name.value, std::move(expr), ln);
    }

    ASTNodePtr parseAssignment() {
        int ln = current().line; Token name = current(); advance();
        if (check(TokenType::TOKEN_ASSIGN)) {
            advance(); auto expr = parseExpression(); expectLineEnd();
            return std::make_unique<AssignNode>(name.value, std::move(expr), ln);
        }
        throw std::runtime_error("Satir " + std::to_string(ln) + ": '=' bekleniyor '" + name.value + "' sonrasinda");
    }

    ASTNodePtr parsePrintStmt() {
        int ln = current().line; advance();
        auto expr = parseExpression(); expectLineEnd();
        return std::make_unique<PrintNode>(std::move(expr), ln);
    }

    ASTNodePtr parseIfStmt() {
        int ln = current().line; advance();
        auto condition = parseComparison();
        expect(TokenType::TOKEN_ISE, "'ise' bekleniyor");
        auto body = parseBlock();
        return std::make_unique<IfNode>(std::move(condition), std::move(body), ln);
    }

    ASTNodePtr parseWhileStmt() {
        int ln = current().line; advance();
        auto condition = parseComparison();
        expect(TokenType::TOKEN_ISE, "'ise' bekleniyor");
        auto body = parseBlock();
        return std::make_unique<WhileNode>(std::move(condition), std::move(body), ln);
    }

    std::vector<ASTNodePtr> parseBlock() {
        expect(TokenType::TOKEN_LPAREN, "'(' bekleniyor (blok baslangici)");
        std::vector<ASTNodePtr> stmts;
        while (!check(TokenType::TOKEN_RPAREN) && !check(TokenType::TOKEN_EOF))
            stmts.push_back(parseStatement());
        expect(TokenType::TOKEN_RPAREN, "')' bekleniyor (blok sonu)");
        return stmts;
    }

    ASTNodePtr parseExpression() { return parseComparison(); }

    ASTNodePtr parseComparison() {
        auto left = parseAddSub();
        if (check(TokenType::TOKEN_LT) || check(TokenType::TOKEN_GT) ||
            check(TokenType::TOKEN_EQ) || check(TokenType::TOKEN_NEQ)) {
            int ln = current().line; std::string op = current().value; advance();
            auto right = parseAddSub();
            return std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), ln);
        }
        return left;
    }

    ASTNodePtr parseAddSub() {
        auto left = parseTerm();
        while (check(TokenType::TOKEN_PLUS) || check(TokenType::TOKEN_MINUS)) {
            int ln = current().line; std::string op = current().value; advance();
            auto right = parseTerm();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), ln);
        }
        return left;
    }

    ASTNodePtr parseTerm() {
        auto left = parseFactor();
        while (check(TokenType::TOKEN_STAR) || check(TokenType::TOKEN_SLASH)) {
            int ln = current().line; std::string op = current().value; advance();
            auto right = parseFactor();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right), ln);
        }
        return left;
    }

    ASTNodePtr parseFactor() {
        int ln = current().line;
        if (check(TokenType::TOKEN_NUMBER)) {
            double val = std::stod(current().value); advance();
            return std::make_unique<NumberNode>(val, ln);
        }
        if (check(TokenType::TOKEN_STRING)) {
            auto parts = current().stringParts; advance();
            return std::make_unique<StringNode>(parts, ln);
        }
        if (check(TokenType::TOKEN_IDENT)) {
            std::string name = current().value; advance();
            return std::make_unique<IdentifierNode>(name, ln);
        }
        if (check(TokenType::TOKEN_BANG)) {
            advance();
            auto expr = parseExpression();
            expect(TokenType::TOKEN_BANG, "Kapanis '!' bekleniyor (grouping)");
            return expr;
        }
        throw std::runtime_error("Satir " + std::to_string(ln) + ": Ifade bekleniyor, '" + current().value + "' bulundu");
    }
};

// ╔══════════════════════════════════════════════════════════════╗
// ║               BÖLÜM 4: BYTECODE & INSTRUCTION SET          ║
// ╚══════════════════════════════════════════════════════════════╝

enum class Opcode : uint8_t {
    PUSH, PUSH_STR, LOAD, STORE,
    ADD, SUB, MUL, DIV,
    CMP_LT, CMP_GT, CMP_EQ, CMP_NEQ,
    JUMP, JUMP_IF_FALSE,
    PRINT, CONCAT, TO_STR, HALT
};

struct Instruction {
    Opcode opcode;
    double numValue = 0.0;
    int intValue = 0;
    std::string strValue;
    explicit Instruction(Opcode op) : opcode(op) {}
    Instruction(Opcode op, double num) : opcode(op), numValue(num) {}
    Instruction(Opcode op, int idx) : opcode(op), intValue(idx) {}
    Instruction(Opcode op, const std::string& name) : opcode(op), strValue(name) {}
};

struct CompiledProgram {
    std::vector<Instruction> instructions;
    std::vector<std::string> stringPool;
};

// ╔══════════════════════════════════════════════════════════════╗
// ║               BÖLÜM 5: COMPILER (AST → BYTECODE)           ║
// ╚══════════════════════════════════════════════════════════════╝

class Compiler {
public:
    CompiledProgram compile(ProgramNode* program) {
        instructions_.clear(); stringPool_.clear();
        for (auto& stmt : program->statements) compileNode(stmt.get());
        emit(Instruction(Opcode::HALT));
        CompiledProgram result;
        result.instructions = std::move(instructions_);
        result.stringPool = std::move(stringPool_);
        return result;
    }

private:
    std::vector<Instruction> instructions_;
    std::vector<std::string> stringPool_;

    void emit(const Instruction& instr) { instructions_.push_back(instr); }
    int currentAddr() const { return static_cast<int>(instructions_.size()); }

    int addString(const std::string& str) {
        for (int i = 0; i < static_cast<int>(stringPool_.size()); i++)
            if (stringPool_[i] == str) return i;
        stringPool_.push_back(str);
        return static_cast<int>(stringPool_.size()) - 1;
    }

    int emitJump(Opcode op) { int idx = currentAddr(); emit(Instruction(op, 0)); return idx; }
    void patchJump(int instrIndex) { instructions_[instrIndex].intValue = currentAddr(); }

    void compileNode(ASTNode* node) {
        if (auto* n = dynamic_cast<VarDeclNode*>(node))  { compileExpr(n->expr.get()); emit(Instruction(Opcode::STORE, n->name)); }
        else if (auto* n = dynamic_cast<AssignNode*>(node))  { compileExpr(n->expr.get()); emit(Instruction(Opcode::STORE, n->name)); }
        else if (auto* n = dynamic_cast<PrintNode*>(node))   { compileExpr(n->expr.get()); emit(Instruction(Opcode::PRINT)); }
        else if (auto* n = dynamic_cast<IfNode*>(node))      { compileIf(n); }
        else if (auto* n = dynamic_cast<WhileNode*>(node))   { compileWhile(n); }
        else throw std::runtime_error("Derleyici hatasi: Bilinmeyen statement tipi");
    }

    void compileExpr(ASTNode* node) {
        if (auto* n = dynamic_cast<NumberNode*>(node))     emit(Instruction(Opcode::PUSH, n->value));
        else if (auto* n = dynamic_cast<IdentifierNode*>(node)) emit(Instruction(Opcode::LOAD, n->name));
        else if (auto* n = dynamic_cast<StringNode*>(node))     compileString(n);
        else if (auto* n = dynamic_cast<BinaryOpNode*>(node))   compileBinaryOp(n);
        else throw std::runtime_error("Derleyici hatasi: Bilinmeyen expression tipi");
    }

    void compileBinaryOp(BinaryOpNode* node) {
        compileExpr(node->left.get());
        compileExpr(node->right.get());
        if      (node->op == "+")  emit(Instruction(Opcode::ADD));
        else if (node->op == "-")  emit(Instruction(Opcode::SUB));
        else if (node->op == "*")  emit(Instruction(Opcode::MUL));
        else if (node->op == "/")  emit(Instruction(Opcode::DIV));
        else if (node->op == "<")  emit(Instruction(Opcode::CMP_LT));
        else if (node->op == ">")  emit(Instruction(Opcode::CMP_GT));
        else if (node->op == "==") emit(Instruction(Opcode::CMP_EQ));
        else if (node->op == "!=") emit(Instruction(Opcode::CMP_NEQ));
    }

    void compileString(StringNode* node) {
        if (node->parts.empty()) { emit(Instruction(Opcode::PUSH_STR, addString(""))); return; }
        bool first = true;
        for (auto& part : node->parts) {
            if (part.isVariable) { emit(Instruction(Opcode::LOAD, part.value)); emit(Instruction(Opcode::TO_STR)); }
            else { emit(Instruction(Opcode::PUSH_STR, addString(part.value))); }
            if (!first) emit(Instruction(Opcode::CONCAT));
            first = false;
        }
    }

    void compileIf(IfNode* node) {
        compileExpr(node->condition.get());
        int jmp = emitJump(Opcode::JUMP_IF_FALSE);
        for (auto& stmt : node->body) compileNode(stmt.get());
        patchJump(jmp);
    }

    void compileWhile(WhileNode* node) {
        int loopStart = currentAddr();
        compileExpr(node->condition.get());
        int jmp = emitJump(Opcode::JUMP_IF_FALSE);
        for (auto& stmt : node->body) compileNode(stmt.get());
        emit(Instruction(Opcode::JUMP, loopStart));
        patchJump(jmp);
    }
};

// ╔══════════════════════════════════════════════════════════════╗
// ║            BÖLÜM 6: STACK-BASED VIRTUAL MACHINE             ║
// ╚══════════════════════════════════════════════════════════════╝

struct Value {
    enum class Type { NUMBER, STRING };
    Type type;
    double numValue = 0.0;
    std::string strValue;

    Value() : type(Type::NUMBER), numValue(0.0) {}
    explicit Value(double n) : type(Type::NUMBER), numValue(n) {}
    explicit Value(const std::string& s) : type(Type::STRING), strValue(s) {}

    std::string toString() const {
        if (type == Type::STRING) return strValue;
        if (numValue == static_cast<int>(numValue))
            return std::to_string(static_cast<int>(numValue));
        return std::to_string(numValue);
    }

    bool isTruthy() const {
        if (type == Type::NUMBER) return numValue != 0.0;
        return !strValue.empty();
    }
};

class VM {
public:
    void execute(const CompiledProgram& program) {
        const auto& instructions = program.instructions;
        const auto& stringPool = program.stringPool;
        stack_.clear(); variables_.clear();
        int ip = 0;

        while (ip < static_cast<int>(instructions.size())) {
            const auto& instr = instructions[ip];
            switch (instr.opcode) {
                case Opcode::PUSH:     push(Value(instr.numValue)); ip++; break;
                case Opcode::PUSH_STR: push(Value(stringPool[instr.intValue])); ip++; break;
                case Opcode::LOAD: {
                    auto it = variables_.find(instr.strValue);
                    if (it == variables_.end())
                        throw std::runtime_error("VM Hatasi: Tanimlanmamis degisken '" + instr.strValue + "'");
                    push(it->second); ip++; break;
                }
                case Opcode::STORE: variables_[instr.strValue] = pop(); ip++; break;
                case Opcode::ADD: {
                    Value b = pop(), a = pop();
                    if (a.type == Value::Type::STRING || b.type == Value::Type::STRING)
                        push(Value(a.toString() + b.toString()));
                    else push(Value(a.numValue + b.numValue));
                    ip++; break;
                }
                case Opcode::SUB: { Value b = pop(), a = pop(); push(Value(a.numValue - b.numValue)); ip++; break; }
                case Opcode::MUL: { Value b = pop(), a = pop(); push(Value(a.numValue * b.numValue)); ip++; break; }
                case Opcode::DIV: {
                    Value b = pop(), a = pop();
                    if (b.numValue == 0.0) throw std::runtime_error("VM Hatasi: Sifira bolme");
                    push(Value(a.numValue / b.numValue)); ip++; break;
                }
                case Opcode::CMP_LT:  { Value b = pop(), a = pop(); push(Value(a.numValue < b.numValue ? 1.0 : 0.0)); ip++; break; }
                case Opcode::CMP_GT:  { Value b = pop(), a = pop(); push(Value(a.numValue > b.numValue ? 1.0 : 0.0)); ip++; break; }
                case Opcode::CMP_EQ:  { Value b = pop(), a = pop();
                    if (a.type == Value::Type::STRING && b.type == Value::Type::STRING)
                        push(Value(a.strValue == b.strValue ? 1.0 : 0.0));
                    else push(Value(a.numValue == b.numValue ? 1.0 : 0.0));
                    ip++; break; }
                case Opcode::CMP_NEQ: { Value b = pop(), a = pop();
                    if (a.type == Value::Type::STRING && b.type == Value::Type::STRING)
                        push(Value(a.strValue != b.strValue ? 1.0 : 0.0));
                    else push(Value(a.numValue != b.numValue ? 1.0 : 0.0));
                    ip++; break; }
                case Opcode::JUMP:          ip = instr.intValue; break;
                case Opcode::JUMP_IF_FALSE: { Value v = pop(); ip = !v.isTruthy() ? instr.intValue : ip + 1; break; }
                case Opcode::CONCAT: { Value b = pop(), a = pop(); push(Value(a.toString() + b.toString())); ip++; break; }
                case Opcode::TO_STR: { Value v = pop(); push(Value(v.toString())); ip++; break; }
                case Opcode::PRINT:  { std::cout << pop().toString() << std::endl; ip++; break; }
                case Opcode::HALT:   return;
                default: throw std::runtime_error("VM Hatasi: Bilinmeyen opcode");
            }
        }
    }

private:
    std::vector<Value> stack_;
    std::unordered_map<std::string, Value> variables_;
    void push(const Value& val) { stack_.push_back(val); }
    Value pop() {
        if (stack_.empty()) throw std::runtime_error("VM Hatasi: Stack bos");
        Value val = std::move(stack_.back()); stack_.pop_back(); return val;
    }
};

// ╔══════════════════════════════════════════════════════════════╗
// ║                    BÖLÜM 7: MAIN — GİRİŞ NOKTASI          ║
// ╚══════════════════════════════════════════════════════════════╝

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Kullanim: compiler <dosya.tc>" << std::endl;
        return 1;
    }

    try {
        std::ifstream file(argv[1], std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "HATA: Dosya acilamiyor: " << argv[1] << std::endl;
            return 1;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        Compiler compiler;
        auto program = compiler.compile(ast.get());

        VM vm;
        vm.execute(program);

    } catch (const std::exception& e) {
        std::cerr << "HATA: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
