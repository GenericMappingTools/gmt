  ! ----- 
  ! Variable definitions and inquiry
  ! ----- 
  function nf90_def_var_Scalar(ncid, name, xtype, varid)
    integer, intent( in) :: ncid
    character (len = *), intent( in) :: name
    integer, intent(in) :: xtype
    integer, intent(out) :: varid
    integer :: nf90_def_var_Scalar
    
    ! Dummy - shouldn't get used
    integer, dimension(1) :: dimids

    ! These may not be used with scalars, but it causes an interface
    ! violation if they are not optional arguments.
    
    nf90_def_var_Scalar = nf_def_var(ncid, name, xtype, 0, dimids, varid)
  end function nf90_def_var_Scalar
  ! ----- 
  function nf90_def_var_oneDim(ncid, name, xtype, dimids, varid, contiguous, &
       chunksizes, deflate_level, shuffle, fletcher32, endianness, &
       cache_size, cache_nelems, cache_preemption)
    integer, intent( in) :: ncid
    character (len = *), intent( in) :: name
    integer, intent(in) :: xtype
    integer, intent(in) :: dimids
    integer, intent(out) :: varid
    logical, optional, intent(in) :: contiguous
    integer, optional, intent(in) :: chunksizes
    integer, optional, intent(in) :: deflate_level
    logical, optional, intent(in) :: shuffle, fletcher32
    integer, optional, intent(in) :: endianness
    integer, optional, intent(in) :: cache_size, cache_nelems, cache_preemption
    integer :: nf90_def_var_oneDim
    
    integer, dimension(1) :: dimidsA, chunksizes1
    integer :: size1 = -1, nelems1 = -1, preemption1 = -1
    integer :: contiguous1

    ! Put this int into an array, where all decent folk keep ids.
    dimidsA(1) = dimids

    ! This is forbidden! Don't even think about it.
    if (present(contiguous)) then
       if (contiguous .and. present(chunksizes)) then
          nf90_def_var_oneDim = nf90_einval
          return
       end if
    end if
    if (present(contiguous)) then
       if (.not. contiguous .and. .not. present(chunksizes)) then
          nf90_def_var_oneDim = nf90_einval
          return
       end if
    end if
    

    ! Define the variable. 
    nf90_def_var_oneDim = nf_def_var(ncid, name, xtype, 1, dimidsA, varid)
    if (nf90_def_var_oneDim .ne. nf90_noerr) return

    ! Handle chunksizes and contiguous.
    if (present(chunksizes) .or. present(contiguous)) then
       if (present(contiguous)) then
          if (contiguous) then
             contiguous1 = nf90_contiguous
          else
             contiguous1 = nf90_notcontiguous
          endif
       endif
       if (present(chunksizes)) then
          contiguous1 = 0
          chunksizes1(1) = chunksizes
       endif
       nf90_def_var_oneDim = nf_def_var_chunking(ncid, varid, contiguous1, chunksizes1)       
    endif
    if (present(contiguous)) then
       if (contiguous) then
          nf90_def_var_oneDim = nf_def_var_chunking(ncid, varid, 1, 0)       
       endif
    endif
    if (nf90_def_var_oneDim .ne. nf90_noerr) return
    
    ! Handle deflate and shuffle.
    if (present(deflate_level)) then
       if (deflate_level .gt. 0) then
          if (present(shuffle)) then
             if (shuffle) then
                nf90_def_var_oneDim = nf_def_var_deflate(ncid, varid, 1, 1, deflate_level)       
             else
                nf90_def_var_oneDim = nf_def_var_deflate(ncid, varid, 0, 1, deflate_level)       
             end if
             if (nf90_def_var_oneDim .ne. nf90_noerr) return
          end if
       end if
    endif

    ! Handle fletcher32.
    if (present(fletcher32)) then
       if (fletcher32) then
          nf90_def_var_oneDim = nf_def_var_fletcher32(ncid, varid, 1)       
          if (nf90_def_var_oneDim .ne. nf90_noerr) return
       endif
    endif

    ! Handle endianness.
    if (present(endianness)) then
       nf90_def_var_oneDim = nf_def_var_endian(ncid, varid, endianness)       
       if (nf90_def_var_oneDim .ne. nf90_noerr) return
    endif

    ! Set the cache if the user wants to.
    if (present(cache_size) .or. present(cache_nelems) .or. &
         present(cache_preemption)) then
       ! Negative values mean leave it alone.
       if (present(cache_size)) size1 = cache_size
       if (present(cache_nelems)) nelems1 = cache_nelems
       if (present(cache_preemption)) preemption1 = cache_preemption
       
       nf90_def_var_oneDim = nf_set_var_chunk_cache(ncid, varid, &
            size1, nelems1, preemption1)
       if (nf90_def_var_oneDim .ne. nf90_noerr) return
    endif

  end function nf90_def_var_oneDim
  ! ----- 
  function nf90_def_var_ManyDims(ncid, name, xtype, dimids, varid, contiguous, &
       chunksizes, deflate_level, shuffle, fletcher32, endianness, cache_size, &
       cache_nelems, cache_preemption)
    integer, intent(in) :: ncid
    character (len = *), intent(in) :: name
    integer, intent( in) :: xtype
    integer, dimension(:), intent(in) :: dimids
    integer, intent(out) :: varid
    logical, optional, intent(in) :: contiguous
    integer, optional, dimension(:), intent(in) :: chunksizes
    integer, optional, intent(in) :: deflate_level
    logical, optional, intent(in) :: shuffle, fletcher32
    integer, optional, intent(in) :: endianness
    integer, optional, intent(in) :: cache_size, cache_nelems, cache_preemption
    integer :: nf90_def_var_ManyDims

    ! Local variables.
    integer :: contiguous1, d
    integer :: size1 = -1, nelems1 = -1, preemption1 = -1
    integer, dimension(nf90_max_dims) :: chunksizes1

    ! This is forbidden!
    if (present(contiguous)) then
       if (contiguous .and. present(chunksizes)) then
          nf90_def_var_ManyDims = nf90_einval
          return
       end if
    end if
    if (present(contiguous)) then
       if (.not. contiguous .and. .not. present(chunksizes)) then
          nf90_def_var_ManyDims = nf90_einval
          return
       endif
    end if

    ! Be nice and check array size.
    if (present(chunksizes)) then
       if (size(chunksizes) .ne. size(dimids)) then
          nf90_def_var_ManyDims = nf90_einval
          return
       end if
    end if
    
    ! Define the variable. 
    nf90_def_var_ManyDims = nf_def_var(ncid, name, xtype, size(dimids), dimids, varid)
    if (nf90_def_var_ManyDims .ne. nf90_noerr) return

    ! Handle chunksizes and contiguous.
    if (present(chunksizes) .or. present(contiguous)) then
       if (present(contiguous)) then
          if (contiguous) then
             contiguous1 = nf90_contiguous
          else
             contiguous1 = nf90_notcontiguous
          endif
       endif
       if (present(chunksizes)) then
          contiguous1 = 0
          do d = 1, size(dimids)
             chunksizes1(d) = chunksizes(d)
          end do
       endif
       nf90_def_var_ManyDims = nf_def_var_chunking(ncid, varid, contiguous1, chunksizes1)       
    endif
    if (present(contiguous)) then
       if (contiguous) then
          nf90_def_var_ManyDims = nf_def_var_chunking(ncid, varid, 1, 0)       
       endif
    endif
    if (nf90_def_var_ManyDims .ne. nf90_noerr) return
    
    ! Handle deflate and shuffle.
    if (present(deflate_level)) then
       if (deflate_level .gt. 0) then
          if (present(shuffle)) then
             if (shuffle) then
                nf90_def_var_ManyDims = nf_def_var_deflate(ncid, varid, 1, 1, deflate_level)       
             else
                nf90_def_var_ManyDims = nf_def_var_deflate(ncid, varid, 0, 1, deflate_level)       
             end if
          else
             nf90_def_var_ManyDims = nf_def_var_deflate(ncid, varid, 0, 1, deflate_level)       
          end if
       end if
    endif
    if (nf90_def_var_ManyDims .ne. nf90_noerr) return

    ! Handle fletcher32.
    if (present(fletcher32)) then
       if (fletcher32) then
          nf90_def_var_ManyDims = nf_def_var_fletcher32(ncid, varid, 1)       
       endif
    endif
    if (nf90_def_var_ManyDims .ne. nf90_noerr) return

    ! Handle endianness.
    if (present(endianness)) then
       nf90_def_var_ManyDims = nf_def_var_endian(ncid, varid, endianness)       
    endif

    ! Set the cache if the user wants to.
    if (present(cache_size) .or. present(cache_nelems) .or. &
         present(cache_preemption)) then
       ! Negative values mean leave it alone.
       if (present(cache_size)) size1 = cache_size
       if (present(cache_nelems)) nelems1 = cache_nelems
       if (present(cache_preemption)) preemption1 = cache_preemption
       
       nf90_def_var_ManyDims = nf_set_var_chunk_cache(ncid, varid, &
            size1, nelems1, preemption1)
       if (nf90_def_var_ManyDims .ne. nf90_noerr) return
    endif

  end function nf90_def_var_ManyDims
  ! ----- 
  function nf90_inq_varid(ncid, name, varid)
    integer, intent(in) :: ncid
    character (len = *), intent( in) :: name
    integer, intent(out) :: varid
    integer :: nf90_inq_varid
    
    nf90_inq_varid = nf_inq_varid(ncid, name, varid)
    if (nf90_inq_varid .ne. nf90_noerr) return

  end function nf90_inq_varid
  ! ----- 
  function nf90_set_var_chunk_cache(ncid, varid, size, nelems, preemption)
    integer, intent(in) :: ncid, varid, size, nelems, preemption
    integer :: nf90_set_var_chunk_cache
    
    nf90_set_var_chunk_cache = nf_set_var_chunk_cache(ncid, varid, &
         size, nelems, preemption)
    if (nf90_set_var_chunk_cache .ne. nf90_noerr) return

  end function nf90_set_var_chunk_cache
  ! ----- 
  function nf90_inquire_variable(ncid, varid, name, xtype, ndims, dimids, nAtts, &
       contiguous, chunksizes, deflate_level, shuffle, fletcher32, endianness, &
       cache_size, cache_nelems, cache_preemption)
    integer, intent(in) :: ncid, varid
    character (len = *), optional, intent(out) :: name
    integer, optional, intent(out) :: xtype, ndims 
    integer, dimension(:), optional, intent(out) :: dimids
    integer, optional, intent(out) :: nAtts
    logical, optional, intent(out) :: contiguous
    integer, optional, dimension(:), intent(out) :: chunksizes
    integer, optional, intent(out) :: deflate_level
    logical, optional, intent(out) :: shuffle, fletcher32
    integer, optional, intent(out) :: endianness
    integer, optional, intent(out) :: cache_size, cache_nelems, cache_preemption
    integer :: nf90_inquire_variable
    
    ! Local variables
    character (len = nf90_max_name) :: varName = ''
    integer :: externalType, numDimensions
    integer, dimension(nf90_max_var_dims) :: dimensionIDs
    integer :: numAttributes
    integer :: deflate1, deflate_level1, contiguous1, shuffle1, fletcher321
    integer, dimension(nf90_max_dims) :: chunksizes1
    integer :: size1, nelems1, preemption1
    integer :: d

    ! Learn the basic facts.
    nf90_inquire_variable = nf_inq_var(ncid, varid, varName, externalType, &
                                       numDimensions, dimensionIDs, numAttributes)
    if (nf90_inquire_variable .ne. nf90_noerr) return
    
    ! Tell the user what he wants to know.
    if (present(name)) name = trim(varName)
    if (present(xtype)) xtype = externalType
    if (present(ndims)) ndims = numDimensions
    if (present(dimids)) then
       if (size(dimids) .ge. numDimensions) then
          dimids(:numDimensions) = dimensionIDs(:numDimensions)
       else
          nf90_inquire_variable = nf90_einval
       endif
    endif
    if (present(nAtts)) nAtts = numAttributes

    ! Get the chunksizes and contiguous settings, if desired.
    if (present(chunksizes) .or. present(contiguous)) then
       nf90_inquire_variable = nf_inq_var_chunking(ncid, varid, contiguous1, chunksizes1)
       if (nf90_inquire_variable .ne. nf90_noerr) return
       if (present(contiguous)) contiguous = contiguous1 .ne. nf90_notcontiguous
       if (present(chunksizes)) then
          do d = 1, numDimensions
             chunksizes(d) = chunksizes1(d)
          end do
       endif
    endif

    ! Get the fletcher32 settings, if desired.
    if (present(fletcher32)) then
       nf90_inquire_variable = nf_inq_var_fletcher32(ncid, varid, fletcher321)
       if (nf90_inquire_variable .ne. nf90_noerr) return
       fletcher32 = fletcher321 .gt. 0
    endif
    
    ! Get the deflate and shuffle settings, if desired.
    if (present(deflate_level) .or. present(shuffle)) then
       nf90_inquire_variable = nf_inq_var_deflate(ncid, varid, shuffle1, deflate1, deflate_level1)
       if (nf90_inquire_variable .ne. nf90_noerr) return
       if (present(deflate_level)) deflate_level = deflate_level1
       if (present(shuffle)) shuffle = shuffle1 .ne. 0
    endif

    ! And the endianness...
    if (present(endianness)) then
       nf90_inquire_variable = nf_inq_var_endian(ncid, varid, endianness)
       if (nf90_inquire_variable .ne. nf90_noerr) return
    endif

    ! Does the user want cache settings?
    if (present(cache_size) .or. present(cache_nelems) .or. present(cache_preemption)) then
       nf90_inquire_variable = nf_get_var_chunk_cache(ncid, varid, &
            size1, nelems1, preemption1)
       if (nf90_inquire_variable .ne. nf90_noerr) return
       if (present(cache_size)) cache_size = size1
       if (present(cache_nelems)) cache_nelems = nelems1
       if (present(cache_preemption)) cache_preemption = preemption1
    endif
  end function nf90_inquire_variable
  ! ----- 
  function nf90_rename_var(ncid, varid, newname)
    integer,             intent( in) :: ncid, varid
    character (len = *), intent( in) :: newname
    integer                          :: nf90_rename_var
    
    nf90_rename_var = nf_rename_var(ncid, varid, newname)
  end function nf90_rename_var
  ! ----- 
