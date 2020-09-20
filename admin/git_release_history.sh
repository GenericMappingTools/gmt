#!/usr/bin/env bash

if [ "$#" == 0 ]; then
    echo "Usage: bash git_release_history.sh tag"
    exit 1
fi

tag=$1
git log --oneline --remove-empty --no-merges --pretty="format:%h %ad %s" --date=format:'%Y-%m-%dT%H:%M:%S' \
	${tag}..master
