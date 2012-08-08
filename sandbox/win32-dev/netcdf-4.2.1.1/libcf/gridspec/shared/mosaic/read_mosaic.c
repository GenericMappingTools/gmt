#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "read_mosaic.h"
#include "constant.h"
#include "mosaic_util.h"
#ifdef use_netCDF
#include <netcdf.h>
#endif
/*********************************************************************
    void netcdf_error( int status )
    status is the returning value of netcdf call. this routine will
    handle the error when status is not NC_NOERR.
********************************************************************/
void handle_netcdf_error(const char *msg, int status )
{
  char errmsg[512];

  sprintf( errmsg, "%s: %s", msg, nc_strerror(status) );
  error_handler(errmsg);

}; /* handle_netcdf_error */

/***************************************************************************
  void get_file_dir(const char *file, char *dir)
  get the directory where file is located. The dir will be the complate path
  before the last "/". If no "/" exist in file, the path will be current ".".
***************************************************************************/
void get_file_dir(const char *file, char *dir)
{
  int len;
  char *strptr = NULL;

  /* get the diretory */
 
  strptr = strrchr(file, '/');
  if(strptr) {
    len = strptr - file;
    strncpy(dir, file, len);
  }
  else {
    len = 1;
    strcpy(dir, ".");
  }
  dir[len] = 0;

}; /* get_file_dir */


int field_exist(const char* file, const char *name)
{
  int ncid, varid, status, existed;
  char msg[512];  
#ifdef use_netCDF
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "field_exist: in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  status = nc_inq_varid(ncid, name, &varid);  
  if(status == NC_NOERR)
    existed = 1;
  else
    existed = 0;
    
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "field_exist: in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }

  return existed;
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");

#endif
  return 0; 
}; /* field_exist */

int get_dimlen(const char* file, const char *name)
{
  int ncid, dimid, status, len;
  size_t size;
  char msg[512];
#ifdef use_netCDF  
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  
  status = nc_inq_dimid(ncid, name, &dimid);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting dimid of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }
  
  status = nc_inq_dimlen(ncid, dimid, &size);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting dimension size of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }
  
  len = size;
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s", file);
    handle_netcdf_error(msg, status);
  }
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");
#endif
  
  return len;
  
}; /* get_dimlen */

/*******************************************************************************
   void get_string_data(const char *file, const char *name, char *data)
   get string data of field with "name" from "file".
******************************************************************************/
void get_string_data(const char *file, const char *name, char *data)
{
  int ncid, varid, status;
  char msg[512];

#ifdef use_netCDF    
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  status = nc_inq_varid(ncid, name, &varid);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting varid of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }     
  status = nc_get_var_text(ncid, varid, data);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting data of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }  
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");
#endif
  
}; /* get_string_data */

/*******************************************************************************
   void get_string_data_level(const char *file, const char *name, const size_t *start, const size_t *nread, char *data)
   get string data of field with "name" from "file".
******************************************************************************/
void get_string_data_level(const char *file, const char *name, char *data, const int *level)
{
  int ncid, varid, status, i;
  size_t start[4], nread[4];
  char msg[512];

#ifdef use_netCDF  
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  status = nc_inq_varid(ncid, name, &varid);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting varid of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }
  for(i=0; i<4; i++) {
    start[i] = 0; nread[i] = 1;
  }
  start[0] = *level; nread[1] = STRING;
  status = nc_get_vara_text(ncid, varid, start, nread, data);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting data of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }  
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");
#endif
  
}; /* get_string_data_level */


/*******************************************************************************
   void get_int_data(const char *file, const char *name, int *data)
   get int data of field with "name" from "file".
******************************************************************************/
void get_int_data(const char *file, const char *name, int *data)
{
  int ncid, varid, status;
  char msg[512];

#ifdef use_netCDF    
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  status = nc_inq_varid(ncid, name, &varid);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting varid of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }     
  status = nc_get_var_int(ncid, varid, data);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting data of %s from file %s", name, file);
    handle_netcdf_error(msg, status);
  }
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }  
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");
#endif
  
}; /* get_int_data */

/*******************************************************************************
   void get_double_data(const char *file, const char *name, double *data)
   get double data of field with "name" from "file".
******************************************************************************/
void get_double_data(const char *file, const char *name, double *data)
{

  int ncid, varid, status;  
  char msg[512];

#ifdef use_netCDF    
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  status = nc_inq_varid(ncid, name, &varid);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting varid of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }     
  status = nc_get_var_double(ncid, varid, data);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting data of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }  
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");
#endif
  
}; /* get_double_data */

/******************************************************************************
   void get_var_text_att(const char *file, const char *name, const char *attname, char *att)
   get text attribute of field 'name' from 'file
******************************************************************************/
void get_var_text_att(const char *file, const char *name, const char *attname, char *att)
{
  int ncid, varid, status;  
  char msg[512];

#ifdef use_netCDF    
  status = nc_open(file, NC_NOWRITE, &ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in opening file %s", file);
    handle_netcdf_error(msg, status);
  }
  status = nc_inq_varid(ncid, name, &varid);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting varid of %s from file %s.", name, file);
    handle_netcdf_error(msg, status);
  }     
  status = nc_get_att_text(ncid, varid, attname, att);
  if(status != NC_NOERR) {
    sprintf(msg, "in getting attribute %s of %s from file %s.", attname, name, file);
    handle_netcdf_error(msg, status);
  }
  status = nc_close(ncid);
  if(status != NC_NOERR) {
    sprintf(msg, "in closing file %s.", file);
    handle_netcdf_error(msg, status);
  }  
#else
  error_handler("read_mosaic: Add flag -Duse_netCDF when compiling");
#endif
  
}; /* get_var_text_att */

/***********************************************************************
  return number of overlapping cells.
***********************************************************************/
#ifndef __AIX
int read_mosaic_xgrid_size_( const char *xgrid_file )
{
  return read_mosaic_xgrid_size(xgrid_file);
}
#endif

int read_mosaic_xgrid_size( const char *xgrid_file )
{
  int ncells;
  
  ncells = get_dimlen(xgrid_file, "ncells");
  return ncells;
}



/****************************************************************************/
#ifndef __AIX
void read_mosaic_xgrid_order1_(const char *xgrid_file, int *i1, int *j1, int *i2, int *j2, double *area )
{
  read_mosaic_xgrid_order1(xgrid_file, i1, j1, i2, j2, area);
  
};
#endif

void read_mosaic_xgrid_order1(const char *xgrid_file, int *i1, int *j1, int *i2, int *j2, double *area )
{
  int    ncells, n;
  int    *tile1_cell, *tile2_cell;
  double garea;
  
  ncells = get_dimlen(xgrid_file, "ncells");

  tile1_cell       = (int *)malloc(ncells*2*sizeof(int));
  tile2_cell       = (int *)malloc(ncells*2*sizeof(int));
  get_int_data(xgrid_file, "tile1_cell", tile1_cell);
  get_int_data(xgrid_file, "tile2_cell", tile2_cell);
  get_double_data(xgrid_file, "xgrid_area", area);
  garea = 4*M_PI*RADIUS*RADIUS;
  
  for(n=0; n<ncells; n++) {
    i1[n] = tile1_cell[n*2] - 1;
    j1[n] = tile1_cell[n*2+1] - 1;
    i2[n] = tile2_cell[n*2] - 1;
    j2[n] = tile2_cell[n*2+1] - 1;
    area[n] /= garea; /* rescale the exchange grid area to unit earth area */
  }

  free(tile1_cell);
  free(tile2_cell);
  
}; /* read_mosaic_xgrid_order1 */

/* NOTE: di, dj is for tile1, */
/****************************************************************************/
#ifndef __AIX
void read_mosaic_xgrid_order2_(const char *xgrid_file, int *i1, int *j1, int *i2, int *j2, double *area, double *di, double *dj )
{
  read_mosaic_xgrid_order2(xgrid_file, i1, j1, i2, j2, area, di, dj);
  
};
#endif

void read_mosaic_xgrid_order2(const char *xgrid_file, int *i1, int *j1, int *i2, int *j2, double *area, double *di, double *dj )
{
  int    ncells, n;
  int    *tile1_cell, *tile2_cell;
  double *tile1_distance;
  double garea;
  
  ncells = get_dimlen(xgrid_file, "ncells");

  tile1_cell       = (int    *)malloc(ncells*2*sizeof(int   ));
  tile2_cell       = (int    *)malloc(ncells*2*sizeof(int   ));
  tile1_distance   = (double *)malloc(ncells*2*sizeof(double));
  get_int_data(xgrid_file, "tile1_cell", tile1_cell);
  get_int_data(xgrid_file, "tile2_cell", tile2_cell);
  get_double_data(xgrid_file, "xgrid_area", area);
  get_double_data(xgrid_file, "tile1_distance", tile1_distance);
  garea = 4*M_PI*RADIUS*RADIUS;
  
  for(n=0; n<ncells; n++) {
    i1[n] = tile1_cell[n*2] - 1;
    j1[n] = tile1_cell[n*2+1] - 1;
    i2[n] = tile2_cell[n*2] - 1;
    j2[n] = tile2_cell[n*2+1] - 1;
    di[n] = tile1_distance[n*2];
    dj[n] = tile1_distance[n*2+1];
    area[n] /= garea; /* rescale the exchange grid area to unit earth area */
  }

  free(tile1_cell);
  free(tile2_cell);
  free(tile1_distance);
  
}; /* read_mosaic_xgrid_order2 */

/******************************************************************************
  int read_mosaic_ntiles(const char *mosaic_file)
  return number tiles in mosaic_file
******************************************************************************/
#ifndef __AIX
int read_mosaic_ntiles_(const char *mosaic_file)
{
  return read_mosaic_ntiles(mosaic_file);
}
#endif
int read_mosaic_ntiles(const char *mosaic_file)
{

  int ntiles;

  ntiles = get_dimlen(mosaic_file, "ntiles");

  return ntiles;
  
}; /* read_mosaic_ntiles */

/******************************************************************************
  int read_mosaic_ncontacts(const char *mosaic_file)
  return number of contacts in mosaic_file
******************************************************************************/
#ifndef __AIX
int read_mosaic_ncontacts_(const char *mosaic_file)
{
  return read_mosaic_ncontacts(mosaic_file);
}
#endif
int read_mosaic_ncontacts(const char *mosaic_file)
{
  char contact_file[STRING], file[STRING], dir[STRING];
  int ncontacts;

  if(field_exist(mosaic_file, CONTACT_FILES_NAME) ) {
    get_file_dir(mosaic_file, dir);
    get_string_data(mosaic_file, CONTACT_FILES_NAME, file);
    sprintf(contact_file, "%s/%s", dir, file);
    ncontacts = get_dimlen(contact_file, NCONTACT_NAME);
  }
  else
    ncontacts = 0;
  
  return ncontacts;
  
}; /* read_mosaic_ncontacts */


/*****************************************************************************
  void read_mosaic_grid_sizes(const char *mosaic_file, int *nx, int *ny)
  read mosaic grid size of each tile, currently we are assuming the refinement is 2.
  We assume the grid files are located at the same directory as mosaic_file.
*****************************************************************************/
#ifndef __AIX
void read_mosaic_grid_sizes_(const char *mosaic_file, int *nx, int *ny)
{
  read_mosaic_grid_sizes(mosaic_file, nx, ny);
}
#endif
void read_mosaic_grid_sizes(const char *mosaic_file, int *nx, int *ny)
{
  int ntiles, n;
  char gridfile[STRING], tilefile[2*STRING];
  char dir[STRING];
  const int x_refine = 2, y_refine = 2;

  get_file_dir(mosaic_file, dir);  
  ntiles = get_dimlen(mosaic_file, "ntiles");
  for(n = 0; n < ntiles; n++) {
    get_string_data_level(mosaic_file, "gridfiles", gridfile, &n);
    sprintf(tilefile, "%s/%s", dir, gridfile);
    nx[n] = get_dimlen(tilefile, "nx");
    ny[n] = get_dimlen(tilefile, "ny");
    if(nx[n]%x_refine != 0) error_handler("Error from read_mosaic_grid_sizes: nx is not divided by x_refine");
    if(ny[n]%y_refine != 0) error_handler("Error from read_mosaic_grid_sizes: ny is not divided by y_refine");
    nx[n] /= x_refine;
    ny[n] /= y_refine;
  }
  
}; /* read_mosaic_grid_sizes */
  

/******************************************************************************
  void read_mosaic_contact(const char *mosaic_file)
  read mosaic contact information
******************************************************************************/
#ifndef __AIX
void read_mosaic_contact_(const char *mosaic_file, int *tile1, int *tile2, int *istart1, int *iend1,
			 int *jstart1, int *jend1, int *istart2, int *iend2, int *jstart2, int *jend2)
{
  read_mosaic_contact(mosaic_file, tile1, tile2, istart1, iend1, jstart1, jend1, istart2, iend2, jstart2, jend2);
}
#endif

void read_mosaic_contact(const char *mosaic_file, int *tile1, int *tile2, int *istart1, int *iend1,
			 int *jstart1, int *jend1, int *istart2, int *iend2, int *jstart2, int *jend2)
{
  char contacts[STRING], tilefile[STRING], tilepath[STRING];
  char **gridtiles;
#define MAXVAR 40
  char pstring[MAXVAR][STRING];
  int ntiles, ncontacts, n, m, l, nstr, found;
  const int x_refine = 2, y_refine = 2;
  char contact_file[STRING], file[STRING], dir[STRING];

  get_file_dir(mosaic_file, dir);  
  ntiles = get_dimlen(mosaic_file, "ntiles");
  gridtiles = (char **)malloc(ntiles*sizeof(char *));
  for(n=0; n<ntiles; n++) {
    gridtiles[n] = (char *)malloc(STRING*sizeof(char));
    get_string_data_level(mosaic_file,  TILE_FILES_NAME, tilefile, &n);
    sprintf(tilepath,"%s/%s", dir, tilefile);
    get_string_data(tilepath, TILE_NAME, gridtiles[n]);
  }

  get_string_data(mosaic_file, CONTACT_FILES_NAME, file);
  sprintf(contact_file, "%s/%s", dir, file);    
  ncontacts = get_dimlen(contact_file, NCONTACT_NAME);
  for(n = 0; n < ncontacts; n++) {
    get_string_data_level(contact_file, "contacts", contacts, &n);
    /* parse the string contacts to get tile number */
    tokenize( contacts, ":", STRING, MAXVAR, pstring, &nstr);
    if(nstr != 4) error_handler("Error from read_mosaic: number of elements "
				 "in contact seperated by :/:: should be 4");
    found = 0;
    for(m=0; m<ntiles; m++) {
      if(strcmp(gridtiles[m], pstring[1]) == 0) { /*found the tile name */
	found = 1;
	tile1[n] = m+1;
	break;
      }
    }
    if(!found) error_handler("error from read_mosaic: the first tile name specified "
			     "in contact is not found in tile list");
    found = 0;
    for(m=0; m<ntiles; m++) {
      if(strcmp(gridtiles[m], pstring[3]) == 0) { /*found the tile name */
	found = 1;
	tile2[n] = m+1;
	break;
      }
    }
    if(!found) error_handler("error from read_mosaic: the second tile name specified "
			     "in contact is not found in tile list");    
    get_string_data_level(contact_file, "contact_index", contacts, &n);
    /* parse the string to get contact index */
    tokenize( contacts, ":,", STRING, MAXVAR, pstring, &nstr);
    if(nstr != 8) error_handler("Error from read_mosaic: number of elements "
				 "in contact_index seperated by :/, should be 8");
    /* make sure the string is only composed of numbers */
    for(m=0; m<nstr; m++) for(l=0; l<strlen(pstring[m]); l++) {
      if(pstring[m][l] > '9' ||  pstring[m][l] < '0' ) {
	error_handler("Error from read_mosaic: some of the character in "
		      "contact_indices except token is not digit number");
      }
    }
    istart1[n] = atoi(pstring[0]);
    iend1[n]   = atoi(pstring[1]);
    jstart1[n] = atoi(pstring[2]);
    jend1[n]   = atoi(pstring[3]);
    istart2[n] = atoi(pstring[4]);
    iend2[n]   = atoi(pstring[5]);
    jstart2[n] = atoi(pstring[6]);
    jend2[n]   = atoi(pstring[7]);
    if(istart1[n] == iend1[n] ) {
      istart1[n] = (istart1[n]+1)/x_refine-1;
      iend1[n]   = istart1[n];
      if( jend1[n] > jstart1[n] ) {
	jstart1[n] -= 1;
	jend1[n]   -= y_refine;
      }
      else if( jend1[n] < jstart1[n] ) {
	jstart1[n] -= y_refine;
	jend1[n]   -= 1;
      }
      else
	error_handler("Error from read_mosaic_contact: jstart1 and jend1 should not be equal when istart1=iend1");

      if(jstart1[n]%y_refine || jend1[n]%y_refine)
	error_handler("Error from read_mosaic_contact: mismatch between y_refine and jstart1/jend1 when istart1=iend1");
      jstart1[n] /= y_refine;
      jend1[n]   /= y_refine;
    }
    else if( jstart1[n] == jend1[n] ) {
      jstart1[n] = (jstart1[n]+1)/y_refine-1;
      jend1[n]   = jstart1[n];      
      if(iend1[n] > istart1[n] ) {
	istart1[n] -= 1;
	iend1[n]   -= x_refine;
      }
      else if(istart1[n] > iend1[n] ) {
	istart1[n] -= x_refine;
	iend1[n]   -= 1;	
      }
      else
	error_handler("Error from read_mosaic_contact: istart1 and iend1 should not be equal when jstart1=jend1");
      
      if(istart1[n]%x_refine || iend1[n]%x_refine)
	error_handler("Error from read_mosaic_contact: mismatch between x_refine and istart1/iend1 when jstart1=jend1");
      istart1[n] /= x_refine;
      iend1[n]   /= x_refine;
    }
    else {
      error_handler("Error from read_mosaic_contact: only line contact is supported now, contact developer");
    }
    if(istart2[n] == iend2[n] ) {
      istart2[n] = (istart2[n]+1)/x_refine-1;
      iend2[n]   = istart2[n];
      if( jend2[n] > jstart2[n] ) {
	jstart2[n] -= 1;
	jend2[n]   -= y_refine;
      }
      else if( jstart2[n] > jend2[n] ) {
	jstart2[n] -= y_refine;
	jend2[n]   -= 1;
      }
      else
	error_handler("Error from read_mosaic_contact: jstart2 and jend2 should not be equal when istart2=iend2");
      
      if(jstart2[n]%y_refine || jend2[n]%y_refine )
	error_handler("Error from read_mosaic_contact: mismatch between y_refine and jstart2/jend2 when istart2=iend2");

      jstart2[n] /= y_refine;
      jend2[n]   /= y_refine;
    }
    else if( jstart2[n] == jend2[n] ) {
      jstart2[n] = (jstart2[n]+1)/y_refine-1;
      jend2[n]   = jstart2[n];
      if(iend2[n] > istart2[n] ) {
	istart2[n] -= 1;
	iend2[n]   -= x_refine;
      }
      else if(istart2[n] > iend2[n] ) {
	istart2[n] -= x_refine;
	iend2[n]   -= 1;
      }
      else
	error_handler("Error from read_mosaic_contact: istart2 and iend2 should not be equal when jstart2=jend2");
      
      if(istart2[n]%x_refine || iend2[n]%x_refine)
	error_handler("Error from read_mosaic_contact: mismatch between x_refine and istart2/iend2 when jstart2=jend2");
      istart2[n] /= x_refine;
      iend2[n]   /= x_refine;
    }
    else {
      error_handler("Error from read_mosaic_contact: only line contact is supported now, contact developer");
    }

  }

}; /* read_mosaic_contact */


/******************************************************************************
  void read_mosaic_grid_data(const char *mosaic_file, const char *name, int nx, int ny,
                             double *data, int level, int ioff, int joff)
  read mosaic grid information onto model grid. We assume the refinement is 2 right now.
  We may remove this restriction in the future. nx and ny are model grid size. level
  is the tile number. ioff and joff to indicate grid location. ioff =0 and joff = 0
  for C-cell. ioff=0 and joff=1 for E-cell, ioff=1 and joff=0 for N-cell,
  ioff=1 and joff=1 for T-cell
******************************************************************************/
void read_mosaic_grid_data(const char *mosaic_file, const char *name, int nx, int ny,
                           double *data, int level, int ioff, int joff)
{
  char   tilefile[STRING], gridfile[STRING], dir[STRING];
  double *tmp;
  int    ni, nj, nxp, nyp, i, j;

  get_file_dir(mosaic_file, dir);
  
  get_string_data_level(mosaic_file, "gridfiles", gridfile, &level);
  sprintf(tilefile, "%s/%s", dir, gridfile);
  
  ni = get_dimlen(tilefile, "nx");
  nj = get_dimlen(tilefile, "ny");

  if( ni != nx*2 || nj != ny*2) error_handler("supergrid size should be double of the model grid size");
  tmp = (double *)malloc((ni+1)*(nj+1)*sizeof(double));
  get_double_data( tilefile, name, tmp);
  nxp = nx + 1 - ioff;
  nyp = ny + 1 - joff;
  for(j=0; j<nyp; j++) for(i=0; i<nxp; i++) data[j*nxp+i] = tmp[(2*j+joff)*(ni+1)+2*i+ioff];
  free(tmp);
   
}; /* read_mosaic_grid_data */


