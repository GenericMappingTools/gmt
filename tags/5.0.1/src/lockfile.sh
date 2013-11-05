#!/bin/bash
#
# $Id$
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the Creative Commons license.
#
# Inspired by http://stackoverflow.com/questions/185451/
#
# To acquire a lockfile simply call this script when you need to get
# exclusive access to something.  An optional lock name parameter lets
# you share locks between different scripts.  There's also two lower
# level functions (exclusive_lock_try and exclusive_lock_retry) should
# you need something more complex.
#
# Syntax: lockfile.sh [remove] [lockname] [retries] [delay]
#

function remove_lock() # [lockname]
{
  local LOCK_NAME="${1:-default}"

  LOCK_DIR="${LOCK_NAME}.lock"
  local LOCK_PID_FILE="${LOCK_DIR}/${LOCK_NAME}.pid"

  if [ -e "${LOCK_DIR}" ]
  then
    /bin/rm -rf "${LOCK_PID_FILE}"
    /bin/rmdir "${LOCK_DIR}" || exit 1
  fi
}

function exclusive_lock_try() # [lockname]
{
  local LOCK_NAME="${1:-default}"

  LOCK_DIR="${LOCK_NAME}.lock"
  local LOCK_PID_FILE="${LOCK_DIR}/${LOCK_NAME}.pid"

  if [ -e "${LOCK_DIR}" ]
  then
    local LOCK_PID="$(cat "${LOCK_PID_FILE}" 2> /dev/null)"
    if [ ! -z "${LOCK_PID}" ] && kill -0 "${LOCK_PID}" 2> /dev/null
    then
      # locked by non-dead process
      echo "\"${LOCK_NAME}\" lock currently held by PID ${LOCK_PID}"
      return 1
    else
      # orphaned lock, take it over
      echo "Found stale lock \"${LOCK_NAME}\". Taking over..."
      remove_lock "${LOCK_NAME}"
      sleep 5
    fi
  fi
  if ! ( umask 077 && mkdir "${LOCK_DIR}" && umask 177 && echo ${PPID} > "${LOCK_PID_FILE}" ) 2> /dev/null
  then
    local LOCK_PID="$(cat "${LOCK_PID_FILE}" 2> /dev/null)"
    # unable to acquire lock, new process got in first
    echo "\"${LOCK_NAME}\" lock currently held by PID ${LOCK_PID}"
    return 1
  fi

  return 0 # got lock
}

function exclusive_lock_retry() # [lockname] [retries] [delay]
{
  local LOCK_NAME="${1}"
  local MAX_TRIES="${2:-60}"
  local DELAY="${3:-5}"

  local TRIES=0
  local LOCK_RETVAL

  while [ "${TRIES}" -lt "${MAX_TRIES}" ]
  do
    if [ "${TRIES}" -gt 0 ]
    then
      sleep "${DELAY}"
    fi
    ((++TRIES))

    if [ "${TRIES}" -lt "${MAX_TRIES}" ]
    then
      exclusive_lock_try "${LOCK_NAME}" > /dev/null
    else
      exclusive_lock_try "${LOCK_NAME}"
    fi
    LOCK_RETVAL="${PIPESTATUS[0]}"

    if [ "${LOCK_RETVAL}" -eq 0 ]
    then
      return 0
    fi
  done

  return "${LOCK_RETVAL}"
}

# remove LOCK_DIR
if [ "${1}" == "remove" ]
then
  remove_lock "${2}" || exit 1
  exit 0
fi

# acquite exclusive lock
exclusive_lock_retry "$@" || exit 1

exit 0
