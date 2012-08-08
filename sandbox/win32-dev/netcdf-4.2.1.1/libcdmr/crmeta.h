#ifndef CRMETA_H
#define CRMETA_H 1

enum Dimcase {DC_UNKNOWN, DC_FIXED, DC_UNLIMITED, DC_VLEN, DC_PRIVATE};

typedef struct CRshape {size_t rank; Dimension** dims;} CRshape;

extern int nccr_buildnc(NCCDMR*,Header*);

extern enum Dimcase classifydim(struct Dimension* dim);
extern int dimsize(struct Dimension*);

extern nc_type cvtstreamtonc(DataType);

extern int crextractshape(CRnode* src, CRshape* dst);

#endif /*CRMETA_H*/
