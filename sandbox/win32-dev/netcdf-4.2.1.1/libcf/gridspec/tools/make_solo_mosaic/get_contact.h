#ifndef GET_CONTACT_
#define GET_CONTACT_
/**********************************************************************
                       get_contact.h
    This header file is used to compute aligned-contact between tiles
**********************************************************************/

int get_align_contact(int tile1, int tile2, int nx1, int ny1, int nx2, int ny2, 
                      const double *x1, const double *y1, const double *x2, 
                      const double *y2, double periodx, double periody,
		      int *istart1, int *iend1, int *jstart1, int *jend1, 
                      int *istart2, int *iend2, int *jstart2, int *jend2);
#endif
