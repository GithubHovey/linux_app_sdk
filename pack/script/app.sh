#!/bin/bash

function start() {
    "$(dirname "$0")"/guide_dart.exe &
}

function stop() {
    pkill -f "guide_dart.exe"
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac