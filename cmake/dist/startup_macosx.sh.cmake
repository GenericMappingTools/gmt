#!/bin/bash

# Startup script for GMT.app in MacOSX.
# Setup environment and start Terminal.

RUNDIR=$(dirname "$0")
cd "${RUNDIR}/.."
BUNDLE_RESOURCES="${PWD}/Resources"

GMT_SHAREDIR=$(echo ${BUNDLE_RESOURCES}/@GMT_SHARE_PATH@)

# run terminal, set path, and run gmt
osascript << EOF
  tell application "Terminal"
    activate
    do script with command "export PATH=\"${BUNDLE_RESOURCES}/@GMT_BINDIR@:\${PATH}\" GMT_SHAREDIR=\"${GMT_SHAREDIR}\"; gmt"
  end tell
EOF

exit 0
