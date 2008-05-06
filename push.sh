#!/bin/sh
params=""

if [ -z "$@" ] ; 
then params='--all'
else params="$@"
fi

exec git-push 'git+ssh://repo.or.cz/srv/git/glugin.git' $params

