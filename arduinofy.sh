#!/bin/sh

COPY="cp -v"

if [ "$1" = "-s" ]; then
    COPY="ln -fvs" shift
fi

DEST="$1"
echo "destination: $DEST"

mkdir -p "${DEST}"

$COPY "${PWD}"/clibrary.cpp "${DEST}/"
$COPY "${PWD}"/debug.cpp "${DEST}/"
$COPY "${PWD}"/expression.cpp "${DEST}/"
$COPY "${PWD}"/heap.cpp "${DEST}/"
$COPY "${PWD}"/include.cpp "${DEST}/"
$COPY "${PWD}"/interpreter.h "${DEST}/"
$COPY "${PWD}"/lex.cpp "${DEST}/"
$COPY "${PWD}"/parse.cpp "${DEST}/"
$COPY "${PWD}"/picoc.h "${DEST}/"
$COPY "${PWD}"/platform.cpp "${DEST}/"
$COPY "${PWD}"/platform.h "${DEST}/"
$COPY "${PWD}"/table.cpp "${DEST}/"
$COPY "${PWD}"/type.cpp "${DEST}/"
$COPY "${PWD}"/util.cpp "${DEST}/"
$COPY "${PWD}"/util.hpp "${DEST}/"
$COPY "${PWD}"/variable.cpp "${DEST}/"
$COPY "${PWD}"/README "${DEST}/"

mkdir -p "${DEST}/utility"
$COPY "${PWD}"/platform/library_arduino.cpp "${DEST}/utility"
$COPY "${PWD}"/platform/platform_arduino.cpp "${DEST}/utility"
