# Releasing GMT

**The admin directory is just for the GMT developers to store configuration
files and scripts useful for building the actual releases.**


## Prerequisites

IN addition to the packages required for building all of GMT (including documentation),
you also need gnu-tar and coreutils.

## Contents

- build-release.sh
- build-macos-external-list.sh
- ConfigReleaseBuild.cmake

## Do the release

Make sure you are in the top gmt directory and then
run

    admin/build-release.sh [-help]
