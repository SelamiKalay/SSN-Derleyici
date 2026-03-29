#pragma once
#include "bytecode.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>

// ============================================================
// VM Value — Stack degeri (sayi veya string)
// ============================================================
struct Value {
    enum class Type { NUMBER, STRING };
    Type type;
    double numValue = 0.0;
    std::string strValue;

    // Constructors
    Value() : type(Type::NUMBER), numValue(0.0) {}
    explicit Value(double n) : type(Type::NUMBER), numValue(n) {}
    explicit Value(const std::string& s) : type(Type::STRING), strValue(s) {}

    // Debug icin string donusumu
    std::string toString() const {
        if (type == Type::STRING) return strValue;
        // Tam sayi ise nokta gosterme
        if (numValue == static_cast<int>(numValue)) {
            return std::to_string(static_cast<int>(numValue));
        }
        return std::to_string(numValue);
    }

    // Sayisal deger (karsilastirma icin)
    double asNumber() const {
        return numValue;
    }

    bool isTruthy() const {
        if (type == Type::NUMBER) return numValue != 0.0;
        return !strValue.empty();
    }
};

// ============================================================
// Virtual Machine — Stack tabanli bytecode yurutucu
// ============================================================
class VM {
public:
    void execute(const CompiledProgram& program);

private:
    std::vector<Value> stack_;
    std::unordered_map<std::string, Value> variables_;

    // Stack islemleri
    void push(const Value& val);
    Value pop();
    Value& top();
};
