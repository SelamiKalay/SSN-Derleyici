#pragma once
#include <string>
#include <vector>
#include <variant>
#include <cstdint>

// ============================================================
// Opcode — Bytecode talimat seti
// ============================================================
enum class Opcode : uint8_t {
    PUSH,           // Sayisal degeri stack'e it
    PUSH_STR,       // String'i constant pool'dan stack'e it
    LOAD,           // Degisken degerini stack'e yukle
    STORE,          // Stack'ten alip degiskene yaz
    ADD,            // Toplama:      [a, b] -> [a+b]
    SUB,            // Cikarma:      [a, b] -> [a-b]
    MUL,            // Carpma:       [a, b] -> [a*b]
    DIV,            // Bolme:        [a, b] -> [a/b]
    CMP_LT,         // Kucuktur:     [a, b] -> [a<b ? 1 : 0]
    CMP_GT,         // Buyuktur:     [a, b] -> [a>b ? 1 : 0]
    CMP_EQ,         // Esittir:      [a, b] -> [a==b ? 1 : 0]
    CMP_NEQ,        // Esit degil:   [a, b] -> [a!=b ? 1 : 0]
    JUMP,           // Kosulsuz sicrama
    JUMP_IF_FALSE,  // 0 ise sicra, degilse devam
    PRINT,          // Stack'ten al ve yazdir
    CONCAT,         // Iki string'i birlestir
    TO_STR,         // Sayiyi string'e donustur
    HALT            // Programi sonlandir
};

// ============================================================
// Instruction — Tek bir bytecode talimati
// ============================================================
struct Instruction {
    Opcode opcode;
    double numValue = 0.0;      // PUSH icin sayi degeri
    int intValue = 0;           // PUSH_STR, JUMP, JUMP_IF_FALSE icin index/adres
    std::string strValue;       // LOAD, STORE icin degisken adi

    // Basit olusturucular
    explicit Instruction(Opcode op)
        : opcode(op) {}

    Instruction(Opcode op, double num)
        : opcode(op), numValue(num) {}

    Instruction(Opcode op, int idx)
        : opcode(op), intValue(idx) {}

    Instruction(Opcode op, const std::string& name)
        : opcode(op), strValue(name) {}
};

// ============================================================
// CompiledProgram — Derlenmis bytecode programi
// ============================================================
struct CompiledProgram {
    std::vector<Instruction> instructions;
    std::vector<std::string> stringPool;  // Constant string'ler
};

// ============================================================
// Opcode'u okunabilir string'e donusturme (debug icin)
// ============================================================
inline const char* opcodeToString(Opcode op) {
    switch (op) {
        case Opcode::PUSH:          return "PUSH";
        case Opcode::PUSH_STR:      return "PUSH_STR";
        case Opcode::LOAD:          return "LOAD";
        case Opcode::STORE:         return "STORE";
        case Opcode::ADD:           return "ADD";
        case Opcode::SUB:           return "SUB";
        case Opcode::MUL:           return "MUL";
        case Opcode::DIV:           return "DIV";
        case Opcode::CMP_LT:       return "CMP_LT";
        case Opcode::CMP_GT:       return "CMP_GT";
        case Opcode::CMP_EQ:       return "CMP_EQ";
        case Opcode::CMP_NEQ:      return "CMP_NEQ";
        case Opcode::JUMP:         return "JUMP";
        case Opcode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case Opcode::PRINT:        return "PRINT";
        case Opcode::CONCAT:       return "CONCAT";
        case Opcode::TO_STR:       return "TO_STR";
        case Opcode::HALT:         return "HALT";
        default:                   return "UNKNOWN";
    }
}
