# Verify that compiling an invalid .myl example fails with a nonzero exit code
# and emits a diagnostic.
#
# Invoked as:
#   cmake -DMYL=<myl> -DEXAMPLE=<file.myl> -P check_fails.cmake

if(NOT DEFINED MYL OR NOT DEFINED EXAMPLE)
    message(FATAL_ERROR "MYL and EXAMPLE are required")
endif()

execute_process(
    COMMAND "${MYL}" check "${EXAMPLE}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)

if(rc EQUAL 0)
    message(FATAL_ERROR "expected '${EXAMPLE}' to fail, but it succeeded\n${out}")
endif()

if(err STREQUAL "")
    message(FATAL_ERROR "expected a diagnostic on stderr for '${EXAMPLE}'")
endif()

message(STATUS "OK: ${EXAMPLE} failed as expected (exit ${rc})")
