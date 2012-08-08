/* Test program for types. */
#include <config.h>
#include <netcdf.h>

/* This macro prints an error message and exits. */
#define BAIL(e) do { \
fprintf(stderr, "Error in file %s, line %d.\n%s\n", \
__FILE__, __LINE__, nc_strerror(e)); \
return 2; \
} while (0)

/* This macro keep track of the number of errors and prints an error
 * message. */
#define ERR do { \
fprintf(stderr, "Error in file %s, line %d.\n", __FILE__, __LINE__); \
errors++; \
} while (0)
 
#define MAX_VARNAME 20
#define NUM_TYPES 6
#define NUM_DIMS 1
#define SIZE 5
#define STRIDE_SIZE 2
#define FILENAME "test.nc"

#define CLEAN_INPUT_BUFFERS  \
   for (i=0; i<SIZE; i++) { \
      ubyte_data_out[i] = 0; \
      ushort_data_out[i] = 0; \
      uint_data_out[i] = 0; \
      int64_data_out[i] = 0; \
      uint64_data_out[i] = 0; \
      bool_data_out[i] = 0; \
   }

int main(int argc, char *argv[])
{
   /* IDs, names, and parameters for the var[asm1] functions. */
   int ncid, varid[NUM_TYPES], attid, dimid;
   char varname[MAX_VARNAME];
   size_t index1[NUM_DIMS], start[NUM_DIMS], offset[NUM_DIMS];
   size_t count[NUM_DIMS], imap[NUM_DIMS];
   ptrdiff_t stride[NUM_DIMS];

   /* Phoney data we will write. */
   unsigned char ubyte_data_out[] = {0,1,2,3,4};
   unsigned short ushort_data_out[] = {0,11,22,33,44}; 
   unsigned int uint_data_out[] = {0,111,222,333,444};
   nc_int64 int64_data_out[] = {0,-111111111,2222222222,-3333333333,444444444};
   nc_uint64 uint64_data_out[] = {0,111111111,2222222222,33333333,44444444};
   unsigned char bool_data_out[] = {0,1,0,1,0};

   /* We will read back in the phoney data with these. */
   unsigned char ubyte_data_in[SIZE];
   unsigned short ushort_data_in[SIZE];
   unsigned int uint_data_in[SIZE];
   nc_int64 int64_data_in[SIZE];
   nc_uint64 uint64_data_in[SIZE];
   unsigned char bool_data_in[SIZE];

   int i;
   int type, num_errors = 0, res = NC_NOERR;
   int errors = 0, total_errors = 0;

#ifdef USE_PARALLEL
   MPI_Init(&argc, &argv);
#endif

   /* Uncomment the following line to get verbose feedback. */
   /*nc_set_log_level(2);*/
   printf("\n\n*** Testing netCDF-4 new atomic types...\n");

   /* Open a netcdf-4 file, and one dimension. */
   if ((res = nc_create(FILENAME, NC_NETCDF4, &ncid)))
      BAIL(res);
   if ((res = nc_def_dim(ncid, "dim1", SIZE, &dimid)))
      BAIL(res);

   /* Create vars of the new types. Take advantage of the fact that
    * new types are numbered from NC_UBYTE (7) through NC_BOOL (12).*/
   for(type = 0; type < NUM_TYPES; type++)
   {
      /* Create a var... */
      sprintf(varname, "var_%d", type);
      printf("*** creating var %s, type: %d...\t\t", varname, type+NC_UBYTE);
      if ((res = nc_def_var(ncid, varname, type+NC_UBYTE, 1, &dimid, &varid[type])))
	 BAIL(res);
      printf("ok!\n");
   }
   
   /* Test the varm functions. */
   printf("*** testing varm functions...\t\t\t");
/*   CLEAN_INPUT_BUFFERS;
   errors = 0;
   start[0] = 0;
   count[0] = 1;
   stride[0] = 1;
   imap[0] = 0;

   if ((res = nc_put_varm_ubyte(ncid, varid[0], start, count, 
				stride, imap, ubyte_data_out)))
      BAIL(res);
   if ((res = nc_get_varm_ubyte(ncid, varid[0], start, count, 
				stride, imap, ubyte_data_in)))
      BAIL(res);
   for (i=0; i<STRIDE_SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_varm_ushort(ncid, varid[1], start, count, 
				 stride, imap, ushort_data_out)))
      BAIL(res);
   if ((res = nc_get_varm_ushort(ncid, varid[1], start, count,
				 stride, imap, ushort_data_in)))
      BAIL(res);
   for (i=0; i<STRIDE_SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_varm_uint(ncid, varid[2], start, 
			       count, stride, imap, uint_data_out)))
      BAIL(res);
   if ((res = nc_get_varm_uint(ncid, varid[2], start, count, 
			       stride, imap, uint_data_in)))
      BAIL(res);
   for (i=0; i<STRIDE_SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_varm_int64(ncid, varid[3], start, count, 
				stride, imap, int64_data_out)))
      BAIL(res);
   if ((res = nc_get_varm_int64(ncid, varid[3], start, count, 
				stride, imap, int64_data_in)))
      BAIL(res);
   for (i=0; i<STRIDE_SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_varm_uint64(ncid, varid[4], start, count, 
				 stride, imap, uint64_data_out)))
      BAIL(res);
   if ((res = nc_get_varm_uint64(ncid, varid[4], start, count,
				 stride, imap, uint64_data_in)))
      BAIL(res);
   for (i=0; i<STRIDE_SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_varm_bool(ncid, varid[5], start, count, 
			       stride, imap, bool_data_out)))
      BAIL(res);
   if ((res = nc_get_varm_bool(ncid, varid[5], start, 
			       count, stride, imap, bool_data_in)))
      BAIL(res);
   for (i=0; i<STRIDE_SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   total_errors += errors;
   if (errors)
      printf("*** ERROR!! - %d errors. Sorry!\n");
   else
      printf("ok!\n");
*/
   /* Test the vars functions. */
   printf("*** testing vars functions...\t\t\t");
   CLEAN_INPUT_BUFFERS;
   errors = 0;
   start[0] = 0;
   count[0] = 2;
   stride[0] = STRIDE_SIZE;

   if ((res = nc_put_vars_uchar(ncid, varid[0], start, count, 
				stride, ubyte_data_out)))
      BAIL(res);
   if ((res = nc_get_vars_uchar(ncid, varid[0], start, count, 
				stride, ubyte_data_in)))
      BAIL(res);
   if (ubyte_data_in[0] != ubyte_data_out[0]) ERR;
   if (ubyte_data_in[1] != ubyte_data_out[STRIDE_SIZE]) ERR;

   if ((res = nc_put_vars_ushort(ncid, varid[1], start, count, 
				 stride, ushort_data_out)))
      BAIL(res);
   if ((res = nc_get_vars_ushort(ncid, varid[1], start, count,
				 stride, ushort_data_in)))
      BAIL(res);
   for (i=0; i<2; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vars_uint(ncid, varid[2], start, 
			       count, stride, uint_data_out)))
      BAIL(res);
   if ((res = nc_get_vars_uint(ncid, varid[2], start, count, 
			       stride, uint_data_in)))
      BAIL(res);
   for (i=0; i<2; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vars_int64(ncid, varid[3], start, count, 
				stride, int64_data_out)))
      BAIL(res);
   if ((res = nc_get_vars_int64(ncid, varid[3], start, count, 
				stride, int64_data_in)))
      BAIL(res);
   for (i=0; i<2; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vars_uint64(ncid, varid[4], start, count, 
				 stride, uint64_data_out)))
      BAIL(res);
   if ((res = nc_get_vars_uint64(ncid, varid[4], start, count,
				 stride, uint64_data_in)))
      BAIL(res);
   for (i=0; i<2; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vars_bool(ncid, varid[5], start, count, 
			       stride, bool_data_out)))
      BAIL(res);
   if ((res = nc_get_vars_bool(ncid, varid[5], start, 
			       count, stride, bool_data_in)))
      BAIL(res);
   for (i=0; i<2; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   total_errors += errors;
   if (errors)
      printf("*** ERROR!! - %d errors. Sorry!\n");
   else
      printf("ok!\n");

   /* Test the vara functions. */
   printf("*** testing vara functions...\t\t\t");
   CLEAN_INPUT_BUFFERS;
   errors = 0;
   start[0] = 0;
   count[0] = SIZE;

   if ((res = nc_put_vara_uchar(ncid, varid[0], start, count, ubyte_data_out)))
      BAIL(res);
   if ((res = nc_get_vara_uchar(ncid, varid[0], start, count, ubyte_data_in)))
      BAIL(res);
   for (i=0; i<SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vara_ushort(ncid, varid[1], start, count, ushort_data_out)))
      BAIL(res);
   if ((res = nc_get_vara_ushort(ncid, varid[1], start, count, ushort_data_in)))
      BAIL(res);
   for (i=0; i<SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vara_uint(ncid, varid[2], start, count, uint_data_out)))
      BAIL(res);
   if ((res = nc_get_vara_uint(ncid, varid[2], start, count, uint_data_in)))
      BAIL(res);
   for (i=0; i<SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vara_int64(ncid, varid[3], start, count, int64_data_out)))
      BAIL(res);
   if ((res = nc_get_vara_int64(ncid, varid[3], start, count, int64_data_in)))
      BAIL(res);
   for (i=0; i<SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vara_uint64(ncid, varid[4], start, count, uint64_data_out)))
      BAIL(res);
   if ((res = nc_get_vara_uint64(ncid, varid[4], start, count, uint64_data_in)))
      BAIL(res);
   for (i=0; i<SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   if ((res = nc_put_vara_bool(ncid, varid[5], start, count, bool_data_out)))
      BAIL(res);
   if ((res = nc_get_vara_bool(ncid, varid[5], start, count, bool_data_in)))
      BAIL(res);
   for (i=0; i<SIZE; i++)
      if (ubyte_data_in[i] != ubyte_data_out[i]) ERR;

   total_errors += errors;
   if (errors)
      printf("*** ERROR!! - %d errors. Sorry!\n");
   else
      printf("ok!\n");

   /* Test the var1 functions. */
   printf("*** testing var1 functions...\t\t\t");
   CLEAN_INPUT_BUFFERS;
   errors = 0;
   index1[0] = 0;

   if ((res = nc_put_var1_uchar(ncid, varid[0], index1, uchar_data_out)))
      BAIL(res);
   if ((res = nc_get_var1_uchar(ncid, varid[0], index1, uchar_data_in)))
      BAIL(res);
   if (uchar_data_in[0] != uchar_data_out[0]) ERR;

   if ((res = nc_put_var1_ushort(ncid, varid[1], index1, ushort_data_out)))
      BAIL(res);
   if ((res = nc_get_var1_ushort(ncid, varid[1], index1, ushort_data_in)))
      BAIL(res);
   if (ushort_data_in[0] != ushort_data_out[0]) ERR;

   if ((res = nc_put_var1_uint(ncid, varid[2], index1, uint_data_out)))
      BAIL(res);
   if ((res = nc_get_var1_uint(ncid, varid[2], index1, uint_data_in)))
      BAIL(res);
   if (uint_data_in[0] != uint_data_out[0]) ERR;

   if ((res = nc_put_var1_int64(ncid, varid[3], index1, int64_data_out)))
      BAIL(res);
   if ((res = nc_get_var1_int64(ncid, varid[3], index1, int64_data_in)))
      BAIL(res);
   if (int64_data_in[0] != int64_data_out[0]) ERR;

   if ((res = nc_put_var1_uint64(ncid, varid[4], index1, uint64_data_out)))
      BAIL(res);
   if ((res = nc_get_var1_uint64(ncid, varid[4], index1, uint64_data_in)))
      BAIL(res);
   if (uint64_data_in[0] != uint64_data_out[0]) ERR;

   if ((res = nc_put_var1_bool(ncid, varid[5], index1, bool_data_out)))
      BAIL(res);
   if ((res = nc_get_var1_bool(ncid, varid[5], index1, bool_data_in)))
      BAIL(res);
   if (bool_data_in[0] != bool_data_out[0]) ERR;

   total_errors += errors;
   if (errors)
      printf("*** ERROR!! - %d errors. Sorry!\n");
   else
      printf("ok!\n");

   if (total_errors)
      printf(" *** %d total errors\n", errors);
   else
      printf(" *** success!\n");

#ifdef USE_PARALLEL
   MPI_Finalize();
#endif   

   return 2 ? errors : 0;
}
