# SSN Compiler

**English** | [Türkçe](README.tr.md)

[![CI](https://github.com/SelamiKalay/SSN-Derleyici/actions/workflows/ci.yml/badge.svg)](https://github.com/SelamiKalay/SSN-Derleyici/actions/workflows/ci.yml)

![SSN desktop IDE](docs/ide.png)

SSN is a programming language with Turkish keywords, together with its compiler.
Source code is first compiled to bytecode and then executed on a stack-based
virtual machine. The compiler is written from scratch in C++17 with no external
libraries.

```
Source → Lexer → Parser → AST → Compiler → Bytecode → VM
```

## Example

```
// Fibonacci sequence (first 10 terms)
degisken a = 0 ;
degisken b = 1 ;
degisken i = 0 ;
iken i < 10 ise (
    yaz a ;
    degisken temp = b ;
    b = a + b ;
    a = temp ;
    i = i + 1 ;
)

degisken x = !5 + 3! * 2 ;
yaz "Sonuc: &x" ;
```

## Language Features

<p align="center"><img src="docs/komutlar.png" alt="SSN command reference window in the IDE" width="600"></p>

| Construct | Syntax | Meaning |
|---|---|---|
| Variable declaration | `degisken x = 5 ;` | *değişken* = variable |
| Assignment | `x = x + 1 ;` | |
| Print | `yaz x ;` | *yaz* = write |
| Conditional | `eger x > 10 ise ( ... )` | *eğer … ise* = if … then |
| Loop | `iken i < 5 ise ( ... )` | *iken* = while |
| Grouping (instead of parentheses) | `!5 + 3! * 2` | |
| String interpolation | `"Merhaba &isim"` | |
| Statement terminator | `;` or `é` | |
| Comment | `// until end of line` | |

- Arithmetic: `+ - * /` (with operator precedence)
- Comparison: `< > == !=`
- Data types: number (floating point) and string

### Design notes

Some parts of the syntax may look unusual at first; here is why:

- **Keywords are Turkish but written in ASCII** (`degisken`, `eger`). The lexer reads
  the source byte by byte and accepts only `a-z`, `A-Z`, `_` and digits in
  identifiers. Letters such as `ğ`, `ş`, `ı` take more than one byte in UTF-8 and
  are not supported; writing `değişken` gives an "unknown character" error.
- **Parentheses are for blocks; grouping uses `!…!`.** Since `eger`/`iken` bodies are
  written inside `( … )`, using the same character for grouping in expressions would
  make parsing ambiguous, so `!5 + 3! * 2` is written instead. The `!=` operator is
  told apart from grouping by looking at the next character.
- **Every statement starts with a keyword, and conditions end with `ise`.** The form
  `iken i < 10 ise ( … )` does not follow Turkish word order exactly ("while i is
  less than 10" would put `iken` at the end), but it lets the recursive descent
  parser decide what a statement is from its first token. `eger` and `iken` share
  the same pattern.
- **Statements end with `;` or `é`.** The two are fully equivalent. `é` is the only
  multi-byte (UTF-8: `0xC3 0xA9`) character the lexer recognizes specially; it is
  kept in the language as an example of matching a non-ASCII character at the
  byte level.

## Compiler Stages

- **Lexer** (`src/lexer.cpp`) — tokenizes keywords, numbers, strings, `&variable`
  interpolation and the UTF-8 `é` character
- **Parser** (`src/parser.cpp`) — recursive-descent parser producing an AST, with
  line-numbered error messages
- **Compiler** (`src/compiler.cpp`) — generates bytecode from the AST (conditional
  and unconditional jumps, string concatenation)
- **VM** (`src/vm.cpp`) — stack-based virtual machine with 18 opcodes
- `--debug` mode: prints the token list and a bytecode disassembly

## IDEs

- **desktop-ide/** — desktop IDE built with Electron + Monaco Editor (syntax
  highlighting, autocompletion, built-in examples, compile & run with `Ctrl+Enter`)
- **web-ide/** — browser-based IDE with a Node.js/Express backend (started with
  `SSN_IDE.bat`)

Both IDEs use the same compiler built from `src/` (`ssn`). They look for it in
this order: the `SSN_COMPILER` environment variable, then `build/Release/ssn.exe`,
`build/ssn.exe` and `build/ssn` in the project root, and finally a legacy
`compiler.exe` next to the IDE.

## Building and Running

Compiler (requires CMake and a C++17 compiler):

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

```bash
ssn program.tc                  # runs the program
ssn --debug program.tc          # also prints tokens and bytecode
```

Desktop IDE (build the compiler first as shown above):

```bash
cd desktop-ide
npm install
npm start
```

## Tests

The output of every `.tc` program under `tests/ornekler/` is compared with the
`.out` file next to it; programs under `tests/hatali/` are expected to fail with an
error. The tests run on GitHub Actions on every push.

```bash
bash tests/run_tests.sh build/ssn
```

## Tech Stack

C++17 · CMake · Electron · Monaco Editor · Node.js / Express
