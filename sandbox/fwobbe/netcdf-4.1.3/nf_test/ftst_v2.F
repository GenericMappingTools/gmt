C     This is part of the netCDF package.
C     Copyright 2009 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests the fortran V2 API with netCDF-4.

C     $Id$

      program ftst_v2
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) FILE_NAME
      parameter (FILE_NAME='ftst_v2.nc')

      integer ncid, timedim, londim, latdim, lonid, latid
      character*(3) lat_name
      parameter (lat_name = 'lat')
      character*(3) lon_name
      parameter (lon_name = 'lon')
      character*(4) time_name
      parameter (time_name = 'time')
      integer LON_LEN, LAT_LEN
      parameter (LON_LEN = 3, LAT_LEN = 5)
      integer NDIMS, NVARS
      parameter (NDIMS = 3, NVARS = 2)
      integer ndims_in, nvars_in, ngatts_in, recdim_in
      integer dimsize_in, natts_in
      integer xtype_in, dimids_in(NDIMS)
      character*(128) name_in
      integer rc

      print *, ''
      print *,'*** Testing v2 fortran API with netCDF-4.'

C     Create the file.
      ncid = nccre(FILE_NAME, NF_NETCDF4, rc)
      timedim = ncddef(ncid, time_name, NCUNLIM, rc)
      londim = ncddef(ncid, lon_name, LON_LEN, rc)
      latdim = ncddef(ncid, lat_name, LAT_LEN, rc)
      lonid = ncvdef(ncid, lon_name, NCFLOAT, 1, londim, rc)
      latid = ncvdef(ncid, lat_name, NCFLOAT, 1, latdim, rc)

C     Check the file.
      call ncinq(ncid, ndims_in, nvars_in, ngatts_in, recdim_in, rc)
      if (ndims_in .ne. NDIMS) stop 2
      if (ndims_in .ne. NDIMS .or. nvars_in .ne. NVARS .or.
     $     ngatts_in .ne. 0 .or. recdim_in .ne. timedim) stop 2
      call ncdinq(ncid, timedim, name_in, dimsize_in, rc)
      if (name_in .ne. time_name .or. dimsize_in .ne. 0) stop 2
      call ncdinq(ncid, londim, name_in, dimsize_in, rc)
      if (name_in .ne. lon_name .or. dimsize_in .ne. LON_LEN) stop 2
      call ncdinq(ncid, latdim, name_in, dimsize_in, rc)
      if (name_in .ne. lat_name .or. dimsize_in .ne. LAT_LEN) stop 2
      call ncvinq(ncid, lonid, name_in, xtype_in, ndims_in, dimids_in, 
     $     natts_in, rc)
      if (name_in .ne. lon_name .or. xtype_in .ne. NCFLOAT .or.
     $     ndims_in .ne. 1 .or. dimids_in(1) .ne. londim) stop 2
      call ncvinq(ncid, latid, name_in, xtype_in, ndims_in, dimids_in, 
     $     natts_in, rc)
      if (name_in .ne. lat_name .or. xtype_in .ne. NCFLOAT .or.
     $     ndims_in .ne. 1 .or. dimids_in(1) .ne. latdim) stop 2

C     Close the file.
      call ncclos(ncid, rc)

C     Reopen, and check again.
      ncid = ncopn(FILE_NAME, NCNOWRIT, rc)
C     Check the file.
      call ncinq(ncid, ndims_in, nvars_in, ngatts_in, recdim_in, rc)
      if (ndims_in .ne. NDIMS) stop 2
      if (ndims_in .ne. NDIMS .or. nvars_in .ne. NVARS .or.
     $     ngatts_in .ne. 0 .or. recdim_in .ne. timedim) stop 2
      call ncdinq(ncid, timedim, name_in, dimsize_in, rc)
      if (name_in .ne. time_name .or. dimsize_in .ne. 0) stop 2
      call ncdinq(ncid, londim, name_in, dimsize_in, rc)
      if (name_in .ne. lon_name .or. dimsize_in .ne. LON_LEN) stop 2
      call ncdinq(ncid, latdim, name_in, dimsize_in, rc)
      if (name_in .ne. lat_name .or. dimsize_in .ne. LAT_LEN) stop 2
      call ncvinq(ncid, lonid, name_in, xtype_in, ndims_in, dimids_in, 
     $     natts_in, rc)
      if (name_in .ne. lon_name .or. xtype_in .ne. NCFLOAT .or.
     $     ndims_in .ne. 1 .or. dimids_in(1) .ne. londim) stop 2
      call ncvinq(ncid, latid, name_in, xtype_in, ndims_in, dimids_in, 
     $     natts_in, rc)
      if (name_in .ne. lat_name .or. xtype_in .ne. NCFLOAT .or.
     $     ndims_in .ne. 1 .or. dimids_in(1) .ne. latdim) stop 2

      call ncclos(ncid, rc)

      print *,'*** SUCCESS!'
      end

