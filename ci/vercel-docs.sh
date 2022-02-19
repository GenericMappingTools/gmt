#!/usr/bin/env bash
#
# Build the GMT documentation for Vercel.
#

set -x -e

# Install GMT dependencies
# Vercel uses Amazon Linux 2 (i.e., EPEL 7)
yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
yum install cmake3 ninja-build libcurl-devel netcdf-devel gdal gdal-devel wget

# Install Sphinx
# importlib-resources is required for Python <3.7
pip install sphinx importlib-resources

# Install dvc
wget https://dvc.org/rpm/dvc.repo -O /etc/yum.repos.d/dvc.repo
yum update
yum install --nogpgcheck dvc

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
