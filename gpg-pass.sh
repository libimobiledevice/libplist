#!/bin/bash
PWD_FILE="$1"
shift 1

export GPG_TTY=`tty`
. /etc/lsb-release
echo Running on $DISTRIB_CODENAME

if [ "$DISTRIB_CODENAME" = "trusty" ]; then
    cat $PWD_FILE | gpg2 --batch --passphrase-fd 0 --yes $@
else
    cat $PWD_FILE | gpg2 --batch --passphrase-fd 0 --pinentry-mode loopback --yes $@
fi
