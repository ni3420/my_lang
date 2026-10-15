# myl Language Specification (MVP)

This document describes the subset of the `myl` language that the current
compiler actually implements. Anything not listed here is **not supported**
(see [Deferred features](#deferred-features)).

## Source files

- Extension: `.myl`
- Encoding: ASCII/UTF-8 bytes; the compiler treats input as a byte stream.
- Comments: `// line comment` and `/* block comment */`.

## Types

| Type     | Literal example       | LLVM representation |
|----------|-----------------------|---------------------|
| `int`    | `42`                  | `i64`               |
| `float`  | `3.14`                | `double`            |
| `bool`   | `true`, `false`       | `i1`                |
| `string` | `"hello"`             | `i8*`               |
| `void`   | — (function results)  | `void`              |

There is no implicit conversion between strings and numeric types. Numeric
`int`/`float` operands are combined to `float` when either side is `float`.

## Variables

```myl
let x = 10;            // type inferred from the initializer
let y: int = 20;       // explicit type
let mut counter = 0;   // mutable binding
counter = counter + 1; // OK
```

- A binding introduced with `let` (without `mut`) is **immutable**.
- Assigning to an immutable binding is a compile-time error.
- `let mut` allows reassignment with `=`.
- The declared type and the initializer must be compatible.

## Expressions

Operators, from lowest to highest precedence:

1. `or`
2. `and`
3. `==` `!=`
4. `<` `<=` `>` `>=`
5. `+` `-`
6. `*` `/` `%`
7. unary `-`, `!`, `not`
8. primary: literals, identifiers, calls, `( ... )`

Logical operators require `bool` operands. Arithmetic and comparison operators
require numeric operands.

## Statements

```myl
// expression statement
foo(1, 2);

// assignment (target must be a `let mut` binding)
counter = counter - 1;

// if / else
if (x > 10) {
    print(x);
} else {
    print(0);
}

// while
while (counter > 0) {
    counter = counter - 1;
}

// return
return 0;
```

Braces introduce a new lexical scope.

## Functions

```myl
fn add(a: int, b: int) -> int {
    return a + b;
}

fn log(message: string) {     // omitted return type means `void`
    print(message);
}
```

- Parameters always require an explicit type.
- A non-`void` function must contain a `return`.
- Calls are checked for argument count and argument types.
- The function named `main` is the entry point for `myl build` / `myl run`.

## Hindi keyword aliases

Every core keyword has a Hinglish alias. The two spellings are interchangeable
and can be mixed freely; they produce identical ASTs and semantics.

| Hindi keyword | English equivalent | Meaning |
|---------------|--------------------|---------|
| `rakho x = 10;` | `let mut x = 10;` | mutable variable declaration |
| `bolo expr;`  | `print(expr);`     | print statement (no parentheses) |
| `agar`        | `if`               | conditional |
| `warna`       | `else`             | alternative branch |
| `repeat`      | `while`            | loop |
| `kaam`        | `fn`               | function declaration |
| `bhejo`       | `return`           | return statement |
| `sahi` / `galat` | `true` / `false` | boolean literals |
| `ank`         | `int`              | integer type |

`rakho` always introduces a **mutable** binding (it is `let mut`, never plain
`let`).

```myl
rakho x = 10;
agar (x > 5) {
    bolo "bada";
} warna {
    bolo "chhota";
}
repeat (x > 0) { x = ghatao(x, 1); }

kaam badhao(a: ank, b: ank) -> ank {
    bhejo jodo(a, b);
}
```

## Script mode (implicit `main`)

A file may contain top-level statements instead of a `main` function. In that
case the compiler wraps all top-level statements into an implicit
`kaam main()` / `fn main()` that returns `0`. Function declarations stay as
they are. This lets small programs be written without a `main` wrapper:

```myl
rakho x = 10;
bolo "Namaste Duniya!";   // prints Namaste Duniya!
bolo x;                   // prints 10
```

If the file declares its own `main` **and** has top-level statements, the
implicit `main` is emitted as a second symbol; avoid mixing the two styles.

## Built-in functions

| Call | Behavior |
|------|----------|
| `print(value)` | Prints one value followed by a newline. Accepts `int`, `float`, `bool`, or `string`. |
| `jodo(a, b)`   | Addition: `a + b`. Numeric operands; result is `float` if either operand is `float`. |
| `ghatao(a, b)` | Subtraction: `a - b`. |
| `guna(a, b)`   | Multiplication: `a * b`. |
| `bhaag(a, b)`  | Division: `a / b` (integer `/` truncates toward zero). |

## Example

```myl
fn add(a: int, b: int) -> int {
    return a + b;
}

fn main() -> int {
    let x = 10;
    let message = "Hello, World!";

    print(message);
    print(x + 20 * 3);        // 70
    print(add(10, 20));        // 30

    if (x > 10) {
        print(x);
    } else {
        print(0);
    }

    let mut counter = 10;
    while (counter > 0) {
        counter = counter - 1;
    }

    return 0;
}
```

## Diagnostics

Errors are written to `stderr` in the form:

```
<file>:<line>:<column>: <message>
```

Examples:

```
examples/type_error.myl:4:15: arithmetic operands must be numeric
examples/invalid_syntax.myl:3:13: syntax error, unexpected Semicolon
```

## Deferred features

Not implemented (documented so they are not mistaken for working):

- Arrays, structs, and user-defined types.
- Modules / imports.
- `for` loops, `break`, `continue`.
- Global variables declared outside functions.
- String concatenation or formatting.
- Forward references to functions defined later in the same file.
- Lexical shadowing in code generation (semantic scoping is enforced, but the
  code generator uses a flat variable map).
