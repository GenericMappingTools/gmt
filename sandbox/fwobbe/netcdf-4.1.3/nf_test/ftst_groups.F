C     This is part of the netCDF package.
C     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
C     See COPYRIGHT file for conditions of use.

C     This program tests netCDF-4 variable functions from fortran.

C     $Id$

      program ftst_groups
      implicit none
      include 'netcdf.inc'

C     This is the name of the data file we will create.
      character*(*) file_name
      parameter (file_name='ftst_groups.nc')

C     Info about the groups we'll create.
      character*(*) group_name, sub_group_name
      parameter (group_name = 'grp', sub_group_name = 'sub')
      character*80 name_in, name_in2
      integer ngroups_in
      integer full_name_len

C     Dimensions and variables.
      character*(*) dim1_name, dim2_name
      parameter (dim1_name = 'd1', dim2_name = 'd2')
      character*(*) var1_name, var2_name
      parameter (var1_name = 'v1', var2_name = 'v2')
      integer nvars, ndims

C     NetCDF IDs.
      integer ncid, grpid, sub_grpid, subgrp_in
      integer grpids(1), grpid_in, dimids(2), varids(2)
      integer varids_in(2), dimids_in(2)

C     Error handling.
      integer retval

      print *, ''
      print *,'*** Testing netCDF-4 groups from F77.'

C     Create the netCDF file.
      retval = nf_create(file_name, NF_NETCDF4, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create a group and a subgroup.
      retval = nf_def_grp(ncid, group_name, grpid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_grp(grpid, sub_group_name, sub_grpid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Create a two dims and two vars.
      retval = nf_def_dim(sub_grpid, dim1_name, 0, dimids(1))
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_dim(sub_grpid, dim2_name, 0, dimids(2))
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_var(sub_grpid, var1_name, NF_UINT64, 2, dimids, 
     &     varids(1))
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_def_var(sub_grpid, var2_name, NF_UINT64, 2, dimids, 
     &     varids(2))
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Reopen the file.
      retval = nf_open(file_name, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      
C     Check the name of the root group.
      retval = nf_inq_grpname(ncid, name_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:1) .ne. '/') stop 2

C     Check the full name of the root group (also "/").
      retval = nf_inq_grpname_full(ncid, full_name_len, name_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (full_name_len .ne. 1 .or. name_in(1:1) .ne. '/') stop 2

C     What groups are there from the root group?
      retval = nf_inq_grps(ncid, ngroups_in, grpids)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ngroups_in .ne. 1) stop 2

C     Check the name of this group.
      retval = nf_inq_grpname(grpids(1), name_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in(1:len(group_name)) .ne. group_name) stop 2

C     Check the length of the full name.
      retval = nf_inq_grpname_len(grpids(1), full_name_len)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (full_name_len .ne. len(group_name) + 1) stop 2

C     Check the full name.
      retval = nf_inq_grpname_full(grpids(1), full_name_len, name_in2)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (name_in2(1:1) .ne. '/' .or. 
     &     name_in2(2:len(group_name)+1) .ne. group_name .or.
     &     full_name_len .ne. len(group_name) + 1) stop 2

C     Check getting the grpid by full name
      retval = nf_inq_grp_full_ncid(ncid, name_in, grpid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (grpid_in .ne. grpids(1)) stop 2

C     Check the parent ncid.
      retval = nf_inq_grp_parent(grpids(1), grpid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (grpid_in .ne. ncid) stop 2

C     Check getting the group by name
      retval = nf_inq_ncid(ncid, group_name, grpid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (grpid_in .ne. grpids(1)) stop 2

C     Check getting the group by name
      retval = nf_inq_ncid(ncid, group_name, grpid_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (grpid_in .ne. grpids(1)) stop 2

C     Get the sub group id, using its name.
      retval = nf_inq_ncid(grpid_in, sub_group_name, subgrp_in)
      if (retval .ne. nf_noerr) call handle_err(retval)

C     Check varids in subgroup.
      retval = nf_inq_varids(subgrp_in, nvars, varids_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (nvars .ne. 2 .or. varids_in(1) .ne. varids(1) .or.
     &     varids_in(2) .ne. varids(2)) stop 2

C     Check dimids in subgroup.
      retval = nf_inq_dimids(subgrp_in, ndims, dimids_in, 0)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ndims .ne. 2 .or. dimids_in(1) .ne. dimids(1) .or.
     &     dimids_in(2) .ne. dimids(2)) stop 2

C     Check dimids including parents (will get same answers, since there
C     are no dims in parent group.
      retval = nf_inq_dimids(subgrp_in, ndims, dimids_in, 1)
      if (retval .ne. nf_noerr) call handle_err(retval)
      if (ndims .ne. 2 .or. dimids_in(1) .ne. dimids(1) .or.
     &     dimids_in(2) .ne. dimids(2)) stop 2

C     Close the file. 
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      print *,'*** SUCCESS!'
      end
