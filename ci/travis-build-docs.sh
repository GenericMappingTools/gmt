#!/bin/bash
# Build GMT documentations

# To return a failure if any commands inside fail
set -e

cd build

make -j docs_html

# Turn off exit on failure.
set +e
