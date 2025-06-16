#!/bin/bash

function start() {
    "$(dirname "$0")"/skg_master &
}

function stop() {
    pkill -f "skg_master"
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