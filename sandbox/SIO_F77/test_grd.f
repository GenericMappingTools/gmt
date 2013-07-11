c
      program test_grd
c
c*****************   main program   ************************
c
c Program to read and write a grd file
c
c***********************************************************
c
      implicit real*8(a,b,d-h,o-z)
      implicit complex*16 (c)
c
c  change ni and nj as needed
c
      parameter(ntot=4096*4096)
      character*80 cin,cout,title
c
      real*4 rin(ntot),rout(ntot)
c
c   get values from command line
c
      narg = iargc()
      if(narg.lt.2) then
        write(*,'(a)')'  '
        write(*,'(a)')
     & 'Usage: test_grd input.grd output.grd'
        write(*,'(a)')
        write(*,'(a)')
     &  '       input.grd  - inout grd file'
        write(*,'(a)')
     &  '       output.grd - output grd file'
        write(*,'(a)')
        stop
      else 
        call getarg(1,cin)
        call getarg(2,cout)
      endif
c
c   read the grd file
c
      call readgrd(rin,nj,ni,rlt0,rln0,
     +            dlt,dln,rdum,title,trim(cin)//char(0))
c
c   copy the input image into the output image and keep track of the maximum
c
      rmax=-9999
      do 100 i=1,ntot
      rout(i)=rin(i)*2
      if(rout(i).gt.rmax) rmax=rout(i)
 100  continue
c
c  write out grd file
c
      rland=1.001*rmax
      rdum=1.001*rmax
      call writegrd(rout,nj,ni,rlt0,rln0,dlt,dln,rland,rdum,
     +              trim(cout)//char(0),trim(cout)//char(0))
      
      stop
      end
