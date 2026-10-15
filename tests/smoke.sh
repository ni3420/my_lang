#!/usr/bin/env bash
# End-to-end smoke test: build the compiler, then compile and run real .myl
# programs and verify observable behavior. Safe to run inside the dev container.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

MYL="${MYL:-build/myl}"

echo "== version =="
"$MYL" --version

echo "== check hello.myl =="
"$MYL" check examples/hello.myl
echo "check: OK"

echo "== emit-ir functions.myl =="
"$MYL" emit-ir examples/functions.myl -o /tmp/functions.ll
grep -q "define" /tmp/functions.ll && echo "emit-ir: OK"

echo "== build + run hello.myl =="
"$MYL" build examples/hello.myl -o /tmp/hello_myl
/tmp/hello_myl

echo "== run functions.myl =="
"$MYL" run examples/functions.myl

echo "== run arithmetic.myl =="
"$MYL" run examples/arithmetic.myl

echo "== invalid_syntax.myl must fail =="
if "$MYL" check examples/invalid_syntax.myl; then
    echo "ERROR: invalid_syntax.myl unexpectedly succeeded"
    exit 1
fi
echo "invalid_syntax: failed as expected"

echo "== type_error.myl must fail =="
if "$MYL" check examples/type_error.myl; then
    echo "ERROR: type_error.myl unexpectedly succeeded"
    exit 1
fi
echo "type_error: failed as expected"

echo "ALL SMOKE TESTS PASSED"
