# Compile and run a .myl example with the myl driver, then check that the
# program's stdout contains the expected substring.
#
# Invoked as:
#   cmake -DMYL=<myl> -DEXAMPLE=<file.myl> -DEXPECT=<regex> -P run_example.cmake

if(NOT DEFINED MYL OR NOT DEFINED EXAMPLE)
    message(FATAL_ERROR "MYL and EXAMPLE are required")
endif()

execute_process(
    COMMAND "${MYL}" run "${EXAMPLE}"
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)

if(NOT rc EQUAL 0)
    message(FATAL_ERROR "myl run failed (exit ${rc})\nstdout:\n${out}\nstderr:\n${err}")
endif()

if(DEFINED EXPECT AND NOT out MATCHES "${EXPECT}")
    message(FATAL_ERROR "expected output to match /${EXPECT}/ but got:\n${out}")
endif()

message(STATUS "OK: ${EXAMPLE} produced expected output")
