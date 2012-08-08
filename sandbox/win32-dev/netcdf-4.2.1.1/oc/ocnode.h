/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCNODE_H
#define OCNODE_H

/*! Specifies the Diminfo. */
/* Track info purely about declared dimensions.
   More information is included in the Dimdata structure (dim.h)
*/
typedef struct OCdiminfo {
    struct OCnode* array;   /* defining array node (if known)*/
    unsigned int arrayindex;/* rank position ofthis dimension in the array*/
    size_t declsize;	    /* from DDS*/
} OCdiminfo;

/*! Specifies the Arrayinfo.*/
typedef struct OCarrayinfo {
    /* The complete set of dimension info applicable to this node*/
    OClist*  dimensions;
    /* convenience (because they are computed so often*/
    unsigned int rank; /* == |dimensions|*/
} OCarrayinfo;

/*! Specifies the Attribute.*/
typedef struct OCattribute {
    char*   name;
    OCtype etype; /* type of the attribute */
    size_t  nvalues;
    char**  values;  /* |values| = nvalues*sizeof(char**)*/
} OCattribute;

/*! Specifies the Attinfo.*/
/* This is the form as it comes out of the DAS parser*/
typedef struct OCattinfo {
    int isglobal;   /* is this supposed to be a global attribute set?*/
    OClist* values; /* oclist<char*>*/
} OCattinfo;

/*! Specified computed information */
typedef struct OCdapinfo {
    /* Size of one element: size == 0 => not uniform*/
    size_t instancesize; /* includes array overhead of subnodes,
                            but not of this node*/
    size_t arraysize; /* instancesize + array overhead*/
} OCdapinfo;

/*! Specifies the OCnode. */
typedef struct OCnode {
    unsigned int    magic;
    OCtype	    octype;
    OCtype          etype; /* essentially the dap type from the dds*/
    char*           name;
    char*           fullname;
    struct OCnode*  container; /* this node is subnode of container */
    struct OCnode*  root;      /* root node of tree containing this node */
    struct OCtree*  tree;      /* !NULL iff this is a root node */
    struct OCnode*  datadds;   /* correlated datadds node, if any */
    OCdiminfo       dim;       /* octype == OC_Dimension*/
    OCarrayinfo     array;     /* octype == {OC_Structure, OC_Primitive}*/
    OCattinfo       att;       /* octype == OC_Attribute */
    OCdapinfo       dap;
    /* primary edge info*/
    OClist* subnodes; /*oclist<OCnode*>*/
    /*int     attributed;*/ /* 1 if merge was done*/
    OClist* attributes; /* oclist<OCattribute*>*/
#ifdef OC_DAP4
    OCtypeinfo      typdef;
    OCgroupinfo     group;
#endif
} OCnode;

#endif /*OCNODE_H*/
