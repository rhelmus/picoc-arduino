#!/bin/sh

COPY="cp -v"

if [ "$1" = "-s" ]; then
    COPY="ln -fvs" shift
fi

DEST="$1"
echo "destination: $DEST"

mkdir -p "${DEST}"

$COPY "${PWD}"/clibrary.c "${DEST}/"
$COPY "${PWD}"/debug.c "${DEST}/"
$COPY "${PWD}"/expression.c "${DEST}/"
$COPY "${PWD}"/heap.c "${DEST}/"
$COPY "${PWD}"/include.c "${DEST}/"
$COPY "${PWD}"/interpreter.h "${DEST}/"
$COPY "${PWD}"/lex.c "${DEST}/"
$COPY "${PWD}"/parse.c "${DEST}/"
$COPY "${PWD}"/picoc.h "${DEST}/"
$COPY "${PWD}"/platform.c "${DEST}/"
$COPY "${PWD}"/platform.h "${DEST}/"
$COPY "${PWD}"/table.c "${DEST}/"
$COPY "${PWD}"/type.c "${DEST}/"
$COPY "${PWD}"/variable.c "${DEST}/"
$COPY "${PWD}"/README "${DEST}/"

mkdir -p "${DEST}/utility"
$COPY "${PWD}"/platform/library_arduino.cpp "${DEST}/utility"
$COPY "${PWD}"/platform/platform_arduino.cpp "${DEST}/utility"
