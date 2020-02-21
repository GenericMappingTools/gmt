#!/usr/bin/env bash
#
# Optimize images to reduce the file sizes
#
# Usage: bash image_optimize.sh *.png
#
# Note: Requires pngquant to be installed
#

if [ "$#" == 0 ]; then
    echo "Usage: bash image_optimize.sh *.png"
    exit 1
fi

if ! [ -x "$(command -v pngquant)" ]; then
    echo 'Error: pngquant is not found in your search PATH.' >&2
    exit 1
fi

for image in "$@"; do
    echo "Optimizing ${image}"
    pngquant --skip-if-larger --strip --force --ext .png ${image}
done
