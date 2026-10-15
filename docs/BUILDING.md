# Building and Testing

All commands assume Docker is installed and the repository is the working
directory. Nothing below requires LLVM, Flex, or Bison on the host.

## Docker workflow

### Build the images

```bash
docker compose build          # builds the dev and test images
```

`docker compose build test` performs a full configure + build + CTest run as
part of building the `test` image. If any test fails, the image build fails.

### Development shell (source bind-mounted)

```bash
docker compose run --rm dev
```

Inside the container:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run the tests

```bash
# from the host, against the source baked into the test image
docker compose run --rm test

# or re-run inside an existing dev container
ctest --test-dir build --output-on-failure
```

### End-to-end smoke test

```bash
docker run --rm my_lang-test bash tests/smoke.sh
```

## CMake options

- `CMAKE_BUILD_TYPE` — `Debug` or `Release` (default in compose: `Release`).
- `CMakePresets.json` provides `default` and `debug` presets configured for
  `clang-15`/`clang++-15`.

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

## Manual (inside the container)

```bash
./build/myl --help
./build/myl check  examples/hello.myl
./build/myl emit-ir examples/hello.myl -o hello.ll
./build/myl build  examples/hello.myl -o hello
./hello
./build/myl run    examples/arithmetic.myl
```

## Debugging

- Configure a `Debug` build and add `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`
  (already on by default) to get `compile_commands.json`.
- Parser conflicts: Bison prints `warning: N shift/reduce conflicts`. Re-run
  Bison manually with `--report=all` or `-Wcounterexamples` for insight.
- To inspect generated frontend sources, look in the build tree at
  `build/generated/lexer.cpp` and `build/generated/parser.cpp`.
- To inspect generated IR, use `myl emit-ir` and run it through `llvm-as` /
  `opt` (available in the image via `llvm-15-tools`).

## Troubleshooting

| Symptom | Cause / fix |
|---------|-------------|
| `Could not locate libLLVM-15 shared library` | `llvm-15-dev` missing from the image. |
| `cc: not found` when linking | `build-essential` missing from the image. |
| Bison `$N has no declared type` | A grammar action references the wrong position; check the rule's RHS. |
| `YYSTYPE` default constructor deleted | A non-trivial type was added to the `%union`. |
