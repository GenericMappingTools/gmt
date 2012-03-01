/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT dap for more information. */

/*
Draft OC External Interface
Created: 4/4/2009
Last Revised: 4/14/2009
*/

#ifndef OC_H
#define OC_H

#include <stdlib.h>

/* if defined, use magic numbers for consistency]) */
#define OC_FASTCONSISTENCY

/* OC_MAX_DIMS should be greater or equal to max allowed by dap or netcdf*/
#define OC_MAX_DIMS 1024

/* Specifies the OCtype.*/
/* Primitives = Duplicate of the NODE_Byte..Node_URL union nc_type*/

typedef unsigned long OCtype;

/* Note: use #define rather than enum so we can extend if needed
   more easily */
/* Primitives*/
/* OC_Ubyte, OC_Char, OC_Int64 and OC_UInt64 are defined for future extension*/
#define OC_NAT ((OCtype)0)
#define OC_Char ((OCtype)1)
#define OC_Byte ((OCtype)2)
#define OC_UByte ((OCtype)3)
#define OC_Int16 ((OCtype)4)
#define OC_UInt16 ((OCtype)5)
#define OC_Int32 ((OCtype)6)
#define OC_UInt32 ((OCtype)7)
#define OC_Int64 ((OCtype)8)
#define OC_UInt64 ((OCtype)9)
#define OC_Float32 ((OCtype)10)
#define OC_Float64 ((OCtype)11)
#define OC_String ((OCtype)12)
#define OC_URL ((OCtype)13)

/* Non-primitives*/
#define OC_Dataset	((OCtype)100)
#define OC_Sequence	((OCtype)101)
#define OC_Grid		((OCtype)102)
#define OC_Structure	((OCtype)103)
#define OC_Dimension	((OCtype)104)
#define OC_Attribute	((OCtype)105)
#define OC_Attributeset	((OCtype)106)
#define OC_Primitive	((OCtype)107)
#define OC_Group	((OCtype)108)
#define OC_Type		((OCtype)109)

/* Define a set of error
  positive are system errors
  (needs work)
*/

typedef int OCerror;
#define OC_NOERR	(0)
#define OC_EBADID	(-1)
#define OC_ECHAR	(-2)
#define OC_EDIMSIZE	(-3)
#define OC_EEDGE	(-4)
#define OC_EINVAL	(-5)
#define OC_EINVALCOORDS	(-6)
#define OC_ENOMEM	(-7)
#define OC_ENOTVAR	(-8)
#define OC_EPERM	(-9)
#define OC_ESTRIDE	(-10)
#define OC_EDAP		(-11)
#define OC_EXDR		(-12)
#define OC_ECURL	(-13)
#define OC_EBADURL	(-14)
#define OC_EBADVAR	(-15)
#define OC_EOPEN	(-16)
#define OC_EIO   	(-17)
#define OC_ENODATA   	(-18)
#define OC_EDAPSVC   	(-19)
#define OC_ENAMEINUSE  	(-20)
#define OC_EDAS		(-21)
#define OC_EDDS		(-22)
#define OC_EDATADDS	(-23)
#define OC_ERCFILE	(-24)
#define OC_ENOFILE	(-25)

typedef enum OCmode {
OCFIELDMODE = OC_Structure,
OCRECORDMODE = OC_Sequence,
OCARRAYMODE = OC_Dimension,
OCSCALARMODE = OC_Primitive,
OCNULLMODE = OC_NAT,
OCEMPTYMODE = 0x8000000 /* internal use only */
} OCmode;

/* Define the classes of DAP DXD objects */
typedef int OCdxd;
#define OCDDS     0
#define OCDAS     1
#define OCDATADDS 2
#define OCDATA OCDATADDS

/* Define the effective API */

#ifndef OCINTERNAL_H

/* The OCobject type references a component of a DAS or DDS
   (e.g. Sequence, Grid, Dataset, etc). These objects
   are nested, so most objects reference a container object
   and subnode objects.
*/


#ifdef OC_FASTCONSISTENCY
/* Use unsigned int * so we can dereference to get magic number */
typedef unsigned int* OCobject;
#define OCNULL NULL
#else
typedef unsigned long OCobject;
#define OCNULL ((OCobject)0)
#endif

/* These are the two critical types*/
/* Think of OClink as analogous to the C stdio FILE structure;
   it "holds" all the other state info about
   a connection to the server, the url request, and the DAS/DDS/DATADDSinfo.
*/
/* Renamed from OCconnection because of confusion about term "connection"
   3/24/2010 by dmh
*/
typedef OCobject OClink;

/* Keep old name for back compatibility */
typedef OClink OCconnection; /*Deprecated*/

/* Tag kind of log entry*/
#define OCLOGNOTE 0
#define OCLOGWARN 1
#define OCLOGERR 2
#define OCLOGDBG 3

/**************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern int oc_dumpnode(OClink conn, OCobject root0);

/**************************************************/
/* Link management */

extern OCerror oc_open(const char* url, OClink*);
extern OCerror oc_close(OClink);

/**************************************************/
/* Root management */

/* Fetch and parse a given class of DXD the server specified
   at open time, and using a specified set of constraints.
   Return the root node of the parsed tree of objects.
*/
extern OCerror oc_fetch(OClink,
			const char* constraints,
			OCdxd,
			OCobject* rootp);

/* Release/reclaim the tree of objects associated with a given root */
extern OCerror oc_root_free(OClink, OCobject root);

/* Return the # of  OCobjects associated with a tree with specified root */
extern unsigned int oc_inq_nobjects(OClink, OCobject root);

/* Return all the OCobjects associated with a tree with specified root */
extern OCobject* oc_inq_objects(OClink, OCobject root);

/* Return the text of the DDS or DAS as received from the server */
extern const char* oc_inq_text(OClink, OCobject root);

/**************************************************/
/* Object Management */

/* Any of the pointers may be NULL in the following procedure call;
  If the object is of type Dataset, then return # of global attributes
  If the object is of type Attribute, then return the # of values in nattrp.
  The caller must free the resulting name string.
*/
extern OCerror oc_inq_object(OClink, OCobject,
		  char** namep,
		  OCtype* typep,
		  OCtype* primitivetypep, /* if objecttype == OC_Primitive */
		  OCobject* parentp,
		  unsigned int* rankp,
		  unsigned int* nsubnodesp,
		  unsigned int* nattrp);

/* Also define some more individual accessors */

extern OCerror oc_inq_name(OClink,OCobject,char**);
extern OCerror oc_inq_class(OClink,OCobject,OCtype*);
extern OCerror oc_inq_type(OClink,OCobject,OCtype*); /*alias for oc_inq_class*/
extern OCerror oc_inq_primtype(OClink,OCobject,OCtype*);
extern OCerror oc_inq_nsubnodes(OClink,OCobject,unsigned int*);
extern OCerror oc_inq_rank(OClink,OCobject,unsigned int*);
extern OCerror oc_inq_nattr(OClink,OCobject,unsigned int*);
extern OCerror oc_inq_root(OClink,OCobject,OCobject*);
extern OCerror oc_inq_container(OClink,OCobject,OCobject*);

/* Return the subnode objects, if any, associated with a given object.
Caller must free the returned subnodes memory.
*/
extern OCerror oc_inq_subnodes(OClink,OCobject,OCobject** subnodes);

/* Return the i'th subnode object, if any, associated with a given object */
/* If there is none such, then return OC_EINVAL */
extern OCerror oc_inq_ith(OClink,OCobject, unsigned int, OCobject*);

/* Return the dimension objects, if any, associated with a given object */
/* Caller must free returned vector for dimids */
/* If there are no dimensions (i.e. rank == 0), then return NULL */
extern OCerror oc_inq_dimset(OClink,OCobject, OCobject** dimids);

/* Return the i'th dim object, if any, associated with a given object */
/* If there is no such dim, then return OC_EINVAL */
extern OCerror oc_inq_ithdim(OClink,OCobject, unsigned int, OCobject*);

/* Return the size and name associated with a given dimension object
   as defined in the DDS
*/
extern OCerror oc_inq_dim(OClink,OCobject,size_t*,char**);

/* Attribute Management */

/* Added: 11/2/2009 DMH:
   Provide access to DDS node attributes
   in the form of the original underlying
   DAS string.
   One specifies the DDS root to get the global attributes.
   Caller must free returned strings.
*/

extern OCerror oc_inq_attrstrings(OClink,OCobject, unsigned int i,
			   char** name, OCtype* octype,
			   unsigned int* nvalues,char*** stringvalues);

/* Obtain the attributes associated with a given DDS OCobject.
   One specifies the DDS root to get the global attributes
   This code takes the DAS strings and does a default
   conversion to binary values.
*/
extern OCerror oc_inq_attr(OClink,OCobject, unsigned int i,
			   char** name,OCtype* octype,
			   unsigned int* nvalues,void** values);

/* Convenience function to simplify reclaiming the allocated attribute
   value memory
*/
extern void oc_attr_reclaim(OCtype, unsigned int nvalues, void* values);

/* Access ith value string of a DAS object.
   OCtype of the object is assumed to be OC_Attribute.
   Note that this is  different than the above inq_attr
   and inq_attrstrings, which work on DDS
   objects. Note also that the return value is always a string.
   Caller must free returned string.
*/

extern OCerror oc_inq_dasattr_nvalues(OClink, OCobject,
			unsigned int* nvaluesp);

extern OCerror oc_inq_dasattr(OClink,OCobject, unsigned int,
			      OCtype* primtypep, char** valuep);

/**************************************************/
/* Data management */

/*
These procedures allow for the location and extraction
of data from the data packet part of a DATADDS.

Data is assumed to be "nested" in the sense that, for example,
the data of each structure field is "contained" in the
data of the structure.  The whole of the data packet data
is the data associated with the Dataset OCobject.

State information about the current data is defined by an
object with type OCdata. Such an object can be reused while
iterating over the elements of, say, a record, so that
proliferation of OCdata objects is avoided (which is why
oc_data_new exists).

Key to the access is the idea that every non-primitive
object is made up of components and that it is possible to
meaningfully define the notion of an "i'th" component.  The
i'th component depends on the type of the object.  The
"mode" of an OCdata object (defined by the OCmode type)
determines what kind of data is being referenced and what
the i value signifies.

OCmode		I'th component
--------------------------------
OCFIELDMODe	i'th field
OCRECORDMODE	i'th record
OCARRAYMODE	i'th element
OCMSCALARMODE	0'th element

For handling of multi-dimensional arrays, see the discussion
in the ocinternals.html document.

The general procedure to get to a particular scalar value or array
of values is as follows:
1. Get the data of the root OCobject; this root is of OCtype OC_Dataset.
2. Get the data associated with the i'th component of the root.
3. Repeatedly and recursively get the data associated
   with the i'th component of some object until the desired
   primitive valued Scalar or Array is reached.

The oc_data_ith function performs these walks.  At the
point at which primitive data has been reached, one can
use the oc_data_get function to obtain the actual data.

*/

typedef OCobject OCdata;

/* Obtain the OCdata object that references the
   whole data from a DATADDS fetch request specified
   by the root object. This does not access the server.
*/
extern OCerror oc_data_root(OClink, OCobject root, OCdata);

/* Create an empty OCdata object */
extern OCdata oc_data_new(OClink);

/* Reclaim a no longer needed OCdata object */
extern OCerror oc_data_free(OClink, OCdata);

/* Given an OCdata object, set the nested subnode OCdata object
   to refer to the data associated with the i'th element of
   the parent data object. NOTE: if the i'th element is not
   present then this function will return OC_ENODATA.
   See the user's manual for details on mapping multi-
   dimensional arrays to single indices.
*/
extern OCerror oc_data_ith(OClink,
				OCdata parentdata,
				size_t index,
				OCdata subdata);

/* Return the actual data values associated with the specified OCdata.
   The OCdata is assumed to be referencing either a scalar
   primitive value or a (1d) array of primitive values.
   If scalar, then index must be 0 and count must be 1.
*/
extern OCerror oc_data_get(OClink,OCdata,
                         void* memory, size_t memsize,
                         size_t index, size_t count);

/* Return the OCdata's current index */
extern OCerror oc_data_index(OClink,OCdata, size_t*);

/* Return the mode associated the specified OCdata object */
extern OCerror oc_data_mode(OClink,OCdata, OCmode*);

/* Return the OCobject associated the specified OCdata object */
extern OCerror oc_data_object(OClink,OCdata, OCobject*);

/* Compute the the count associated with the specified OCdata
   instance. Note: this is potentially computationally costly
   when computing # records.
*/
extern OCerror oc_data_count(OClink, OCdata, size_t*);

/**************************************************/
/* Misc. OCtype-related functions */

/* Return size of the given type (Primitive only) */
extern size_t oc_typesize(OCtype);

/* Return a canonical printable string describing a given type:
   e.g. Byte, Int16, etc.
*/
extern char* oc_typetostring(OCtype);

/* Given a value of a primitive OC type, provide a canonical
   string representing that value
*/
extern OCerror oc_typeprint(OCtype, char* buf, size_t bufsize, void* value);

/**************************************************/
/* Logging */

extern void oc_loginit(void);
extern void oc_setlogging(int onoff); /* 1=>start logging 0=>stop */
extern void oc_logopen(const char* logfilename);
extern void oc_logclose(void);

extern void oc_logtext(int tag, const char* text);

extern void oc_log(int tag, const char* fmt, ...);

/**************************************************/
/* Miscellaneous */

/* Convert an OCerror to a human readable string */
extern char* oc_errstring(int err);

/* Get client parameters from the URL
   DO NOT free the result
*/
extern const char* oc_clientparam_get(OClink, const char* param);

extern OCerror oc_clientparam_delete(OClink, const char* param);
extern OCerror oc_clientparam_insert(OClink, const char* param, const char* value);
extern OCerror oc_clientparam_replace(OClink, const char* param, const char* value);

/**************************************************/
/* Merging operations */

/* Merge a specified DAS into a specified DDS or DATADDS */
extern OCerror oc_attach_das(OClink, OCobject dasroot, OCobject ddsroot);

/**************************************************/
/* Debugging */
extern OCerror oc_dd(OClink,OCobject);

/* When a server error is detected, then it is possible
   to get the server error info using this procedure */
extern OCerror oc_svcerrordata(OClink link, char** codep,
                               char** msgp, long* httpp);



/**************************************************/
/* Experimental */

extern OCerror oc_compile(OClink,OCobject);

/* New 10/31/2009: return raw information about a datadds
*/

extern OCerror oc_raw_xdrsize(OClink, OCobject, size_t*);

/* Resend a url as a head request to check the Last-Modified time */
extern OCerror oc_update_lastmodified_data(OClink);

/* Get last known modification time; -1 => data unknown */
extern long oc_get_lastmodified_data(OClink);

/**************************************************/

#endif /*OCINTERNAL_H*/

#ifdef __cplusplus
}
#endif

#endif /*OC_H*/
