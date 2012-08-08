divert(-1)

changequote(<<,>>)
define(<<index>>, defn(index))

define(<<CODE>>, <<\fB$1\fR>>)

define(<<ARG>>, <<\fI$1\fP>>)

define(<<HEADER_FILE>>, 
    <<ifelse(API,C,
	$1.h,
	$1.inc)>>)

define(<<INCLUDE>>, 
    <<ifelse(API,C,
	<<#include>> "HEADER_FILE($1)",
	<<<<include>>>> HEADER_FILE($1))>>)

define(<<COMPILER>>,
    <<ifelse(API,C,
	cc,
	f77)>>)

define(<<LANGUAGE>>,
    <<ifelse(API,C,
	C,
	FORTRAN)>>)

define(<<RETSTR>>,
    <<ifelse(API,C,
	const char*,
	character*80)>>)

define(<<FNAME>>,
    <<ifelse(API,C,
	nc_$1,
	nf_$1)>>)

define(<<VOID_ARG>>,
    <<ifelse(API,C,,void)>>)

define(<<MACRO>>,
    <<CODE(ifelse(API,C,
	NC_$1,
	NF_$1))>>)

dnl AQUAL(io, rank)
define(<<AQUAL>>, <<ifelse(API,C,
    <<ifelse($1, output, , <<ifelse($2, 0, , const )>>)>>)>>)

dnl CTYPE(type)
define(<<CTYPE>>,
    <<ifelse($1,text,char,
    <<ifelse($1,uchar,unsigned char,
    <<ifelse($1,schar,signed char,
    <<ifelse($1,short,short,
    <<ifelse($1,int,int,
    <<ifelse($1,nc_type,nc_type,
    <<ifelse($1,size_t,size_t,
    <<ifelse($1,ptrdiff_t,ptrdiff_t,
    <<ifelse($1,long,long,
    <<ifelse($1,int64,long long,
    <<ifelse($1,float,float,
    <<ifelse($1,double,double,
    <<ifelse($1,ubyte,unsigned char,
    <<ifelse($1,ushort,unsigned short,
    <<ifelse($1,uint,unsigned int,
    <<ifelse($1,int64,long long,
    <<ifelse($1,uint64,unsigned long long,
    <<ifelse($1,string,char *,
    <<ifelse($1,voidp,void *,
    )>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)

dnl CSTAR(io, rank)
define(<<CSTAR>>, <<ifelse($1,input,,<<ifelse($2,0,*)>>)>>)

dnl FTYPE(type, rank)
define(<<FTYPE>>, 
    <<ifelse($1,text,<<character*ifelse($2,0,1,(*))>>,
    <<ifelse($1,schar,integer*1,
    <<ifelse($1,short,integer*2,
    <<ifelse($1,int,integer,
    <<ifelse($1,nc_type,integer,
    <<ifelse($1,size_t,integer,
    <<ifelse($1,ptrdiff_t,integer,
    <<ifelse($1,long,integer,
    <<ifelse($1,int64,integer*8,
    <<ifelse($1,float,real,
    <<ifelse($1,double,doubleprecision,
    <<ifelse($1,ubyte,integer*1,
    <<ifelse($1,ushort,integer*2,
    <<ifelse($1,uint,integer*4,
    <<ifelse($1,int64,integer*8,
    <<ifelse($1,uint64,integer*8,
    <<ifelse($1,string,character*,
    <<ifelse($1,voidp,void *,
)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)

dnl ATYPE(io,rank,type)
define(<<ATYPE>>, <<ifelse(API,C,
    <<CTYPE($3)<<>>CSTAR($1,$2)>>, 
    <<FTYPE($3,$2)>>)>>)

dnl AID(name, rank, type)
define(<<AID>>, <<ARG($1)<<>>ifelse(API,C,
    <<ifelse($2,0,,[])>>, 
    <<ifelse($3,text,,<<ifelse($2,0,,(1))>>)>>)>>)

dnl ADECL(io, rank, type, name)
define(<<ADECL>>, <<AQUAL($1,$2)ATYPE($1,$2,$3) AID($4,$2,$3)>>)

define(<<ITEXT>>,	<<ADECL(input,0,text,$1)>>)
define(<<ITEXTV>>,	<<ADECL(input,1,text,$1)>>)
define(<<OTEXT>>,	<<ADECL(output,0,text,$1)>>)
define(<<OTEXTV>>,	<<ADECL(output,1,text,$1)>>)

define(<<IUCHAR>>,	<<ADECL(input,0,uchar,$1)>>)
define(<<IUCHARV>>,	<<ADECL(input,1,uchar,$1)>>)
define(<<OUCHAR>>,	<<ADECL(output,0,uchar,$1)>>)
define(<<OUCHARV>>,	<<ADECL(output,1,uchar,$1)>>)

define(<<ISCHAR>>,	<<ADECL(input,0,schar,$1)>>)
define(<<ISCHARV>>,	<<ADECL(input,1,schar,$1)>>)
define(<<OSCHAR>>,	<<ADECL(output,0,schar,$1)>>)
define(<<OSCHARV>>,	<<ADECL(output,1,schar,$1)>>)

define(<<ISHORT>>,	<<ADECL(input,0,short,$1)>>)
define(<<ISHORTV>>,	<<ADECL(input,1,short,$1)>>)
define(<<OSHORT>>,	<<ADECL(output,0,short,$1)>>)
define(<<OSHORTV>>,	<<ADECL(output,1,short,$1)>>)

define(<<IINT>>,	<<ADECL(input,0,int,$1)>>)
define(<<IINTV>>,	<<ADECL(input,1,int,$1)>>)
define(<<OINT>>,	<<ADECL(output,0,int,$1)>>)
define(<<OINTV>>,	<<ADECL(output,1,int,$1)>>)

define(<<IINT64>>,	<<ADECL(input,0,int64,$1)>>)
define(<<IINT64V>>,	<<ADECL(input,1,int64,$1)>>)
define(<<OINT64>>,	<<ADECL(output,0,int64,$1)>>)
define(<<OINT64V>>,	<<ADECL(output,1,int64,$1)>>)

define(<<INCTYPE>>,	<<ADECL(input,0,nc_type,$1)>>)
define(<<INCTYPEV>>,	<<ADECL(input,1,nc_type,$1)>>)
define(<<ONCTYPE>>,	<<ADECL(output,0,nc_type,$1)>>)
define(<<ONCTYPEV>>,	<<ADECL(output,1,nc_type,$1)>>)

define(<<ISIZET>>,	<<ADECL(input,0,size_t,$1)>>)
define(<<ISIZETV>>,	<<ADECL(input,1,size_t,$1)>>)
define(<<OSIZET>>,	<<ADECL(output,0,size_t,$1)>>)
define(<<OSIZETV>>,	<<ADECL(output,1,size_t,$1)>>)

define(<<IPTRDIFFT>>,	<<ADECL(input,0,ptrdiff_t,$1)>>)
define(<<IPTRDIFFTV>>,	<<ADECL(input,1,ptrdiff_t,$1)>>)
define(<<OPTRDIFFT>>,	<<ADECL(output,0,ptrdiff_t,$1)>>)
define(<<OPTRDIFFTV>>,	<<ADECL(output,1,ptrdiff_t,$1)>>)

define(<<ILONG>>,	<<ADECL(input,0,long,$1)>>)
define(<<ILONGV>>,	<<ADECL(input,1,long,$1)>>)
define(<<OLONG>>,	<<ADECL(output,0,long,$1)>>)
define(<<OLONGV>>,	<<ADECL(output,1,long,$1)>>)

define(<<IFLOAT>>,	<<ADECL(input,0,float,$1)>>)
define(<<IFLOATV>>,	<<ADECL(input,1,float,$1)>>)
define(<<OFLOAT>>,	<<ADECL(output,0,float,$1)>>)
define(<<OFLOATV>>,	<<ADECL(output,1,float,$1)>>)

define(<<IDOUBLE>>,	<<ADECL(input,0,double,$1)>>)
define(<<IDOUBLEV>>,	<<ADECL(input,1,double,$1)>>)
define(<<ODOUBLE>>,	<<ADECL(output,0,double,$1)>>)
define(<<ODOUBLEV>>,	<<ADECL(output,1,double,$1)>>)

define(<<IUBYTE>>,	<<ADECL(input,0,ubyte,$1)>>)
define(<<IUBYTEV>>,	<<ADECL(input,1,ubyte,$1)>>)
define(<<OUBYTE>>,	<<ADECL(output,0,ubyte,$1)>>)
define(<<OUBYTEV>>,	<<ADECL(output,1,ubyte,$1)>>)

define(<<IUSHORT>>,	<<ADECL(input,0,ushort,$1)>>)
define(<<IUSHORTV>>,	<<ADECL(input,1,ushort,$1)>>)
define(<<OUSHORT>>,	<<ADECL(output,0,ushort,$1)>>)
define(<<OUSHORTV>>,	<<ADECL(output,1,ushort,$1)>>)

define(<<IUINT>>,	<<ADECL(input,0,uint,$1)>>)
define(<<IUINTV>>,	<<ADECL(input,1,uint,$1)>>)
define(<<OUINT>>,	<<ADECL(output,0,uint,$1)>>)
define(<<OUINTV>>,	<<ADECL(output,1,uint,$1)>>)

define(<<IINT64>>,	<<ADECL(input,0,int64,$1)>>)
define(<<IINT64V>>,	<<ADECL(input,1,int64,$1)>>)
define(<<OINT64>>,	<<ADECL(output,0,int64,$1)>>)
define(<<OINT64V>>,	<<ADECL(output,1,int64,$1)>>)

define(<<IUINT64>>,	<<ADECL(input,0,uint64,$1)>>)
define(<<IUINT64V>>,	<<ADECL(input,1,uint64,$1)>>)
define(<<OUINT64>>,	<<ADECL(output,0,uint64,$1)>>)
define(<<OUINT64V>>,	<<ADECL(output,1,uint64,$1)>>)

define(<<ISTRING>>,	<<ADECL(input,0,string,$1)>>)
define(<<ISTRINGV>>,	<<ADECL(input,1,string,$1)>>)
define(<<OSTRING>>,	<<ADECL(output,0,string,$1)>>)
define(<<OSTRINGV>>,	<<ADECL(output,1,string,$1)>>)

define(<<IVOIDP>>,	<<ADECL(input,0,voidp,$1)>>)
define(<<IVOIDPV>>,	<<ADECL(input,1,voidp,$1)>>)
define(<<OVOIDP>>,	<<ADECL(output,0,voidp,$1)>>)
define(<<OVOIDPV>>,	<<ADECL(output,1,voidp,$1)>>)

dnl CCOMP(type)
define(<<CCOMP>>, 
    <<ifelse($1,text,text,
    <<ifelse($1,uchar,uchar,
    <<ifelse($1,schar,schar,
    <<ifelse($1,short,short,
    <<ifelse($1,int,int,
    <<ifelse($1,long,long,
    <<ifelse($1,float,float,
    <<ifelse($1,double,double,
    <<ifelse($1,ubyte,ubyte,
    <<ifelse($1,ushort,ushort,
    <<ifelse($1,uint,uint,
    <<ifelse($1,int64,int64,
    <<ifelse($1,uint64,uint64,
    <<ifelse($1,string,string,
    <<ifelse($1,voidp,void *,
)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)

dnl FCOMP(type)
define(<<FCOMP>>, 
    <<ifelse($1,text,text,
    <<ifelse($1,schar,int1,
    <<ifelse($1,short,int2,
    <<ifelse($1,int,int,
    <<ifelse($1,float,real,
    <<ifelse($1,double,double,
    <<ifelse($1,ubyte,ubyte,
    <<ifelse($1,ushort,ushort,
    <<ifelse($1,uint,uint,
    <<ifelse($1,uint64,uint64,
    <<ifelse($1,string,string,
    <<ifelse($1,voidp,any,
)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)>>)

dnl COMP(type)
define(<<COMP>>, <<ifelse(API,C,<<CCOMP($1)>>,<<FCOMP($1)>>)>>)

define(<<FDECL_TYPE>>,
    <<ifelse(API,C, 
	int,
	integer function)>>)

dnl DECL(return-type, name, argument-list)
define(<<DECL>>, <<CODE($1 FNAME($2)$3)>>)

dnl FDECL(name, argument-list)
define(<<FDECL>>, <<DECL(FDECL_TYPE, $1, $2)>>)

dnl IODECL(name, type, argument-list)
define(<<IODECL>>, <<FDECL($1_<<>>COMP($2), $3)>>)

dnl FREF(name)
define(<<FREF>>, <<CODE(FNAME($1)(\|))>>)

dnl FOLD(cname, fname)
define(<<FOLD>>, <<CODE(ifelse(API,C, nc$1, nc$2)(\|))>>)

dnl Function Input Arguments:
define(<<IATTNUM>>, <<IINT(attnum)>>)
define(<<ICMODE>>, <<IINT(cmode)>>)
define(<<ICOUNT>>, <<ISIZETV(count)>>)
define(<<IDIMID>>, <<IINT(dimid)>>)
define(<<IDIMIDS>>, <<IINTV(dimids)>>)
define(<<IFILLMODE>>, <<IINT(fillmode)>>)
define(<<IH_MINFREE>>, <<ISIZET(h_minfree)>>)
define(<<IINDEX>>, <<ISIZETV(index)>>)
define(<<IINITSIZE>>, <<ISIZET(initialsize)>>)
define(<<ILEN>>, <<ISIZET(<<len>>)>>)
define(<<IMAP>>, <<IPTRDIFFTV(imap)>>)
define(<<IMODE>>, <<IINT(mode)>>)
define(<<INAME>>, <<ITEXTV(name)>>)
define(<<INCID>>, <<IINT(ncid)>>)
define(<<INCIDIN>>, <<IINT(ncid_in)>>)
define(<<INCIDOUT>>, <<IINT(ncid_out)>>)
define(<<INDIMS>>, <<IINT(ndims)>>)
define(<<INEWNAME>>, <<ITEXTV(newname)>>)
define(<<IPATH>>, ITEXTV(path))
define(<<IPE>>, <<IINT(pe)>>)
define(<<IR_ALIGN>>, <<ISIZET(r_align)>>)
define(<<ISTART>>, <<ISIZETV(start)>>)
define(<<ISTATUS>>, <<IINT(status)>>)
define(<<ISTRIDE>>, <<ISIZETV(stride)>>)
define(<<IV_ALIGN>>, <<ISIZET(v_align)>>)
define(<<IV_MINFREE>>, <<ISIZET(v_minfree)>>)
define(<<IVARID>>, <<IINT(varid)>>)
define(<<IVARIDIN>>, <<IINT(varid_in)>>)
define(<<IVARIDOUT>>, <<IINT(varid_out)>>)
define(<<IXTYPE>>, <<INCTYPE(xtype)>>)
define(<<IPARACCESS>>, <<IINT(par_access)>>)

dnl Function Output Arguments:
define(<<OATTNUM>>, <<OINT(attnum)>>)
define(<<OCHUNKSIZE>>, <<OSIZET(chunksize)>>)
define(<<ODIMID>>, <<OINT(dimid)>>)
define(<<ODIMIDS>>, <<OINTV(dimids)>>)
define(<<OLEN>>, <<OSIZET(<<len>>)>>)
define(<<ONAME>>, <<OTEXTV(name)>>)
define(<<ONATTS>>, <<OINT(natts)>>)
define(<<ONCID>>, <<OINT(ncid)>>)
define(<<ONDIMS>>, <<OINT(ndims)>>)
define(<<ONVARS>>, <<OINT(nvars)>>)
define(<<OOLDFILLMODE>>, <<OINT(old_fillemode)>>)
define(<<OPE>>, <<OINT(pe)>>)
define(<<OVARID>>, <<OINT(varid)>>)
define(<<OUNLIMDIMID>>, <<OINT(unlimdimid)>>)
define(<<OFORMATN>>, <<OINT(formatn)>>)
define(<<OXTYPE>>, <<ONCTYPE(xtype)>>)

dnl Argument References:
define(<<ATTNUM>>, <<ARG(attnum)>>)
define(<<COUNT>>, <<ARG(count)>>)
define(<<DIMID>>, <<ARG(dimid)>>)
define(<<DIMIDS>>, <<ARG(dimids)>>)
define(<<FILLMODE>>, <<ARG(fillmode)>>)
define(<<IN>>, <<ARG(in)>>)
define(<<INDEX>>, <<ARG(index)>>)
define(<<LEN>>, <<ARG(<<len>>)>>)
define(<<IMAP>>, <<ARG(imap)>>)
define(<<NAME>>, <<ARG(name)>>)
define(<<NATTS>>, <<ARG(natts)>>)
define(<<NCID>>, <<ARG(ncid)>>)
define(<<NCIDIN>>, <<ARG(ncid_in)>>)
define(<<NCIDOUT>>, <<ARG(ncid_out)>>)
define(<<NDIMS>>, <<ARG(ndims)>>)
define(<<NEWNAME>>, <<ARG(newname)>>)
define(<<NULL>>, <<CODE(<<<<NULL>>>>)>>)
define(<<NVARS>>, <<ARG(nvars)>>)
define(<<NVATTS>>, <<ARG(nvatts)>>)
define(<<OLDFILLMODE>>, <<ARG(old_fillmode)>>)
define(<<OUT>>, <<ARG(out)>>)
define(<<START>>, <<ARG(start)>>)
define(<<STRIDE>>, <<ARG(stride)>>)
define(<<UNLIMDIMID>>, <<ARG(unlimdimid)>>)
define(<<FORMATN>>, <<ARG(formatn)>>)
define(<<VARID>>, <<ARG(varid)>>)
define(<<VARIDIN>>, <<ARG(varid_in)>>)
define(<<VARIDOUT>>, <<ARG(varid_out)>>)
define(<<XTYPE>>, <<ARG(xtype)>>)

define(<<UPCASE>>, 
<<translit($1,abcdefghijklmnopqrstuvwxyz,ABCDEFGHIJKLMNOPQRSTUVWXYZ)>>)

dnl Variable "Put" Functions:
define(<<VOUT>>, <<I<<>>UPCASE($1)<<>>ifelse($2,1,,V)(ifelse($2,1,*)out)>>)
define(<<VPUT>>, <<IODECL(put_var$1, $2, (INCID(), IVARID()$3, VOUT($2,$1)))>>)
define(<<PUT_VAR>>, <<VPUT(,$1)>>)
define(<<PUT_VAR1>>,<<VPUT(1,$1,<<, IINDEX()>>)>>)
define(<<PUT_VARA>>,<<VPUT(a,$1,<<, ISTART(), ICOUNT()>>)>>)
define(<<PUT_VARS>>,<<VPUT(s,$1,<<, ISTART(), ICOUNT(), ISTRIDE()>>)>>)
define(<<PUT_VARM>>,<<VPUT(m,$1,<<, ISTART(), ICOUNT(), ISTRIDE(), IMAP()>>)>>)

dnl Variable "Get" Functions:
define(<<VIN>>, <<O<<>>UPCASE($1)<<>>ifelse($2,1,,V)(in)>>)
define(<<VGET>>, <<IODECL(get_var$1, $2, (INCID(), IVARID()$3, VIN($2,$1)))>>)
define(<<GET_VAR>>, <<VGET(,$1)>>)
define(<<GET_VAR1>>,<<VGET(1,$1,<<, IINDEX()>>)>>)
define(<<GET_VARA>>,<<VGET(a,$1,<<, ISTART(), ICOUNT()>>)>>)
define(<<GET_VARS>>,<<VGET(s,$1,<<, ISTART(), ICOUNT(), ISTRIDE()>>)>>)
define(<<GET_VARM>>,<<VGET(m,$1,<<, ISTART(), ICOUNT(), ISTRIDE(), IMAP()>>)>>)

dnl Attribute "Put" Functions:
define(<<AOUT>>, <<I<<>>UPCASE($1)<<>>V(out)>>)
define(<<APUT>>,<<IODECL(put_att,$1,(INCID(), IVARID(), INAME(), IXTYPE(), ILEN(), AOUT($1)))>>)

dnl Attribute "Get" Functions:
define(<<AIN>>, <<O<<>>UPCASE($1)<<>>V(in)>>)
define(<<AGET>>,<<IODECL(get_att,$1,(INCID(), IVARID(), INAME(), AIN($1)))>>)

dnl Function Family Listing:
define(<<FUNC_FAMILY>>,
<<.HP
$1(text)
ifelse(API,C,
<<.HP
$1(uchar)>>)
.HP
$1(schar)
.HP
$1(short)
.HP
$1(int)
ifelse(API,C,
<<.HP
$1(long)>>)
.HP
$1(float)
.HP
$1(double)
ifelse(NETCDF4,TRUE,
<<.HP
$1(ubyte)
.HP
$1(ushort)
.HP
$1(uint)
.HP
$1(int64)
.HP
$1(uint64)
.HP
$1(string)>>
)
>>)

divert(0)dnl
.nr yr \n(yr+1900
.af mo 01
.af dy 01
.TH NETCDF 3 "1997-04-18" "Printed: \n(yr-\n(mo-\n(dy" "UNIDATA LIBRARY FUNCTIONS"
.SH N<<>>AME
netcdf \- Unidata's Network Common Data Form (netCDF) library interface
.SH SYNOPSIS
.ft B
.na
.nh
INCLUDE(netcdf)
.sp
ifelse(API,C,,
.SS Most Systems:)
ifelse(NETCDF4,TRUE,
COMPILER() ...  -lnetcdf -lhdf5_hl -lhdf5 -lz -lm,
COMPILER() ... -lnetcdf)
ifelse(API,C,,
.sp
.SS CRAY PVP Systems:
f90 -dp -i64 ... -lnetcdf
)
.ad
.hy
Complete documentation for the netCDF libraries can be found at the netCDF website: http://www.unidata.ucar.edu/software/netcdf/.
.sp
.SH "LIBRARY VERSION"
.LP
ifelse(NETCDF4,TRUE,
This document describes versions 3 and 4,
This document describes version 3
)
of Unidata netCDF data-access interface
for the LANGUAGE() programming language.
.HP
DECL(RETSTR(), inq_libvers, (VOID_ARG))
.sp
Returns a string identifying the version of the netCDF library, and
when it was built, like: "3.1a of Aug 22 1996 12:57:47 $".
.LP
The RCS \fBident(1)\fP command will find a string like
"$\|Id: @\|(#) netcdf library version 3.1a of Sep  6 1996 15:56:26 $"
in the library. The SCCS \fBwhat(1)\fP command will find a string like
"netcdf library version 3.1a of Aug 23 1996 16:07:40 $".
.SH "RETURN VALUES"
.LP
All netCDF functions (except
FREF(inq_libvers) and FREF(strerror)) return an integer status.

If this returned status value is not equal to
MACRO(NOERR) (zero), it
indicates that an error occurred. The possible status values are defined in 
ifelse(API,C, system <<<<include>>>> file <errno.h> and in )<<>>dnl
ifelse(API,C,")HEADER_FILE(netcdf)<<>>ifelse(API,C,").
.HP
DECL(RETSTR(), strerror, (ISTATUS()))
.sp
Returns a string textual translation of the \fIstatus\fP
value, like "Attribute or variable name contains illegal characters"
or "No such file or directory".
.sp
.SH "FILE OPERATIONS"
.LP
.HP
FDECL(create, (IPATH(), ICMODE(), ONCID()))
.sp
Creates a new netCDF dataset at ARG(path),
returning a netCDF ID in ARG(ncid).
The argument ARG(cmode) may <<include>> the bitwise-or
of the following flags:
MACRO(NOCLOBBER)
to protect existing datasets (default
silently blows them away),
MACRO(SHARE)
for synchronous dataset updates for classic format files
(default is to buffer accesses),
.sp
When a netCDF dataset is created, is is opened
MACRO(WRITE).
The new netCDF dataset is in <<define>> mode.
MACRO(64BIT_OFFSET).
to create a file in the 64-bit offset format 
(as opposed to classic format, the default). 
ifelse(NETCDF4,TRUE,
<<MACRO(NETCDF4) to create a netCDF-4/HDF5 file, 
and MACRO(CLASSIC_MODEL) to guarantee that netCDF-4/HDF5 files maintain compatibility 
with the netCDF classic data model.>>,
.
)
.HP
FDECL(_create, (IPATH(), ICMODE(), IINITSIZE(), OCHUNKSIZE(), ONCID()))
.sp
Like FREF(create) but has additional performance tuning parameters.
.sp
The argument ARG(initialsize) sets the initial size of the file at
creation time.
.sp
See FREF(_open) below for an explanation of the ARG(chunksize)
parameter.
.HP
FDECL(open, (IPATH(), IMODE(), ONCID()))
.sp
(Corresponds to FOLD(open, opn) in version 2)
.sp
Opens a existing netCDF dataset at ARG(path)
returning a netCDF ID
in ARG(ncid).
The type of access is described by the ARG(mode) parameter,
which may <<include>> the bitwise-or
of the following flags:
MACRO(WRITE)
for read-write access (default
read-only),
MACRO(SHARE)
for synchronous dataset updates (default is
to buffer accesses), and
MACRO(LOCK)
(not yet implemented).
.sp
ifelse(DAP,TRUE,
<<As of NetCDF version 4.1, and if DAP support was enabled
when the NetCDF library was built, the path parameter
may specify a DAP URL. In this case, the access mode is
forced to be read-only.>>)
.HP
FDECL(_open, (IPATH(), IMODE(), OCHUNKSIZE(), ONCID()))
.sp
Like FREF(open) but has an additional performance tuning parameter.
.sp
The argument referenced by ARG(chunksize) controls a space versus time
tradeoff, memory allocated in the netcdf library versus number of system
calls.
Because of internal requirements, the value may not be set to exactly
the value requested.
The actual value chosen is returned by reference.
Using the value MACRO(SIZEHINT_DEFAULT) causes the library to choose a
default.
How the system choses the default depends on the system.
On many systems, the "preferred I/O block size" is available from the 
CODE(stat()) system call, CODE(struct stat) member CODE(st_blksize).
If this is available it is used. Lacking that, twice the system pagesize
is used.
Lacking a call to discover the system pagesize, we just set default
chunksize to 8192.
.sp
The chunksize is a property of a given open netcdf descriptor
ARG(ncid), it is not a persistent property of the netcdf dataset.
.sp
ifelse(DAP,TRUE,
<<As with FREF(_open), the path parameter
may specify a DAP URL, but the tuning parameters are ignored.>>)
.HP
FDECL(redef, (INCID()))
.sp
(Corresponds to FOLD(redef, redf) in version 2)
.sp
Puts an open netCDF dataset into <<define>> mode, 
so dimensions, variables, and attributes can be added or renamed and 
attributes can be deleted.
.HP
FDECL(enddef, (INCID()))
.sp
(Corresponds to FOLD(endef, endf) in version 2)
.sp
Takes an open netCDF dataset out of <<define>> mode.
The changes made to the netCDF dataset
while it was in <<define>> mode are checked and committed to disk if no
problems occurred.  Some data values may be written as well,
see "VARIABLE PREFILLING" below.
After a successful call, variable data can be read or written to the dataset.
.HP
FDECL(_enddef, (INCID(), IH_MINFREE(), IV_ALIGN(), IV_MINFREE(), IR_ALIGN()))
.sp
Like FREF(enddef) but has additional performance tuning parameters.
.sp
Caution: this function exposes internals of the netcdf version 1 file
<<format>>.
It may not be available on future netcdf implementations.
.sp
The current netcdf file <<format>> has three sections,
the "header" section, the data section for fixed size variables, and
the data section for variables which have an unlimited dimension (record
variables).
The header begins at the beginning of the file. The <<index>>
(offset) of the beginning of the other two sections is contained in the
header. Typically, there is no space between the sections. This causes
copying overhead to accrue if one wishes to change the size of the
sections,
as may happen when changing names of things, text attribute values,
adding
attributes or adding variables. Also, for buffered i/o, there may be
advantages
to aligning sections in certain ways.
.sp
The minfree parameters allow one to control costs of future calls
to FREF(redef), FREF(enddef) by requesting that ARG(minfree) bytes be
available at the end of the section.
The ARG(h_minfree) parameter sets the pad
at the end of the "header" section. The ARG(v_minfree) parameter sets
the pad at the end of the data section for fixed size variables.
.sp
The align parameters allow one to set the alignment of the beginning of
the corresponding sections. The beginning of the section is rounded up
to an <<index>> which is a multiple of the align parameter. The flag value
MACRO(ALIGN_CHUNK) tells the library to use the chunksize (see above)
as the align parameter.
The ARG(v_align) parameter controls the alignment of the beginning of
the data section for fixed size variables.
The ARG(r_align) parameter controls the alignment of the beginning of
the data section for variables which have an unlimited dimension (record
variables).
.sp
The file <<format>> requires mod 4 alignment, so the align parameters
are silently rounded up to multiples of 4. The usual call,
CODE(FNAME(enddef)(NCID()))
is equivalent to
CODE(FNAME(_enddef)(NCID(), 0, 4, 0, 4)).
.sp
The file <<format>> does not contain a "record size" value, this is
calculated from the sizes of the record variables. This unfortunate fact
prevents us from providing minfree and alignment control of the
"records"
in a netcdf file. If you add a variable which has an unlimited
dimension,
the third section will always be copied with the new variable added.
.HP
FDECL(sync, (INCID()))
.sp
(Corresponds to FOLD(sync, snc) in version 2)
.sp
Unless the
MACRO(SHARE)
bit is set in
FREF(open) or FREF(create),
accesses to the underlying netCDF dataset are
buffered by the library. This function synchronizes the state of
the underlying dataset and the library.
This is done automatically by
FREF(close) and FREF(enddef).
.HP
FDECL(abort, (INCID()))
.sp
(Corresponds to FOLD(abort, abor) in version 2)
.sp
You don't need to call this function.
This function is called automatically by
FREF(close)
if the netCDF was in <<define>> mode and something goes wrong with the commit.
If the netCDF dataset isn't in <<define>> mode, then this function is equivalent to
FREF(close).
If it is called after
FREF(redef),
but before
FREF(enddef),
the new definitions are not committed and the dataset is closed.
If it is called after
FREF(create)
but before
FREF(enddef),
the dataset disappears.
.HP
FDECL(close, (INCID()))
.sp
(Corresponds to
FOLD(close, clos) in version 2)
.sp
Closes an open netCDF dataset.
If the dataset is in <<define>> mode,
FREF(enddef)
will be called before closing.
After a dataset is closed, its ID may be reassigned to another dataset.
.HP
FDECL(inq, (INCID(), ONDIMS(), ONVARS(),
ONATTS(), OUNLIMDIMID()))
.HP
FDECL(inq_ndims, (INCID(), ONDIMS()))
.HP
FDECL(inq_nvars, (INCID(), ONVARS()))
.HP
FDECL(inq_natts, (INCID(), ONATTS()))
.HP
FDECL(inq_unlimdim, (INCID(), OUNLIMDIMID()))
.HP
FDECL(inq_format, (INCID(), OFORMATN()))
.sp
Use these functions to find out what is in a netCDF dataset.
Upon successful return,
NDIMS() will contain  the
number of dimensions defined for this netCDF dataset,
NVARS() will contain the number of variables,
NATTS() will contain the number of attributes, and
UNLIMDIMID() will contain the
dimension ID of the unlimited dimension if one exists, or
ifelse(API,C, <<-1>>, <<0>>) otherwise.
FORMATN() will contain the version number of the dataset <format>, one of
MACRO(FORMAT_CLASSIC), MACRO(FORMAT_64BIT), MACRO(FORMAT_NETCDF4), or
MACRO(FORMAT_NETCDF4_CLASSIC).
ifelse(API,C,
<<If any of the
return parameters is a NULL() pointer, then the corresponding information
will not be returned; hence, no space need be allocated for it.>>)
.HP
FDECL(def_dim, (INCID(), INAME(), ILEN(), ODIMID()))
.sp
(Corresponds to FOLD(dimdef, ddef) in version 2)
.sp
Adds a new dimension to an open netCDF dataset, which must be 
in <<define>> mode.
NAME() is the dimension name.
ifelse(API,C,dnl
<<If DIMID() is not a NULL() pointer then upon successful completion >>)<<>>dnl
DIMID() will contain the dimension ID of the newly created dimension.
ifelse(NETCDF4,TRUE,
<<
.SH "USER DEFINED TYPES"
.LP
Users many define types for a netCDF-4/HDF5 file (unless the
MACRO(CLASSIC_MODEL) was used when the file was creates). Users may
define compound types, variable length arrays, enumeration types, and
opaque types.
.sp

.HP
FDECL(def_compound, (INCID(), ISIZET(size), INAME(), OINT(typeidp)))
.sp
Define a compound type.
.HP
FDECL(insert_compound, (INCID(), INCTYPE(), INAME(), ISIZET(offset), INCTYPE(field_typeid)))
.sp
Insert an element into a compound type. May not be done after type has been used, or after the type has been written by an enddef.
.HP
FDECL(insert_array_compound, (INCID(), INCTYPE(), INAME(), ISIZET(offset), INCTYPE(field_typeid), INDIMS(), IINTV(dim_sizes)))
.sp 
Insert an array into a compound type.
.HP
FDECL(inq_type, (INCID(), INCTYPE(), ONAME(), OSIZET(sizep)))
.sp
Learn about a type.
.HP
FDECL(inq_compound, (INCID(), INCTYPE(), ONAME(), OSIZET(sizep), OSIZET(nfieldsp)))
.HP
FDECL(inq_compound_name, (INCID(), INCTYPE(), ONAME()))
.HP
FDECL(inq_compound_size, (INCID(), INCTYPE(), OSIZET(sizep)))
.HP
FDECL(inq_compound_nfields, (INCID(), INCTYPE(), OSIZET(nfieldsp)))
.HP
FDECL(inq_compound_fieldname, (INCID(), INCTYPE(), IINT(fieldid), ONAME()))
.HP
FDECL(inq_compound_fieldindex, (INCID(), INCTYPE(), INAME(), OINT(fieldidp)))
.HP
FDECL(inq_compound_fieldoffset, (INCID(), INCTYPE(), IINT(fieldid), OSIZET(offsetp)))
.HP
FDECL(inq_compound_fieldtype, (INCID(), INCTYPE(), IINT(fieldid), ONCTYPE(field_typeid)))
.HP
FDECL(inq_compound_fieldndims, (INCID(), INCTYPE(), IINT(fieldid), ONDIMS()))
.HP
FDECL(inq_compound_fielddim_sizes, (INCID(), INCTYPE(), IINT(fieldid), OINTV(dim_sizes)))
.sp
Learn about a compound type.
.HP
FDECL(def_vlen, (INCID(), INAME(), INCTYPE(base_typeid), ONCTYPE(xtypep)))
.sp
Create a varaible length array type.
.HP
FDECL(inq_vlen, (INCID(), INCTYPE(), ONAME(), OSIZET(datum_sizep), ONCTYPE(base_nc_typep)))
.sp
Learn about a varaible length array type.
.HP
FDECL(free_vlen, (nc_vlen_t *vl))
.sp
Free memory comsumed by reading data of a varaible length array type.
.HP
FDECL(put_vlen_element, (INCID(), INCTYPE(), IVOIDP(vlen_element), ISIZET(len), IVOIDP(data)))
.sp
Write one VLEN.
.HP
FDECL(get_vlen_element, (INCID(), INCTYPE(), OVOIDP(vlen_element), ISIZET(len), OVOIDP(data)))
.sp
Read one VLEN.
.HP
FDECL(free_string, (ISIZET(len), char **data))
.sp
Free memory comsumed by reading data of a string type.
.HP
FDECL(inq_user_type, (INCID(), INCTYPE(), ONAME(), OSIZET(), ONCTYPE(), OSIZET(), OINT()))
.sp
Learn about a user define type.
.HP
FDECL(def_enum, (INCID(), INCTYPE(base_typeid), INAME(), ONCTYPE(typeidp)))
.sp
Define an enumeration type.
.HP
FDECL(insert_enum, (INCID(), INCTYPE(base_typeid), INAME(), const void *value))
.sp
Insert a name-value pair into enumeration type.
.HP
FDECL(inq_enum_member, (INCID(), INCTYPE(xtype), IINT(idx), ONAME(), void *value))
.HP
FDECL(inq_enum_ident, (INCID(), INCTYPE(xtype), IINT(idx), IINT64(value), OTEXTV(identifier)))
.sp
Learn about a name-value pair into enumeration type.
.HP
FDECL(def_opaque, (INCID(), ISIZET(size), INAME(), ONCTYPE(xtypep)))
.sp
Create an opaque type.
.HP
FDECL(inq_opaque, (INCID(), INCTYPE(xtype), ONAME(), OSIZET(sizep)))
.sp
Learn about opaque type.
.HP
.SH "GROUPS"
.sp
Users may organize data into hierarchical groups in netCDF-4/HDF5 files (unless MACRO(CLASSIC_MODEL) was used when creating the file).
.HP
FDECL(inq_grps, (INCID(), OINT(numgrps), OINTV(ncids)))
.sp
Learn how many groups (and their ncids) are available from the group represented by ncid.
.HP
FDECL(inq_grpname, (INCID(), ONAME()))
.HP
FDECL(inq_grpname_full, (INCID(), OLEN(), ONAME()))
.HP
FDECL(inq_grpname_len, (INCID(), OLEN()))
.HP
FDECL(inq_grp_parent, (INCID(), ONCID()))
.HP
FDECL(inq_grp_ncid, (INCID(), ONAME(), ONCID()))
.HP
FDECL(inq_full_ncid, (INCID(), ONAME(), ONCID()))
.sp
Learn about a group.
.HP
FDECL(inq_varids, (INCID(), ONVARS(), OINT()))
.sp
Get the varids in a group.
.HP
FDECL(inq_dimids, (INCID(), ONDIMS(), OINT(dimids), IINT(include_parents)))
.sp
Get the dimids in a group and (potentially) its parents.
.HP
FDECL(inq_typeids, (INCID(), OINT(ntypes), OINTV(typeids)))
.sp
Get the typeids of user-defined types in a group.
.HP
FDECL(def_grp, (INCID(), ONAME(), ONCID()))
.sp
Create a group.
.LP
>>)
.SH "DIMENSIONS"
.LP
.HP
FDECL(inq_dimid, (INCID(), INAME(), ODIMID()))
.sp
(Corresponds to FOLD(dimid, did) in version 2)
.sp
Given a dimension name, returns the ID of a netCDF dimension in DIMID().
.HP
FDECL(inq_dim, (INCID(), IDIMID(), ONAME(), OLEN()))
.HP
FDECL(inq_dimname, (INCID(), IDIMID(), ONAME()))
.HP
FDECL(inq_dimlen, (INCID(), IDIMID(), OLEN()))
.sp
Use these functions to find out about a dimension.
ifelse(API,C,
<<If either the NAME()
argument or LEN() argument is a NULL() pointer, then
the associated information will not be returned.  Otherwise,>>)
NAME() should be  big enough (MACRO(MAX_NAME))
to hold the dimension name as the name will be copied into your storage.
The length return parameter, LEN()
will contain the size of the dimension.
For the unlimited dimension, the returned length is the current
maximum value used for writing into any of the variables which use
the dimension.
.HP
FDECL(rename_dim, (INCID(), IDIMID(), INAME()))
.sp
(Corresponds to FOLD(dimrename, dren) in version 2)
.sp
Renames an existing dimension in an open netCDF dataset.
If the new name is longer than the old name, the netCDF dataset must be in 
<<define>> mode.
You cannot rename a dimension to have the same name as another dimension.
.SH "VARIABLES"
.LP
.HP
FDECL(def_var, (INCID(), INAME(), IXTYPE(), INDIMS(), IDIMIDS(), OVARID()))
.sp
(Corresponds to FOLD(vardef, vdef) in version 2)
.sp
Adds a new variable to a netCDF dataset. The netCDF must be in <<define>> mode.
ifelse(API,C, <<If not NULL(), then >>)dnl
VARID() will be set to the netCDF variable ID.
.HP
FDECL(inq_varid, (INCID(), INAME(), OVARID()))
.sp
(Corresponds to FOLD(varid, vid) in version 2)
.sp
Returns the ID of a netCDF variable in VARID() given its name.
.HP
FDECL(inq_var, (INCID(), IVARID(), ONAME(), OXTYPE(), ONDIMS(), ODIMIDS(),
ONATTS()))
.HP
FDECL(inq_varname, (INCID(), IVARID(), ONAME()))
.HP
FDECL(inq_vartype, (INCID(), IVARID(), OXTYPE()))
.HP
FDECL(inq_varndims, (INCID(), IVARID(), ONDIMS()))
.HP
FDECL(inq_vardimid, (INCID(), IVARID(), ODIMIDS()))
.HP
FDECL(inq_varnatts, (INCID(), IVARID(), ONATTS()))
.sp
Returns information about a netCDF variable, given its ID.
ifelse(API,C,
<<If any of the
return parameters (NAME(), XTYPE(), NDIMS(), DIMIDS(), or
NATTS()) is a NULL() pointer, then the corresponding information
will not be returned; hence, no space need be allocated for it.>>)
.HP
FDECL(rename_var, (INCID(), IVARID(), INAME()))
.sp
(Corresponds to FOLD(varrename, vren) in version 2)
.sp
Changes the name of a netCDF variable.
If the new name is longer than the old name, the netCDF must be in <<define>> mode.
You cannot rename a variable to have the name of any existing variable.
ifelse(NETCDF4,TRUE,
<<
.SH "VARIABLES IN NETCDF-4 FILES"
.LP
The following functions may only be used on variables in a
netCDF-4/HDF5 data file. These functions must be called after the
variable is defined, but before an enddef call.
.sp
FDECL(def_var_deflate, (INCID(), IVARID(), IINT(shuffle), IINT(deflate), IINT(deflate_level)))
.sp
Turn on compression and/or shuffle filter. (Shuffle filter is only useful for integer data.)
.HP
FDECL(inq_var_deflate, (INCID(), IVARID(), OINT(shufflep), OINT(deflatep), OINT(deflate_levelp)))
.sp
Learn about a variable's deflate settings.
.HP
FDECL(def_var_fletcher32, (INCID(), IVARID(), IINT(fletcher32)))
.sp
Turn on checksumming for a variable.
.HP
FDECL(inq_var_fletcher32, (INCID(), IVARID(), OINT(fletcher32)))
.sp
Learn about checksumming for a variable.
.HP
FDECL(def_var_chunking, (INCID(), IVARID(), IINT(storage), ISIZETV(chunksizesp)))
.sp
Set chunksizes for a variable.
.HP
FDECL(inq_var_chunking, (INCID(), IVARID(), OINT(storagep), OSIZETV(chunksizesp)))
.sp
Learn about chunksizes for a variable.
.HP
FDECL(def_var_fill, (INCID(), IVARID(), IINT(no_fill), ISIZETV(chunksizesp)))
.sp
Set a fill value for a variable.
.HP
FDECL(inq_var_fill, (INCID(), IVARID(), OINT(storagep), OSIZETV(chunksizesp)))
.sp
Learn the fill value for a variable.
.HP
FDECL(def_var_endian, (INCID(), IVARID(), IINT(endian)))
.sp
Set endianness of variable.
.HP
FDECL(inq_var_endian, (INCID(), IVARID(), OINT(endianp)))
.sp
Learn the endianness of a variable.
.HP
>>)
.SH "WRITING AND READING WHOLE VARIABLES"
.LP
FUNC_FAMILY(<<PUT_VAR>>)
.sp
Writes an entire netCDF variable (i.e. all the values).  The netCDF
dataset must be open and in data mode.  The type of the data is
specified in the function name, and it is converted to the external
type of the specified variable, if possible, otherwise an
MACRO(ERANGE) error is returned. Note that rounding is not performed
during the conversion. Floating point numbers are truncated when
converted to integers.
FUNC_FAMILY(<<GET_VAR>>)
.sp
Reads an entire netCDF variable (i.e. all the values).
The netCDF dataset must be open and in data mode.  
The data is converted from the external type of the specified variable,
if necessary, to the type specified in the function name.  If conversion is
not possible, an MACRO(ERANGE) error is returned.
.SH "WRITING AND READING ONE DATUM"
.LP
FUNC_FAMILY(<<PUT_VAR1>>)
.sp
Puts a single data value into a variable at the position INDEX() of an
open netCDF dataset that is in data mode.  The type of the data is
specified in the function name, and it is converted to the external type
of the specified variable, if possible, otherwise an MACRO(ERANGE)
error is returned.
FUNC_FAMILY(<<GET_VAR1>>)
.sp
Gets a single data value from a variable at the position INDEX()
of an open netCDF dataset that is in data mode.  
The data is converted from the external type of the specified variable,
if necessary, to the type specified in the function name.  If conversion is
not possible, an MACRO(ERANGE) error is returned.
.SH "WRITING AND READING AN ARRAY"
.LP
FUNC_FAMILY(<<PUT_VARA>>)
.sp
Writes an array section of values into a netCDF variable of an open
netCDF dataset, which must be in data mode.  The array section is specified
by the START() and COUNT() vectors, which give the starting <<index>>
and count of values along each dimension of the specified variable.
The type of the data is
specified in the function name and is converted to the external type
of the specified variable, if possible, otherwise an MACRO(ERANGE)
error is returned.
FUNC_FAMILY(<<GET_VARA>>)
.sp
Reads an array section of values from a netCDF variable of an open
netCDF dataset, which must be in data mode.  The array section is specified
by the START() and COUNT() vectors, which give the starting <<index>>
and count of values along each dimension of the specified variable.
The data is converted from the external type of the specified variable,
if necessary, to the type specified in the function name.  If conversion is
not possible, an MACRO(ERANGE) error is returned.
.SH "WRITING AND READING A SLICED ARRAY"
.LP
FUNC_FAMILY(<<PUT_VARS>>)
.sp
These functions are used for \fIstrided output\fP, which is like the
array section output described above, except that
the sampling stride (the interval between accessed values) is
specified for each dimension.
For an explanation of the sampling stride
vector, see COMMON ARGUMENTS DESCRIPTIONS below.
FUNC_FAMILY(<<GET_VARS>>)
.sp
These functions are used for \fIstrided input\fP, which is like the
array section input described above, except that 
the sampling stride (the interval between accessed values) is
specified for each dimension.
For an explanation of the sampling stride
vector, see COMMON ARGUMENTS DESCRIPTIONS below.
.SH "WRITING AND READING A MAPPED ARRAY"
.LP
FUNC_FAMILY(<<PUT_VARM>>)
.sp
These functions are used for \fImapped output\fP, which is like
strided output described above, except that an additional <<index>> mapping
vector is provided to specify the in-memory arrangement of the data
values.
For an explanation of the <<index>>
mapping vector, see COMMON ARGUMENTS DESCRIPTIONS below.
FUNC_FAMILY(<<GET_VARM>>)
.sp
These functions are used for \fImapped input\fP, which is like
strided input described above, except that an additional <<index>> mapping
vector is provided to specify the in-memory arrangement of the data
values.
For an explanation of the <<index>>
mapping vector, see COMMON ARGUMENTS DESCRIPTIONS below.
.SH "ATTRIBUTES"
.LP
FUNC_FAMILY(<<APUT>>)
.HP
FDECL(put_att, (INCID(), IVARID(), INAME(), INCTYPE(xtype), ISIZET(len), IVOIDP(ip)))
.HP
FDECL(get_att, (INCID(), IVARID(), INAME(), OVOIDP(ip)))
.sp
Unlike variables, attributes do not have 
separate functions for defining and writing values.
This family of functions defines a new attribute with a value or changes
the value of an existing attribute.
If the attribute is new, or if the space required to
store the attribute value is greater than before,
the netCDF dataset must be in <<define>> mode.
The parameter LEN() is the number of values from OUT() to transfer.
It is often one, except that for
FREF(put_att_text) it will usually be
ifelse(API,C, <<CODE(strlen(OUT())).>>, <<CODE(len_trim(OUT())).>>)
.sp
For these functions, the type component of the function name refers to
the in-memory type of the value, whereas the XTYPE() argument refers to the
external type for storing the value.  An MACRO(ERANGE)
error results if
a conversion between these types is not possible.  In this case the value
is represented with the appropriate fill-value for the associated 
external type.
.HP
FDECL(inq_attname, (INCID(), IVARID(), IATTNUM(), ONAME()))
.sp
Gets the
name of an attribute, given its variable ID and attribute number.
This function is useful in generic applications that
need to get the names of all the attributes associated with a variable,
since attributes are accessed by name rather than number in all other
attribute functions.  The number of an attribute is more volatile than
the name, since it can change when other attributes of the same variable
are deleted.  The attributes for each variable are numbered
from ifelse(API,C,0,1) (the first attribute) to
NVATTS()<<>>ifelse(API,C,-1),
where NVATTS() is
the number of attributes for the variable, as returned from a call to
FREF(inq_varnatts).
ifelse(API,C,
<<If the NAME() parameter is a NULL() pointer, no name will be
returned and no space need be allocated.>>)
.HP
FDECL(inq_att, (INCID(), IVARID(), INAME(), OXTYPE(), OLEN()))
.HP
FDECL(inq_attid, (INCID(), IVARID(), INAME(), OATTNUM()))
.HP
FDECL(inq_atttype, (INCID(), IVARID(), INAME(), OXTYPE()))
.HP
FDECL(inq_attlen, (INCID(), IVARID(), INAME(), OLEN()))
.sp
These functions return information about a netCDF attribute,
given its variable ID and name.  The information returned is the
external type in XTYPE()
and the number of elements in the attribute as LEN().
ifelse(API,C,
<<If any of the return arguments is a NULL() pointer,
the specified information will not be returned.>>)
.HP
FDECL(copy_att, (INCID(), IVARIDIN(), INAME(), INCIDOUT(), IVARIDOUT()))
.sp
Copies an
attribute from one netCDF dataset to another.  It can also be used to
copy an attribute from one variable to another within the same netCDF.
NCIDIN() is the netCDF ID of an input netCDF dataset from which the
attribute will be copied.
VARIDIN()
is the ID of the variable in the input netCDF dataset from which the
attribute will be copied, or MACRO(GLOBAL)
for a global attribute.
NAME()
is the name of the attribute in the input netCDF dataset to be copied.
NCIDOUT()
is the netCDF ID of the output netCDF dataset to which the attribute will be 
copied.
It is permissible for the input and output netCDF ID's to be the same.  The
output netCDF dataset should be in <<define>> mode if the attribute to be
copied does not already exist for the target variable, or if it would
cause an existing target attribute to grow.
VARIDOUT()
is the ID of the variable in the output netCDF dataset to which the attribute will
be copied, or MACRO(GLOBAL) to copy to a global attribute.
.HP
FDECL(rename_att, (INCID(), IVARID(), INAME(), INEWNAME()))
.sp
Changes the
name of an attribute.  If the new name is longer than the original name,
the netCDF must be in <<define>> mode.  You cannot rename an attribute to
have the same name as another attribute of the same variable.
NAME() is the original attribute name.
NEWNAME()
is the new name to be assigned to the specified attribute.  If the new name
is longer than the old name, the netCDF dataset must be in <<define>> mode.
.HP
FDECL(del_att, (INCID(), IVARID(), INAME()))
.sp
Deletes an attribute from a netCDF dataset.  The dataset must be in
<<define>> mode.
FUNC_FAMILY(<<AGET>>)
.sp
Gets the value(s) of a netCDF attribute, given its
variable ID and name.  Converts from the external type to the type
specified in
the function name, if possible, otherwise returns an MACRO(ERANGE)
error.
All elements of the vector of attribute
values are returned, so you must allocate enough space to hold
them.  If you don't know how much space to reserve, call
FREF(inq_attlen)
first to find out the length of the attribute.
.SH "COMMON ARGUMENT DESCRIPTIONS"
.LP
In this section we <<define>> some common arguments which are used in the 
"FUNCTION DESCRIPTIONS" section.
.TP
INCID()
is the netCDF ID returned from a previous, successful call to
FREF(open) or FREF(create)
.TP
ONAME()
is the name of a dimension, variable, or attribute. The names of 
dimensions, variables and attributes consist of arbitrary
sequences of alphanumeric characters (as well as underscore '_',
period '.' and hyphen '-'), beginning with a letter or
underscore. (However names commencing with underscore are reserved for
system use.) Case is significant in netCDF names. A zero-length name
is not allowed.
ifelse(API,C,<<As an input argument, 
it shall be a pointer to a 0-terminated string; as an output argument, it 
shall be the address of a buffer in which to hold such a string.>>)
The maximum allowable number of characters 
ifelse(API,C,(excluding the terminating 0)) is MACRO(MAX_NAME).
.TP
IXTYPE()
specifies the external data type of a netCDF variable or attribute and
is one of the following:
MACRO(BYTE), MACRO(CHAR), MACRO(SHORT), MACRO(INT), 
MACRO(FLOAT), or MACRO(DOUBLE).
These are used to specify 8-bit integers,
characters, 16-bit integers, 32-bit integers, 32-bit IEEE floating point
numbers, and 64-bit IEEE floating-point numbers, respectively.
ifelse(API,C,
<<(MACRO(INT) corresponds to MACRO(LONG) in version 2, to specify a
32-bit integer).>>)
.TP
ODIMIDS()
is a vector of dimension ID's and defines the shape of a netCDF variable.
The size of the vector shall be greater than or equal to the
rank (i.e. the number of dimensions) of the variable (NDIMS()).
The vector shall be ordered by the speed with which a dimension varies:
DIMIDS()<<>>ifelse(API,C,<<[NDIMS()-1]>>,<<(1)>>)
shall be the dimension ID of the most rapidly
varying dimension and
DIMIDS()<<>>ifelse(API,C,<<[0]>>,<<(NDIMS())>>)
shall be the dimension ID of the most slowly
varying dimension.
The maximum possible number of
dimensions for a variable is given by the symbolic constant
MACRO(MAX_VAR_DIMS).
.TP
IDIMID()
is the ID of a netCDF dimension.
netCDF dimension ID's are allocated sequentially from the 
ifelse(API,C,non-negative, positive)
integers beginning with ifelse(API,C,0,1).
.TP
INDIMS()
is either the total number of dimensions in a netCDF dataset or the rank
(i.e. the number of dimensions) of a netCDF variable.
The value shall not be negative or greater than the symbolic constant 
MACRO(MAX_VAR_DIMS).
.TP
IVARID()
is the ID of a netCDF variable or (for the attribute-access functions) 
the symbolic constant
MACRO(GLOBAL),
which is used to reference global attributes.
netCDF variable ID's are allocated sequentially from the 
ifelse(API,C,non-negative,positive)
integers beginning with ifelse(API,C,0,1).
.TP
ONATTS()
is the number of global attributes in a netCDF dataset  for the
FREF(inquire)
function or the number
of attributes associated with a netCDF variable for the
FREF(varinq)
function.
.TP
IINDEX()
specifies the indicial coordinates of the netCDF data value to be accessed.
The indices start at ifelse(API,C,0,1);
thus, for example, the first data value of a
two-dimensional variable is ifelse(API,C,(0,0),(1,1)).
The size of the vector shall be at least the rank of the associated
netCDF variable and its elements shall correspond, in order, to the
variable's dimensions.
.TP
ISTART()
specifies the starting point
for accessing a netCDF variable's data values
in terms of the indicial coordinates of 
the corner of the array section.
The indices start at ifelse(API,C,0,1);
thus, the first data
value of a variable is ifelse(API,C,(0, 0, ..., 0),(1, 1, ..., 1)).
The size of the vector shall be at least the rank of the associated
netCDF variable and its elements shall correspond, in order, to the
variable's dimensions.
.TP
ICOUNT()
specifies the number of indices selected along each dimension of the
array section.
Thus, to access a single value, for example, specify COUNT() as
(1, 1, ..., 1).
Note that, for strided I/O, this argument must be adjusted
to be compatible with the STRIDE() and START() arguments so that 
the interaction of the
three does not attempt to access an invalid data co-ordinate.
The elements of the
COUNT() vector correspond, in order, to the variable's dimensions.
.TP
ISTRIDE()
specifies the sampling interval along each dimension of the netCDF
variable.   The elements of the stride vector correspond, in order,
to the netCDF variable's dimensions (ARG(stride)<<>>ifelse(API,C,[0],<<(1)>>))
gives the sampling interval along the most ifelse(API,C,slowly,rapidly) 
varying dimension of the netCDF variable).  Sampling intervals are
specified in type-independent units of elements (a value of 1 selects
consecutive elements of the netCDF variable along the corresponding
dimension, a value of 2 selects every other element, etc.).
ifelse(API,C,<<A NULL() stride argument is treated as (1, 1, ... , 1).>>)
.TP
IMAP()
specifies the mapping between the dimensions of a netCDF variable and
the in-memory structure of the internal data array.  The elements of
the <<index>> mapping vector correspond, in order, to the netCDF variable's
dimensions (ARG(imap)<<>>ifelse(API,C,[0],<<(1)>>) gives the distance
between elements of the internal array corresponding to the most
ifelse(API,C,slowly,rapidly) varying dimension of the netCDF variable).
Distances between elements are specified in type-independent units of
elements (the distance between internal elements that occupy adjacent
memory locations is 1 and not the element's byte-length as in netCDF 2).
ifelse(API,C,<<A NULL() pointer means the memory-resident values have
the same structure as the associated netCDF variable.>>)
.SH "VARIABLE PREFILLING"
.LP
By default, the netCDF interface sets the values of
all newly-defined variables of finite length (i.e. those that do not have
an unlimited, dimension) to the type-dependent fill-value associated with each 
variable.  This is done when FREF(enddef)
is called.  The
fill-value for a variable may be changed from the default value by
defining the attribute `CODE(_FillValue)' for the variable.  This
attribute must have the same type as the variable and be of length one.
.LP
Variables with an unlimited dimension are also prefilled, but on
an `as needed' basis.  For example, if the first write of such a
variable is to position 5, then
positions
ifelse(API,C,0 through 4, 1 through 4)
(and no others)
would be set to the fill-value at the same time.
.LP
This default prefilling of data values may be disabled by
or'ing the
MACRO(NOFILL)
flag into the mode parameter of FREF(open) or FREF(create),
or, by calling the function FREF(set_fill)
with the argument MACRO(NOFILL).
For variables that do not use the unlimited dimension,
this call must
be made before
FREF(enddef).
For variables that
use the unlimited dimension, this call
may be made at any time.
.LP
One can obtain increased performance of the netCDF interface by using 
this feature, but only at the expense of requiring the application to set
every single data value.  The performance
enhancing behavior of this function is dependent on the particulars of
the implementation and dataset <<format>>.
The flag value controlled by FREF(set_fill)
is per netCDF ID,
not per variable or per write. 
Allowing this to change affects the degree to which
a program can be effectively parallelized.
Given all of this, we state that the use
of this feature may not be available (or even needed) in future
releases. Programmers are cautioned against heavy reliance upon this
feature.
.HP
FDECL(setfill, (INCID(), IFILLMODE(), OOLDFILLMODE()))
ifelse(API,C,
<<.sp
(Corresponds to FOLD(setfill) in version 2)>>)
.sp
Determines whether or not variable prefilling will be done (see 
above).
The netCDF dataset shall be writable.
FILLMODE() is either MACRO(FILL)
to enable prefilling (the
default) or MACRO(NOFILL)
to disable prefilling.
This function returns the previous setting in OLDFILLMODE().
ifelse(PARALLEL_IO,TRUE,
<<.SH "PARALLEL I/O"
.LP
.HP
FDECL(create_par, (IPATH(), ICMODE(), MPI_Comm comm, MPI_Info info, ONCID()))
.sp
Like FREF(create) but creates for parallel I/O access. The mode must specify a 
netCDF-4/HDF5 dataset.
.sp
.HP
FDECL(open_par, (IPATH(), IMODE(), MPI_Comm comm, MPI_Info info, ONCID()))
.sp
Opens for parallel I/O access. Must be a netCDF-4/HDF5 dataset.
.HP
FDECL(var_par_access, (INCID(), VARID(), IPARACCESS()))
.sp
May be used to change access to a variable from independent to collective data access.>>)
.HP
.SH "MPP FUNCTION DESCRIPTIONS"
.LP
Additional functions for use on SGI/Cray MPP machines (_CRAYMPP).
These are used to set and inquire which PE is the base for MPP
for a particular netCDF. These are only relevant when
using the SGI/Cray ``global''
Flexible File I/O layer and desire to have
only a subset of PEs to open the specific netCDF file.
For technical reasons, these functions are available on all platforms.
On a platform other than SGI/Cray MPP, it is as if
only processor available were processor 0.
.LP
To use this feature, you need to specify a communicator group and call
CODE(glio_group_mpi(\|)) or CODE(glio_group_shmem(\|)) prior to the netCDF
FREF(open) and FREF(create) calls.
.HP
FDECL(_create_mp, (IPATH(), ICMODE(), IINITSIZE(), IPE(), OCHUNKSIZE(), ONCID()))
.sp
Like FREF(_create) but allows the base PE to be set.
.sp
The argument ARG(pe) sets the base PE at creation time. In the MPP
environment, FREF(_create) and FREF(create) set the base PE to processor
zero by default.
.HP
FDECL(_open_mp, (IPATH(), IMODE(), IPE(), OCHUNKSIZE(), ONCID()))
.sp
Like FREF(_open) but allows the base PE to be set.
The argument ARG(pe) sets the base PE at creation time. In the MPP
environment, FREF(_open) and FREF(open) set the base PE to processor
zero by default.
.HP
FDECL(inq_base_pe, (INCID(), OPE()))
.sp
Inquires of the netCDF dataset which PE is being used as the base for MPP use.
This is safe to use at any time.
.HP
FDECL(set_base_pe, (INCID(), IPE()))
.sp
Resets the base PE for the netCDF dataset.
Only perform this operation when the affected communicator group
synchronizes before and after the call.
This operation is very risky and should only be contemplated
under only the most extreme cases.
.SH "ENVIRONMENT VARIABLES"
.TP 4
.B NETCDF_FFIOSPEC
Specifies the Flexible File I/O buffers for netCDF I/O when executing
under the UNICOS operating system (the variable is ignored on other
operating systems).
An appropriate specification can greatly increase the efficiency of 
netCDF I/O -- to the extent that it can actually surpass FORTRAN binary 
I/O.
This environment variable has been made a little more generalized,
such that other FFIO option specifications can now be added.
The default specification is \fBbufa:336:2\fP,
unless a current FFIO specification is in operation,
which will be honored.
See UNICOS Flexible File I/O for more information.
.SH "MAILING-LISTS"
.LP
Both a mailing list and a digest are available for
discussion of the netCDF interface and announcements about netCDF bugs,
fixes, and enhancements.
To begin or change your subscription to either the mailing-list or the
digest, send one of the following in the body (not
the subject line) of an email message to "majordomo@unidata.ucar.edu".
Use your email address in place of \fIjdoe@host.inst.domain\fP.
.sp
To subscribe to the netCDF mailing list:
.RS
\fBsubscribe netcdfgroup \fIjdoe@host.inst.domain\fR
.RE
To unsubscribe from the netCDF mailing list:
.RS
\fBunsubscribe netcdfgroup \fIjdoe@host.inst.domain\fR
.RE
To subscribe to the netCDF digest:
.RS
\fBsubscribe netcdfdigest \fIjdoe@host.inst.domain\fR
.RE
To unsubscribe from the netCDF digest:
.RS
\fBunsubscribe netcdfdigest \fIjdoe@host.inst.domain\fR
.RE
To retrieve the general introductory information for the mailing list:
.RS
\fBinfo netcdfgroup\fR
.RE
To get a synopsis of other majordomo commands:
.RS
\fBhelp\fR
.RE
.SH "SEE ALSO"
.LP
.BR ncdump (1),
.BR ncgen (1),
.BR netcdf (3<<>>ifelse(API,C,,f)).
.LP
\fInetCDF User's Guide\fP, published
by the Unidata Program Center, University Corporation for Atmospheric
Research, located in Boulder, Colorado.

NetCDF home page at http:/www.unidata.ucar.edu/netcdf.
