C	$Id: fprototype.f,v 1.3 2006-03-28 22:58:47 pwessel Exp $
C	Example of using GMTAPI from Fortran
C
C	We will need some sort of include file to set the parameters later

	program fprototype
	parameter (GMTAPI_N_ARRAY_ARGS=8, GMTAPI_FLOAT=3, GMTAPI_ARRAY=8, GMTAPI_FDESC=4)
	parameter (NROW=4, NCOL=3, NREC=6)
	integer row, col, unit6
	integer error
C	integer array_par(GMTAPI_N_ARRAY_ARGS)
	integer array_par(8)
	integer inarg(2)
	integer outarg
	real matrix(3,4)
	
	error = GMTAPI_Create_Session (0)
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
	inarg(1) = GMTAPI_Register_Import (GMTAPI_ARRAY, matrix, array_par)
	inarg(2) = 0
	write (*,30) 'fprototype.f: Registered 2D array as ID = ', inarg(1)
   30	format (a, i2)
	
C	Register stdout as the output
	
	unit6 = 6
	outarg = GMTAPI_Register_Export (GMTAPI_FDESC, unit6, array_par)
	write (*,30) 'fprototype.f: Registered output as ID =', outarg

C	Call the GMT function
	
	error = GMT_read_all_write_all_records ('no command yet', inarg, outarg)
	if (error .ne. 0) then
		call GMTAPI_Report_Error (error)
	endif

C	Shut down everything
	
	error = GMTAPI_Destroy_Session ()
	if (error .ne. 0) then
		call GMTAPI_Report_Error (error)
	endif
	stop
	end
