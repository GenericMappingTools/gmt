#ifndef NC_NCGEN_H
#define NC_NCGEN_H
/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/ncgen.h,v 1.18 2010/06/01 15:34:53 ed Exp $
 *********************************************************************/


#ifdef USE_NETCDF4
#define CLASSICONLY 0
#else
#define CLASSICONLY 1
#endif

#define MAX_NC_ATTSIZE    20000	/* max size of attribute (for ncgen) */
#define MAXTRST		  5000	/* max size of string value (for ncgen) */

/* Name of the root group,*/
/* although prefix construction*/
/* elides it.*/
#define ROOTGROUPNAME "root"

/* Define the possible classes of objects*/
/* extend the NC_XXX values*/
#define NC_GRP      100
#define NC_DIM      101
#define NC_VAR      102
#define NC_ATT      103
#define NC_TYPE     104
#define NC_ECONST   105
#define NC_FIELD    106
#define NC_ARRAY    107
#define NC_PRIM     108 /*Including NC_STRING */
#define NC_STRUCT  NC_COMPOUND /* alias */

/* Extend nc types with generic fill value*/
#define NC_FILLVALUE    31

/* Must be a better way to do this */
#ifndef INFINITE
#define NANF (0.0f/0.0f)
#define NAN (0.0/0.0)
#define INFINITEF (1.0f/0.0f)
#define NEGINFINITEF (-INFINITEF)
#define INFINITE (1.0/0.0)
#define NEGINFINITE (-INFINITEF)
#endif

/* nc_class is one of:
        NC_GRP NC_DIM NC_VAR NC_ATT NC_TYPE
*/
typedef nc_type nc_class;

/* nc_subclass is one of:
	NC_PRIM NC_OPAQUE NC_ENUM
	NC_FIELD NC_VLEN NC_COMPOUND
	NC_ECONST NC_ARRAY NC_FILLVALUE
*/
typedef nc_type nc_subclass;

/*
Define data structure
to hold special attribute values
for a given variable.
Global values are kept as
various C global variables
*/

/* Define a bit set for indicating which*/
/* specials were explicitly specified*/
#define _STORAGE_FLAG       0x001
#define _CHUNKSIZES_FLAG    0x002
#define _FLETCHER32_FLAG    0x004
#define _DEFLATE_FLAG       0x008
#define _SHUFFLE_FLAG       0x010
#define _ENDIAN_FLAG        0x020
#define _NOFILL_FLAG        0x040
#define _FILLVALUE_FLAG     0x080
#define _FORMAT_FLAG        0x100

struct Kvalues {
char* name;
int mode;
};

#define NKVALUES 16
extern struct Kvalues legalkinds[NKVALUES];

/* Note: non-variable specials (e.g. _Format) are not included in this struct*/
typedef struct Specialdata {
    int flags;
    Datalist*     _Fillvalue;
    int           _Storage;      /* NC_CHUNKED | NC_CONTIGUOUS*/
    size_t*       _ChunkSizes;     /* NULL => defaults*/
        int nchunks;     /*  |_Chunksize| ; 0 => not specified*/
    int           _Fletcher32;     /* 1=>fletcher32*/
    int           _DeflateLevel; /* 0-9 => level*/
    int           _Shuffle;      /* 0 => false, 1 => true*/
    int           _Endianness;   /* 1 =>little, 2 => big*/
    int           _Fill ;        /* 0 => false, 1 => true WATCHOUT: this is inverse of NOFILL*/
} Specialdata;

/* Track a set of dimensions*/
/* (Note: the netcdf type system is deficient here)*/
typedef struct Dimset {
    int		     ndims;
    struct Symbol*   dimsyms[NC_MAX_VAR_DIMS]; /* Symbol for dimension*/
} Dimset;

typedef struct Diminfo {
    int   isconstant; /* separate constant from named dimension*/
    size_t  unlimitedsize; /* if unlimited */
    size_t  declsize; /* 0 => unlimited/unspecified*/
} Diminfo;

typedef struct Attrinfo {
    struct Symbol*   var; /* NULL => global*/
} Attrinfo;

typedef struct Typeinfo {
        struct Symbol*  basetype;
	int             hasvlen;  /* 1 => this type contains a vlen*/
	nc_type         typecode;
        unsigned long   offset;   /* fields in struct*/
        unsigned long   alignment;/* fields in struct*/
        Constant        econst;   /* for enum values*/
        Dimset          dimset;     /* for NC_VAR/NC_FIELD/NC_ATT*/
        size_t   size;     /* for opaque, compound, etc.*/
        size_t   nelems;   /* size in terms of # of datalist constants
			      it takes to represent it */
} Typeinfo;

typedef struct Varinfo {
    int		nattributes; /* |attributes|*/
    List*       attributes;  /* List<Symbol*>*/
    Specialdata special;
} Varinfo;

typedef struct Groupinfo {
    int is_root;
    struct Symbol* unlimiteddim;
} Groupinfo;

typedef struct Symbol {  /* symbol table entry*/
        struct Symbol*  next;    /* Linked list of all defined symbols*/
        nc_class        objectclass;  /* NC_DIM|NC_VLEN|NC_OPAQUE...*/
        nc_class        subclass;  /* NC_STRUCT|...*/
        char*           name;
        struct Symbol*  ref;  /* ptr to the symbol if is_ref is true*/
        struct Symbol*  container;  /* The group containing this symbol.*/
				    /* for fields or enumids, it is*/
				    /* the parent type.*/
	struct Symbol*   location;   /* current group when symbol was created*/
	List*            subnodes;  /* sublist for enum or struct or group*/
	int              is_prefixed; /* prefix was specified (vs computed).*/
        List*            prefix;  /* List<Symbol*>*/
        struct Datalist* data; /* shared by variables and attributes*/
	/* Note that we cannot union these because some kinds of symbols*/
        /* use more than one part*/
        Typeinfo  typ; /* type info for e.g. var, att, etc.*/
        Varinfo   var;
        Attrinfo  att;        
        Diminfo   dim;
        Groupinfo grp;
	/* Misc pieces of info*/
	int             lineno;  /* at point of creation*/
	int		touched; /* for sorting*/
        int             is_ref;  /* separate name defs  from refs*/
	char*           lname; /* cached C or FORTRAN name*/
        int             ncid;  /* from netcdf API: varid, or dimid, or etc.*/
} Symbol;


#endif /*!NC_NCGEN_H*/
