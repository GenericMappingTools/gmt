#!/usr/bin/env bash
#
# Build the GMT documentation for Vercel.
#

set -x -e

# Install GMT dependencies
# Vercel uses Amazon Linux 2 (i.e., EPEL 7)
yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
yum install cmake3 ninja-build libcurl-devel netcdf-devel gdal gdal-devel libsqlite3x-devel

# Need to configure Python with sqlite extensions for dvc
curl -SLO https://www.python.org/ftp/python/3.9.10/Python-3.9.10.tgz
echo '1440acb71471e2394befdb30b1a958d1  Python-3.9.10.tgz' | md5sum -c
tar -xvf Python-3.9.10.tgz
cd Python-3.9.10
./configure --enable-loadable-sqlite-extensions
make
make altinstall
cd ../
python3.9 --version

# Install Python packages
# importlib-resources is required for Python <3.7
python3.9 -m pip install --user --upgrade pip
python3.9 -m venv env
source env/bin/activate
python3.9 -m pip install sphinx dvc

# Install latest gs
curl -SLO https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs9533/ghostscript-9.53.3-linux-x86_64.tgz
tar -xvf ghostscript-9.53.3-linux-x86_64.tgz ghostscript-9.53.3-linux-x86_64/gs-9533-linux-x86_64
mv ghostscript-9.53.3-linux-x86_64/gs-9533-linux-x86_64 /usr/bin/gs
gs --version

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
