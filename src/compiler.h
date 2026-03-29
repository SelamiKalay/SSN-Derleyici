#pragma once
#include "ast.h"
#include "bytecode.h"
#include <vector>
#include <string>

// ============================================================
// Compiler — AST'den Bytecode Uretimi
// Visitor pattern ile AST'yi dolaşir ve bytecode dizisi uretir.
// ============================================================
class Compiler {
public:
    CompiledProgram compile(ProgramNode* program);

    // Debug: uretilen bytecode'u yazdirma
    static void disassemble(const CompiledProgram& prog);

private:
    std::vector<Instruction> instructions_;
    std::vector<std::string> stringPool_;

    // AST node derleyicileri
    void compileNode(ASTNode* node);
    void compileExpression(ASTNode* node);
    void compileVarDecl(VarDeclNode* node);
    void compileAssign(AssignNode* node);
    void compilePrint(PrintNode* node);
    void compileIf(IfNode* node);
    void compileWhile(WhileNode* node);
    void compileStringNode(StringNode* node);
    void compileBinaryOp(BinaryOpNode* node);

    // Yardimci
    void emit(const Instruction& instr);
    int emitJump(Opcode op);           // Placeholder jump emit, adres dondurur
    void patchJump(int instrIndex);     // Backpatching: placeholder'i guncelle
    int addString(const std::string& str); // String pool'a ekle
    int currentAddr() const;
};
