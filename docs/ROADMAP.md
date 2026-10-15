# Roadmap

This roadmap is intentionally incremental. Each item lists the concrete work
required; nothing is considered done until it is built and tested in Docker.

## Near term

1. **Resolve the dangling-`else` ambiguity.**
   The grammar currently reports 4 shift/reduce conflicts (both `if`/`else` and
   `agar`/`warna`), resolved by Bison shifting (conventional behavior). Make
   the rule explicit or document it with `%expect`.

2. **Scoped code generation.**
   Replace the flat variable map in `CodeGenContext` with a scope stack so
   lexical shadowing and per-function symbols work correctly.

3. **Forward references / prototypes.**
   Pre-declare all functions before generating bodies so functions can be
   called before their definition.

4. **Return-path analysis.**
   Verify that every control-flow path in a non-void function returns.

5. **Better diagnostics.**
   Colored/structured messages, error recovery in the parser, and multiple
   errors per run.

## Language features

6. `for` loops, `break`, `continue`.
7. Arrays and indexing.
8. Structs (named aggregate types).
9. String concatenation and richer `print` formatting.
10. Modules / imports and a compilation unit story.

## Tooling

11. Debug information (`llvm::DIBuilder`).
12. JIT execution (`llvm::ExecutionEngine`) for `myl run`.
13. Language server (LSP) for editor support.
14. Golden-file test harness for diagnostics.

## Infrastructure

15. Pin GitHub Actions to immutable commit SHAs.
16. Cache the Docker layers in CI to shorten feedback.
17. Restricted execution profile for untrusted programs (dropped
    capabilities, read-only rootfs, cpu/memory/pid limits, no network).
