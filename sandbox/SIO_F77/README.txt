# $Id: README.txt 11855 2013-06-30 05:22:46Z pwessel $

How to use the SIO F77 C extensions in GMT 5.
We have a C layer called SIO_GMT_io.c which sits between
SIO Fortran programs needed to read/write GMT files and
the F77 extensions in the GMT 5 shared lib.

Programs must be linked with SIO_GMT_io.o and with the
shared GMT5 library.

Dave Sandwell, SIO wrote SIO_GMT_io.c; Paul Wessel modified
it to work with GMT 5.

The Makefile shows what to do.

Paul Wessel, June 2013
