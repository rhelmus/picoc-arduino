#!/bin/sh

COPY="cp -v"

if [ "$1" = "-s" ]; then
    COPY="ln -vs" shift
fi

DEST="$1"
echo "destination: $DEST"

mkdir -p "${DEST}"

$COPY clibrary.c "${DEST}/"
$COPY debug.c "${DEST}/"
$COPY expression.c "${DEST}/"
$COPY heap.c "${DEST}/"
$COPY include.c "${DEST}/"
$COPY interpreter.h "${DEST}/"
$COPY lex.c "${DEST}/"
$COPY parse.c "${DEST}/"
$COPY picoc.h "${DEST}/"
$COPY platform.c "${DEST}/"
$COPY platform.h "${DEST}/"
$COPY table.c "${DEST}/"
$COPY type.c "${DEST}/"
$COPY variable.c "${DEST}/"
$COPY README "${DEST}/"

mkdir -p "${DEST}/utility"
$COPY platform/library_arduino.c "${DEST}/utility"
$COPY platform/platform_arduino.cpp "${DEST}/utility"
