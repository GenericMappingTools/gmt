## Development and CI/CD Dockerfile for GenericMappingTools/GMT
##
## Base Image https://github.com/nc5ng/gmt-builder-docker
##
## Pre Build Arguments
## ===================
##
## BUILD_IMAGE image source for build image
## BUILD_IMAGE_TAG image tag for build image
##
## CMAKE_INSTALL_PREFIX internal install directory
## GMT_CMAKE_ARGS extra arguments for cmake
## BUILD_PARALLEL number of parallel compilers
##

## Base/Build Image
## ----------------
## Fetch from remote to save on CI Build Time

ARG BUILD_IMAGE=nc5ng/gmt-builder
ARG BUILD_IMAGE_TAG=6.0.0pre1
FROM $BUILD_IMAGE:$BUILD_IMAGE_TAG


## Build Arguments
## ---------------
## Customize the build

ARG GMT_CMAKE_ARGS=""	    
ARG BUILD_PARALLEL=1



## Copy and Build Source
## ----------------------
## GMT_INSTALL_DIR comes from Base Image

COPY . $GMT_INSTALL_DIR/gmt-src

WORKDIR $GMT_INSTALL_DIR/build

RUN cmake     -D DCW_ROOT=$GMT_INSTALL_DIR/dcw-gmt-$DCW_VERSION		  \
	      -D GSHHG_PATH=$GMT_INSTALL_DIR/gshhg-gmt-$GSHHG_VERSION 	  \
	      $GMT_CMAKE_ARGS						  \
	      $GMT_INSTALL_DIR/gmt-src					&&\
	make -j $BUILD_PARALLEL 					&&\
    	make install


## Test Environment
## ----------------

WORKDIR /workspace

ENTRYPOINT [ "/usr/local/bin/gmt" ]
