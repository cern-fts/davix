#!/bin/bash

MY_DIR=$(dirname $(readlink -f "$BASH_SOURCE"))

ARGS="$1"

export ACTION="start"

if [[ "$1" == "stop" ]];
then
	ACTION="stop"
fi



OPTS_SERV_1=" -D /tmp -n -d ${ACTION} -P 8008 -i 0"
OPTS_SERV_2=" -D /tmp -u test -p tester -d ${ACTION}  -P 8009 -i 1"

export PYTHONPATH=$MY_DIR/


$MY_DIR/pywebdav/server/server.py $OPTS_SERV_1


$MY_DIR/pywebdav/server/server.py $OPTS_SERV_2



