 /*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *   $Header: /upc/share/CVS/netcdf-3/libncdap3/constraints3.h,v 1.10 2010/04/13 03:36:30 dmh Exp $
  *********************************************************************/
#ifndef CONSTRAINTS3_H
#define CONSTRAINTS3_H 1

extern NCerror parsedapconstraints(NCDAPCOMMON*, char*, DCEconstraint*);
extern NCerror mapconstraints3(NCDAPCOMMON*);

extern char* simplepathstring(NClist* segments, char* separator);
extern void makesegmentstring3(NClist* segments, NCbytes* buf, char* separator);

extern NCerror mergeprojections3(NClist*, NClist*);
extern int iswholeprojection(DCEprojection*);

extern void freegetvara(struct Getvara* vara);

extern NCerror slicemerge3(DCEslice* dst, DCEslice* src);

extern int iswholeslice(DCEslice*, struct CDFnode* dim);
extern int iswholesegment(DCEsegment*);

extern int iswholeconstraint(DCEconstraint* con);

extern char* buildprojectionstring3(NClist* projections);
extern char* buildselectionstring3(NClist* selections);
extern char* buildconstraintstring3(DCEconstraint* constraints);

extern void makewholesegment3(DCEsegment*,struct CDFnode*);
extern void makewholeslice3(DCEslice* slice, struct CDFnode* dim);

#endif /*CONSTRAINTS3_H*/
