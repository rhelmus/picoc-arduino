#!/bin/bash

runFile()
{
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

kill $PICOC_PID >/dev/null
kill $SLEEP_PID >/dev/null

rm pipe