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
#define NC_Primitive	57

#undef OCCOMPILEBYDEFAULT

#define DEFAULTSTRINGLENGTH 64
/* The sequence limit default is zero because
   most servers do not implement projections
   on sequences.
*/
#define DEFAULTSEQLIMIT 0

/**************************************************/
/* sigh, do the forwards */
struct NCDAP3;
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

/* Base translations */
#define NCF_NC3      (0x01)    /* DAP->netcdf-3 */
#define NCF_NC4      (0x02) /* DAP->netcdf-4 */

/* OR'd with the translation model */
#define NCF_NCDAP    (0x04) /* libnc-dap mimic */
#define NCF_COORD    (0x08) /* libnc-dap mimic + add coordinate variables */
#define NCF_VLEN     (0x10) /* map sequences to vlen+struct */

/*  Cache control flags */
#define NCF_CACHE    (0x20) /* Cache enabled/disabled */

/*  Misc control flags */
#define NCF_NOUNLIM         (0x40) /* suppress bad sequences  */
#define NCF_UPGRADE         (0x80) /* Do proper type upgrades */
#define NCF_UNCONSTRAINABLE (0x100) /* Not a constrainable URL */
#define NCF_SHOWFETCH       (0x200) /* show fetch calls */

/* Currently, defalt is on */
#define DFALTCACHEFLAG (0)

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
    int wholevariable; /* Is this cache entry constrained? */
    int prefetch; /* is this the prefetch cache entry? */
    size_t xdrsize;
    DCEconstraint* constraint; /* as used to create this node */
    NClist* vars; /* vars potentially covered by this cache node */
    struct CDFnode* datadds;
    OCobject ocroot;
    OCdata content;
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
    OCconnection conn;
    char* urltext; /* as given to nc3d_open*/
    OCURI* uri; /* as given to nc3d_open and parsed*/
    OCobject ocdasroot;
    DCEconstraint* dapconstraint; /* from url */
} NCOC;

typedef struct NCCDF {
    struct CDFnode* ddsroot; /* unconstrained dds */
    /* Collected sets of useful nodes (in unconstrainted tree space) */
    NClist*  varnodes; /* nodes which can represent netcdf variables */
    NClist*  seqnodes; /* sequence nodes; */
    NClist*  gridnodes; /* grid nodes */
    unsigned int defaultstringlength;
    unsigned int defaultsequencelimit; /* global sequence limit;0=>no limit */
    struct NCcache* cache;
    size_t fetchlimit;
    size_t smallsizelimit; /* what constitutes a small object? */
    size_t totalestimatedsize;
    const char* separator; /* constant; do not free */
    /* unlimited dimension */
    struct CDFnode* unlimited;
    char* recorddim; /* From DODS_EXTRA */
    /* libncdap4 only */
    NClist*  usertypes; /* nodes which will represent netcdf types */
} NCCDF;

/* Define a structure holding common info for NCDAP{3,4} */

typedef struct NCDAPCOMMON {
    NC*   controller; /* Parent instance of NCDAP3 or NCDAP4 */
    NCCDF cdf;
    NCOC  oc;
    NCCONTROLS controls; /* Control flags and parameters */
} NCDAPCOMMON;

/**************************************************/
/* Create our own node tree to mimic ocnode trees*/

/* Each root CDFnode contains info about the whole tree */
typedef struct CDFtree {
    OCobject ocroot;
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
#define CDFDIMUNLIM	0x8
#define CDFDIMANON	0x10
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
} CDFdim;

typedef struct CDFarray {
    NClist*  dimensions; /* inherited+originals */
    NClist*  dimensions0; /* Complete set of dimensions for this var */
    struct CDFnode* stringdim;
    /* Track sequence containment information */
    struct CDFnode* seqdim;
    /* note: unlike string dim; seqdim is also stored in dimensions vector */
    struct CDFnode* sequence; /* containing usable sequence, if any */
    struct CDFnode* basevar; /* for duplicate grid variables*/
} CDFarray;

typedef struct NCattribute {
    char*   name;
    nc_type etype; /* dap type of the attribute */
    NClist*   values; /* strings come from the oc values */
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
    nc_type          nctype; /* redundant but convenient*/
    nc_type          etype;  /* redundant but convenient*/
    char*            name;   /* oc base name; redundant but convenient*/
    OCobject         dds;    /* mirror node*/
    struct CDFnode*  container;
    struct CDFnode*  root;
    CDFtree*         tree; /* pointer so we can pass it around */
    CDFdim           dim;
    CDFarray         array;
    NClist*          subnodes; /*NClist<OCobject>*/
    NClist*          attributes; /*NClist<NCattribute*>*/
    NCDODS           dodsspecial; /*these are the special attributes like
                                       maxStrlen */
    char*            ncfullname;     /* with parent name prefixing*/
    char*            ncbasename;     /* without parent name prefixing, but legitimate */
    nc_type          externaltype;   /* the type as represented to nc_inq*/
    int              ncid;           /* relevant NC id for this object*/
    unsigned long    maxstringlength;
    unsigned long    sequencelimit; /* 0=>unlimited */
    BOOL	     usesequence; /* If this sequence is usable */
    BOOL             elided;  /* 1 => node does not partipate in naming*/
    BOOL	     visible; /* 1 => node is present in constrained tree;
                                 independent of elided flag */
    BOOL	     zerodim; /* 1 => node has a zero dimension */
    /* These two flags track the effects on grids of constraints */
    BOOL             virtual; /* Is this node added ? */
    BOOL             projected; /* Is this a node referenced by projection */
    struct CDFnode* attachment;  /* DDS<->DATADDS cross link*/
    struct CDFnode* template; /* temporary field for regridding */
    /* Fields for use by libncdap4 */
    NCtypesize       typesize;
    int              typeid;    /* when treating field as type */
    int              basetypeid;   /* when typeid is vlen */
    char*            typename;
    char*            vlenname; /* for sequence types */
    int              singleton; /* for singleton sequences */
    unsigned long    estimatedsize; /* > 0 Only for var nodes */
} CDFnode;

/**************************************************/
/* Give PSEUDOFILE a value */
#define PSEUDOFILE "/tmp/pseudofileXXXXXX"

/**************************************************/
/* Shared procedures */

/* From ncdap3.c*/
extern NCerror cleanNCDAP3(struct NCDAP3* drno);
extern NCerror cleanNCDAPCOMMON(struct NCDAPCOMMON*);
extern NCerror fetchtemplatemetadata3(NCDAPCOMMON* drno);

/* From error.c*/
extern NCerror ocerrtoncerr(OCerror);

/* From: common34.c */
extern NCerror fixgrid34(struct NCDAPCOMMON* drno, CDFnode* grid);
extern NCerror computecdfinfo34(struct NCDAPCOMMON*, NClist*);
extern char* cdfname34(char* basename);
extern NCerror augmentddstree34(struct NCDAPCOMMON*, NClist*);
extern NCerror clonecdfdims34(struct NCDAPCOMMON*);
extern NCerror computecdfdimnames34(struct NCDAPCOMMON*);
extern NCerror buildcdftree34(struct NCDAPCOMMON*, OCobject, OCdxd, CDFnode**);
extern CDFnode* makecdfnode34(struct NCDAPCOMMON*, char* nm, OCtype,
			    /*optional*/ OCobject ocnode, CDFnode* container);
extern void freecdfroot34(CDFnode*);

extern NCerror findnodedds34(struct NCDAPCOMMON* drno, CDFnode* ddssrc);
extern NCerror makegetvar34(struct NCDAPCOMMON*, struct CDFnode*, void*, nc_type, struct Getvara**);
extern NCerror applyclientparams34(struct NCDAPCOMMON* drno);
extern NCerror attach34(CDFnode* xroot, CDFnode* ddstarget);
extern NCerror attachall34(CDFnode* xroot, CDFnode* ddsroot);
extern NCerror attachsubset34(CDFnode*, CDFnode*);
extern void unattach34(CDFnode*);
extern int nodematch34(CDFnode* node1, CDFnode* node2);
extern int simplenodematch34(CDFnode* node1, CDFnode* node2);
extern CDFnode* findxnode34(CDFnode* target, CDFnode* xroot);
extern int constrainable34(OCURI*);
extern char* makeconstraintstring34(DCEconstraint*);
extern size_t estimatedataddssize34(CDFnode* datadds);
extern void restrictprojection34(NClist*, NClist*);

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
