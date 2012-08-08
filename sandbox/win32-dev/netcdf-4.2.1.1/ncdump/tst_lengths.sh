#!/bin/sh
# This shell script tests lengths of small netcdf files and tests 
# that rewriting a numeric value doesn't change file length
# $Id$

# cat > rewrite-scalar.c << EOF
# #include <stdio.h>
# #include <netcdf.h>
# #define ERR do {fflush(stdout); fprintf(stderr, "Error, %s, line: %d\n", __FILE__, __LINE__); return(1);} while (0)

# int
# main(int ac, char *av[]) {
#     int ncid, varid, data[] = {42};
#     if (nc_open(av[1], NC_WRITE, &ncid)) ERR;
#     if (nc_inq_varid(ncid, av[2], &varid)) ERR;
#     if (nc_put_var_int(ncid, varid, data)) ERR;
#     if (nc_close(ncid)) ERR;
#     return 0;
# }
# EOF
# cat > test-len.sh << 'EOF'
# #!/bin/sh
# # test that length of file $1 is $2
# len=`ls -l $1|awk '{print $5}'`
# if [ $len = $2 ]; then
#   exit 0
# else
#   echo "### Failure: file $1 has length $len instead of expected $2"
#   exit 1
# fi
# EOF
# chmod +x ./test-len.sh
# cc -g -o rewrite-scalar -I../libsrc rewrite-scalar.c -L../libsrc -lnetcdf
# echo "netcdf small {variables: byte t; data: t = 1;}" > small.cdl
set -e
echo ""
echo "*** testing length of classic file"
../ncgen/ncgen -b ${srcdir}/small.cdl
if test `wc -c < small.nc` != 68; then
    exit 1
fi

echo "*** testing length of classic file written with NOFILL"
../ncgen/ncgen -b -x ${srcdir}/small.cdl
if test `wc -c < small.nc` != 68; then
    exit 1
fi

echo "*** testing length of rewritten classic file"
../ncgen/ncgen -b ${srcdir}/small.cdl && ./rewrite-scalar small.nc t
if test `wc -c < small.nc` != 68; then
    exit 1
fi

echo "*** testing length of rewritten classic file written with NOFILL"
../ncgen/ncgen -b -x ${srcdir}/small.cdl && ./rewrite-scalar small.nc t
if test `wc -c < small.nc` != 68; then
    exit 1
fi

echo "*** testing length of 64-bit offset file"
../ncgen/ncgen -b -k64-bit-offset ${srcdir}/small.cdl
if test `wc -c < small.nc` != 72; then
    exit 1
fi

echo "*** testing length of 64-bit offset file written with NOFILL"
../ncgen/ncgen -b -k64-bit-offset -x ${srcdir}/small.cdl
if test `wc -c < small.nc` != 72; then
    exit 1
fi

echo "*** testing length of rewritten 64-bit offset file"
../ncgen/ncgen -b -k64-bit-offset ${srcdir}/small.cdl && ./rewrite-scalar small.nc t
if test `wc -c < small.nc` != 72; then
    exit 1
fi

echo "*** testing length of rewritten 64-bit offset file written with NOFILL"
../ncgen/ncgen -b -k64-bit-offset -x ${srcdir}/small.cdl && ./rewrite-scalar small.nc t
if test `wc -c < small.nc` != 72; then
    exit 1
fi

# test with only one record variable of type byte or short, which need
# not be 4-byte aligned
echo "*** testing length of one-record-variable classic file"
../ncgen/ncgen -b ${srcdir}/small2.cdl
if test `wc -c < small2.nc` != 101; then
    exit 1
fi

echo "*** testing length of one-record-variable classic file written with NOFILL"
../ncgen/ncgen -b -x ${srcdir}/small2.cdl
if test `wc -c < small2.nc` != 101; then
    exit 1
fi

echo "*** testing length of one-record-variable 64-bit offset file"
../ncgen/ncgen -b -k64-bit-offset ${srcdir}/small2.cdl
if test `wc -c < small2.nc` != 105; then
    exit 1
fi

echo "*** testing length of one-record-variable 64-bit offset file written with NOFILL"
../ncgen/ncgen -b -k64-bit-offset -x ${srcdir}/small2.cdl
if test `wc -c < small2.nc` != 105; then
    exit 1
fi
