#!/bin/bash

runFile()
{
    echo Executing $1...
    cat "${1}" >pipe
    sleep 1
    echo "main();" >pipe
    sleep 1
    echo $'\cd' >pipe # send EOF
    sleep 1
}

test -f pipe || mkfifo pipe

# avoid EOF on pipe after cat, see http://stackoverflow.com/questions/8410439/how-to-avoid-echo-closing-fifo-named-pipes-funny-behavior-of-unix-fifos
sleep 10000 >pipe &
SLEEP_PID=$!

../picoc -i <pipe &
PICOC_PID=$!

runFile 00_assignment.c
runFile 01_comment.c
runFile 02_printf.c
runFile 03_struct.c
runFile 04_for.c
runFile 05_array.c
runFile 06_case.c
runFile 07_function.c
runFile 08_while.c
runFile 09_do_while.c
runFile 10_pointer.c
runFile 11_precedence.c
runFile 12_hashdefine.c
runFile 13_integer_literals.c
runFile 14_if.c
runFile 15_recursion.c
runFile 16_nesting.c
runFile 17_enum.c
runFile 18_include.c
runFile 19_pointer_arithmetic.c
runFile 20_pointer_comparison.c
runFile 21_char_array.c
runFile 23_type_coercion.c
runFile 25_quicksort.c
runFile 26_character_constants.c
runFile 28_strings.c
runFile 29_array_address.c
runFile 30_hanoi.c
#runFile 31_args.c
runFile 32_led.c
runFile 33_ternary_op.c
runFile 34_array_assignment.c
runFile 35_sizeof.c
runFile 36_array_initialisers.c
runFile 37_sprintf.c
runFile 38_multiple_array_index.c
runFile 39_typedef.c
runFile 41_hashif.c
runFile 43_void_param.c
runFile 44_scoped_declarations.c
runFile 45_empty_for.c
runFile 47_switch_return.c
runFile 48_nested_break.c
runFile 49_bracket_evaluation.c
runFile 50_logical_second_arg.c
runFile 51_static.c
runFile 52_unnamed_enum.c
runFile 54_goto.c
runFile 55_array_initialiser.c
runFile 56_cross_structure.c
runFile 57_macro_bug.c
runFile 58_return_outside.c
runFile 59_break_before_loop.c
runFile 60_local_vars.c
runFile 62_float.c
runFile 64_double_prefix_op.c
runFile 66_printf_undefined.c
#runFile 67_macro_crash.c
runFile 68_return.c


kill $PICOC_PID >/dev/null
kill $SLEEP_PID >/dev/null

rm pipe
