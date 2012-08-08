#ifndef NCDAP4_H
#define NCDAP4_H

#include "config.h"

#undef READCHECK
#undef ALIGNCHECK

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#include "ncbytes.h"
#include "nclist.h"
#include "nchashmap.h"
#include "nclog.h"
#include "dceconstraints.h"

#include "netcdf.h"
#include "ncdispatch.h"
#include "nc4internal.h"
#include "nc.h"

#include "oc.h"
#include "ocuri.h"

#include "nccommon.h"
#include "ncdap3.h"

#include "dapdebug.h"
#include "daputil.h"


/**************************************************/
/* The NCDAP4 structure is subtype of NC_INFO_TYPE_T (libsrc4) */
/* It also contains some of the NCDAP3 info */

/* Warning: fields from BEGIN DAP COMMON to END DAP COMMON must be same for:
	1. NCDAP3 (libncdap3/ncdap3.h)
	2. NCDAP4 (libncdap4/ncdap4.h)
*/

typedef struct NCDAP4 {
    NC_FILE_INFO_T info;
    NCDAPCOMMON    dap;
} NCDAP4;

/**************************************************/

#include "getvara.h"
#include "constraints3.h"
#include "constraints4.h"

/**************************************************/

extern ptrdiff_t dapsinglestride4[NC_MAX_VAR_DIMS];

extern int lnc4_redef(int ncid);
extern int lnc4_enddef(int ncid);
extern int lnc4_sync(int ncid);
extern int lnc4_abort(int ncid);
extern int lnc4_close(int ncid);

extern int lnc4_open_file(const char *path, int mode, int basepe,
                          size_t *chunksizehintp, int use_parallel,
                          MPI_Comm comm, MPI_Info info, int *ncidp);

extern int l4nc4_get_vara(NC_FILE_INFO_T *nc, int ncid, int varid, const size_t *startp, const size_t *countp, nc_type mem_nc_type, int is_long, void *data);

extern int l4nc4_put_vara(NC_FILE_INFO_T *nc, int ncid, int varid, const size_t *startp, const size_t *countp, nc_type mem_nc_type, int is_long, void *data);

extern int nc4_nc4f_list_add(NC_FILE_INFO_T *nc, const char *path, int mode);
extern void nc4_file_list_del(NC_FILE_INFO_T *nc);
extern int nc4_nc4f_list_add(NC_FILE_INFO_T *nc, const char *path, int mode);
extern int close_netcdf4_file(NC_HDF5_FILE_INFO_T *h5, int abort);

extern short drno_delta_file_id(short);
extern int drno_delta_numfiles(int);

/**********************************************************/
extern int ncceparse(char*, int, DCEconstraint*, char**);

extern NCerror computecdfnodesets4(NCDAPCOMMON*);
extern NCerror fixgrids4(NCDAPCOMMON*);
extern NCerror computecdfdimnames4(NCDAPCOMMON*);
extern NCerror computetypenames4(NCDAPCOMMON*, CDFnode* tnode);
extern NCerror computeusertypes4(NCDAPCOMMON*);
extern int singletonsequence(CDFnode* node);
extern CDFnode* getsingletonfield(NClist* list);
extern void setvarbasetype(NCDAPCOMMON*, CDFnode* field);
extern NCerror shortentypenames4(NCDAPCOMMON*);

#endif /*NCDAP4_H*/
