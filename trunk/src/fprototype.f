C	$Id$
C	Example of using GMTAPI from Fortran
C
C	We will need some sort of include file to set the parameters later

	program fprototype
	parameter (GMTAPI_N_ARRAY_ARGS=8, GMTAPI_FLOAT=3, GMTAPI_ARRAY=3,
     +  GMTAPI_FDESC=2)
	parameter (NROW=4, NCOL=3, NREC=6)
	integer row, col, unit6
	integer error, val
	real*8 array_par(12)
	integer inarg(2)
	integer outarg
	real*4 matrix(3,4)
	
	call GMTAPI_Create_Session (0, error)
	if (error .ne. 0) then
		call GMTAPI_Report_Error (error)
	endif

C	TEST 1: Create a 2-D array and register it with GMT 
	
	do 10 row = 1,NROW
		do 20 col = 1,NCOL
			matrix(row,col) = row*NCOL+col
   20		continue
   10	continue
	array_par(1) = GMTAPI_FLOAT
	array_par(2) = 2
	array_par(3) = NROW
	array_par(4) = NCOL
	array_par(5) = 1
	array_par(6) = array_par(4)
	array_par(7) = 1
	val = GMTAPI_ARRAY
	call GMTAPI_Register_Import (val, matrix,
     +	array_par, val, error)
	inarg(1) = val
	inarg(2) = 0
	write (*,30) 'fprototype.f: Reg 2D array as ID = ', inarg(1)
   30	format (a, i2)
	
C	Register stdout as the output
	
	unit6 = 6
	val = GMTAPI_FDESC
	call GMTAPI_Register_Export (val, unit6, array_par,
     +	outarg, error)
	write (*,30) 'fprototype.f: Registered output as ID =', outarg

C	Call the GMT function
	
	call GMT_read_all_write_all_records ('no command yet',
     +	inarg, outarg, error)
	if (error .ne. 0) then
		call GMTAPI_Report_Error (error)
	endif

C	Shut down everything
	
	call GMTAPI_Destroy_Session (error)
	if (error .ne. 0) then
		call GMTAPI_Report_Error (error)
	endif
	stop
	end
