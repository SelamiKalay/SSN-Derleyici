#include "vm.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cmath>

// ============================================================
// Stack Islemleri
// ============================================================
void VM::push(const Value& val) {
    stack_.push_back(val);
}

Value VM::pop() {
    if (stack_.empty()) {
        throw std::runtime_error("VM Hatasi: Stack bos, pop yapilamiyor");
    }
    Value val = std::move(stack_.back());
    stack_.pop_back();
    return val;
}

Value& VM::top() {
    if (stack_.empty()) {
        throw std::runtime_error("VM Hatasi: Stack bos");
    }
    return stack_.back();
}

// ============================================================
// Ana Execution Loop
//
// Stack-Based VM Mimarisi:
// - IP (Instruction Pointer): bytecode dizisindeki geçerli konum
// - Stack: islenen degerler icin LIFO yapisi
// - Variables: degisken deposu (isim → Value)
// - String Pool: sabit stringler (compile time'da olusturulur)
//
// Her instruction stack uzerinde calisir:
//   PUSH    → stack'e deger it
//   POP     → stack'ten deger çek
//   Binary  → iki degeri cek, islem yap, sonucu it
// ============================================================
void VM::execute(const CompiledProgram& program) {
    const auto& instructions = program.instructions;
    const auto& stringPool = program.stringPool;

    stack_.clear();
    variables_.clear();

    int ip = 0; // Instruction Pointer

    while (ip < static_cast<int>(instructions.size())) {
        const auto& instr = instructions[ip];

        switch (instr.opcode) {
            // ============================================================
            // PUSH: Sayisal degeri stack'e it
            // Stack: [] → [numValue]
            // ============================================================
            case Opcode::PUSH: {
                push(Value(instr.numValue));
                ip++;
                break;
            }

            // ============================================================
            // PUSH_STR: String pool'dan string'i stack'e it
            // Stack: [] → [strValue]
            // ============================================================
            case Opcode::PUSH_STR: {
                if (instr.intValue < 0 || instr.intValue >= static_cast<int>(stringPool.size())) {
                    throw std::runtime_error("VM Hatasi: Gecersiz string pool indeksi " +
                        std::to_string(instr.intValue));
                }
                push(Value(stringPool[instr.intValue]));
                ip++;
                break;
            }

            // ============================================================
            // LOAD: Degisken degerini stack'e yukle
            // Stack: [] → [variable value]
            // ============================================================
            case Opcode::LOAD: {
                auto it = variables_.find(instr.strValue);
                if (it == variables_.end()) {
                    throw std::runtime_error("VM Hatasi: Tanimlanmamis degisken '" +
                        instr.strValue + "'");
                }
                push(it->second);
                ip++;
                break;
            }

            // ============================================================
            // STORE: Stack'ten degeri çek ve degiskene yaz
            // Stack: [val] → []
            // ============================================================
            case Opcode::STORE: {
                Value val = pop();
                variables_[instr.strValue] = std::move(val);
                ip++;
                break;
            }

            // ============================================================
            // Aritmetik Islemler: Iki degeri çek, islem yap, sonucu it
            // Stack: [a, b] → [sonuc]
            // ============================================================
            case Opcode::ADD: {
                Value b = pop();
                Value a = pop();
                // String + String = concatenation
                if (a.type == Value::Type::STRING || b.type == Value::Type::STRING) {
                    push(Value(a.toString() + b.toString()));
                } else {
                    push(Value(a.numValue + b.numValue));
                }
                ip++;
                break;
            }

            case Opcode::SUB: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numValue - b.numValue));
                ip++;
                break;
            }

            case Opcode::MUL: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numValue * b.numValue));
                ip++;
                break;
            }

            case Opcode::DIV: {
                Value b = pop();
                Value a = pop();
                if (b.numValue == 0.0) {
                    throw std::runtime_error("VM Hatasi: Sifira bolme");
                }
                push(Value(a.numValue / b.numValue));
                ip++;
                break;
            }

            // ============================================================
            // Karsilastirma Islemleri: Iki degeri çek, karsilastir, 0/1 it
            // Stack: [a, b] → [0 veya 1]
            // ============================================================
            case Opcode::CMP_LT: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numValue < b.numValue ? 1.0 : 0.0));
                ip++;
                break;
            }

            case Opcode::CMP_GT: {
                Value b = pop();
                Value a = pop();
                push(Value(a.numValue > b.numValue ? 1.0 : 0.0));
                ip++;
                break;
            }

            case Opcode::CMP_EQ: {
                Value b = pop();
                Value a = pop();
                if (a.type == Value::Type::STRING && b.type == Value::Type::STRING) {
                    push(Value(a.strValue == b.strValue ? 1.0 : 0.0));
                } else {
                    push(Value(a.numValue == b.numValue ? 1.0 : 0.0));
                }
                ip++;
                break;
            }

            case Opcode::CMP_NEQ: {
                Value b = pop();
                Value a = pop();
                if (a.type == Value::Type::STRING && b.type == Value::Type::STRING) {
                    push(Value(a.strValue != b.strValue ? 1.0 : 0.0));
                } else {
                    push(Value(a.numValue != b.numValue ? 1.0 : 0.0));
                }
                ip++;
                break;
            }

            // ============================================================
            // JUMP: Kosulsuz sicrama
            // IP'yi hedef adrese ayarlar
            // ============================================================
            case Opcode::JUMP: {
                ip = instr.intValue;
                break;
            }

            // ============================================================
            // JUMP_IF_FALSE: Kosullu sicrama
            // Stack'ten degeri çeker:
            //   - 0 (false) ise → hedef adrese sicra
            //   - 1 (true) ise  → sonraki instruction'a devam
            // Stack: [val] → []
            // ============================================================
            case Opcode::JUMP_IF_FALSE: {
                Value val = pop();
                if (!val.isTruthy()) {
                    ip = instr.intValue;
                } else {
                    ip++;
                }
                break;
            }

            // ============================================================
            // CONCAT: Iki string'i birlestir
            // Stack: [str1, str2] → [str1+str2]
            // ============================================================
            case Opcode::CONCAT: {
                Value b = pop();
                Value a = pop();
                push(Value(a.toString() + b.toString()));
                ip++;
                break;
            }

            // ============================================================
            // TO_STR: Stack'teki degeri string'e donustur
            // Stack: [val] → [str(val)]
            // ============================================================
            case Opcode::TO_STR: {
                Value val = pop();
                push(Value(val.toString()));
                ip++;
                break;
            }

            // ============================================================
            // PRINT: Stack'ten degeri çek ve konsola yazdir
            // Stack: [val] → []
            // ============================================================
            case Opcode::PRINT: {
                Value val = pop();
                std::cout << val.toString() << std::endl;
                ip++;
                break;
            }

            // ============================================================
            // HALT: Programi sonlandir
            // ============================================================
            case Opcode::HALT: {
                return;
            }

            default: {
                throw std::runtime_error("VM Hatasi: Bilinmeyen opcode " +
                    std::to_string(static_cast<int>(instr.opcode)));
            }
        }
    }
}
