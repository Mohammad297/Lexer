# Lexer

A customizable, language-agnostic C++ lexer library.

Can be used to tokenize source code of a custom grammar regardless of language by passing in the reserved symbols with their expected token types. It expects operators and delimiters, and basic literals common in most programming languages.

## Install

    git clone https://github.com/Mohammad297/Lexer

### Build

Run `build.bat release` or `build.bat debug` inside x64 Native Tools Command Prompt (Developer Command Prompt for VS).

Doing _selected-config_ from above will generate:

1. A static `.lib` library in `lib\selected-config` named `lexer.lib`
2. A test CLI in `bin\selected-config` with the name `lexer_test.exe` and suffix `_debug` for debug builds
3. Object files and other intermediate files in `build\selected-config`
   - To clean files produced in **1-3** above, run `build.bat clean`
   - Files produced in **4-6** are **NOT DELETED** by running `build.bat clean`
   - `DIST_DIR` is defined in `build.bat` as a target global C++ library directory

4. A copy of the public API in `src\` into `DIST_DIR\include\lex`
5. A copy of the test CLI in `bin\selected-config` into `DIST_DIR\bin`
6. A copy of the static `.lib` library in `lib\selected-config` into `DIST_DIR\lib\selected-config`

### Dependencies

The project structure currently only supports compilation with MSVC using VSCode.

- C++ 20
- MSVC
- VSCode
- Python 3.12+ for test generation using `tests\gen_test.py`
- tqdm Python library for the progress bar

## Usage

Since it is a utility library, you can either statically link to it, or run tests.

### Testing

Run `python tests\gen_test.py target-file token-count` to generate approximately `token-count` tokens in a pseudo-syntax of a combination of predefined keywords in `target-file`.  
**Warning:** Doing this will overwrite the contents of any existing file with that name.  
The generated file's name can be passed as a parameter to the testing CLI.  
**Example:** `bin\release\lexer_test.exe \tests\keywords_10mil.txt` produces an output like:

    File: \tests\keywords_10mil.txt
    Unknown tokens: []
    Tokens:
    <keyword: 'default'> <keyword: 'default'> <keyword: 'if'> <keyword: 'switch'> <keyword: 'switch'> <keyword: 'if'> <keyword: 'case'> <keyword: 'switch'> <keyword: 'static'> <keyword: 'while'> ...
    <keyword: 'continue'> <keyword: 'default'> <keyword: 'while'> <keyword: 'default'> <keyword: 'continue'> <keyword: 'break'> <keyword: 'switch'> <keyword: 'if'> <keyword: 'const'> <keyword: 'const'>
    Tests:
    \tests\keywords_10mil.txt: 1s332ms tokens: 10000001

Unrecognized tokens are printed between a single pair of square brackets before the other tokens. In this case, no unrecognized tokens.
Each identified token is in the format of:

- if it has a body: `<type: 'body'>`
- otherwise: `<type>`

Token output above 50 tokens is truncated, showing the first and last 10 tokens only.
Multiple test files can be passed as parameters, which will be tokenized and printed in order, then an overview of each with execution time and token count.

## API

Under the `namespace lex`, the following are available:

### Public Headers:

- `token.hpp` - Contains definitions of essential lexer definitions, token types and structs, and their `to_string()` functions.
- `engine.hpp` - Contains the `class Engine` that:
  - is constructed with reserved strings via `template <typename Iter> explicit Engine(Iter firstReserved, Iter lastReserved){...}`
  - manages tokenization calls via `TokenOut Engine::processFile(std::string_view filename) const`
- `token_stream.hpp` - Contains the `class TokenStream` that:
  - wraps a `lex::TokenOut` and walks through it using methods like `peek()`, `consume()`, `match(TokenType type)`, etc.
