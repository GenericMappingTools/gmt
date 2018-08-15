Docker User Guide
=================

Dockerfiles are provided  for development build and testing purposes.
Using this one can build and distribute GMT using containers that have the
environment with them. Additionally, contributors can test  development
locally without needing to install the full build toolchain on their
local machine

> **Note:** The Dockerfiles are not yet optimized for image size, but some care
has been taken to preserve static data in the layer cache, the first build
on your machine may download ~1GB of data to create the base image, afterwards
it should re-use downloaded portions and rebuild much quicker


Software Requirements
---------------------

- `docker`


Two Dockerfiles
---------------

The size of the image produced by docker tends to be very large when
using in-container builds. The default docker image is optimized slightly
to reduce this overhead by deleting build dependencies after it is complete.
As such it is useful to build a local version for testing and other use, but
can be inconvenient as a development image.

An alternative, development image is provided in the dockerfile [`Dockerfile-dev`](Dockerfile-dev) . Which retains build dependencies and can be rerun to test code changes
without redownloading requirements. 

Building the GMT Docker Image
-----------------------------

Building the docker image, and GMT with it, can be done with one command

    docker build .


for convenience however, it is usually a good idea to tag the image so it
can be referred to more easily

    docker build --tag gmt .


This should provide a suitable image to test or use `gmt`

     docker run --rm gmt defaults -Vd
     docker run --rm gmt pscoast -R0/10/0/10 -JM6i -Ba -Ggray -P -Vd > test.ps


an alias or shell script can be written to wrap the docker details to use the image
directly in shell scripts


     alias gmt='docker run --rm gmt'
     gmt defaults -Vd
     gmt pscoast -R0/10/0/10 -JM6i -Ba Ggray -P -Vd > test.ps


Using the Alternate Development Image
-------------------------------------

The alternate development image is useful in that the build
step is seperated into a different layer from the dependency fetching
so a developer or contirbutor can use the alternate file to test
development quickly using docker, without waiting a long time for the full
rebuild


    docker build --tag gmt-dev -f Dockerfile-dev .

rerunning this command after editing a file will rebuild gmt with your changes
quickly






