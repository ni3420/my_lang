# myl — a small statically-typed language compiled with LLVM

`myl` is a small programming language and compiler built with:

| Component | Technology |
|-----------|------------|
| Lexer     | Flex       |
| Parser    | GNU Bison  |
| IR / codegen | LLVM 15 (C++ API) |
| Build     | CMake + Ninja |
| Tests     | CTest      |
| Environment | Docker / Docker Compose |
| CI        | GitHub Actions |

The compiler is named `myl` and reads `.myl` source files. It is meant to be
built and tested **inside Docker**, so you do not need LLVM, Flex, or Bison on
your host machine.

## Prerequisites

- Docker Engine or Docker Desktop (with the `docker compose` plugin).
- That's it. No local C++ toolchain, LLVM, Flex, or Bison is required.

> Windows: the commands below are shown for both PowerShell and a Unix shell.
> Run them from the repository root.

## Quick start

### 1. Build the toolchain image

```powershell
docker compose build
```

```bash
docker compose build
```

### 2. Start a development shell

```powershell
docker compose run --rm dev
```

```bash
docker compose run --rm dev
```

Inside the container the source tree is mounted at `/home/developer/project`.

### 3. Build the compiler

```powershell
docker compose run --rm dev bash -lc "cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build"
```

```bash
docker compose run --rm dev bash -lc "cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build"
```

### 4. Run the test suite

The dedicated `test` service builds the compiler from a clean copy of the
source baked into the image and runs the full CTest suite:

```powershell
docker compose run --rm test
```

```bash
docker compose run --rm test
```

For an end-to-end smoke test (compiles and runs real programs):

```powershell
docker run --rm my_lang-test bash tests/smoke.sh
```

```bash
docker run --rm my_lang-test bash tests/smoke.sh
```

### 5. Compile and run a `.myl` program

```powershell
docker compose run --rm dev bash -lc "./build/myl run examples/hello.myl"
```

```bash
docker compose run --rm dev bash -lc "./build/myl run examples/hello.myl"
```

Expected output:

```
Hello, World!
70
0
```

## Using the CLI

| Command | Description |
|---------|-------------|
| `myl --help` | Show usage |
| `myl --version` | Show compiler version |
| `myl check <file.myl>` | Parse and type-check only |
| `myl emit-ir <file.myl> -o out.ll` | Emit LLVM IR |
| `myl build <file.myl> -o prog` | Build a native executable |
| `myl run <file.myl>` | Compile and run |

`build` and `run` use LLVM code generation and the system `cc` linker
(`cc -no-pie`, x86-64 System V).

## A taste of the language

```myl
fn add(a: int, b: int) -> int {
    return a + b;
}

fn main() -> int {
    let x = 10;
    let mut counter = 5;
    while (counter > 0) {
        counter = counter - 1;
    }
    print(add(x, 20));   // prints 30
    return 0;
}
```

See [`docs/LANGUAGE_SPEC.md`](docs/LANGUAGE_SPEC.md) for the full grammar.

## Repository layout

```
.
├── AGENTS.md              # authoritative project guide for contributors/AI
├── README.md
├── Dockerfile             # base / dev / test stages
├── docker-compose.yml
├── CMakeLists.txt
├── CMakePresets.json
├── .github/workflows/ci.yml
├── docs/                  # language spec, architecture, build guide, roadmap
├── include/myl/           # public headers
├── src/                   # compiler implementation (lexer.l, parser.y, ...)
├── tests/                 # unit tests, integration tests, smoke test
└── examples/              # sample .myl programs
```

## Troubleshooting

- **`docker compose` is not recognized** — use a Compose V2 install
  (`docker compose`) rather than the legacy `docker-compose`.
- **Image build fails fetching LLVM** — the `Dockerfile` adds the
  `apt.llvm.org` repository for Ubuntu 22.04 (`jammy`). Ensure network access
  during `docker build`.
- **`myl run`/`myl build` fails to link** — the container needs the `cc`
  linker and glibc, which are provided by `build-essential` in the image.
- **Build artifacts end up owned differently on the host** — artifacts are
  written under `build/`, which is git-ignored. Delete it to start clean.

## Cleaning up

```powershell
docker compose run --rm dev bash -lc "rm -rf build"
docker image rm my_lang-test my_lang-dev 2>$null
```

```bash
docker compose run --rm dev bash -lc "rm -rf build"
docker image rm my_lang-test my_lang-dev 2>/dev/null || true
```

## License

Released for educational use. See `AGENTS.md` for project goals.
