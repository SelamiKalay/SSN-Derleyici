#include "compiler.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// ============================================================
// Ana Derleme Fonksiyonu
// ============================================================
CompiledProgram Compiler::compile(ProgramNode* program) {
    instructions_.clear();
    stringPool_.clear();

    for (auto& stmt : program->statements) {
        compileNode(stmt.get());
    }

    // Program sonuna HALT ekle
    emit(Instruction(Opcode::HALT));

    CompiledProgram result;
    result.instructions = std::move(instructions_);
    result.stringPool = std::move(stringPool_);
    return result;
}

// ============================================================
// Yardimci Fonksiyonlar
// ============================================================
void Compiler::emit(const Instruction& instr) {
    instructions_.push_back(instr);
}

int Compiler::currentAddr() const {
    return static_cast<int>(instructions_.size());
}

int Compiler::addString(const std::string& str) {
    // Ayni string varsa tekrar ekleme
    for (int i = 0; i < static_cast<int>(stringPool_.size()); i++) {
        if (stringPool_[i] == str) return i;
    }
    stringPool_.push_back(str);
    return static_cast<int>(stringPool_.size()) - 1;
}

// ============================================================
// Jump Yonetimi — Backpatching
//
// emitJump: Placeholder olan bir JUMP/JUMP_IF_FALSE emit eder.
//           Adres henuz bilinmiyor, 0 yazilir.
//           Placeholder'in instruction dizisindeki indexini dondurur.
//
// patchJump: Daha sonra, hedef adres belirlenince
//            placeholder'in intValue'sunu gecerli adres ile gunceller.
// ============================================================
int Compiler::emitJump(Opcode op) {
    int idx = currentAddr();
    emit(Instruction(op, 0)); // placeholder adres = 0
    return idx;
}

void Compiler::patchJump(int instrIndex) {
    instructions_[instrIndex].intValue = currentAddr();
}

// ============================================================
// AST Node Derleme — Dispatcher
// ============================================================
void Compiler::compileNode(ASTNode* node) {
    if (auto* n = dynamic_cast<VarDeclNode*>(node)) {
        compileVarDecl(n);
    } else if (auto* n = dynamic_cast<AssignNode*>(node)) {
        compileAssign(n);
    } else if (auto* n = dynamic_cast<PrintNode*>(node)) {
        compilePrint(n);
    } else if (auto* n = dynamic_cast<IfNode*>(node)) {
        compileIf(n);
    } else if (auto* n = dynamic_cast<WhileNode*>(node)) {
        compileWhile(n);
    } else {
        throw std::runtime_error("Derleyici hatasi: Bilinmeyen statement tipi");
    }
}

// ============================================================
// Expression Derleme
// ============================================================
void Compiler::compileExpression(ASTNode* node) {
    if (auto* n = dynamic_cast<NumberNode*>(node)) {
        // PUSH <sayi>
        emit(Instruction(Opcode::PUSH, n->value));
    }
    else if (auto* n = dynamic_cast<IdentifierNode*>(node)) {
        // LOAD <degisken_adi>
        emit(Instruction(Opcode::LOAD, n->name));
    }
    else if (auto* n = dynamic_cast<StringNode*>(node)) {
        compileStringNode(n);
    }
    else if (auto* n = dynamic_cast<BinaryOpNode*>(node)) {
        compileBinaryOp(n);
    }
    else {
        throw std::runtime_error("Derleyici hatasi: Bilinmeyen expression tipi");
    }
}

// ============================================================
// Ikili Operator Derleme
// Sol ve sag operandlar derlenir (stack'e itilir),
// sonra uygun opcode emit edilir.
// ============================================================
void Compiler::compileBinaryOp(BinaryOpNode* node) {
    compileExpression(node->left.get());
    compileExpression(node->right.get());

    if (node->op == "+")       emit(Instruction(Opcode::ADD));
    else if (node->op == "-")  emit(Instruction(Opcode::SUB));
    else if (node->op == "*")  emit(Instruction(Opcode::MUL));
    else if (node->op == "/")  emit(Instruction(Opcode::DIV));
    else if (node->op == "<")  emit(Instruction(Opcode::CMP_LT));
    else if (node->op == ">")  emit(Instruction(Opcode::CMP_GT));
    else if (node->op == "==") emit(Instruction(Opcode::CMP_EQ));
    else if (node->op == "!=") emit(Instruction(Opcode::CMP_NEQ));
    else {
        throw std::runtime_error("Derleyici hatasi: Bilinmeyen operator '" + node->op + "'");
    }
}

// ============================================================
// degisken x = expr
// Expression'i derle (sonuc stack'te), STORE ile degiskene yaz
// ============================================================
void Compiler::compileVarDecl(VarDeclNode* node) {
    compileExpression(node->expr.get());
    emit(Instruction(Opcode::STORE, node->name));
}

// ============================================================
// x = expr (atama)
// Expression'i derle (sonuc stack'te), STORE ile degiskene yaz
// ============================================================
void Compiler::compileAssign(AssignNode* node) {
    compileExpression(node->expr.get());
    emit(Instruction(Opcode::STORE, node->name));
}

// ============================================================
// yaz expr
// Expression'i derle, PRINT ile yazdir
// ============================================================
void Compiler::compilePrint(PrintNode* node) {
    compileExpression(node->expr.get());
    emit(Instruction(Opcode::PRINT));
}

// ============================================================
// String Interpolation Derleme
//
// "Sonuc = &x" gibi bir string icin:
//   1. PUSH_STR "Sonuc = "    (literal parca)
//   2. LOAD x                 (degisken yukle)
//   3. TO_STR                 (sayiyi string'e cevir)
//   4. CONCAT                 (birlestir)
//
// Birden fazla parca varsa, her yeni parca CONCAT ile birlestirilir.
// ============================================================
void Compiler::compileStringNode(StringNode* node) {
    if (node->parts.empty()) {
        // Bos string
        int idx = addString("");
        emit(Instruction(Opcode::PUSH_STR, idx));
        return;
    }

    bool first = true;
    for (auto& part : node->parts) {
        if (part.isVariable) {
            // Degisken referansi: LOAD + TO_STR
            emit(Instruction(Opcode::LOAD, part.value));
            emit(Instruction(Opcode::TO_STR));
        } else {
            // Literal string parcasi: PUSH_STR
            int idx = addString(part.value);
            emit(Instruction(Opcode::PUSH_STR, idx));
        }

        if (!first) {
            // Onceki parca ile birlestir
            emit(Instruction(Opcode::CONCAT));
        }
        first = false;
    }
}

// ============================================================
// If Blogu Derleme — Backpatching ile
//
// Uretilen bytecode yapisi:
//   [condition bytecode]     → stack'e 0 veya 1 itilir
//   JUMP_IF_FALSE @end       → 0 ise body'yi atla
//   [body bytecode]
//   @end:                    → devam noktasi
//
// Asamalar:
//   1. Condition'i derle
//   2. JUMP_IF_FALSE emit et (adres bilinmiyor → placeholder)
//   3. Body'yi derle
//   4. Placeholder'i guncelle (backpatch) → body sonrasi adres
// ============================================================
void Compiler::compileIf(IfNode* node) {
    // 1. Condition'i derle
    compileExpression(node->condition.get());

    // 2. JUMP_IF_FALSE placeholder emit et
    int jumpIfFalse = emitJump(Opcode::JUMP_IF_FALSE);

    // 3. Body'yi derle
    for (auto& stmt : node->body) {
        compileNode(stmt.get());
    }

    // 4. Backpatch: JUMP_IF_FALSE adresini guncelle
    patchJump(jumpIfFalse);
}

// ============================================================
// While Dongusu Derleme — Backpatching ile
//
// Uretilen bytecode yapisi:
//   @loopStart:              → dongu basi
//   [condition bytecode]
//   JUMP_IF_FALSE @end       → 0 ise donguyu bitir
//   [body bytecode]
//   JUMP @loopStart          → dongu basina geri don
//   @end:                    → cikis noktasi
//
// Asamalar:
//   1. loopStart adresini kaydet
//   2. Condition'i derle
//   3. JUMP_IF_FALSE placeholder emit et
//   4. Body'yi derle
//   5. JUMP loopStart emit et (dongu basina geri sicra)
//   6. JUMP_IF_FALSE placeholder'i backpatch et
// ============================================================
void Compiler::compileWhile(WhileNode* node) {
    // 1. Dongu basi adresi
    int loopStart = currentAddr();

    // 2. Condition'i derle
    compileExpression(node->condition.get());

    // 3. JUMP_IF_FALSE placeholder
    int jumpIfFalse = emitJump(Opcode::JUMP_IF_FALSE);

    // 4. Body'yi derle
    for (auto& stmt : node->body) {
        compileNode(stmt.get());
    }

    // 5. Dongu basina geri sicra
    emit(Instruction(Opcode::JUMP, loopStart));

    // 6. Backpatch: JUMP_IF_FALSE cikis noktasi
    patchJump(jumpIfFalse);
}

// ============================================================
// Disassembler — Debug icin bytecode ciktisi
// ============================================================
void Compiler::disassemble(const CompiledProgram& prog) {
    std::cout << "\n=== BYTECODE DISASSEMBLY ===" << std::endl;

    // String pool
    if (!prog.stringPool.empty()) {
        std::cout << "\n--- String Pool ---" << std::endl;
        for (int i = 0; i < static_cast<int>(prog.stringPool.size()); i++) {
            std::cout << "  [" << i << "] \"" << prog.stringPool[i] << "\"" << std::endl;
        }
    }

    std::cout << "\n--- Instructions ---" << std::endl;
    for (int i = 0; i < static_cast<int>(prog.instructions.size()); i++) {
        const auto& instr = prog.instructions[i];
        std::cout << std::setw(4) << i << ": " << std::setw(16) << std::left
                  << opcodeToString(instr.opcode);

        switch (instr.opcode) {
            case Opcode::PUSH:
                std::cout << instr.numValue;
                break;
            case Opcode::PUSH_STR:
                std::cout << "[" << instr.intValue << "] \""
                          << prog.stringPool[instr.intValue] << "\"";
                break;
            case Opcode::LOAD:
            case Opcode::STORE:
                std::cout << instr.strValue;
                break;
            case Opcode::JUMP:
            case Opcode::JUMP_IF_FALSE:
                std::cout << "@" << instr.intValue;
                break;
            default:
                break;
        }
        std::cout << std::endl;
    }
    std::cout << "===========================" << std::endl;
}
