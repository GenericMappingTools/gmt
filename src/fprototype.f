C	$Id: fprototype.f,v 1.1 2006-03-26 10:56:13 pwessel Exp $
C	Example of using GMTAPI from Fortran
C
C	We will need some sort of include file to set the parameters later

	parameter (GMTAPI_N_ARRAY_ARGS=8, GMTAPI_FLOAT=3, GMTAPI_ARRAY=8, GMTAPI_FDESC=4)
	integer row, col, unit6
	integer error = 0
	integer array_par(GMTAPI_N_ARRAY_ARGS)
	integer inarg(2)
	integer outarg
	real matrix(3,4)
	
	error = GMT_Create_Session (0)
	if (error .ne. 0) then
		GMT_Error (error)
	endif

C	Create a 2-D array and register it with GMT 
	
	do 10 row = 1,4
		do 20 col = 1, 2
			matrix(row,col) = row*3+col
   20		continue
   10	continue
	array_par(1) = GMTAPI_FLOAT
	array_par(2) = 2
	array_par(3) = 4
	array_par(4) = 3
	array_par(5) = 1
	array_par(6) = array_par(4)
	array_par(7) = 1
	inarg(0) = GMT_Register_Input (GMTAPI_ARRAY, matrix, array_par)
	inarg(1) = 0
	write (*,20) 'fprototype.f: Registered 2D array as ID = ', inarg(0)
   20	format (a, i2)
	
C	Register stdout as the output
	
	unit6 = 6
	outarg = GMT_Register_Output (GMTAPI_FDESC, unit6, array_par)
	write (*,20) 'fprototype.f: Registered output as ID =', outarg

C	Call the GMT function
	
	error = GMT_copy_all ('no command yet', inarg, outarg)
	if (error .ne. 0) then
		GMT_Error (error)
	endif

C	Shut down everything
	
	error = GMT_Destroy_Session ()
	if (error .ne. 0) then
		GMT_Error (error)
	endif
	stop
	end

	exit (EXIT_SUCCESS)	
