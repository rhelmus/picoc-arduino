#!/usr/bin/env python
# -*- coding: utf-8 -*-

import datetime
import serial
import sys
import time

# --- config ---

serialPort = '/dev/pts/5' # '/dev/ttyACM0'
serialBaud = 115200
serialTimeout = 0
serialTerminator = b'DONE!!11!1!'

# ---

serInterface = serial.Serial()

def init():
    serInterface.port = serialPort
    serInterface.baudrate = serialBaud
    serInterface.timeout = serialTimeout

    print "Waiting until port {} can be opened...\n".format(serialPort)
    while True:
	try:
	    serInterface.open()
	    break
	except OSError:
	    time.sleep(1)

    time.sleep(2) # wait to settle after open (only needed if board resets)
    print("Connected and initialized!")

outputTmpBuf = b""
def checkOutput(checkterm=False):
    global outputTmpBuf
    while True:
        line = serInterface.readline()
        outputTmpBuf += line
        if len(outputTmpBuf) > 0 and outputTmpBuf[-1] == '\n':
            if outputTmpBuf.strip():
                if checkterm and outputTmpBuf.strip() == serialTerminator:
                    outputTmpBuf = b""
                    return
                print outputTmpBuf,
            outputTmpBuf = b""
	if not checkterm:
	    break

def runTest(filename):
    serInterface.write(b'exit();\n')
    time.sleep(3)

    print "Running test {}...\n---".format(filename)

    file = open(filename, 'r')

    for line in file:
	serInterface.write(line)
	checkOutput()
    file.close()

    serInterface.write(b'main();\n')
    serInterface.write(b'printf("\\n{}\\n");\n'.format(serialTerminator))

    time.sleep(3)
    checkOutput(True)
    print("---\nFinished!")

def main():
    init()

    if len(sys.argv) > 1:
	runTest(sys.argv[1])
    else:
	for file in test_files:
	    runTest(file)

#    serInterface.write(b'exit();\n')
#    time.sleep(0.5)
#    checkOutput(True)

#    while True:
#	checkOutput()

# test files
test_files = [
    '00_assignment.c',
    '01_comment.c',
    '02_printf.c',
    '03_struct.c',
    '04_for.c',
    '05_array.c',
    '06_case.c',
    '07_function.c',
    '08_while.c',
    '09_do_while.c',
    '10_pointer.c',
    '11_precedence.c',
    '12_hashdefine.c',
    '13_integer_literals.c',
    '14_if.c',
    '15_recursion.c',
    '16_nesting.c',
    '17_enum.c',
    #'18_include.c',
    '19_pointer_arithmetic.c',
    '20_pointer_comparison.c',
    '21_char_array.c',
    '23_type_coercion.c',
    '25_quicksort.c',
    '26_character_constants.c',
    '28_strings.c',
    '29_array_address.c',
    '30_hanoi.c',
    #'31_args.c',
    '32_led.c',
    '33_ternary_op.c',
    '34_array_assignment.c',
    '35_sizeof.c',
    '36_array_initialisers.c',
    '37_sprintf.c',
    '38_multiple_array_index.c',
    '39_typedef.c',
    '41_hashif.c',
    '43_void_param.c',
    '44_scoped_declarations.c',
    '45_empty_for.c',
    '47_switch_return.c',
    '48_nested_break.c',
    '49_bracket_evaluation.c',
    '50_logical_second_arg.c',
    '51_static.c',
    '52_unnamed_enum.c',
    '54_goto.c',
    '55_array_initialiser.c',
    '56_cross_structure.c',
    '57_macro_bug.c',
    '58_return_outside.c',
    '59_break_before_loop.c',
    #'60_local_vars.c',
    '62_float.c',
    '64_double_prefix_op.c',
    '66_printf_undefined.c',
    #'67_macro_crash.c',
    '68_return.c'
]

if __name__=="__main__":
   main()
