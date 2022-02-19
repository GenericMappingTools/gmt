#!/usr/bin/env bash
#
# Build the GMT documentation for Vercel.
#

set -x -e

# Install GMT dependencies
# Vercel uses Amazon Linux 2 (i.e., EPEL 7)
yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
yum install cmake3 ninja-build libcurl-devel netcdf-devel gdal gdal-devel

# Install spjhinx and dvc via miniconda
curl -o ~/miniconda.sh https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash ~/miniconda.sh -b -p $HOME/miniconda
export PATH=$HOME/miniconda/bin:$PATH
conda install mamba -c conda-forge -y
mamba install sphinx dvc ghostscript -c conda-forge -y

# Following variables can be modified via environment variables
GMT_INSTALL_DIR=${HOME}/gmt-install-dir

# Configure GMT
cat > cmake/ConfigUser.cmake << EOF
set (CMAKE_INSTALL_PREFIX "${GMT_INSTALL_DIR}")
set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement \${CMAKE_C_FLAGS}")
set (CMAKE_C_FLAGS "-Wextra \${CMAKE_C_FLAGS}")
EOF

# Pull images from DAGsHub repository
dvc pull

# Build and install GMT
mkdir build
cd build
cmake3 .. -G Ninja
cmake3 --build .
cmake3 --build . --target docs_depends
#cmake3 --build . --target optimize_images
cmake3 --build . --target docs_html
# cmake3 --build . --target docs_man
# cmake3 --build . --target install
cd ..

mv build/doc/rst/html/ public/

set -x -e
