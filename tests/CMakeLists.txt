# --- Unit tests -------------------------------------------------------------
add_executable(test_lexer test_lexer.cpp)
target_link_libraries(test_lexer myl_compiler)

add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser myl_compiler)

add_executable(test_semantic test_semantic.cpp)
target_link_libraries(test_semantic myl_compiler)

add_executable(test_codegen test_codegen.cpp)
target_link_libraries(test_codegen myl_compiler)

add_test(NAME lexer_test COMMAND test_lexer)
add_test(NAME parser_test COMMAND test_parser)
add_test(NAME semantic_test COMMAND test_semantic)
add_test(NAME codegen_test COMMAND test_codegen)

# --- Integration tests ------------------------------------------------------
# These compile *and run* .myl programs and verify observable behavior.
set(EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)
set(RUN_HELPER ${CMAKE_CURRENT_SOURCE_DIR}/integration/run_example.cmake)
set(CHECK_HELPER ${CMAKE_CURRENT_SOURCE_DIR}/integration/check_fails.cmake)

function(add_runner_test name example expect)
    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            -DMYL=$<TARGET_FILE:myl>
            -DEXAMPLE=${EXAMPLES_DIR}/${example}
            -DEXPECT=${expect}
            -P ${RUN_HELPER})
endfunction()

add_runner_test(integration_hello hello.myl "Hello, World!")
add_runner_test(integration_functions functions.myl "30")
add_runner_test(integration_arithmetic arithmetic.myl "70")
add_runner_test(integration_hindi hindi.myl "Namaste Duniya!")
add_runner_test(integration_hindi_functions hindi_functions.myl "sahi")

function(add_negative_test name example)
    add_test(NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            -DMYL=$<TARGET_FILE:myl>
            -DEXAMPLE=${EXAMPLES_DIR}/${example}
            -P ${CHECK_HELPER})
endfunction()

add_negative_test(invalid_syntax_test invalid_syntax.myl)
add_negative_test(type_error_test type_error.myl)
add_negative_test(hindi_type_error_test hindi_type_error.myl)
