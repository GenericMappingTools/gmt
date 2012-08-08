/* This is part of the netCDF package. Copyright 2008 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use. See www.unidata.ucar.edu for more info.

   Create a test netCDF-4 file with compression, chunking, endianness
   settings to test ncdump "-s" option for displaying special
   attributes.

   $Id$
*/

#include <nc_tests.h>
#include <netcdf.h>

#define FILE_NAME "tst_special_atts.nc"
#define DIM1_NAME "dim1"
#define DIM1_LEN  10
#define DIM2_NAME "dim2"
#define DIM2_LEN  20
#define DIM3_NAME "dim3"
#define DIM3_LEN  30
#define VAR1_NAME "var1"
#define VAR1_RANK 1
#define VAR2_NAME "var2"
#define VAR2_RANK 2
#define VAR3_NAME "var3"
#define VAR3_RANK 3
#define VAR4_NAME "var4"
#define VAR4_RANK 3
#define CHUNK1 (DIM1_LEN/2 + 1)
#define CHUNK2 (DIM2_LEN/3 + 1)
#define CHUNK3 (DIM3_LEN/4 + 1)
#define DEFLATE_LEVEL 2
#define COMPRESS 1
#define TYPE6_NAME "obs_t"

int
main(int argc, char **argv)
{
   int ncid;
   int i, j, k, m;

   int dimids[VAR3_RANK];
   int var1id, var2id, var3id, var4id, var5id;
   size_t chunksizes[] = {CHUNK1, CHUNK2, CHUNK3};
   int data1[DIM1_LEN];
   int data1_in[DIM1_LEN];
   int data2[DIM1_LEN][DIM2_LEN];
   int data2_in[DIM1_LEN][DIM2_LEN];
   int data3[DIM1_LEN][DIM2_LEN][DIM3_LEN];
   int data3_in[DIM1_LEN][DIM2_LEN][DIM3_LEN];
   
   printf("\n*** Testing '-s' option for special attributes.\n");
   printf("*** creating special attributes test file %s...", FILE_NAME);
   if (nc_create(FILE_NAME, NC_CLOBBER | NC_NETCDF4, &ncid)) ERR;
   
   /* Declare dimensions and variables */
   if (nc_def_dim(ncid, DIM1_NAME, DIM1_LEN, &dimids[0])) ERR;
   if (nc_def_dim(ncid, DIM2_NAME, DIM2_LEN, &dimids[1])) ERR;
   if (nc_def_dim(ncid, DIM3_NAME, DIM3_LEN, &dimids[2])) ERR;
   if (nc_def_var(ncid, VAR1_NAME, NC_INT, VAR1_RANK, dimids, &var1id)) ERR;
   if (nc_def_var(ncid, VAR2_NAME, NC_INT, VAR2_RANK, dimids, &var2id)) ERR;
   if (nc_def_var(ncid, VAR3_NAME, NC_INT, VAR3_RANK, dimids, &var3id)) ERR;
   if (nc_def_var(ncid, VAR4_NAME, NC_INT, VAR4_RANK, dimids, &var4id)) ERR;
   {

       typedef struct obs_t {
	   char day;
	   short elev;
	   int count;
	   float relhum;
	   double time;
	   unsigned char category;
	   unsigned short id;
	   unsigned int particularity;
	   long long attention_span;
       } obs_t ;
       nc_type typeid;

       /* Create a compound type. */
       if (nc_def_compound(ncid, sizeof(obs_t), TYPE6_NAME, &typeid)) ERR;
       if (nc_insert_compound(ncid, typeid, "day", 
			      NC_COMPOUND_OFFSET(struct obs_t, day), NC_BYTE)) ERR;
       if (nc_insert_compound(ncid, typeid, "elev", 
			      NC_COMPOUND_OFFSET(struct obs_t, elev), NC_SHORT)) ERR;
       if (nc_insert_compound(ncid, typeid, "count", 
			      NC_COMPOUND_OFFSET(struct obs_t, count), NC_INT)) ERR;
       if (nc_insert_compound(ncid, typeid, "relhum", 
			      NC_COMPOUND_OFFSET(struct obs_t, relhum), 
			      NC_FLOAT)) ERR;
       if (nc_insert_compound(ncid, typeid, "time", 
			      NC_COMPOUND_OFFSET(struct obs_t, time), 
			      NC_DOUBLE)) ERR;
       if (nc_insert_compound(ncid, typeid, "category", 
			      NC_COMPOUND_OFFSET(struct obs_t, category), 
			      NC_UBYTE)) ERR;
       if (nc_insert_compound(ncid, typeid, "id", 
			      NC_COMPOUND_OFFSET(struct obs_t, id), 
			      NC_USHORT)) ERR;
       if (nc_insert_compound(ncid, typeid, "particularity", 
			      NC_COMPOUND_OFFSET(struct obs_t, particularity), 
			      NC_UINT)) ERR;
       if (nc_insert_compound(ncid, typeid, "attention_span", 
			      NC_COMPOUND_OFFSET(struct obs_t, attention_span), 
			      NC_INT64)) ERR;
       /* create a variable of that compound type */
       if (nc_def_var(ncid, "var5", typeid, VAR1_RANK, dimids, &var5id)) 
	   ERR; 
   }

   /* Specify contiguous storage and endianness explicitly for var1. */
   if (nc_def_var_chunking(ncid, var1id, NC_CONTIGUOUS, NULL)) ERR;
   if (nc_def_var_endian(ncid, var1id, NC_ENDIAN_LITTLE)) ERR;

   /* Specify chunking for var2, note only uses CHUNK1 and CHUNK2.
      Also, specify using a checksum for this variable. */
   if (nc_def_var_chunking(ncid, var2id, NC_CHUNKED, chunksizes)) ERR;
   if (nc_def_var_fletcher32(ncid, var2id, NC_FLETCHER32)) ERR;
   if (nc_def_var_endian(ncid, var2id, NC_ENDIAN_BIG)) ERR;

   /* Use compression but no shuffle for var3, also set to big endian */
   if (nc_def_var_deflate(ncid, var3id, NC_NOSHUFFLE, COMPRESS, DEFLATE_LEVEL)) ERR;
   if (nc_def_var_chunking(ncid, var3id, NC_CHUNKED, chunksizes)) ERR;
   if (nc_def_var_endian(ncid, var3id, NC_ENDIAN_LITTLE)) ERR;

   /* Use compression, chunking, shuffle filter, and no-fill for var4 */
   if (nc_def_var_deflate(ncid, var4id, NC_SHUFFLE, COMPRESS, DEFLATE_LEVEL)) ERR;
   if (nc_def_var_chunking(ncid, var4id, NC_CHUNKED, chunksizes)) ERR;
   if (nc_def_var_endian(ncid, var4id, NC_ENDIAN_LITTLE)) ERR;
   if (nc_def_var_fill(ncid, var4id, NC_NOFILL, NULL)) ERR;

   /* Set endianness, chunking, checksums, shuffle, and compression for var5 */
   if (nc_def_var_chunking(ncid, var5id, NC_CHUNKED, chunksizes)) ERR;
   if (nc_def_var_fletcher32(ncid, var5id, NC_FLETCHER32)) ERR;
   if (nc_def_var_deflate(ncid, var5id, NC_SHUFFLE, COMPRESS, DEFLATE_LEVEL)) ERR;

   if (nc_enddef(ncid)) ERR;

   /* Some artificial data */
   m = 0;
   for(i = 0; i < DIM1_LEN; i++) {
       data1[i] = m++;
       for(j = 0; j < DIM2_LEN; j++) {
	   data2[i][j] = m++;
	   for(k = 0; k < DIM3_LEN; k++) {
	       data3[i][j][k] = m++;
	   }
       }
   }

   /* Store data in each variable */
   if(nc_put_var(ncid, var1id, &data1[0])) ERR;
   if(nc_put_var(ncid, var2id, &data2[0][0])) ERR;
   if(nc_put_var(ncid, var3id, &data3[0][0][0])) ERR;
   if(nc_put_var(ncid, var4id, &data3[0][0][0])) ERR;
   
   if (nc_close(ncid)) ERR;
   
   /* Check it out. */
   if (nc_open(FILE_NAME, NC_NOWRITE, &ncid)) ERR;
   if (nc_inq_varid(ncid, VAR1_NAME, &var1id)) ERR;
   if (nc_inq_varid(ncid, VAR2_NAME, &var2id)) ERR;
   if (nc_inq_varid(ncid, VAR3_NAME, &var3id)) ERR;
   if (nc_inq_varid(ncid, VAR4_NAME, &var4id)) ERR;
   
   /* Check chunk sizes */
   {
       size_t chunks_in[VAR3_RANK];
       if (nc_inq_var_chunking(ncid, var2id, NULL, chunks_in)) ERR;
       for(i = 0; i < VAR2_RANK; i++)
	   if(chunks_in[i] != chunksizes[i]) ERR;
       if (nc_inq_var_chunking(ncid, var3id, NULL, chunks_in)) ERR;
       for(i = 0; i < VAR3_RANK; i++)
	   if(chunks_in[i] != chunksizes[i]) ERR;
       if (nc_inq_var_chunking(ncid, var4id, NULL, chunks_in)) ERR;
       for(i = 0; i < VAR4_RANK; i++)
	   if(chunks_in[i] != chunksizes[i]) ERR;
   }
   if(nc_get_var(ncid, var1id, &data1_in[0])) ERR;
   for(i = 0; i < DIM1_LEN; i++)
       if(data1_in[i] != data1[i]) ERR;
   
   if(nc_get_var(ncid, var2id, &data2_in[0][0])) ERR;
   for(i = 0; i < DIM1_LEN; i++)
       for(j = 0; j < DIM2_LEN; j++)
	   if(data2_in[i][j] != data2[i][j]) ERR;
   
   if(nc_get_var(ncid, var3id, &data3_in[0][0][0])) ERR;
   for(i = 0; i < DIM1_LEN; i++)
       for(j = 0; j < DIM2_LEN; j++)
	   for(k = 0; k < DIM3_LEN; k++)
	       if(data3_in[i][j][k] != data3[i][j][k]) ERR;
   
   if(nc_get_var(ncid, var4id, &data3_in[0][0][0])) ERR;
   for(i = 0; i < DIM1_LEN; i++)
       for(j = 0; j < DIM2_LEN; j++)
	   for(k = 0; k < DIM3_LEN; k++)
	       if(data3_in[i][j][k] != data3[i][j][k]) ERR;

   SUMMARIZE_ERR;
   FINAL_RESULTS;
}

