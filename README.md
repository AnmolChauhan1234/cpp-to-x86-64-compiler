# MyCompiler

A simple compiler written in C++ that translates a custom programming language into x86-64 assembly code. This compiler demonstrates the fundamental phases of compilation: lexical analysis, parsing, and code generation.

## Features

- **Custom Programming Language**: Simple syntax supporting constants, arithmetic operations, comparisons, and control flow
- **Multi-phase Compilation**: Clean separation of tokenization, parsing, and code generation
- **x86-64 Assembly Output**: Generates native assembly code for Linux x86-64 systems
- **Expression Evaluation**: Support for complex arithmetic and comparison expressions
- **Variable Scoping**: Block-scoped constant variables with shadowing support
- **Memory Management**: Custom arena allocator for efficient AST node allocation
- **Conditional Statements**: Support for if/else/elif control flow statements
- **Print Statements**: Built-in print functionality for debugging and output
- **Unary Operations**: Support for unary minus (negation) operator
- **Extended Operators**: Complete set of comparison operators (==, !=, <, >, <=, >=) and modulo (%)
- **Type System**: Explicit type declarations with integer data type support

## Language Syntax

### Data Types

- **Integers**: Literal integer values (e.g., `42`, `100`)

### Variables

- **Constants**: Immutable variables declared with explicit type

```
const int x = 10;
const int y = x + 5;
```

### Expressions

- **Arithmetic Operations**: `+`, `-`, `*`, `/`, `%`
- **Comparison Operations**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Unary Operations**: `-` (negation)
- **Parentheses**: For grouping expressions `(expression)`

### Statements

- **Constant Declaration**: `const type identifier = expression;`
- **Exit Statement**: `exit expression;` (terminates program with expression value as exit code)
- **Print Statement**: `print expression;` (outputs expression value)
- **Block Scope**: `{ statements... }` for scoped variable declarations
- **Conditional Statements**:
  - `if (condition) { statements... }`
  - `if (condition) { statements... } else { statements... }`
  - `if (condition) { statements... } elif (condition) { statements... } else { statements... }`

### Example Program

```
const int x = 10;
const int y = 20;
{
    const int z = x + y;
    print z;
    if (z > 25) {
        print 1;
        exit 1;
    } else {
        print 0;
        exit 0;
    }
}
```

## Prerequisites

- **C++ Compiler**: Supporting C++20 standard (GCC 10+ or Clang 10+)
- **CMake**: Version 3.20 or higher
- **NASM**: Netwide Assembler for x86-64
- **GNU Binutils**: For linking (`ld`)
- **Linux x86-64**: Currently targets Linux systems

### Installing Prerequisites

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install build-essential cmake nasm binutils
```

**Arch Linux:**

```bash
sudo pacman -S base-devel cmake nasm binutils
```

**Fedora:**

```bash
sudo dnf install gcc-c++ cmake nasm binutils
```

## Building

### Option 1: Using Makefile (Recommended)

1. **Clone the repository:**

```bash
git clone https://github.com/02YashRajput/mycompiler.git
cd mycompiler
```

2. **Build using Make:**

```bash
make build    # Configure with CMake
make run      # Build and run with input.txt
```

### Option 2: Using CMake Directly

1. **Clone the repository:**

```bash
git clone https://github.com/02YashRajput/mycompiler.git
cd mycompiler
```

2. **Create build directory:**

```bash
mkdir -p build
cd build
```

3. **Configure and build:**

```bash
cmake ..
cmake --build .
```

## Usage

### Basic Usage

1. **Create a source file** (e.g., `program.txt`):

```
const int result = 15 + 10;
print result;
exit result;
```

2. **Compile and run:**

```bash
./build/mycompiler program.txt
./out
echo $?  # Shows the exit code
```

### Using Make Commands

The project includes a Makefile with convenient targets:

```bash
make clean    # Clean build directory
make build    # Configure with CMake
make run      # Build and run with input.txt
```

### Using the Convenience Script

The project includes a `run.sh` script that automates the build and execution process:

```bash
chmod +x run.sh
./run.sh
```

This script will:

- Build the compiler using CMake
- Compile the `input.txt` file
- Execute the resulting binary
- Display the exit code

### Step-by-Step Process

The compiler follows these phases:

1. **Tokenization**: Converts source code into tokens
2. **Parsing**: Builds an Abstract Syntax Tree (AST)
3. **Code Generation**: Produces x86-64 assembly code
4. **Assembly**: Uses NASM to create object files
5. **Linking**: Uses LD to create executable

## Project Structure

```
mycompiler/
├── src/
│   ├── main.cpp           # Main driver program
│   ├── tokenization.hpp   # Lexical analyzer
│   ├── parser.hpp         # Parser and AST definitions
│   ├── generator.hpp      # x86-64 code generator
│   └── arenaAllocator.hpp # Memory allocator for AST nodes
├── CMakeLists.txt         # Build configuration
├── Makefile              # Make build targets
├── Dockerfile             # Container build setup
├── docker-compose.yaml    # Container orchestration
├── flake.nix             # Nix development environment
├── run.sh                # Convenience build/run script
├── input.txt             # Example source code
└── README.md             # This file
```

## Architecture

### Tokenizer (`tokenization.hpp`)

- Converts source code into a stream of tokens
- Handles keywords, operators, identifiers, and literals
- Performs basic syntax validation

### Parser (`parser.hpp`)

- Implements recursive descent parsing
- Builds Abstract Syntax Tree (AST)
- Handles operator precedence and associativity
- Uses arena allocator for memory management

### Code Generator (`generator.hpp`)

- Traverses AST and generates x86-64 assembly
- Manages register allocation and stack operations
- Implements variable scoping and symbol tables
- Handles system calls for program termination

## Development

### Docker Support

Build and run using Docker:

```bash
# Using Make targets
make docker-run     # Start container
make docker-exec    # Execute into container
make docker-clean   # Clean up containers

# Using docker-compose directly
docker-compose build
docker-compose run compiler
```

### Nix Support

For reproducible development environment:

```bash
nix develop  # Enter development shell
```

## Examples

### Simple Arithmetic with Print

```
const int a = 10;
const int b = 20;
const int sum = a + b;
print sum;
exit sum;
```

### Conditional Statements

```
const int x = 15;
const int y = 10;
if (x > y) {
    print 1;  # Prints 1 (true)
    exit 1;
} else {
    print 0;
    exit 0;
}
```

### If-Elif-Else Chain

```
const int score = 85;
if (score >= 90) {
    print 1;  # Grade A
} elif (score >= 80) {
    print 2;  # Grade B
} elif (score >= 70) {
    print 3;  # Grade C
} else {
    print 4;  # Grade F
}
exit 0;
```

### Scoped Variables with Unary Operations

```
const int global = 100;
{
    const int local = 50;
    const int negative = -local;
    const int result = global + negative;
    print result;
    exit result;
}
```

### Complex Expressions with All Operators

```
const int a = 10;
const int b = 3;
const int result = (a + b) * 2 - a % b;
print result;
if (result != 25) {
    exit 1;
} else {
    exit 0;
}
```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Make your changes and add tests
4. Commit your changes: `git commit -am 'Add new feature'`
5. Push to the branch: `git push origin feature-name`
6. Submit a pull request

## Future Enhancements

- [ ] Implement loops (`while`, `for`)
- [ ] Add function definitions and calls
- [ ] Support for more data types (strings, booleans, floats)
- [ ] Enhanced error reporting with line numbers and column numbers
- [ ] Optimization passes for generated assembly
- [ ] Support for additional target architectures
- [ ] Variable assignment and mutation
- [ ] Arrays and data structures
- [ ] String literals and string operations
- [ ] Boolean literals and logical operators (&&, ||, !)

## License

This project is open source. See the repository for license details.

## Author

Created by [02YashRajput](https://github.com/02YashRajput)
