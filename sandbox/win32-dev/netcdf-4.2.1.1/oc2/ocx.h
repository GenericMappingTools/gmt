/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT dap for more information. */

/*
Internal library debugging interface
(undocumented)
*/

#ifndef OCX_H
#define OCX_H

/**************************************************/
/* Flags defining the structure of an OCdata object */

/* Must be consistent with ocutil.c.ocdtmodestring */
typedef unsigned int OCDT;
#define OCDT_FIELD     ((OCDT)(1)) /* field of a container */
#define OCDT_ELEMENT   ((OCDT)(2)) /* element of a structure array */
#define OCDT_RECORD    ((OCDT)(4)) /* record of a sequence */
#define OCDT_ARRAY     ((OCDT)(8)) /* is structure array */
#define OCDT_SEQUENCE  ((OCDT)(16)) /* is sequence */
#define OCDT_ATOMIC    ((OCDT)(32)) /* is atomic leaf */

/* Return mode for this data */
extern OCDT oc_data_mode(OClink, OCdatanode);

extern OCerror oc_dds_dd(OClink, OCddsnode, int);
extern OCerror oc_dds_ddnode(OClink, OCddsnode);
extern OCerror oc_data_ddpath(OClink, OCdatanode, char**);
extern OCerror oc_data_ddtree(OClink, OCdatanode root);

#endif /*OCX_H*/

