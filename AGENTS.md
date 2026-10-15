Build My Custom Programming Language — Docker-First, Tested MVP, Scalable Architecture

Act as a senior compiler engineer, C++ architect, LLVM expert, Docker engineer, and CI/CD engineer.

Build a working MVP of my own programming language using:

C++ as the primary implementation language
Flex for lexical analysis
GNU Bison for parsing
LLVM for IR generation, optimization, and native code generation
CMake for the build system
CTest for automated testing
Docker for a reproducible development and testing environment
Docker Compose for convenient developer workflows where useful
GitHub Actions for automated CI

The most important requirement: implement, build, test, and run the actual project. Do not stop after creating a plan or generating files.

Docker must be the primary supported environment for building and testing the compiler. Developers should not need to install Flex, Bison, LLVM, or a C++ toolchain directly on their host machines.

1. Inspect the Repository Before Changing Anything

First inspect the actual workspace and determine:

Existing files, directories, and Git status
Existing AGENTS.md and other project instructions
Operating system and available shell
Docker and Docker Compose availability
C++ compiler, CMake, Flex, Bison, and LLVM versions
Existing tests, examples, build artifacts, and documentation

Use the actual available tools and terminal rather than assuming commands exist.

Preserve useful existing work. Do not overwrite files unnecessarily.

If Docker is unavailable, explain the precise blocker and continue implementing what can be completed safely. Do not claim that Docker builds or tests passed if they were not executed.

2. Create Permanent Project Instructions

Create a comprehensive root-level AGENTS.md.

It must be the authoritative project-specific guide for future AI coding sessions.

Document:

Project goals and MVP boundaries
Language syntax and semantics
Compiler pipeline and module responsibilities
Directory structure
C++ conventions and ownership rules
AST design and semantic analysis
LLVM version and compatibility decisions
Docker development workflow
Build, test, debugging, and execution commands
Error diagnostics
Dependency management
Testing expectations
Security considerations
How to implement and document new language features
Architecture rules that prevent unnecessary coupling
Future roadmap

Keep this file synchronized with the implementation. Never document an unimplemented feature as working.

3. Docker-First Development Environment

Create and maintain these files:

Dockerfile
.dockerignore
docker-compose.yml
CMakeLists.txt
CMakePresets.json, if appropriate
.github/workflows/ci.yml

The Docker image must provide compatible versions of:

A supported C++ compiler and standard library
CMake
Ninja or Make
Flex
GNU Bison
LLVM development libraries
LLVM tools such as llvm-config and the required native code-generation tools
Git and other necessary build dependencies

Choose an LLVM version supported by the implementation and available packages. Verify compatibility among LLVM, the C++ standard, the host compiler, and CMake.

Pin or explicitly document important dependency versions. Avoid latest tags and unverified package names.

Use a stable Linux base image, a non-root development user where practical, and a reproducible build configuration.

The Docker image must support configuring, building, testing, and executing the compiler.

Do not require privileged containers.

Use multi-stage builds when they provide a meaningful benefit. Keep the development image easy to debug and the runtime image minimal where appropriate.

Docker Compose workflow

Provide convenient, documented commands equivalent to:

docker compose build
docker compose run --rm dev
docker compose run --rm test


Configure these services and targets to perform their advertised actions.

The dev service should provide a development shell or build environment.

The test service must build the project if needed and run the automated test suite, returning a nonzero exit code on failure.

Mount the source directory into the development container when appropriate so edits on the host are visible inside Docker.

Ensure container-generated files and permissions do not unnecessarily disrupt the host workspace.

Provide clear Windows PowerShell and Unix shell instructions where command syntax differs.

4. Implement the Programming Language MVP

Use .myl for source files and myl as the compiler executable name.

Support these features incrementally and test each one.

Variables and primitive types
let x = 10;
let y = 20;
let message = "Hello, World!";
let enabled = true;


Support documented integer, floating-point, boolean, string, and void types where appropriate.

Define variable mutability consistently. If reassignment is supported, introduce a clear syntax or rule rather than silently treating immutable bindings as mutable.

Expressions
let result = 10 + 20 * 3;
print(result);


Support arithmetic, parentheses, operator precedence, unary minus, comparisons, and boolean operators.

Control flow
if (x > 10) {
    print(x);
} else {
    print(0);
}

while (x > 0) {
    x = x - 1;
}

Functions
fn add(a: int, b: int) -> int {
    return a + b;
}

let result = add(10, 20);
print(result);


Implement function declarations, parameters, return types, function calls, scope handling, and return validation.

Keep the MVP small and correct. Document deferred features instead of pretending they work.

5. Implement the Real Compiler Pipeline

Implement this pipeline:

Source File → Flex Lexer → Bison Parser → AST → Semantic Analysis → LLVM IR Generation → LLVM IR Verification → Optional IR Optimization → Native Object Code or Executable

Use:

Flex for tokens and source locations
Bison for grammar and syntax errors
A separately defined AST
Semantic analysis for names, types, scopes, function calls, and return types
LLVM C++ APIs for IR construction and target code generation

Generate lexer and parser sources through CMake rather than manually maintaining generated files.

Keep compiler stages modular. Use RAII and clear ownership conventions. Avoid global mutable compiler state, fragile string-generated LLVM IR, and unnecessary abstractions.

Verify generated LLVM IR before emitting executable code.

6. Implement a Functional CLI

Implement and document these commands:

myl --help
myl --version
myl check examples/hello.myl
myl emit-ir examples/hello.myl -o hello.ll
myl build examples/hello.myl -o hello
myl run examples/hello.myl


Requirements:

check parses and performs semantic analysis without producing an executable.
emit-ir writes valid LLVM IR.
build generates a native executable using LLVM-supported code generation and a suitable system linker.
run compiles and runs the program using a clearly implemented execution workflow.
Errors produce useful diagnostics and meaningful exit codes.
Compiler diagnostics go to stderr; program output goes to stdout where practical.
Platform-specific output names and executable extensions are handled correctly.

Do not advertise unsupported flags or commands.

7. Automated Testing Is Mandatory

Create automated unit and integration tests.

Cover:

Lexical analysis
Parser precedence and grammar
Invalid syntax
Variables and scope resolution
Type errors
Undefined identifiers
Function calls and arguments
Return-type validation
Conditional statements
Loops
LLVM IR verification
Native executable generation
Runtime output
Runtime exit codes
Malformed input and useful diagnostics

Include valid and invalid .myl programs.

Use CTest and, where appropriate, small shell or CMake-based integration-test helpers.

Integration tests must verify actual program behavior, not just whether the compiler process starts.

Examples of expected behavior:

A hello-world program compiles and prints the expected output.
Arithmetic respects operator precedence.
Functions return correct results.
Conditional statements choose the correct branch.
Loops terminate and produce expected results.
Invalid syntax fails with a nonzero exit code.
Type errors are rejected before code generation.
Invalid LLVM IR is never accepted as a successful compilation result.

Avoid brittle tests that depend on exact absolute paths or irrelevant wording in diagnostics.

8. Actually Build, Test, and Run Inside Docker

Do not finish after creating the Dockerfile.

Execute the real workflow whenever Docker is available.

Use commands equivalent to:

docker compose build
docker compose run --rm test


The Docker test service must:

Configure CMake.
Generate the Flex lexer and Bison parser.
Compile the C++ compiler.
Run CTest.
Execute integration tests against generated native programs.
Return a nonzero exit code if any required step fails.

Also perform a manual end-to-end smoke test inside the container:

Build the myl compiler.
Compile examples/hello.myl.
Run the resulting native executable.
Verify its output.
Test at least one arithmetic program.
Test at least one intentionally invalid program.
Verify that invalid input fails correctly.

Capture the actual command results. Fix failures and rerun the relevant tests.

If the environment does not allow a command to run, state exactly which command could not be executed and why. Never fabricate successful output or test results.

9. Docker Security and Runtime Behavior

Do not execute untrusted programs with unrestricted host access.

For test execution and potentially untrusted .myl programs, use suitable isolation where practical:

Run as a non-root user
Drop unnecessary Linux capabilities
Disable network access when unnecessary
Apply CPU, memory, and process limits
Use a read-only filesystem when feasible
Restrict writable directories
Avoid mounting the Docker socket
Avoid privileged mode

Do not claim Docker is a complete security boundary for arbitrary hostile native executables.

Keep ordinary development commands convenient while documenting a separate restricted execution mode for untrusted programs where feasible.

10. GitHub Actions CI

Create .github/workflows/ci.yml.

On pushes and pull requests, the workflow must:

Check out the repository.
Build the Docker image.
Configure and build the compiler in Docker.
Run automated tests.
Run end-to-end integration tests.
Fail the workflow when required checks fail.

Use supported GitHub Actions versions and pin third-party actions to immutable commit SHAs where practical.

Do not make CI depend on developer-specific local paths.

Keep the Docker and CI test commands aligned so the same core test suite runs locally and remotely.

11. Maintainable Architecture

Use a structure similar to:

.
├── AGENTS.md
├── README.md
├── Dockerfile
├── .dockerignore
├── docker-compose.yml
├── CMakeLists.txt
├── CMakePresets.json
├── .github/
│   └── workflows/
│       └── ci.yml
├── docs/
│   ├── LANGUAGE_SPEC.md
│   ├── ARCHITECTURE.md
│   ├── BUILDING.md
│   └── ROADMAP.md
├── include/
│   └── myl/
├── src/
├── tests/
└── examples/


Adapt the structure to the real repository. Do not create empty directories or placeholder modules without a purpose.

Separate frontend, AST, semantic analysis, code generation, and CLI responsibilities.

Keep the design ready for future arrays, structs, modules, improved diagnostics, debug information, JIT execution, and language-server support. Do not implement these prematurely.

12. Documentation and Developer Experience

The README must provide a complete first-time workflow:

Prerequisites
Docker installation references
Building the Docker image
Starting a development shell
Building the compiler
Running tests
Compiling and running a .myl program
Using the CLI
Troubleshooting common failures
Cleaning up build artifacts

Include exact commands that work with the repository's actual file structure and Docker configuration.

Create a language specification, architecture document, build guide, and roadmap.

Keep generated files and build artifacts out of version control.

13. Required Development Workflow

Follow this sequence:

Inspect the repository and current environment.
Identify existing instructions and dependencies.
Create or update AGENTS.md.
Design the smallest working Docker environment.
Implement the minimum buildable compiler skeleton.
Add Flex and Bison.
Add AST and semantic analysis.
Add LLVM IR generation and verification.
Implement native compilation and CLI commands.
Add unit tests and integration tests.
Build and test inside Docker.
Run actual .myl programs inside Docker.
Fix failures and rerun tests.
Verify GitHub Actions configuration.
Update all documentation to match the verified implementation.

Work incrementally and keep the project buildable.

Do not stop to ask for permission for ordinary, reversible implementation choices. Ask only when a genuine blocker or destructive decision requires input.

14. Final Completion Report

At completion, report:

Files created and modified
Implemented language features
Docker image and Compose workflow
Build commands executed
Test commands executed
Actual pass/fail results
End-to-end program output
Invalid-program test results
Whether Docker was genuinely available
Any tests or commands that could not be executed
Known limitations
Prioritized next steps

Clearly distinguish implemented, tested, partially implemented, and deferred features.

Definition of done: A developer can clone the repository, build the Docker image, compile the C++ compiler inside the container, run automated tests, compile a .myl program into a native executable, execute it, and receive correct output. All of this must be supported by actual execution evidence, not assumptions.

Start by inspecting the repository. Then implement the project, run the real Docker workflow, fix failures, and report verified results.

---

# Appendix: Implementation Status (verified)

This appendix is kept in sync with the code. It records what is actually
implemented and verified. Nothing here is aspirational.

## Verified toolchain
Ubuntu 22.04, Clang/LLVM 15.0.7, Flex 2.6.4, Bison 3.8.2, CMake 4.2.3, Ninja.

## Verified evidence
- `docker compose build test` / `docker build --target test` configures CMake,
  generates the Flex lexer and Bison parser, builds `myl`, and runs CTest
  (12/12 tests pass).
- `docker run --rm my_lang-test bash tests/smoke.sh` runs the full end-to-end
  smoke suite and passes.
- `bash tests/smoke.sh` builds and runs real programs:
  `hello.myl` -> `Hello, World!` / `70`, `functions.myl` -> `30`,
  `arithmetic.myl` -> `70` / `90` / `75` / `1`;
  `hindi.myl` -> `Namaste Duniya!` / `70` / `sahi`,
  `hindi_functions.myl` -> `30` / `sahi`; `invalid_syntax.myl`,
  `type_error.myl`, and `hindi_type_error.myl` fail with diagnostics and
  nonzero exit codes.

## Implemented
- Flex lexer (`src/lexer.l`) and Bison parser (`src/parser.y`) building the AST.
- AST (`include/myl/ast.hpp`, `src/ast.cpp`).
- Semantic analysis (`src/semantic.cpp`): scopes, name resolution, mutability,
  type checks, function arity/argument checking, return validation.
- LLVM 15 IR generation + verification, optimization, native object/executable
  generation (`src/codegen.cpp`).
- Built-in `print` for int/float/bool/string values.
- Hindi keyword aliases (Hinglish syntax), all interchangeable with the English
  spelling: `rakho` (= `let mut`, always mutable), `bolo expr;` (= `print`
  statement, no parentheses), `agar`/`warna` (`if`/`else`), `repeat` (`while`),
  `kaam` (`fn`), `bhejo` (`return`), `sahi`/`galat` (`true`/`false`),
  `ank` (`int`).
- Built-in arithmetic functions `jodo`/`ghatao`/`guna`/`bhaag` (add/sub/mul/div)
  with numeric type checking; result is `float` if either operand is `float`.
  Names are reserved in the global scope.
- Script mode: top-level statements are wrapped into an implicit `main`
  returning `0` (`StmtDecl` in the AST).
- CLI: `check`, `emit-ir`, `build`, `run`, `--help`, `--version`.
- Language: variables (`let` / `let mut`), typed and inferred declarations,
  assignment, arithmetic/comparison/logical operators with precedence, unary
  `-`/`!`/`not`, `if`/`else`, `while`, functions with parameters/return types,
  return statements, `//` and `/* */` comments.
- Unit tests (lexer, parser, semantic, codegen) and integration tests that
  execute compiled programs and verify their output.

## Deferred (documented, not implemented)
- Arrays, structs, modules/imports, `for`, `break`/`continue`.
- True global variables shared across functions (top-level `let`/`rakho`
  become locals of the implicit `main`); string concatenation/formatting.
- Debug information, JIT execution, language server.

## Known limitations
- The grammar has 4 shift/reduce conflicts (classic dangling-`else`); Bison
  resolves them by shifting, i.e. `else`/`warna` binds to the nearest
  `if`/`agar`.
- Script mode (`main` synthesized from top-level statements) is emitted after
  user functions. Declaring `main` explicitly *and* using top-level statements
  produces an extra `main` symbol; use one style per file.
- Integers are i64 and `main` is emitted as `i64 @main()`, linked via
  `cc -no-pie` (works on x86-64 System V; exit status uses the low 32 bits).
- Codegen uses a flat variable map (no lexical shadowing) and cannot reference
  functions defined later in the file.
- Semantic return analysis only checks for a top-level `return`, not all paths.
