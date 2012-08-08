#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <netcdf.h>
#include "mpp.h"
#include "mpp_domain.h"
#include "mpp_io.h"

#define  MAXFILE 50
#define  MAXVAR  1024

typedef struct {
  int fldid;
  char name[128];
  nc_type type;
} VarType;

typedef struct {
  int  ncid;
  char name[512];
  int  action;  /* indicate the action, MPP_WRITE or MPP_READ */
  int  status;  /* indicate if the file is opened or closed */
  int  nvar;
  VarType *var;
} FileType;

FileType files[MAXFILE];
int      nfiles = 0;

/*********************************************************************
    void netcdf_error( int status )
    status is the returning value of netcdf call. this routine will
    handle the error when status is not NC_NOERR.
********************************************************************/
void netcdf_error(const char *msg, int status )
{
  char errmsg[512];

  sprintf( errmsg, "%s: %s", msg, nc_strerror(status) );
  mpp_error(errmsg);

}; /* netcdf_error */


/*************************************************************
 int mpp_open(char *filename, int action)
 open netcdf file to read or write. return the id for the file opened.
 Here the id is not the netcdf ncid of the file opened, it is the index
 in the mpp_io data files. If the file is already opened, will exit with
 an error message. For the write action, mpp_open can only be called once.
 For the read action, the file could be open and then close and then open
 again. The action should be MPP_READ, MPP_WRITE, a constant defined in
 mpp_io.h. When action is MPP_WRITE, file will be created on root pe.
************************************************************/

int mpp_open(const char *file, int action) {
  char curfile[128];
  char errmsg[512];  
  int ncid, status, n, fid;
  
  /* write only from root pe. */
  if(action == MPP_WRITE && mpp_pe() != mpp_root_pe() ) return -1;
  /*if file is not ended with .nc add .nc at the end. */
  strcpy(curfile, file);
  if(strstr(curfile, ".nc") == NULL) strcat(curfile,".nc");

  /* look through currently files to make sure the file is not in the list*/
  fid = -1;
  for(n=0; n<nfiles; n++) {
    if(!strcmp(files[n].name, file)) {
      fid = n;
      break;
    }
  }
  if(fid > -1) {
    if(files[n].action == MPP_WRITE) {
      sprintf( errmsg, "mpp_io(mpp_open): %s is already created for write", file);
      mpp_error(errmsg);
    }
    if(files[n].status) {
      sprintf( errmsg, "mpp_io(mpp_open): %s is already opened", file);
      mpp_error(errmsg);
    }
  }
  else {
    fid = nfiles;
    nfiles++;
    if(nfiles > MAXFILE) mpp_error("mpp_io(mpp_open): nfiles is larger than MAXFILE, increase MAXFILE");
    strcpy(files[fid].name, file);
    files[fid].nvar = 0;
    files[fid].var = (VarType *)malloc(MAXVAR*sizeof(VarType));
  }
    
  switch (action) {
  case MPP_WRITE:
#ifdef NC_64BIT_OFFSET
    status = nc_create(curfile, NC_64BIT_OFFSET, &ncid);
#else
    status = nc_create(curfile, NC_WRITE, &ncid);
#endif
    break;
  case MPP_READ:
    status = nc_open(curfile,NC_NOWRITE, &ncid);
    break;
  default:
    sprintf(errmsg, "mpp_io(mpp_open): the action should be MPP_WRITE or MPP_READ when opening file %s", file);
    mpp_error(errmsg);
  }
  
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_open): error in opening file %s", file);
    netcdf_error(errmsg, status);
  }

  files[fid].ncid   = ncid;
  files[fid].status = 1;
  files[fid].action = action;
  
  return fid;
}

/* close the file */
void mpp_close(int fid)
{
  int status;
  char errmsg[512];

  if(fid == -1 && mpp_pe() != mpp_root_pe() ) return;
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_close): invalid id number, id should be "
				    "a nonnegative integer that less than nfiles");
  
  status = nc_close(files[fid].ncid);
  if(status != NC_NOERR) {
    sprintf( errmsg, "mpp_io(mpp_close): error in closing files %s ", files[fid].name);
    netcdf_error(errmsg, status);
  }
  files[fid].ncid = 0;
  files[fid].status = 0;
  
}

/*******************************************************************************/
/*                                                                             */
/*           The following are routines that retrieve information              */
/*                                                                             */
/*******************************************************************************/

/*********************************************************************
  int mpp_get_varid(int fid, const char *varname)
  get the id of the varname from file with fid, the id will be the index
  in files[fid].var.
*********************************************************************/
int mpp_get_varid(int fid, const char *varname)
{
  int status, fldid, vid, n;
  char errmsg[512];
  
  /* First look through existing variables to see
     if the fldid of varname is already retrieved. */
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_varid): invalid id number, id should be "
				    "a nonnegative integer that less than nfiles");
  
  for(n=0; n<files[fid].nvar; n++) {
    if( !strcmp(files[fid].var[n].name, varname) ) return n;
  }

  vid = files[fid].nvar;
  files[fid].nvar++;
  if(files[fid].nvar > MAXVAR ) mpp_error("mpp_io(mpp_get_varid): nvar is larger than MAXVAR, increase MAXVAR");
  
  status =  nc_inq_varid(files[fid].ncid, varname, &fldid);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_varid): error in get field_id of variable %s from file %s", varname, files[fid].name);
    netcdf_error(errmsg, status);
  }

  status = nc_inq_vartype(files[fid].ncid, fldid, &(files[fid].var[vid].type));
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_varid): Error in getting type of of field %s in file %s ",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
  files[fid].var[vid].fldid = fldid;
  strcpy(files[fid].var[vid].name, varname);
  return vid;

};/* mpp_get_varid */

/********************************************************************
  int mpp_get_dimlen(char* file, char *name)
  Get the dimension.
 *******************************************************************/
int mpp_get_dimlen(int fid, const char *name)
{
  int ncid, dimid, status, len;
  size_t size;
  char errmsg[512];
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_dimlen): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  ncid = files[fid].ncid;
  status = nc_inq_dimid(ncid, name, &dimid);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_dimlen): error in inquiring dimid of %s from file %s", name, files[fid].name);
    netcdf_error(errmsg, status);
  }
  status = nc_inq_dimlen(ncid, dimid, &size);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_dimlen): error in inquiring dimlen of %s from file %s", name, files[fid].name);
    netcdf_error(errmsg, status);
  }
  len = size;
  return len;
  
}; /* mpp_get_dimlen */

/*********************************************************************
  void mpp_get_var_value(int fid, int vid, void *data)
  read part of var data, the part is defined by start and nread.
*********************************************************************/
void mpp_get_var_value(int fid, int vid, void *data)
{
  int status;
  char errmsg[512];
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_value_block): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_value_block): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");

  switch(files[fid].var[vid].type) {
  case NC_DOUBLE: case NC_FLOAT:
    status = nc_get_var_double(files[fid].ncid, files[fid].var[vid].fldid, data);
    break;
  case NC_INT:
    status = nc_get_var_int(files[fid].ncid, files[fid].var[vid].fldid, data);
    break;     
  case NC_CHAR:
    status = nc_get_var_text(files[fid].ncid, files[fid].var[vid].fldid, data);
    break; 
  default:
    sprintf(errmsg, "mpp_io(mpp_get_var_value): field %s in file %s has an invalid type, "
	    "the type should be NC_DOUBLE, NC_FLOAT", files[fid].var[vid].name, files[fid].name );
    mpp_error(errmsg);
  }    
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var_value_block): Error in getting value of variable %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
}; /* mpp_get_var_value */

/*********************************************************************
  void mpp_get_var_value_block(int fid, int vid, const size_t *start, const size_t *nread, void *data)
  read part of var data, the part is defined by start and nread.
*********************************************************************/
void mpp_get_var_value_block(int fid, int vid, const size_t *start, const size_t *nread, void *data)
{
  int status;
  char errmsg[512];
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_value_block): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_value_block): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");

  switch(files[fid].var[vid].type) {
  case NC_DOUBLE: case NC_FLOAT:
    status = nc_get_vara_double(files[fid].ncid, files[fid].var[vid].fldid, start, nread, data);
    break;
  case NC_INT:
    status = nc_get_vara_int(files[fid].ncid, files[fid].var[vid].fldid, start, nread, data);
    break;     
  case NC_CHAR:
    status = nc_get_vara_text(files[fid].ncid, files[fid].var[vid].fldid, start, nread, data);
    break; 
  default:
    sprintf(errmsg, "mpp_io(mpp_get_var_value): field %s in file %s has an invalid type, "
	    "the type should be NC_DOUBLE, NC_FLOAT", files[fid].var[vid].name, files[fid].name );
    mpp_error(errmsg);
  }    
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var_value_block): Error in getting value of variable %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
}; /* mpp_get_var_value_block */

/*******************************************************************
 void mpp_get_var_att(int fid, int vid, const char *name, void *val)
 get the attribute value of vid from file fid.
 ******************************************************************/
void mpp_get_var_att(int fid, int vid, const char *name, void *val)
{
  int status;
  char errmsg[512];
  nc_type type;
  
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_att): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_att): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");

  status = nc_inq_atttype(files[fid].ncid, files[fid].var[vid].fldid, name, &type);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var_att): Error in getting type of attribute %s of field %s in file %s ",
	    name, files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
  switch(type) {
  case NC_DOUBLE: case NC_FLOAT:
    status = nc_get_att_double(files[fid].ncid, files[fid].var[vid].fldid, name, val);
    break;
  default:
    sprintf(errmsg, "mpp_io(mpp_get_var_att): attribute %s of field %s in file %s has an invalid type, "
	    "the type should be NC_DOUBLE, NC_FLOAT", name, files[fid].var[vid].name, files[fid].name );
    mpp_error(errmsg);
  }
  
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var_att): Error in getting value of attribute %s of variable %s from file %s",
	    name, files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
}

/*******************************************************************
 void mpp_get_global_att(int fid, const char *name, void *val)
 get the global attribute from file fid.
 ******************************************************************/
void mpp_get_global_att(int fid, const char *name, void *val)
{
  int status;
  char errmsg[512];
  nc_type type;
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_global_att): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  status = nc_inq_atttype(files[fid].ncid, NC_GLOBAL, name, &type);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_global_att): Error in getting type of global attribute %s in file %s ",
	    name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
  switch(type) {
  case NC_DOUBLE: case NC_FLOAT:
    status = nc_get_att_double(files[fid].ncid, NC_GLOBAL, name, val);
    break;
  case NC_CHAR:
    status = nc_get_att_text(files[fid].ncid, NC_GLOBAL, name, val);
    break;  
  default:
    sprintf(errmsg, "mpp_io(mpp_get_global_att): global attribute %s in file %s has an invalid type, "
	    "the type should be NC_DOUBLE, NC_FLOAT", name, files[fid].name );
    mpp_error(errmsg);
  }
  
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_global_att): Error in getting value of global attribute %s from file %s",
	    name, files[fid].name );
    netcdf_error(errmsg, status);
  }

}

/********************************************************************
  int mpp_get_var_ndim(int fid, int vid)
********************************************************************/
int mpp_get_var_ndim(int fid, int vid)
{
  int status, ndim;
  char errmsg[512];
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_ndim): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_ndim): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");
  
  status = nc_inq_varndims(files[fid].ncid, files[fid].var[vid].fldid, &ndim);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var_ndim): Error in getting ndims of var %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
  return ndim;
}

/********************************************************************
  nc_type mpp_get_var_type(int fid, int vid)
  get var type
********************************************************************/
nc_type mpp_get_var_type(int fid, int vid)
{
  char errmsg[512];
  
  nc_type vartype;
  int status;

  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_ndim): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_ndim): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");
  
  status = nc_inq_vartype(files[fid].ncid, files[fid].var[vid].fldid, &vartype);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var_type): Error in getting type of var %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }

  return vartype;
}


/*********************************************************************
void mpp_get_var_dimname(int fid, int vid, int i, char *name)
For each dimension we are assuming there is a 1-d field have the same name as the dimension.
*********************************************************************/
void mpp_get_var_dimname(int fid, int vid, int ind, char *name)
{
  int status, ncid, fldid, ndims, dims[4];
  char errmsg[512];
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_dimname): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_dimname): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");
  ncid = files[fid].ncid;
  fldid = files[fid].var[vid].fldid;
  
  status = nc_inq_varndims(ncid, fldid, &ndims);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var2D_dimname): Error in getting ndims of var %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }

  if(ind < 0 || ind >= ndims) mpp_error("mpp_io(mpp_get_var_dimname): invalid ind value, ind should be between 0 and ndim-1");
  
  status = nc_inq_vardimid(ncid,fldid,dims);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var2D_dimname): Error in getting dimid of var %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  status = nc_inq_dimname(ncid, dims[ind], name);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_get_var2D_dimname): Error in getting %d dimension name of var %s from file %s",
	    ind, files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }

}; /* mpp_get_var_dimname */


/***************************************************************************
  char mpp_get_var_cart(int fid, int vid)
  get the cart of the dimension variable
  *************************************************************************/
char mpp_get_var_cart(int fid, int vid)
{
  char cart;
  int ncid, fldid, status;
  char errmsg[512];
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_cart): invalid fid number, fid should be "
				      "a nonnegative integer that less than nfiles"); 
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_cart): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");
  cart = 'N';

  ncid = files[fid].ncid;
  fldid = files[fid].var[vid].fldid;
  status = nc_get_att_text(ncid, fldid, "cartesian_axis", &cart);
  if(status != NC_NOERR)status = nc_get_att_text(ncid, fldid, "axis", &cart);
  if(status != NC_NOERR){
    sprintf(errmsg, "mpp_io(mpp_get_var_cart): Error in getting attribute cartesian_axis/axis of "
	    "dimension variable %s from file %s", files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
    
  return cart;
}

/***************************************************************************
 void mpp_get_var_bndname(int fid, int vid, char *bndname)
 Get the bound name of dimension variable if it exist, otherwise the value will be 'none'
 for time axis, the bounds may be 'climatology' 
 **************************************************************************/
void mpp_get_var_bndname(int fid, int vid, char *bndname)
{
  int ncid, fldid, status;
  char errmsg[512], name[32];
  size_t siz;
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_get_var_cart): invalid fid number, fid should be "
				      "a nonnegative integer that less than nfiles"); 
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_get_var_cart): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");
  ncid = files[fid].ncid;
  fldid = files[fid].var[vid].fldid;  
  strcpy(name, "climatology");
  status = nc_inq_attlen(ncid, fldid, name, &siz);
  if(status != NC_NOERR){
    strcpy(name, "bounds");
    status = nc_inq_attlen(ncid, fldid, name, &siz);
  }
  if(status != NC_NOERR){
    strcpy(name, "edges");
    status = nc_inq_attlen(ncid, fldid, name, &siz);
  }
  if(status != NC_NOERR) {
    strcpy(bndname, "none");
  }
  else {
    status = nc_get_att_text(ncid, fldid, name, bndname);
    bndname[siz] = '\0';
    if(status != NC_NOERR) {
      sprintf(errmsg, "mpp_io(mpp_get_var_bndname): Error in getting attribute %s of "
	      "dimension variable %s from file %s", name, files[fid].var[vid].name, files[fid].name );
      netcdf_error(errmsg, status);
    }
  }  
}

/***************************************************************************
  int mpp_var_att_exist(int fid, int vid, const char *att)
  check the field var has the attribute "att" or not.
***************************************************************************/
int mpp_var_att_exist(int fid, int vid, const char *att)
{
  int    status;
  size_t attlen;
  nc_type atttype;

  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_var_att_exist): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_var_att_exist): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");
  
  status = nc_inq_att(files[fid].ncid, files[fid].var[vid].fldid, att, &atttype, &attlen);
  if(status == NC_NOERR) 
    return 1;
  else
    return 0;
  
}; /* mpp_att_exist */


/*******************************************************************************/
/*                                                                             */
/*     The following are routines to write out data                            */
/*                                                                             */
/*******************************************************************************/

/********************************************************************
 int mpp_def_dim(int fid, char* name, int size)
 define dimension. 
********************************************************************/
int mpp_def_dim(int fid, const char* name, int size) {
  int dimid, status;
  char errmsg[512];
  
  if( mpp_pe() != mpp_root_pe() ) return 0;
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_def_dim): invalid fid number, fid should be "
				      "a nonnegative integer that less than nfiles");
  
  status = nc_def_dim(files[fid].ncid, name, size, &dimid);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_def_dim): Error in defining dimension %s of file %s",
	    name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  return dimid;
} /* mpp_def_dim */

/********************************************************************
 int mpp_def_var(nt fid, const char* name, int type, int ndim, int *dims, int natts ... )
 define metadata of field.
********************************************************************/
int mpp_def_var(int fid, const char* name, nc_type type, int ndim, const int *dims, int natts, ...) {
  int fldid, status, i, vid, ncid;
  va_list ap;
  char errmsg[512];
  
  if( mpp_pe() != mpp_root_pe() ) return 0;
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_def_var): invalid fid number, fid should be "
				      "a nonnegative integer that less than nfiles");

  ncid = files[fid].ncid;
  status = nc_def_var(ncid, name, type, ndim, dims, &fldid);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_def_var): Error in defining var %s of file %s",
	    name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  vid = files[fid].nvar;
  files[fid].nvar++;
  if(files[fid].nvar > MAXVAR ) mpp_error("mpp_io(mpp_def_var): nvar is larger than MAXVAR, increase MAXVAR");  
  files[fid].var[vid].fldid = fldid;
  files[fid].var[vid].type = type;
  strcpy(files[fid].var[vid].name, name);
  
  va_start(ap, natts);
  for( i=0; i<natts; i++) {
    char* attname = va_arg(ap, char*);
    char* attval = va_arg(ap, char*);
    if( attname == NULL || attval == NULL) {
      mpp_error("mpp_io: attribute name and attribute value not defined suitably, check the arguments list.");
    }
    status = nc_put_att_text(ncid,fldid,attname,strlen(attval),attval);
    if(status != NC_NOERR ) {
      sprintf(errmsg, "mpp_io(mpp_def_var): Error in put attribute %s of var %s of file %s",
	      attname, name, files[fid].name );
      netcdf_error(errmsg, status);
    }
  }
  va_end(ap);
  return vid;
} /* mpp_define_var */

/*********************************************************************
  void mpp_def_global_att(int fid, const char *name, const char *val)
  write out global attribute
 ********************************************************************/
void mpp_def_global_att(int fid, const char *name, const char *val)
{
  size_t status;
  char errmsg[512];
  
  if( mpp_pe() != mpp_root_pe() ) return;

  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_def_global_att): invalid fid number, fid should be "
				      "a nonnegative integer that less than nfiles");
  status = nc_put_att_text(files[fid].ncid, NC_GLOBAL, name, strlen(val), val);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_def_global_att): Error in put glboal attribute %s of file %s",
	    name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
}; /* mpp_def_global_att */

/**********************************************************************
  void mpp_copy_var_att(fid_in, fid_out)
  copy all the field attribute from infile to outfile
**********************************************************************/
void mpp_copy_var_att(int fid_in, int vid_in, int fid_out, int vid_out)
{
  int natt, status, i, ncid_in, ncid_out, fldid_in, fldid_out;
  char name[256];
  char errmsg[512];

  if( mpp_pe() != mpp_root_pe() ) return;
  
  if(fid_in<0 || fid_in >=nfiles) mpp_error("mpp_io(mpp_copy_var_att): invalid fid_in number, fid should be "
				      "a nonnegative integer that less than nfiles");
  if(fid_out<0 || fid_out >=nfiles) mpp_error("mpp_io(mpp_copy_var_att): invalid fid_out number, fid should be "
				      "a nonnegative integer that less than nfiles");
        
  ncid_in   = files[fid_in].ncid;
  ncid_out  = files[fid_out].ncid;  
  fldid_in  = files[fid_in].var[vid_in].fldid;
  fldid_out = files[fid_out].var[vid_out].fldid;
  
  status = nc_inq_varnatts(ncid_in, fldid_in, &natt);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_copy_var_att): Error in inquiring natts of var %s of file %s",
	    files[fid_in].var[vid_in].name, files[fid_in].name );
    netcdf_error(errmsg, status);
  }
  
  for(i=0; i<natt; i++) {
    status = nc_inq_attname(ncid_in, fldid_in, i, name);
    if(status != NC_NOERR) {
      sprintf(errmsg, "mpp_io(mpp_copy_var_att): Error in inquiring %d attname of var %s of file %s", i, 
	      files[fid_in].var[vid_in].name, files[fid_in].name );
      netcdf_error(errmsg, status);
    }
    status = nc_copy_att(ncid_in, fldid_in, name, ncid_out, fldid_out);
    if(status != NC_NOERR) {
      sprintf(errmsg, "mpp_io(mpp_copy_var_att): Error in copying att %s of var %s of file %s", name,  
	      files[fid_in].var[vid_in].name, files[fid_in].name );
      netcdf_error(errmsg, status);
    }
  }

}; /* mpp_copy_field_att */


/**********************************************************************
  void mpp_copy_global_att(fid_in, fid_out)
  copy all the global attribute from infile to outfile
**********************************************************************/
void mpp_copy_global_att(int fid_in, int fid_out)
{
  int natt, status, i, ncid_in, ncid_out;
  char name[256], errmsg[512];

  if( mpp_pe() != mpp_root_pe() ) return;

  if(fid_in<0 || fid_in >=nfiles) mpp_error("mpp_io(mpp_copy_global_att): invalid fid_in number, fid should be "
				      "a nonnegative integer that less than nfiles");
  if(fid_out<0 || fid_out >=nfiles) mpp_error("mpp_io(mpp_copy_global_att): invalid fid_out number, fid should be "
				      "a nonnegative integer that less than nfiles");
  ncid_in = files[fid_in].ncid;
  ncid_out = files[fid_out].ncid;
  
  status = nc_inq_varnatts(ncid_in, NC_GLOBAL, &natt);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_copy_global_att): Error in inquiring natts(global) of file %s",
	    files[fid_in].name );
    netcdf_error(errmsg, status);
  }

  for(i=0; i<natt; i++) {
    status = nc_inq_attname(ncid_in, NC_GLOBAL, i, name);
    if(status != NC_NOERR) {
      sprintf(errmsg, "mpp_io(mpp_copy_global_att): Error in inquiring %d global attname of file %s", i, 
	      files[fid_in].name );
      netcdf_error(errmsg, status);
    }    

    status = nc_copy_att(ncid_in, NC_GLOBAL, name, ncid_out, NC_GLOBAL);
    if(status != NC_NOERR) {
      sprintf(errmsg, "mpp_io(mpp_copy_global_att): Error in copying %d global att %s of file %s", i,  name,  
	      files[fid_in].name );
      netcdf_error(errmsg, status);
    }
  }

}; /* mpp_copy_global_att */

/*********************************************************************
  void mpp_put_var_value(int fid, int vid, const void* data)
  write out string-type data

 ********************************************************************/
void mpp_put_var_value(int fid, int vid, const void* data)
{
  size_t status;
  char errmsg[600];
  
  if( mpp_pe() != mpp_root_pe() ) return;

  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_put_var_value): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_put_var_value): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");

  switch(files[fid].var[vid].type) {
  case NC_DOUBLE: case NC_FLOAT:
    status = nc_put_var_double(files[fid].ncid, files[fid].var[vid].fldid, data);
    break;
  case NC_INT:
    status = nc_put_var_int(files[fid].ncid, files[fid].var[vid].fldid, data);
    break;     
  default:
    sprintf(errmsg, "mpp_io(mpp_put_var_value): field %s in file %s has an invalid type, "
	    "the type should be NC_DOUBLE, NC_FLOAT", files[fid].var[vid].name, files[fid].name );
    mpp_error(errmsg);
  }
  
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_put_var_value): Error in putting value of variable %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
}; /* mpp_put_var_value*/

/*********************************************************************
  void mpp_put_var_value_block(int fid, int vid, const size_t *start, const size_t *nread, void *data)
  read part of var data, the part is defined by start and nread.
*********************************************************************/
void mpp_put_var_value_block(int fid, int vid, const size_t *start, const size_t *nwrite, const void *data)
{
  int status;
  char errmsg[512];

  if( mpp_pe() != mpp_root_pe() ) return;
  
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_put_var_value_block): invalid fid number, fid should be "
				    "a nonnegative integer that less than nfiles");
  if(vid<0 || vid >=files[fid].nvar) mpp_error("mpp_io(mpp_put_var_value_block): invalid vid number, vid should be "
				    "a nonnegative integer that less than nvar");

  switch(files[fid].var[vid].type) {
  case NC_DOUBLE: case NC_FLOAT:
    status = nc_put_vara_double(files[fid].ncid, files[fid].var[vid].fldid, start, nwrite, data);
    break;
  case NC_INT:
    status = nc_put_vara_int(files[fid].ncid, files[fid].var[vid].fldid, start, nwrite, data);
    break;     
  case NC_CHAR:
    status = nc_put_vara_text(files[fid].ncid, files[fid].var[vid].fldid, start, nwrite, data);
    break; 
  default:
    sprintf(errmsg, "mpp_io(mpp_get_var_value_block): field %s in file %s has an invalid type, "
	    "the type should be NC_DOUBLE, NC_FLOAT", files[fid].var[vid].name, files[fid].name );
    mpp_error(errmsg);
  }    
  
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_put_var_value_block): Error in putting value of variable %s from file %s",
	    files[fid].var[vid].name, files[fid].name );
    netcdf_error(errmsg, status);
  }
  
}; /* mpp_put_var_value_block */

/*********************************************************************
   void mpp_end_def(int ncid) 
   end the definition of netcdf file with ncid.
 *******************************************************************/
void mpp_end_def(int fid) {
  int status;
  char errmsg[512];
  
  if( mpp_pe() != mpp_root_pe() ) return;
  if(fid<0 || fid >=nfiles) mpp_error("mpp_io(mpp_end_def): invalid fid number, fid should be "
				      "a nonnegative integer that less than nfiles");
  
  status = nc_enddef(files[fid].ncid);
  if(status != NC_NOERR) {
    sprintf(errmsg, "mpp_io(mpp_end_def): Error in end definition of file %s", files[fid].name );
    netcdf_error(errmsg, status);
  }
} /* mpp_end_def */

/*******************************************************************************
  int mpp_file_exist(const char *file)
  check to see if file exist or not.
*******************************************************************************/
int mpp_file_exist(const char *file)
{
  int status, ncid;
  
  status = nc_open(file,NC_NOWRITE, &ncid);
  if(status == NC_NOERR) {
    status = nc_close(ncid);
    if(status != NC_NOERR) netcdf_error("mpp_file_exist(mpp_io):in closing file", status);
    return 1;
  }
  else
    return 0;
};

int mpp_field_exist(const char *file, const char *field)
{
  int fid, status, varid;

  fid = mpp_open(file, MPP_READ);
  status = nc_inq_varid(files[fid].ncid, field, &varid);
  mpp_close(fid);

  if(status == NC_NOERR)
    return 1;
  else
    return 0;

}

 


