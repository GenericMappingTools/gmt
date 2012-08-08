#ifndef NCCRMETA_H
#define NCCRMETA_H 1

/*Forward*/
struct Dimension;

enum Dimcase {DC_UNKNOWN, DC_FIXED, DC_UNLIMITED, DC_VLEN, DC_PRIVATE};

extern int nccr_buildnc(NCCR* nccr, Header* hdr);

extern enum Dimcase classifydim(struct Dimension* dim);
extern int dimsize(struct Dimension*);

#endif /*NCCRMETA_H*/
