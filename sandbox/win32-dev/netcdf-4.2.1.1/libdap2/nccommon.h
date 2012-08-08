/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libnccommon/nccommon.h,v 1.40 2010/05/30 19:45:52 dmh Exp $
  *********************************************************************/
#ifndef NCCOMMON_H
#define NCCOMMON_H 1

/* Mnemonics */
#ifndef BOOL
#define BOOL int
#endif
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef nullfree
#define nullfree(m) {if((m)!=NULL) {free(m);} else {}}
#endif


#define FILLCONSTRAINT TRUE


/* Use an extended version of the netCDF-4 type system */
#define NC_URL		50
#define NC_SET		51
/* Merge relevant translations of OC types */
#define NC_Dataset	52
#define NC_Sequence	53
#define NC_Structure	54
#define NC_Grid		55
#define NC_Dimension	56
#define NC_Atomic	57

#undef OCCOMPILEBYDEFAULT

#define DEFAULTSTRINGLENGTH 64
/* The sequence limit default is zero because
   most servers do not implement projections
   on sequences.
*/
#define DEFAULTSEQLIMIT 0

/**************************************************/
/* sigh, do the forwards */
struct NCDAPCOMMON;
struct NCprojection;
struct NCselection;
struct Getvara;
struct NCcachenode;
struct NCcache;
struct NCslice;
struct NCsegment;
struct OClist;
/**************************************************/
/*
Collect single bit flags that
affect the operation of the system.
*/

typedef unsigned int NCFLAGS;
#  define SETFLAG(controls,flag) ((controls.flags) |= (flag))
#  define CLRFLAG(controls,flag) ((controls.flags) &= ~(flag))
#  define FLAGSET(controls,flag) (((controls.flags) & (flag)) != 0)

/* Defined flags */
#define NCF_NC3             (0x0001) /* DAP->netcdf-3 */
#define NCF_NC4             (0x0002) /* DAP->netcdf-4 */
#define NCF_NCDAP           (0x0004) /* Do libnc-dap mimic */
#define NCF_CACHE           (0x0008) /* Cache enabled/disabled */
#define NCF_PREFETCH        (0x0010) /* Cache prefetch enabled/disabled */
#define NCF_UPGRADE         (0x0020) /* Do proper type upgrades */
#define NCF_UNCONSTRAINABLE (0x0040) /* Not a constrainable URL */
#define NCF_SHOWFETCH       (0x0080) /* show fetch calls */
#define NCF_ONDISK          (0x0100) /* cause oc to store data on disk */
#define NCF_WHOLEVAR        (0x0200) /* retrieve only whole variables (as opposed to partial variable)
                                        into cache */

/* Define all the default on flags */
#define DFALT_ON_FLAGS (NCF_PREFETCH)

typedef struct NCCONTROLS {
    NCFLAGS  flags;
} NCCONTROLS;

struct NCTMODEL {
    int translation;
    char* model;
    unsigned int flags;
};

/* Detail information about each cache item */
typedef struct NCcachenode {
    int wholevariable; /* does this cache node only have wholevariables? */
    int prefetch; /* is this the prefetch cache entry? */
    off_t xdrsize;
    DCEconstraint* constraint; /* as used to create this node */
    NClist* vars; /* vars potentially covered by this cache node */
    struct CDFnode* datadds;
    OCddsnode ocroot;
    OCdatanode content;
} NCcachenode;

/* All cache info */
typedef struct NCcache {
    size_t cachelimit; /* max total size for all cached entries */
    size_t cachesize; /* current size */
    size_t cachecount; /* max # nodes in cache */
    NCcachenode* prefetch;
    NClist* nodes; /* cache nodes other than prefetch */
} NCcache;

/**************************************************/
/* The DAP packet info from OC */
typedef struct NCOC {
    OClink conn;
    char* rawurltext; /* as given to nc3d_open */
    char* urltext;    /* as modified by nc3d_open */
    NC_URI* url;      /* parse of rawuritext */
    OCdasnode ocdasroot;
    DCEconstraint* dapconstraint; /* from url */
    int inmemory; /* store fetched data in memory? */
} NCOC;

typedef struct NCCDF {
    struct CDFnode* ddsroot; /* constrained dds */
    struct CDFnode* fullddsroot; /* unconstrained dds */
    /* Collected sets of useful nodes (in ddsroot tree space) */
    NClist*  varnodes; /* nodes which can represent netcdf variables */
    NClist*  seqnodes; /* sequence nodes; */
    NClist*  gridnodes; /* grid nodes */
    NClist*  dimnodes; /* (base) dimension nodes */
    NClist*  projectedvars; /* vars appearing in nc_open url projections */
    unsigned int defaultstringlength;
    unsigned int defaultsequencelimit; /* global sequence limit;0=>no limit */
    struct NCcache* cache;
    size_t fetchlimit;
    size_t smallsizelimit; /* what constitutes a small object? */
    size_t totalestimatedsize;
    const char* separator; /* constant; do not free */
    /* global string dimension */
    struct CDFnode* globalstringdim;
    char* recorddimname; /* From DODS_EXTRA */
    struct CDFnode* recorddim;
    /* libncdap4 only */
    NClist*  usertypes; /* nodes which will represent netcdf types */
} NCCDF;

/* Define a structure holding common info for NCDAP{3,4} */

typedef struct NCDAPCOMMON {
    NC*   controller; /* Parent instance of NCDAPCOMMON */
    NCCDF cdf;
    NCOC oc;
    NCCONTROLS controls; /* Control flags and parameters */
} NCDAPCOMMON;

/**************************************************/
/* Create our own node tree to mimic ocnode trees*/

/* Each root CDFnode contains info about the whole tree */
typedef struct CDFtree {
    OCddsnode ocroot;
    OCdxd occlass;
    NClist* nodes; /* all nodes in tree*/
    struct CDFnode* root; /* cross link */
    struct NCDAPCOMMON*          owner;
    /* Classification flags */
    int regridded; /* Was this tree passed thru regrid3? */
} CDFtree;

/* Track the kinds of dimensions */
typedef int CDFdimflags;
#define CDFDIMNORMAL	0x0
#define CDFDIMSEQ	0x1
#define CDFDIMSTRING	0x2
#define CDFDIMCLONE	0x4
#define CDFDIMRECORD	0x20

#define DIMFLAG(d,flag) ((d)->dim.dimflags & (flag))
#define DIMFLAGSET(d,flag) ((d)->dim.dimflags |= (flag))
#define DIMFLAGCLR(d,flag) ((d)->dim.dimflags &= ~(flag))

typedef struct CDFdim {
    CDFdimflags    dimflags;
    struct CDFnode* basedim; /* for duplicate dimensions*/
    struct CDFnode* array; /* parent array node */
    size_t declsize;	    /* from constrained DDS*/
    size_t declsize0;	    /* from unconstrained DDS*/
    int    index1;          /* dimension name index +1; 0=>no index */
} CDFdim;

typedef struct CDFarray {
    NClist*  dimsetall; /* inherited+originals+pseudo */
    NClist*  dimsetplus; /* originals+pseudo */
    NClist*  dimset0; /* original dims from the dds */
    struct CDFnode* stringdim;
    /* Track sequence related information */
    struct CDFnode* seqdim; /* if this node is a sequence */
    /* note: unlike string dim; seqdim is also stored in dimensions vector */
    struct CDFnode* sequence; /* containing usable sequence, if any */
    struct CDFnode* basevar; /* for duplicate grid variables*/
} CDFarray;

typedef struct NCattribute {
    char*   name;
    nc_type etype; /* dap type of the attribute */
    NClist* values; /* strings come from the oc values */
    int     invisible; /* Do not materialize to the user */
} NCattribute;

/* Extend as additional DODS attribute values are defined */
typedef struct NCDODS {
    size_t maxstrlen;
    char* dimname;
} NCDODS;

typedef struct NCalignment {
    unsigned long    size; /* size of single instance of this type*/
    unsigned long    alignment; /* alignment of this field */
    unsigned long    offset;    /* offset of this field in parent */
} NCalignment;

typedef struct NCtypesize {
    BOOL             aligned; /*  have instance and field been defined? */
    NCalignment      instance; /* Alignment, etc for instance data */
    NCalignment      field; /* Alignment, etc WRT to parent */
} NCtypesize;

/* Closely mimics struct OCnode*/
typedef struct CDFnode {
    nc_type          nctype;     /* e.g. var, dimension  */
    nc_type          etype;      /* e.g. NC_INT, NC_FLOAT if applicable,*/
    char*            ocname;     /* oc base name */
    char*            ncbasename; /* generally cdflegalname(ocname) */
    char*            ncfullname; /* complete path name from root to this node*/
    OCddsnode        ocnode;     /* oc mirror node*/
    struct CDFnode*  group;	 /* null => in root group */
    struct CDFnode*  container;  /* e.g. struct or sequence, but not group */
    struct CDFnode*  root;
    CDFtree*         tree;          /* root level metadata;only defined if root*/
    CDFdim           dim;           /* nctype == dimension */
    CDFarray         array;         /* nctype == grid,var,etc. with dimensions */
    NClist*          subnodes;      /* if nctype == grid, sequence, etc. */
    NClist*          attributes;    /*NClist<NCattribute*>*/
    NCDODS           dodsspecial;   /* special attributes like maxStrlen */
    nc_type          externaltype;  /* the type as represented to nc_inq*/
    int              ncid;          /* relevant NC id for this object*/
    unsigned long    maxstringlength;
    unsigned long    sequencelimit; /* 0=>unlimited */
    BOOL	     usesequence;   /* If this sequence is usable */
    BOOL             elided;        /* 1 => node does not partipate in naming*/
    struct CDFnode*  basenode;      /* derived tree map to template tree */
    BOOL	     visible;       /* 1 => node is present in derived tree; independent of elided flag */
    BOOL	     zerodim;       /* 1 => node has a zero dimension */
    /* These two flags track the effects on grids of constraints */
    BOOL             virtual;       /* node added by regrid */
#ifdef PROJECTED
    BOOL             projected;     /* node referenced by projection */
#endif
    struct CDFnode* attachment;     /* DDS<->DATADDS cross link*/
    struct CDFnode* template;       /* temporary field for regridding */
    /* Fields for use by libncdap4 */
    NCtypesize       typesize;
    int              typeid;        /* when treating field as type */
    int              basetypeid;    /* when typeid is vlen */
    char*            typename;
    char*            vlenname;      /* for sequence types */
    int              singleton;     /* for singleton sequences */
    unsigned long    estimatedsize; /* > 0 Only for var nodes */
} CDFnode;

/**************************************************/
/* Shared procedures */

/* From ncdap3.c*/
extern NCerror freeNCDAPCOMMON(NCDAPCOMMON*);
extern NCerror fetchtemplatemetadata3(NCDAPCOMMON*);

/* From error.c*/
extern NCerror ocerrtoncerr(OCerror);

/* From: common34.c */
extern NCerror fixgrid34(NCDAPCOMMON* drno, CDFnode* grid);
extern NCerror computecdfinfo34(NCDAPCOMMON*, NClist*);
extern char* cdfname34(char* basename);
extern NCerror augmentddstree34(NCDAPCOMMON*, NClist*);
extern NCerror computecdfdimnames34(NCDAPCOMMON*);
extern NCerror buildcdftree34(NCDAPCOMMON*, OCddsnode, OCdxd, CDFnode**);
extern CDFnode* makecdfnode34(NCDAPCOMMON*, char* nm, OCtype,
			    /*optional*/ OCddsnode ocnode, CDFnode* container);
extern void freecdfroot34(CDFnode*);

extern NCerror findnodedds34(NCDAPCOMMON* drno, CDFnode* ddssrc);
extern NCerror makegetvar34(NCDAPCOMMON*, struct CDFnode*, void*, nc_type, struct Getvara**);
extern NCerror applyclientparams34(NCDAPCOMMON* drno);
extern NCerror attach34(CDFnode* xroot, CDFnode* ddstarget);
extern NCerror attachall34(CDFnode* xroot, CDFnode* ddsroot);
extern NCerror attachsubset34(CDFnode*, CDFnode*);
extern void unattach34(CDFnode*);
extern int nodematch34(CDFnode* node1, CDFnode* node2);
extern int simplenodematch34(CDFnode* node1, CDFnode* node2);
extern CDFnode* findxnode34(CDFnode* target, CDFnode* xroot);
extern int constrainable34(NC_URI*);
extern char* makeconstraintstring34(DCEconstraint*);
extern size_t estimatedataddssize34(CDFnode* datadds);
extern void canonicalprojection34(NClist*, NClist*);
extern NClist* getalldims34(NCDAPCOMMON* nccomm, int visibleonly);

/* From cdf3.c */
extern NCerror dimimprint3(NCDAPCOMMON*);
extern NCerror definedimsets3(NCDAPCOMMON*);

/* From cache.c */
extern int iscached(NCDAPCOMMON*, CDFnode* target, NCcachenode** cachenodep);
extern NCerror prefetchdata3(NCDAPCOMMON*);
extern NCerror buildcachenode34(NCDAPCOMMON*,
	        DCEconstraint* constraint,
		NClist* varlist,
		NCcachenode** cachep,
		int isprefetch);
extern NCcachenode* createnccachenode(void);
extern void freenccachenode(NCDAPCOMMON*, NCcachenode* node);
extern NCcache* createnccache(void);
extern void freenccache(NCDAPCOMMON*, NCcache* cache);

/* Add an extra function whose sole purpose is to allow
   configure(.ac) to test for the presence of thiscode.
*/
extern int nc__opendap(void);

#endif /*NCCOMMON_H*/
