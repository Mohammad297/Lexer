# Lexer

A customizable, language-agnostic C++ lexer library.

Can be used to tokenize source code of a custom grammar regardless of language by passing in the reserved symbols with their expected token types. It expects operators and delimiters, and basic literals common in most programming languages.

## Build

### Installation

You can clone this repository:

`git clone https://github.com/Mohammad297/Lexer`

### Integration

You can use CMake to include this library to your project.
Using **`FetchContent`** will handle downloading it from this repository.

```cmake
include(FetchContent)

FetchContent_Declare(
    remote_lexer
    GIT_REPOSITORY https://github.com/Mohammad297/Lexer.git
    GIT_TAG        main # Or use a specific commit hash
)
FetchContent_MakeAvailable(remote_lexer)

# Link it to your target (e.g., your parser library or executable)
target_link_libraries(your_project_target PRIVATE lexer_engine)
```

Then inside the source code, these public endpoints will be available to be included:

```cpp
#include <lex/engine.hpp>
#include <lex/token.hpp>
#include <lex/token_stream.hpp>
```

### Dependencies

- CMake 3.15
- C++ 20
- Python 3.12 for test generation using `tests\gen_test.py`
- tqdm Python library for the progress bar

## Usage

Since it is a utility library, you can either link to it, or run tests via the provided test `main.cpp`.

### Testing

Run `python tests\gen_test.py target-file token-count` to generate approximately `token-count` tokens in a pseudo-syntax of a combination of predefined keywords in `target-file`.  
**Warning:** Doing this will overwrite the contents of any existing file with that name.  
The generated file's name can be passed as a parameter to the testing CLI.  
**Example:** `./out/build/x64-release/Lexer/Lexer.exe ./Lexer/tests/all_tokens_10mil.txt` produces an output like:

    File: ./Lexer/tests/all_tokens_10mil.txt
    Tokens:
    <typename: 'uint'> <keyword: 'continue'> <int-literal: '+3'> <minus: '-'> <bool-literal: 'true'> <minus: '-'> <bool-literal: 'true'> <keyword: 'while'> <bool-literal: 'true'> <plus: '+'> ...
    <keyword: 'break'> <keyword: 'for'> <keyword: 'continue'> <keyword: 'const'> <keyword: 'default'> <int-literal: '+3'> <bitwise-or: '|'> <bool-literal: 'true'> <bitwise-or: '|'> <bool-literal: 'true'>
    Tests:
            ./Lexer/tests/all_tokens_10mil.txt:     1s248ms         tokens: 9221200

Each identified token is in the format of:

- if it has a body: `<type: 'body'>`
- otherwise: `<type>`

Token output above 50 tokens is truncated, showing the first and last 10 tokens only.
Multiple test files can be passed as parameters, which will be tokenized and printed in order, then an overview of each with execution time and token count.

## API

Under the `namespace lex`, the following are available:

### Public Headers:

- `token.hpp` - Contains the essential lexer definitions, token types and structs, and their `to_string()` functions.
- `engine.hpp` - Contains the `class Engine` that:
  - is constructed with reserved strings via `template <typename Iter> explicit Engine(Iter firstReserved, Iter lastReserved){...}`
  - manages tokenization calls via `TokenOut Engine::processFile(std::string_view filename) const`
- `token_stream.hpp` - Contains the `class TokenStream` that:
  - wraps a `lex::TokenOut` and walks through it using methods like `peek()`, `consume()`, `match(TokenType type)`, etc.
