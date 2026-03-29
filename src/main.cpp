// ============================================================
// MühendisC — Türkçe Sözdizimli Bytecode Compiler & VM
//
// Kullanim: muhendisC <dosya.tc>
//           muhendisC --debug <dosya.tc>
//
// Pipeline: Kaynak Kod → Lexer → Parser → AST → Compiler → Bytecode → VM
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

// Dosya icerigini oku
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Dosya acilamiyor: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    // Komut satiri argumanlarini isle
    bool debugMode = false;
    std::string filePath;

    if (argc < 2) {
        std::cerr << "MühendisC — Türkçe Bytecode Compiler & VM" << std::endl;
        std::cerr << "Kullanim: " << argv[0] << " [--debug] <dosya.tc>" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Secenekler:" << std::endl;
        std::cerr << "  --debug    Bytecode disassembly ciktisini goster" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug") {
            debugMode = true;
        } else {
            filePath = arg;
        }
    }

    if (filePath.empty()) {
        std::cerr << "Hata: Kaynak dosya belirtilmedi" << std::endl;
        return 1;
    }

    try {
        // 1. Kaynak kodu oku
        std::string source = readFile(filePath);

        // 2. LEXER: Kaynak kodu tokenlara ayir
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        if (debugMode) {
            std::cout << "=== TOKENLAR ===" << std::endl;
            for (const auto& tok : tokens) {
                std::cout << "  [" << static_cast<int>(tok.type) << "] '"
                          << tok.value << "' (satir " << tok.line << ")" << std::endl;
            }
        }

        // 3. PARSER: Tokenlardan AST olustur
        Parser parser(tokens);
        auto ast = parser.parse();

        // 4. COMPILER: AST'den bytecode uret
        Compiler compiler;
        CompiledProgram program = compiler.compile(ast.get());

        if (debugMode) {
            Compiler::disassemble(program);
            std::cout << std::endl;
        }

        // 5. VM: Bytecode'u calistir
        if (debugMode) {
            std::cout << "=== CIKTI ===" << std::endl;
        }

        VM vm;
        vm.execute(program);

    } catch (const std::exception& e) {
        std::cerr << "HATA: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
