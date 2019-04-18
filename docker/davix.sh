#!/bin/sh

#####
# This script provides a sub-command type access to the various davix binaries
# installed in /usr/local/bin. Instead of calling /usr/local/bin/davix-ls, the
# script makes it possible to call /usr/local/bin/davix.sh ls with exactly the
# same set of options. The script is tuned to be used as the entrypoint of the
# Dockerfile, allowing to run commands such as the following, which is slightly
# more convenient: docker run -it --rm ... davix ls ...
#####

cmd=$1
shift

if [ -x "/usr/local/bin/davix-$cmd" ]; then
    exec /usr/local/bin/davix-$cmd $@
else
    echo "$cmd is an unknown subcommand!" >& 2
    commands=
    cd /usr/local/bin
    for binary in $(ls -1 davix-*); do
        commands="$commands ${binary#*-}"
    done
    echo "Should be one of: $commands" >& 2
fi
