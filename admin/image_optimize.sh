#!/usr/bin/env bash
#
# Optimize images to reduce file size
#
# Requirement: pngquant
#

if [ "$#" == 0 ]; then
    echo "Usage: bash image_optimize.sh *.png"
    exit 1
fi

if ! [ -x "$(command -v pngquant)" ]; then
    echo 'Error: pngquant is not found in your search PATH.' >&2
    exit 0
fi

for image in "$@"; do
    echo "Optimizing ${image}"
    pngquant --skip-if-larger --strip --ext .optimized.png ${image}
    if [[ -f "${image%.png}.optimized.png" ]]; then
        mv ${image%.png}.optimized.png ${image}
    fi
done
