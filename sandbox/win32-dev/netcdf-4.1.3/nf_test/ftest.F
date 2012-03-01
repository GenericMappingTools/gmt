!********************************************************************
!   Copyright 1993, UCAR/Unidata
!   See netcdf/COPYRIGHT file for copying and redistribution conditions.
!   $Id$
!********************************************************************

#include "nfconfig.inc"

!
!     program to test the netCDF-2 Fortran API
!
      program ftest

#include "netcdf.inc"

!     name of first test cdf
      character*31 name
!     name of second test cdf
      character*31 name2
      
!     Returned error code.
      integer iret 
!     netCDF ID
      integer ncid
!     ID of dimension lat
      integer  latdim
!     ID of dimension lon
      integer londim
!     ID of dimension level
      integer leveldim
!     ID of dimension time
      integer timedim
!     ID of dimension len
      integer lendim

!     Count the errors.
      integer nfails

!     variable used to control error-handling behavior
      integer ncopts
      integer dimsiz(MAXNCDIM)
!      allowable roundoff 
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz
      data name/'test.nc'/
      data name2/'copy.nc'/

      print *, ''
      print *,'*** Testing netCDF-2 Fortran 77 API.'

100   format(' *** testing ', a, ' ...')
!     set error-handling to verbose and non-fatal
      ncopts = NCVERBOS
      call ncpopt(ncopts)

!     This will be a count of how many failures we experience.
      nfails = 0

!     create a netCDF named 'test.nc'
      write(*,100) 'nccre'
      ncid = nccre(name, NCCLOB, iret)
      if (ncid .eq. -1) then nfails = nfails + 1

!     test ncddef
      write(*,100) 'ncddef'
      call tncddef(ncid, nfails)

!     test ncvdef
      write(*,100) 'ncvdef'
      call tncvdef(ncid, nfails)

!     test ncapt
      write(*, 100) 'ncapt, ncaptc'
      call tncapt(ncid, nfails)

!     close 'test.nc'
      write(*, 100) 'ncclos'
      call ncclos(ncid, iret)
      if (ncid .eq. -1) then nfails = nfails + 1

!     test ncvpt1
      write(*, 100) 'ncvpt1'
      call tncvpt1(name, nfails)

!     test ncvgt1
      write(*, 100) 'ncvgt1'
      call tncvgt1(name, nfails)

!     test ncvpt
      write(*, 100) 'ncvpt'
      call tncvpt(name, nfails)

!     test ncinq
      write(*, 100) 'ncopn, ncinq, ncdinq, ncvinq, ncanam, ncainq'
      call tncinq(name, nfails)

!     test ncvgt
      write(*, 100) 'ncvgt, ncvgtc'
      call tncvgt(name, nfails)

!     test ncagt
      write(*, 100) 'ncagt, ncagtc'
      call tncagt(name, nfails)

!     test ncredf
      write(*, 100) 'ncredf, ncdren, ncvren, ncaren, ncendf'
      call tncredf(name, nfails)

      call tncinq(name, nfails)

!     test ncacpy
      write(*, 100) 'ncacpy'
      call tncacpy(name, name2, nfails)

!     test ncadel
      write(*, 100) 'ncadel'
      call tncadel(name2, nfails)

!     test fill values
      write(*, 100) 'fill values'
      call tfills(nfails)

      print *,'Total number of failures: ', nfails
      if (nfails .ne. 0) stop 2

      print *,'*** SUCCESS!'

      end
!
!     subroutine to test ncacpy
!
      subroutine tncacpy(iname, oname, nfails)
      character*31 iname, oname
#include "netcdf.inc"
      integer ndims, nvars, natts, recdim, iret
      character*31 vname, attnam
      integer attype, attlen
      integer vartyp, nvdims, vdims(MAXVDIMS), nvatts
      integer lenstr
!     existing netCDF id
      integer incdf
!     netCDF id of the output netCDF file to which the attribute
!     will be copied
      integer outcdf

      integer mattlen
      parameter (mattlen = 80)
      character*80 charval
      doubleprecision doubval(2)
      real flval(2)
      integer lngval(2)
      NCSHORT_T shval(2)
      integer i, j, k
      character*31 varnam, attname(2,7), gattnam(2)
      NCBYTE_T bytval(2)
      common /atts/attname, gattnam
      NCSHORT_T svalidrg(2)
      real rvalidrg(2)
      integer lvalidrg(2)
      doubleprecision dvalidrg(2)
      NCBYTE_T bvalidrg(2)
      character*31 gavalue(2), cavalue(2)
      real epsilon

      data bvalidrg/-127,127/
      data svalidrg/-100,100/
      data lvalidrg/0,360/
      data rvalidrg/0.0, 5000.0/
      data dvalidrg/0D0,500D0/
      data gavalue/'NWS', '88/10/25 12:00:00'/
      data cavalue/'test string', 'a'/
      data lenstr/80/   
      data epsilon /.000001/

      incdf = ncopn(iname, NCNOWRIT, iret)
      if (iret .ne. 0) nfails = nfails + 1

      outcdf = nccre(oname, NCCLOB, iret)
      if (iret .ne. 0) nfails = nfails + 1

      call tncddef(outcdf, nfails)
      call tncvdef(outcdf, nfails)
      call ncinq (incdf, ndims, nvars, natts, recdim, iret)
      if (iret .ne. 0) nfails = nfails + 1
      do 5 j = 1, natts
         call ncanam (incdf, NCGLOBAL, j, attnam, iret)
         if (iret .ne. 0) nfails = nfails + 1
         call ncacpy (incdf, NCGLOBAL, attnam, outcdf, NCGLOBAL, iret)
         if (iret .ne. 0) nfails = nfails + 1
 5    continue
      do 10 i = 1, nvars
         call ncvinq (incdf, i, vname, vartyp, nvdims,
     +        vdims, nvatts, iret)
         if (iret .ne. 0) nfails = nfails + 1
         do 20 k = 1, nvatts
            call ncanam (incdf, i, k, attnam, iret)
            if (iret .ne. 0) nfails = nfails + 1
            call ncacpy (incdf, i, attnam, outcdf, i, iret)
            if (iret .ne. 0) nfails = nfails + 1
 20      continue
 10   continue
!     
!     get global attributes first
!     
      do 100 i = 1, natts
         call ncanam (outcdf, NCGLOBAL, i, attnam, iret)
         if (iret .ne. 0) nfails = nfails + 1
         call ncainq (outcdf, NCGLOBAL, attnam, attype, attlen,
     +        iret)
         if (iret .ne. 0) nfails = nfails + 1
         if (attlen .gt. mattlen) then
            write (*,*) 'global attribute too big!', attlen, mattlen
            stop 2
         else if (attype .eq. NCBYTE) then
            call ncagt (outcdf, NCBYTE, attnam, bytval, iret)
            if (iret .ne. 0) nfails = nfails + 1
         else if (attype .eq. NCCHAR) then
            call ncagtc (outcdf, NCGLOBAL, attnam, charval, 
     +           lenstr, iret)
            if (iret .ne. 0) nfails = nfails + 1
            if (attnam .ne. gattnam(i)) write(*,*) 'error in ncagt G'
            if (charval .ne. gavalue(i))
     + write(*,*) 'error in ncagt G2', lenstr, charval, gavalue(i)
                  charval = ' '
         else if (attype .eq. NCSHORT) then
            call ncagt (outcdf, NCGLOBAL, attnam, shval, iret)
            if (iret .ne. 0) nfails = nfails + 1
         else if (attype .eq. NCLONG) then
            call ncagt (outcdf, NCGLOBAL, attnam, lngval, iret)            
            if (iret .ne. 0) nfails = nfails + 1
         else if (attype .eq. NCFLOAT) then
            call ncagt (outcdf, NCGLOBAL, attnam, flval, iret)
            if (iret .ne. 0) nfails = nfails + 1
         else 
            call ncagt (outcdf, NCGLOBAL, attnam, doubval,iret)          
            if (iret .ne. 0) nfails = nfails + 1
         end if
 100   continue
!
!     get variable attributes
!
      do 200 i = 1, nvars
         call ncvinq (outcdf, i, varnam, vartyp, nvdims, vdims,
     +                nvatts, iret)
         if (iret .ne. 0) nfails = nfails + 1
         do 250 j = 1, nvatts
            call ncanam (outcdf, i, j, attnam, iret)
            if (iret .ne. 0) nfails = nfails + 1
            call ncainq (outcdf, i, attnam, attype, attlen,
     +                   iret)
            if (iret .ne. 0) nfails = nfails + 1
            if (attlen .gt. mattlen) then
               write (*,*) 'variable ', i,  'attribute too big !'
               stop 2
            else 
               if (attype .eq. NCBYTE) then
                  call ncagt (outcdf, i, attnam, bytval, 
     +                 iret)
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i))
     +               write(*,*) 'error in ncagt BYTE N'
                  if (bytval(j) .ne. bvalidrg(j)) write(*,*)
     + 'ncacpy: byte ', bytval(j), ' .ne. ', bvalidrg(j)
               else if (attype .eq. NCCHAR) then
                  call ncagtc (outcdf, i, attnam, charval, 
     +                 lenstr, iret)
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt CHAR N'
                  if (charval .ne. cavalue(j)) 
     +               write(*,*) 'error in ncagt'
                  charval = ' '
               else if (attype .eq. NCSHORT) then
                  call ncagt (outcdf, i, attnam, shval, 
     +                 iret)  
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt SHORT N'
                  if (shval(j) .ne. svalidrg(j)) then
                     write(*,*) 'error in ncagt SHORT'
                  end if
               else if (attype .eq. NCLONG) then
                  call ncagt (outcdf, i, attnam, lngval, 
     +                 iret)
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt LONG N'
                  if (lngval(j) .ne. lvalidrg(j)) 
     +               write(*,*) 'error in ncagt LONG'
               else if (attype .eq. NCFLOAT) then
                  call ncagt (outcdf, i, attnam, flval, 
     +                 iret)            
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt FLOAT N'
                  if (flval(j) .ne. rvalidrg(j)) 
     +               write(*,*) 'error in ncagt FLOAT'
               else if (attype .eq. NCDOUBLE) then
                  call ncagt (outcdf, i, attnam, doubval,
     +                 iret)          
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt DOUBLE N'
                  if ( abs(doubval(j) - dvalidrg(j)) .gt. epsilon)
     + write(*,*) 'error in ncagt DOUBLE'
               end if
            end if
 250     continue
 200   continue
      call ncclos(incdf, iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncclos(outcdf, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end


      
!     
!     subroutine to test ncadel
!
      subroutine tncadel (cdfname, nfails)
      character*31 cdfname
#include "netcdf.inc"
      
      integer  bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid
      integer ncid, iret, i, j
      integer ndims, nvars, natts, recdim
      integer vartyp, nvdims, vdims(MAXVDIMS), nvatts
      character*31 varnam, attnam

      ncid = ncopn(cdfname, NCWRITE, iret)
      if (iret .ne. 0) nfails = nfails + 1
!     put cdf in define mode
      call ncredf (ncid,iret)
      if (iret .ne. 0) nfails = nfails + 1
!     get number of global attributes
      call ncinq (ncid, ndims, nvars, natts, recdim, iret)
      if (iret .ne. 0) nfails = nfails + 1
      do 10 i = natts, 1, -1
!     get name of global attribute
         call ncanam (ncid, NCGLOBAL, i, attnam, iret)
         if (iret .ne. 0) nfails = nfails + 1
!     delete global attribute
         call ncadel (ncid, NCGLOBAL, attnam, iret)
         if (iret .ne. 0) nfails = nfails + 1
 10   continue

      do 100 i = 1, nvars
!     get number of variable attributes
         call ncvinq (ncid, i, varnam, vartyp, nvdims, vdims,
     +        nvatts, iret)
         if (iret .ne. 0) nfails = nfails + 1
         do 200 j = nvatts, 1, -1
            call ncanam (ncid, i, j, attnam, iret)
            if (iret .ne. 0) nfails = nfails + 1
            call ncadel (ncid, i, attnam, iret)
            if (iret .ne. 0) nfails = nfails + 1
 200     continue
 100  continue
      call ncinq (ncid, ndims, nvars, natts, recdim, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (natts .ne. 0) write(*,*) 'error in ncadel'
!     put netCDF into data mode
      call ncendf (ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncclos (ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end

!
!     subroutine to test ncagt and ncagtc

      subroutine tncagt(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname
            
!     maximum length of an attribute
      integer mattlen
      parameter (mattlen = 80)
      integer ncid, ndims, nvars, natts, recdim
      integer bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid
      integer i, j
      integer attype, attlen, lenstr, iret
      character*31 attnam
      character*80 charval
      doubleprecision doubval(2)
      real flval(2)
      integer lngval(2)
      NCSHORT_T shval(2)
      NCBYTE_T bytval(2)
      integer vartyp, nvdims, vdims(MAXVDIMS), nvatts

      character*31 varnam, attname(2,7), gattnam(2)
      common /atts/attname, gattnam
      NCSHORT_T svalidrg(2)
      real rvalidrg(2)
      integer lvalidrg(2)
      doubleprecision dvalidrg(2)
      NCBYTE_T bvalidrg(2)
      character*31 gavalue(2), cavalue(2)
      real epsilon

      data bvalidrg/-127,127/
      data svalidrg/-100,100/
      data lvalidrg/0,360/
      data rvalidrg/0.0, 5000.0/
      data dvalidrg/0D0,500D0/
      data gavalue/'NWS', '88/10/25 12:00:00'/
      data cavalue/'test string', 'a'/
      data lenstr/80/   
      data epsilon /.000001/
      
      ncid = ncopn (cdfname, NCNOWRIT, iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncinq (ncid, ndims, nvars, natts, recdim, iret)
      if (iret .ne. 0) nfails = nfails + 1
!     
!     get global attributes first
!     
      do 10 i = 1, natts
!     get name of attribute
         call ncanam (ncid, NCGLOBAL, i, attnam, iret)
         if (iret .ne. 0) nfails = nfails + 1
!     get attribute type and length
         call ncainq (ncid, NCGLOBAL, attnam, attype, attlen,
     +        iret)
         if (iret .ne. 0) nfails = nfails + 1
         if (attlen .gt. mattlen) then
            write (*,*) 'global attribute too big!'
            stop 2
         else if (attype .eq. NCBYTE) then
            call ncagt (ncid, NCBYTE, attnam, bytval, iret)
            if (iret .ne. 0) nfails = nfails + 1
         else if (attype .eq. NCCHAR) then
            call ncagtc (ncid, NCGLOBAL, attnam, charval, 
     +           lenstr, iret)
            if (iret .ne. 0) nfails = nfails + 1
            if (attnam .ne. gattnam(i)) write(*,*) 'error in ncagt'
            if (charval .ne. gavalue(i)) write(*,*) 'error in ncagt'
            charval = '                                        '
         else if (attype .eq. NCSHORT) then
            call ncagt (ncid, NCGLOBAL, attnam, shval, iret) 
            if (iret .ne. 0) nfails = nfails + 1
         else if (attype .eq. NCLONG) then
            call ncagt (ncid, NCGLOBAL, attnam, lngval, iret)            
            if (iret .ne. 0) nfails = nfails + 1
         else if (attype .eq. NCFLOAT) then
            call ncagt (ncid, NCGLOBAL, attnam, flval, iret)
            if (iret .ne. 0) nfails = nfails + 1
         else 
            call ncagt (ncid, NCGLOBAL, attnam, doubval,iret)          
            if (iret .ne. 0) nfails = nfails + 1
         end if
 10   continue

!
!     get variable attributes
!
      do 20 i = 1, nvars
         call ncvinq (ncid, i, varnam, vartyp, nvdims, vdims,
     +                nvatts, iret)
         if (iret .ne. 0) nfails = nfails + 1
         do 25 j = 1, nvatts
            call ncanam (ncid, i, j, attnam, iret)
            if (iret .ne. 0) nfails = nfails + 1
            call ncainq (ncid, i, attnam, attype, attlen,
     +                   iret)
            if (iret .ne. 0) nfails = nfails + 1
            if (attlen .gt. mattlen) then
               write (*,*) 'variable ', i,  'attribute too big !'
               stop 2
            else 
               if (attype .eq. NCBYTE) then
                  call ncagt (ncid, i, attnam, bytval, 
     +                 iret)
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt BYTE name'
                  if (bytval(j) .ne. bvalidrg(j)) write(*,*)
     + 'ncacpy: byte ', bytval(j), ' .ne. ', bvalidrg(j)
               else if (attype .eq. NCCHAR) then
                  call ncagtc (ncid, i, attnam, charval, 
     +                 lenstr, iret)
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt CHAR name'
                  if (charval .ne. cavalue(j)) 
     +               write(*,*) 'error in ncagt CHAR name'
                 charval = '                                        '
               else if (attype .eq. NCSHORT) then
                  call ncagt (ncid, i, attnam, shval, 
     +                 iret)  
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt SHORT name'
                  if (shval(j) .ne. svalidrg(j)) then
                     write(*,*) 'error in ncagt SHORT'
                  end if
               else if (attype .eq. NCLONG) then
                  call ncagt (ncid, i, attnam, lngval, 
     +                 iret)
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt LONG name'
                  if (lngval(j) .ne. lvalidrg(j)) 
     +               write(*,*) 'error in ncagt LONG'
               else if (attype .eq. NCFLOAT) then
                  call ncagt (ncid, i, attnam, flval, 
     +                 iret)            
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt FLOAT name'
                  if (flval(j) .ne. rvalidrg(j)) 
     +               write(*,*) 'error in ncagt FLOAT'
               else if (attype .eq. NCDOUBLE) then
                  call ncagt (ncid, i, attnam, doubval,
     +                 iret)          
                  if (iret .ne. 0) nfails = nfails + 1
                  if (attnam .ne. attname(j,i)) 
     +               write(*,*) 'error in ncagt DOUBLE name'
                  if ( abs(doubval(j) - dvalidrg(j)) .gt. epsilon)
     + write(*,*) 'error in ncagt DOUBLE'
               end if
            end if
 25      continue
 20   continue
      call ncclos(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end
!
!     subroutine to test ncapt
!
      subroutine tncapt (ncid, nfails)
#include "netcdf.inc"
      integer ncid, iret

! attribute vectors
      NCSHORT_T svalidrg(2)
      real rvalidrg(2)
      integer lvalidrg(2)
      doubleprecision dvalidrg(2)
      NCBYTE_T bvalidrg(2)

!     variable ids
      integer  bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid

! assign attributes
      
!
!     byte
!
      
      bvalidrg(1) = -127
      bvalidrg(2) =  127
      call ncapt (ncid, bid, 'validrange', NCBYTE, 2,
     +bvalidrg, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     short
!

      svalidrg(1) = -100
      svalidrg(2) = 100
      call ncapt (ncid, sid, 'validrange', NCSHORT, 2, 
     +svalidrg, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     long
!

      lvalidrg(1) = 0
      lvalidrg(2) = 360
      call ncapt (ncid, lid, 'validrange', NCLONG, 2,
     +lvalidrg, iret)
      if (iret .ne. 0) nfails = nfails + 1
      
!
!     float
!

      rvalidrg(1) = 0.0
      rvalidrg(2) = 5000.0
      call ncapt (ncid, fid, 'validrange', NCFLOAT, 2,
     +rvalidrg, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     double
!

      dvalidrg(1) = 0D0
      dvalidrg(2) = 500D0
      call ncapt (ncid, did, 'validrange', NCDOUBLE, 2,
     +dvalidrg, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     global
!

      call ncaptc (ncid, NCGLOBAL, 'source', NCCHAR, 3, 
     +'NWS', iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncaptc (ncid, NCGLOBAL, 'basetime', NCCHAR, 17, 
     +'88/10/25 12:00:00', iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     char
!

      call ncaptc (ncid, chid, 'longname', NCCHAR, 11,
     +'test string', iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncaptc (ncid, chid, 'id', NCCHAR, 1,
     +'a', iret)
      if (iret .ne. 0) nfails = nfails + 1

      return
      end

!
!     initialize variables in labelled common blocks
!
      block data
      common /cdims/ dimnam
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz
      common /varn/varnam
      common /atts/attname, gattnam
      integer  latdim, londim, leveldim, timedim, lendim

!     should include 'netcdf.inc' for MAXNCDIM, but it has EXTERNAL
!     declaration, which is not permitted in a BLOCK DATA unit.

      integer dimsiz(1024)
      character*31 dimnam(1024)
      character*31 varnam(7)
      character*31 attname(2,7)
      character*31 gattnam(2)

      data dimnam /'time', 'lat', 'lon', 'level',
     + 'length', 1019*'0'/
      data dimsiz /4, 5, 5, 4, 80, 1019*0/
      data varnam/'bytev', 'shortv', 'longv', 'floatv', 'doublev', 
     + 'chv', 'cv'/
      
      data attname/'validrange', '0', 'validrange', '0', 'validrange',
     + '0', 'validrange', '0', 'validrange', '0', 'longname', 'id',
     + '0', '0'/
      data gattnam/'source','basetime'/
      end


!
!     subroutine to test ncddef
!

      subroutine tncddef(ncid, nfails)
#include "netcdf.inc"
      integer ncid

!     sizes of dimensions of 'test.nc' and 'copy.nc'
      integer  ndims
      parameter(ndims=5)
! dimension ids
      integer  latdim, londim, leveldim, timedim, lendim
      integer iret
!     function to define a netCDF dimension
      integer dimsiz(MAXNCDIM)
      character*31 dimnam(MAXNCDIM)
      
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz
      common /cdims/ dimnam

! define dimensions
      timedim = ncddef(ncid, dimnam(1), NCUNLIM, iret)
      if (iret .ne. 0) nfails = nfails + 1
      latdim = ncddef(ncid, dimnam(2), dimsiz(2), iret)
      if (iret .ne. 0) nfails = nfails + 1
      londim = ncddef(ncid, dimnam(3), dimsiz(3), iret)
      if (iret .ne. 0) nfails = nfails + 1
      leveldim = ncddef(ncid, dimnam(4), dimsiz(4), iret)
      if (iret .ne. 0) nfails = nfails + 1
      lendim = ncddef(ncid, dimnam(5), dimsiz(5), iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end
!
!     subroutine to test ncinq, ncdinq, ncdid, ncvinq, ncanam
!     and ncainq
!
      subroutine tncinq(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname

!     netCDF id
      integer ncid
!     returned number of dimensions
      integer ndims
!     returned number of variables
      integer nvars
!     returned number of global attributes
      integer natts
!     returned id of the unlimited dimension
      integer recdim
!     returned error code
      integer iret
!     returned name of record dimension
      character*31 recnam
!     returned size of record dimension
      integer recsiz
!     loop control variables
      integer i, j, k
!     returned size of dimension
      integer dsize
!     returned dimension ID
      integer dimid
!     returned dimension name
      character*31 dname
!     returned variable name
      character*31 vname
!     returned attribute name
      character*31 attnam
!     returned netCDF datatype of variable
      integer vartyp
!     returned number of variable dimensions
      integer nvdims
!     returned number of variable attributes
      integer nvatts
!     returned vector of nvdims dimension IDS corresponding to the
!     variable dimensions
      integer vdims(MAXNCDIM)
!     returned attribute length
      integer attlen
!     returned attribute type
      integer attype
      character*31 dimnam(MAXNCDIM)
      character*31 varnam(7)
      character*31 attname(2,7)
      character*31 gattnam(2)
      integer vdlist(5,7), vtyp(7), vndims(7), vnatts(7)
      integer attyp(2,7),atlen(2,7),gattyp(2),gatlen(2)
      integer timedim,latdim,londim,leveldim,lendim
      integer dimsiz(MAXNCDIM)
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz
      common /varn/varnam
      common /atts/attname, gattnam
      common /cdims/ dimnam

      data vdlist/1,0,0,0,0,1,0,0,0,0,2,0,0,0,0,4,3,2,1,0,4,3,2,1,0,
     + 5,1,0,0,0,1,0,0,0,0/
      data vtyp/NCBYTE, NCSHORT, NCLONG, NCFLOAT, NCDOUBLE, NCCHAR,
     + NCCHAR/
      data vndims/1,1,1,4,4,2,1/
      data vnatts/1,1,1,1,1,2,0/
      data attyp/NCBYTE, 0, NCSHORT, 0, NCLONG, 0, NCFLOAT, 0,
     + NCDOUBLE, 0, NCCHAR, NCCHAR, 0, 0/
      data atlen/2,0,2,0,2,0,2,0,2,0,11,1, 0, 0/
      data gattyp/NCCHAR,NCCHAR/
      data gatlen/3,17/

      ncid = ncopn (cdfname, NCNOWRIT, iret)
      call ncinq (ncid, ndims, nvars, natts, recdim, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (ndims .ne. 5) write(*,*) 'error in ncinq or ncddef'
      if (nvars .ne. 7) write(*,*) 'error in ncinq or ncvdef'
      if (natts .ne. 2) write(*,*) 'error in ncinq or ncapt'
      call ncdinq (ncid, recdim, recnam, recsiz, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (recnam .ne. 'time') write(*,*) 'error: bad recdim from ncinq'
!
!     dimensions
!
      do 10 i = 1, ndims
         call ncdinq (ncid, i, dname, dsize, iret)
         if (iret .ne. 0) nfails = nfails + 1
         if (dname .ne. dimnam(i)) 
     +      write(*,*) 'error in ncdinq or ncddef, dname=', dname
         if (dsize .ne. dimsiz(i)) 
     +      write(*,*) 'error in ncdinq or ncddef, dsize=',dsize
         dimid = ncdid (ncid, dname, iret)
         if (dimid .ne. i) write(*,*)
     +      'error in ncdinq or ncddef, dimid=', dimid
 10   continue
!
!     variables
!
      do 30 i = 1, nvars
         call ncvinq (ncid, i, vname, vartyp, nvdims,
     +        vdims, nvatts, iret)
         if (iret .ne. 0) nfails = nfails + 1
         if (vname .ne. varnam(i)) 
     +      write(*,*) 'error: from ncvinq, wrong name returned: ', 
     +         vname, ' .ne. ', varnam(i)
         if (vartyp .ne. vtyp(i)) 
     +      write(*,*) 'error: from ncvinq, wrong type returned: ', 
     +         vartyp, ' .ne. ', vtyp(i)
         if (nvdims .ne. vndims(i)) 
     +      write(*,*) 'error: from ncvinq, wrong num dims returned: ', 
     +         vdims, ' .ne. ', vndims(i)
         do 35 j = 1, nvdims
            if (vdims(j) .ne. vdlist(j,i)) 
     +         write(*,*) 'error: from ncvinq wrong dimids: ',
     +            vdims(j), ' .ne. ', vdlist(j,i)
 35      continue
         if (nvatts .ne. vnatts(i)) 
     +      write(*,*) 'error in ncvinq or ncvdef'
!
!     attributes
!
         do 45 k = 1, nvatts
            call ncanam (ncid, i, k, attnam, iret)
            if (iret .ne. 0) nfails = nfails + 1
            call ncainq (ncid, i, attnam, attype, attlen, iret)
            if (iret .ne. 0) nfails = nfails + 1
            if (attnam .ne. attname(k,i)) 
     +         write(*,*) 'error in ncanam or ncapt'
            if (attype .ne. attyp(k,i)) 
     +         write(*,*) 'error in ncainq or ncapt'
            if (attlen .ne. atlen(k,i)) 
     +         write(*,*) 'error in ncainq or ncapt'
 45      continue
 30   continue
      do 40 i = 1, natts
         call ncanam (ncid, NCGLOBAL, i, attnam, iret)
         if (iret .ne. 0) nfails = nfails + 1
         call ncainq (ncid, NCGLOBAL, attnam, attype, attlen, iret)
         if (iret .ne. 0) nfails = nfails + 1
         if (attnam .ne. gattnam(i)) 
     +      write(*,*) 'error in ncanam or ncapt'
         if (attype .ne. gattyp(i)) 
     +      write(*,*) 'error in ncainq or ncapt'
         if (attlen .ne. gatlen(i)) 
     +      write(*,*) 'error in ncainq or ncapt'
 40   continue
      call ncclos(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end
      
      
      
!     subroutine to test ncredf, ncdren, ncvren, ncaren, and 
!     ncendf

      subroutine tncredf(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname
      character*31 attname(2,7)
      character*31 gattnam(2)
      common /atts/attname, gattnam
      common /cdims/ dimnam
      character*31 dimnam(MAXNCDIM)
      character*31 varnam(7)
      common /varn/varnam
      integer ncid, iret, latid, varid

      dimnam(2) = 'latitude'
      varnam(4) = 'realv'
      attname(1,6) = 'stringname'
      gattnam(1) = 'agency'
      ncid = ncopn(cdfname, NCWRITE, iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncredf(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      latid = ncdid(ncid, 'lat', iret)
      call ncdren(ncid, latid, 'latitude', iret)
      if (iret .ne. 0) nfails = nfails + 1
      varid = ncvid(ncid, 'floatv', iret)
      call ncvren(ncid, varid, 'realv', iret)
      if (iret .ne. 0) nfails = nfails + 1
      varid = ncvid(ncid, 'chv', iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncaren(ncid, varid, 'longname', 'stringname', iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncaren(ncid, NCGLOBAL, 'source', 'agency', iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncendf(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      call ncclos(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end
!     
!     subroutine to test ncvdef
!

      subroutine tncvdef(ncid, nfails)
#include "netcdf.inc"
      integer ncid

!     function to define a netCDF variable
      integer dimsiz(MAXNCDIM)
      integer  latdim, londim, leveldim, timedim, lendim
      common /dims/timedim, latdim, londim, leveldim, lendim, 
     + dimsiz

! variable ids
      integer  bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid

! variable shapes
      integer  bdims(1), fdims(4), ddims(4), ldims(1), sdims(1) 
      integer chdims(2), cdims(1)

      integer iret
!
! define variables
!
!     byte
! 
      bdims(1) = timedim
      bid = ncvdef(ncid, 'bytev', NCBYTE, 1, bdims, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     short
!
      sdims(1) = timedim
      sid = ncvdef (ncid, 'shortv', NCSHORT, 1, sdims, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     long
!
      ldims(1) = latdim
      lid = ncvdef (ncid, 'longv', NCLONG, 1, ldims, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     float
!
      fdims(4) = timedim
      fdims(1) = leveldim
      fdims(2) = londim
      fdims(3) = latdim
      fid = ncvdef (ncid, 'floatv', NCFLOAT, 4, fdims, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     double
!
      ddims(4) = timedim
      ddims(1) = leveldim
      ddims(2) = londim
      ddims(3) = latdim
      did = ncvdef (ncid, 'doublev', NCDOUBLE, 4, ddims, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     char
!
      chdims(2) = timedim
      chdims(1) = lendim
      chid = ncvdef (ncid, 'chv', NCCHAR, 2, chdims, iret)
      if (iret .ne. 0) nfails = nfails + 1

      cdims(1) = timedim
      cid = ncvdef (ncid, 'cv', NCCHAR, 1, cdims, iret)
      if (iret .ne. 0) nfails = nfails + 1


      return
      end


!    
!     subroutine to test ncvgt and ncvgtc
!
      subroutine tncvgt(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname

      integer ndims, times, lats, lons, levels, lenstr
      parameter (times=4, lats=5, lons=5, levels=4)

      integer start(4), count(4)
      integer ncid, iret, i, m
      integer  latdim, londim, leveldim, timedim, lendim
      integer dimsiz(MAXNCDIM)
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz

      integer bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid
      integer itime, ilev, ilat, ilon

!     arrays of data values to be read
      NCBYTE_T barray(times), byval(times)
      NCSHORT_T sarray(times), shval(times)
      integer larray(lats)
      real farray(levels, lats, lons, times)
      doubleprecision darray(levels, lats, lons, times)
!     character array of data values to be read
      character*31 string
      character*31 varnam
      integer nvars, natts, recdim
      integer vartyp, nvdims, vdims(MAXVDIMS), nvatts

      data start/1,1,1,1/
      data count/levels, lats, lons, times/
      data byval /97, 98, 99, 100/
      data shval /10, 11, 12, 13/

      ncid = ncopn (cdfname, NCWRITE, iret)
      if (iret .ne. 0) nfails = nfails + 1
!     get number of variables in netCDF
      call ncinq (ncid, ndims, nvars, natts, recdim, iret)
      if (iret .ne. 0) nfails = nfails + 1
      do 5 m = 1, nvars-1
!     get variable name, datatype, number of dimensions
!     vector of dimension ids, and number of variable attributes
         call ncvinq (ncid, m, varnam, vartyp, nvdims, vdims,
     +                nvatts, iret)
         if (iret .ne. 0) nfails = nfails + 1
         if (vartyp .eq. NCBYTE) then
!
!     byte
!
            count(1) = times
            call ncvgt (ncid, m, start, count, barray, iret)
            if (iret .ne. 0) nfails = nfails + 1
            do 10 i = 1, times
               if (barray(i) .ne. byval(i)) then 
                  write(*,*) 'ncvgt of bytes, got ', barray(i), ' .ne. '
     +                       , byval(i)
               end if
 10         continue
         else if (vartyp .eq. NCSHORT) then
!
!     short
!
            count(1) = times
            call ncvgt (ncid, m, start, count, sarray, iret)
            if (iret .ne. 0) nfails = nfails + 1
            do 20 i = 1, times
               if (sarray(i) .ne. shval(i)) then 
                  write(*,*) 'ncvgt of short, got ', sarray(i), ' .ne. '
     +                       , shval(i)
               end if
 20         continue
         else if (vartyp .eq. NCLONG) then
!     
!     long
!
            count(1) = lats
            call ncvgt (ncid, m, start, count, larray, iret)
            if (iret .ne. 0) nfails = nfails + 1
            do 30 i = 1, lats
               if (larray(i) .ne. 1000) then 
                  write(*,*) 'long error in ncvgt'
               end if
 30         continue
         else if (vartyp .eq. NCFLOAT) then
!     
!     float
!
            count(1) = levels
            call ncvgt (ncid, m, start, count, farray, iret)
            if (iret .ne. 0) nfails = nfails + 1
            i = 0
            do 40 itime = 1,times
               do 41 ilon = 1, lons
                  do 42 ilat = 1, lats
                     do 43 ilev = 1, levels
                        i = i + 1
                        if (farray(ilev, ilat, ilon, itime) .ne.
     + real(i)) then
                           write (*,*) 'float error in ncvgt'
                        end if
 43         continue
 42         continue
 41         continue
 40         continue
         else if (vartyp .eq. NCDOUBLE) then
!
!     double
!
            count(1) = levels
            call ncvgt (ncid, m, start, count, darray, iret)
            if (iret .ne. 0) nfails = nfails + 1
            i = 0
            do 50 itime = 1, times
               do 51 ilon = 1, lons
                  do 52 ilat = 1, lats
                     do 53 ilev = 1, levels
                        i = i + 1
                        if (darray(ilev, ilat, ilon, itime) .ne.
     +                       real (i)) then
                           write(*,*) 'double error in ncvgt:', i,
     +              darray(ilev, ilat, ilon, itime), '.ne.', 
     +              real (i)
                        end if
 53         continue
 52         continue
 51         continue
 50         continue
         else 
!     
!     char
!
            count(1) = 3
            count(2) = 4
            lenstr = 31
            call ncvgtc (ncid, m, start, count, string, lenstr, iret)
            if (iret .ne. 0) nfails = nfails + 1
            if (string .ne. 'testhikin of') then 
               write(*,*) 'error in ncvgt, returned string =', string
            end if
         end if
 5    continue
      call ncclos(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end

      
      subroutine tncvgt1(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname

      integer ncid, iret
      integer  latdim, londim, leveldim, timedim, lendim
      integer dimsiz(MAXNCDIM)
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz

      integer bindx(1), sindx(1), lindx(1), findx(4), dindx(4), cindx(1)

      integer bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid

      NCBYTE_T bvalue
      NCSHORT_T svalue
      integer lvalue
      real fvalue
      doubleprecision dvalue
      character*1 c
      real epsilon
      doubleprecision onethird

      data epsilon /.000001/
      data lindx/1/, bindx/1/, sindx/1/, findx/1,1,1,1/
     +dindx/1,1,1,1/, cindx/1/
      data onethird/0.3333333333D0/
      
      ncid = ncopn (cdfname, NCNOWRIT, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     test ncvgt1 for byte
!
      call ncvgt1 (ncid, bid, bindx, bvalue, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (bvalue .ne. ichar('z')) write(*,*) 'error in ncvgt1 byte:',
     + bvalue, ' .ne.', ichar('z')
!
!     test ncvgt1 for short
!
      call ncvgt1 (ncid, sid, sindx, svalue, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (svalue .ne. 10) write(*,*) 'error in ncvgt1 short:',
     + svalue, ' .ne.', 10
!     
!     test ncvgt1 for long
!
      call ncvgt1 (ncid, lid, lindx, lvalue, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (lvalue .ne. 1000) write(*,*) 'error in ncvgt1 long:',
     + lvalue,  ' .ne.', 1000
!
!     test ncvgt1 for float
!
      call ncvgt1 (ncid, fid, findx, fvalue, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (abs(fvalue - 3.14159) .gt. epsilon) 
     +   write(*,*) 'error in ncvgt 1 float:', fvalue, 
     +      ' not close to', 3.14159
!
!     test ncvgt1 for double
!
      call ncvgt1 (ncid, did, dindx, dvalue, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (abs(dvalue - onethird) .gt. epsilon) write(*,*)
     + 'error in ncvgt1 double:', dvalue, ' not close to',
     +     onethird
!
!     test ncvg1c for char
!
      call ncvg1c (ncid, cid, cindx, c, iret)
      if (iret .ne. 0) nfails = nfails + 1
      if (c .ne. 'a') write(*,*) 'error in ncvg1c'
      call ncclos(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1
      return
      end

      
      
!
!     subroutine to test ncvpt and ncvptc
!
      subroutine tncvpt(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname

!     size of dimensions
      integer times, lats, lons, levels
      parameter (times=4, lats=5, lons=5, levels=4)

      integer ncid, iret
!     loop control variables
      integer itime, ilev, ilon, ilat, i
      integer  latdim, londim, leveldim, timedim, lendim
      integer dimsiz(MAXNCDIM)
      common /dims/timedim, latdim, londim, leveldim, lendim,
     + dimsiz
      integer lenstr
      integer bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid

!     vector of integers specifying the corner of the  hypercube
!     where the first of the data values will be written
      integer start(4)
!     vector of integers specifying the edge lengths from the
!     corner of the hypercube where the first of the data values
!     will be written
      integer count(4)

!     arrays of data values to be written
      NCBYTE_T barray(times)
      NCSHORT_T sarray(times)
      integer larray(lats)
      real farray(levels, lats, lons, times)
      doubleprecision darray(levels, lats, lons, times)
      character*31 string

      data start/1,1,1,1/
      data count/levels, lats, lons, times/
      data barray /97, 98, 99, 100/
      data sarray /10, 11, 12, 13/

      ncid = ncopn (cdfname, NCWRITE, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     byte
!
      count(1) = times
      call ncvpt (ncid, bid, start, count, barray, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     short
!
      count(1) = times
      call ncvpt (ncid, sid, start, count, sarray, iret)
      if (iret .ne. 0) nfails = nfails + 1
!
!     long
!
      do 30 i = 1,lats
         larray(i) = 1000
 30   continue
      count(1) = lats
      call ncvpt (ncid, lid, start, count, larray, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     float
!
      i = 0
      do 40 itime = 1,times
         do 41 ilon = 1, lons
            do 42 ilat = 1, lats
               do 43 ilev = 1, levels
                  i = i + 1
                  farray(ilev, ilat, ilon, itime) = real (i)
 43   continue
 42   continue
 41   continue
 40   continue
      count(1) = levels
      call ncvpt (ncid, fid, start, count, farray, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     double
!
      i = 0
      do 50 itime = 1, times
         do 51 ilon = 1, lons
            do 52 ilat = 1, lats
               do 53 ilev = 1, levels
                  i = i + 1
                  darray(ilev, ilat, ilon, itime) = real (i)
 53   continue
 52   continue
 51   continue
 50   continue
      count(1) = levels
      call ncvpt (ncid, did, start, count, darray, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     char
!
      start(1) = 1
      start(2) = 1
      count(1) = 4
      count(2) = 4
      lenstr = 31       
      string = 'testthiskind of '
      call ncvptc (ncid, chid,start, count, string, lenstr, iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncclos(ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1

      return
      end


      subroutine tncvpt1(cdfname, nfails)
#include "netcdf.inc"
      character*31 cdfname


      integer iret, ncid
      integer  latdim, londim, leveldim, timedim, lendim
      integer dimsiz(MAXNCDIM)
      common /dims/timedim, latdim, londim, leveldim, lendim, 
     + dimsiz

      integer bindx(1), sindx(1), lindx(1), findx(4), dindx(4), cindx(1)

      integer lvalue
      NCSHORT_T svalue
      NCBYTE_T bvalue
      doubleprecision onethird
      integer bid, sid, lid, fid, did, cid, chid
      common /vars/bid, sid, lid, fid, did, cid, chid
      data lindx/1/, bindx/1/, sindx/1/, findx/1,1,1,1/
     +dindx/1,1,1,1/, cindx/1/
      data lvalue /1000/
      data svalue/10/
      data onethird/0.3333333333D0/

      bvalue = ichar('z')
      
      ncid = ncopn (cdfname, NCWRITE, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     test ncvpt1 for byte
!
      call ncvpt1 (ncid, bid, bindx, bvalue, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     test ncvpt1 for short
!
      call ncvpt1 (ncid, sid, sindx, svalue, iret)
      if (iret .ne. 0) nfails = nfails + 1

!     
!     test ncvpt1 for long
!
      call ncvpt1 (ncid, lid, lindx, lvalue, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     test ncvpt1 for float
!
      call ncvpt1 (ncid, fid, findx, 3.14159, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     test ncvpt1 for double
!
      call ncvpt1 (ncid, did, dindx, onethird, iret)
      if (iret .ne. 0) nfails = nfails + 1

!
!     test ncvp1c for char
!
      call ncvp1c (ncid, cid, cindx, 'a', iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncclos (ncid, iret)
      if (iret .ne. 0) nfails = nfails + 1

      return
      end

!
! subroutine to test default fill values
!
      subroutine tfills(nfails)
#include "netcdf.inc"
      integer ncid
      integer bid, sid, lid, fid, did
      integer ix(1)
      integer l
      NCSHORT_T s
      doubleprecision d
      real f
      NCBYTE_T b

      ncid = NCOPN('fills.nc', NCNOWRIT, iret)
      if (iret .ne. 0) nfails = nfails + 1

      bid = ncvid(ncid, 'b', iret)
      if (iret .ne. 0) nfails = nfails + 1

      sid = ncvid(ncid, 's', iret)
      if (iret .ne. 0) nfails = nfails + 1

      lid = ncvid(ncid, 'l', iret)
      if (iret .ne. 0) nfails = nfails + 1

      fid = ncvid(ncid, 'f', iret)
      if (iret .ne. 0) nfails = nfails + 1

      did = ncvid(ncid, 'd', iret)
      if (iret .ne. 0) nfails = nfails + 1


      ix(1) = 2
      call ncvgt1(ncid, bid, ix, b, iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncvgt1(ncid, sid, ix, s, iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncvgt1(ncid, lid, ix, l, iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncvgt1(ncid, fid, ix, f, iret)
      if (iret .ne. 0) nfails = nfails + 1

      call ncvgt1(ncid, did, ix, d, iret)
      if (iret .ne. 0) nfails = nfails + 1


      if (b .ne. FILBYTE) write(*,*) 'error in byte fill value'
      if (d .ne. FILDOUB) write(*,*) 'error in double fill value'
      if (f .ne. FILFLOAT) write(*,*) 'error in float fill value'
      if (l .ne. FILLONG) write(*,*) 'error in long fill value'
      if (s .ne. FILSHORT) write(*,*) 'error in short fill value'

      return
      end
