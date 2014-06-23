#!/bin/sh

board=
for arg in "$@"; do
    case $arg in
    board=*)
        board=${arg#board=}
        ;;
    esac
done

if test $board; then
    path="works/arduino/code/autotemp"
    rsync='rsync -zavrl --partial --exclude=*.sw* --exclude=*build-* --progress --no-p --chmod=ugo=rwX' 
    for p in ../Arduino-Makefile ../libraries ../`basename $PWD`; do
        $rsync $p oneiric3222:$path/../ || exit
    done
    ssh oneiric3222 "cd $path && make $@"
else
    make "$@"
fi
