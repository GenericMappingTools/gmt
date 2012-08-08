





   function nf90_put_var_OneByteInt(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_put_var_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_put_var_OneByteInt = nf_put_var1_int1(ncid, varid, localIndex, values)
   end function nf90_put_var_OneByteInt


   function nf90_put_var_TwoByteInt(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_put_var_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_put_var_TwoByteInt = nf_put_var1_int2(ncid, varid, localIndex, values)
   end function nf90_put_var_TwoByteInt


   function nf90_put_var_FourByteInt(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_put_var_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_put_var_FourByteInt = nf_put_var1_int(ncid, varid, localIndex, int(values))
   end function nf90_put_var_FourByteInt


   function nf90_put_var_FourByteReal(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_put_var_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_put_var_FourByteReal = nf_put_var1_real(ncid, varid, localIndex, values)
   end function nf90_put_var_FourByteReal


   function nf90_put_var_EightByteReal(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_put_var_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_put_var_EightByteReal = nf_put_var1_double(ncid, varid, localIndex, values)
   end function nf90_put_var_EightByteReal


   function nf90_get_var_OneByteInt(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_get_var_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_get_var_OneByteInt = nf_get_var1_int1(ncid, varid, localIndex, values)
   end function nf90_get_var_OneByteInt


   function nf90_get_var_TwoByteInt(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_get_var_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_get_var_TwoByteInt = nf_get_var1_int2(ncid, varid, localIndex, values)
   end function nf90_get_var_TwoByteInt


   function nf90_get_var_FourByteInt(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_get_var_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
     integer                               :: defaultInteger
     
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_get_var_FourByteInt = nf_get_var1_int(ncid, varid, localIndex, defaultInteger)
     values = defaultInteger
   end function nf90_get_var_FourByteInt


   function nf90_get_var_FourByteReal(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_get_var_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_get_var_FourByteReal = nf_get_var1_real(ncid, varid, localIndex, values)
   end function nf90_get_var_FourByteReal


   function nf90_get_var_EightByteReal(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: nf90_get_var_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
 
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     nf90_get_var_EightByteReal = nf_get_var1_double(ncid, varid, localIndex, values)
   end function nf90_get_var_EightByteReal



   function nf90_put_var_1D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_1D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_1D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_1D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_1D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_1D_OneByteInt


   function nf90_put_var_2D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_2D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_2D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_2D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_2D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_2D_OneByteInt


   function nf90_put_var_3D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_3D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_3D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_3D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_3D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_3D_OneByteInt


   function nf90_put_var_4D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_4D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_4D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_4D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_4D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_4D_OneByteInt


   function nf90_put_var_5D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_5D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_5D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_5D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_5D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_5D_OneByteInt


   function nf90_put_var_6D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_6D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_6D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_6D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_6D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_6D_OneByteInt


   function nf90_put_var_7D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_7D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_7D_OneByteInt = &
          nf_put_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_7D_OneByteInt = &
          nf_put_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_7D_OneByteInt = &
          nf_put_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_7D_OneByteInt


   function nf90_put_var_1D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_1D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_1D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_1D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_1D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_1D_TwoByteInt


   function nf90_put_var_2D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_2D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_2D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_2D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_2D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_2D_TwoByteInt


   function nf90_put_var_3D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_3D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_3D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_3D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_3D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_3D_TwoByteInt


   function nf90_put_var_4D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_4D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_4D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_4D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_4D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_4D_TwoByteInt


   function nf90_put_var_5D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_5D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_5D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_5D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_5D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_5D_TwoByteInt


   function nf90_put_var_6D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_6D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_6D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_6D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_6D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_6D_TwoByteInt


   function nf90_put_var_7D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_7D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_7D_TwoByteInt = &
          nf_put_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_7D_TwoByteInt = &
          nf_put_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_7D_TwoByteInt = &
          nf_put_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_7D_TwoByteInt


   function nf90_put_var_1D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_1D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_1D_FourByteInt = &
          nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       nf90_put_var_1D_FourByteInt = &
          nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
       nf90_put_var_1D_FourByteInt = &
          nf_put_vara_int(ncid, varid, localStart, localCount, int(values))
     end if
   end function nf90_put_var_1D_FourByteInt


   function nf90_put_var_2D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_2D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
        localMap   (:size(map))    = map(:)
        nf90_put_var_2D_FourByteInt = &
             nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
        nf90_put_var_2D_FourByteInt = &
             nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
        print *, values(1, 1), values(1, 2), values(1, 3), values(1, 4)
        nf90_put_var_2D_FourByteInt = &
             nf_put_vara_int(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_2D_FourByteInt


   function nf90_put_var_3D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_3D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_3D_FourByteInt = &
          nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       nf90_put_var_3D_FourByteInt = &
          nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
       nf90_put_var_3D_FourByteInt = &
          nf_put_vara_int(ncid, varid, localStart, localCount, int(values))
     end if
   end function nf90_put_var_3D_FourByteInt


   function nf90_put_var_4D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_4D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_4D_FourByteInt = &
          nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       nf90_put_var_4D_FourByteInt = &
          nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
       nf90_put_var_4D_FourByteInt = &
          nf_put_vara_int(ncid, varid, localStart, localCount, int(values))
     end if
   end function nf90_put_var_4D_FourByteInt


   function nf90_put_var_5D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_5D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_5D_FourByteInt = &
          nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       nf90_put_var_5D_FourByteInt = &
          nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
       nf90_put_var_5D_FourByteInt = &
          nf_put_vara_int(ncid, varid, localStart, localCount, int(values))
     end if
   end function nf90_put_var_5D_FourByteInt


   function nf90_put_var_6D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_6D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_6D_FourByteInt = &
          nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       nf90_put_var_6D_FourByteInt = &
          nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
       nf90_put_var_6D_FourByteInt = &
          nf_put_vara_int(ncid, varid, localStart, localCount, int(values))
     end if
   end function nf90_put_var_6D_FourByteInt


   function nf90_put_var_7D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_7D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_7D_FourByteInt = &
          nf_put_varm_int(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       nf90_put_var_7D_FourByteInt = &
          nf_put_vars_int(ncid, varid, localStart, localCount, localStride, int(values))
     else
       nf90_put_var_7D_FourByteInt = &
          nf_put_vara_int(ncid, varid, localStart, localCount, int(values))
     end if
   end function nf90_put_var_7D_FourByteInt


    function nf90_put_var_1D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_1D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_1D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_1D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_1D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_1D_FourByteReal


   function nf90_put_var_2D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_2D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_2D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_2D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_2D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_2D_FourByteReal


   function nf90_put_var_3D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_3D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_3D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_3D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_3D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_3D_FourByteReal


   function nf90_put_var_4D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_4D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_4D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_4D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_4D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_4D_FourByteReal


   function nf90_put_var_5D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_5D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_5D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_5D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_5D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_5D_FourByteReal


   function nf90_put_var_6D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_6D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_6D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_6D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_6D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_6D_FourByteReal


   function nf90_put_var_7D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_7D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_7D_FourByteReal = &
          nf_put_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_7D_FourByteReal = &
          nf_put_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_7D_FourByteReal = &
          nf_put_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_7D_FourByteReal


   function nf90_put_var_1D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_1D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_1D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_1D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_1D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_1D_EightByteReal


   function nf90_put_var_2D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_2D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_2D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_2D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_2D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_2D_EightByteReal


   function nf90_put_var_3D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_3D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_3D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_3D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_3D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_3D_EightByteReal


   function nf90_put_var_4D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_4D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_4D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_4D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_4D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_4D_EightByteReal


   function nf90_put_var_5D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_5D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_5D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_5D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_5D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_5D_EightByteReal


   function nf90_put_var_6D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_6D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_6D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_6D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_6D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_6D_EightByteReal


   function nf90_put_var_7D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_7D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_7D_EightByteReal = &
          nf_put_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_put_var_7D_EightByteReal = &
          nf_put_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_put_var_7D_EightByteReal = &
          nf_put_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_put_var_7D_EightByteReal


   function nf90_get_var_1D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_1D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_1D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_1D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_1D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_1D_OneByteInt


   function nf90_get_var_2D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_2D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_2D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_2D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_2D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_2D_OneByteInt


   function nf90_get_var_3D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_3D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_3D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_3D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_3D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_3D_OneByteInt


   function nf90_get_var_4D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_4D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_4D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_4D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_4D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_4D_OneByteInt


   function nf90_get_var_5D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_5D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_5D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_5D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_5D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_5D_OneByteInt


   function nf90_get_var_6D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_6D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_6D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_6D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_6D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_6D_OneByteInt


   function nf90_get_var_7D_OneByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = OneByteInt), dimension(:, :, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_7D_OneByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_7D_OneByteInt = &
          nf_get_varm_int1(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_7D_OneByteInt = &
          nf_get_vars_int1(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_7D_OneByteInt = &
          nf_get_vara_int1(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_7D_OneByteInt


   function nf90_get_var_1D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_1D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_1D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_1D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_1D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_1D_TwoByteInt


   function nf90_get_var_2D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_2D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_2D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_2D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_2D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_2D_TwoByteInt


   function nf90_get_var_3D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_3D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_3D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_3D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_3D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_3D_TwoByteInt


   function nf90_get_var_4D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_4D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_4D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_4D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_4D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_4D_TwoByteInt


   function nf90_get_var_5D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_5D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_5D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_5D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_5D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_5D_TwoByteInt


   function nf90_get_var_6D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_6D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_6D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_6D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_6D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_6D_TwoByteInt


   function nf90_get_var_7D_TwoByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = TwoByteInt), dimension(:, :, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_7D_TwoByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_7D_TwoByteInt = &
          nf_get_varm_int2(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_7D_TwoByteInt = &
          nf_get_vars_int2(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_7D_TwoByteInt = &
          nf_get_vara_int2(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_7D_TwoByteInt


   function nf90_get_var_1D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_1D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_1D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_1D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_1D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_1D_FourByteInt


   function nf90_get_var_2D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_2D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_2D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_2D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_2D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:, :) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_2D_FourByteInt


   function nf90_get_var_3D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_3D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_3D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_3D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_3D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:, :, :) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_3D_FourByteInt


   function nf90_get_var_4D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_4D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_4D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_4D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_4D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:, :, :, :) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_4D_FourByteInt


   function nf90_get_var_5D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_5D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_5D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_5D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_5D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:, :, :, :, :) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_5D_FourByteInt


   function nf90_get_var_6D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_6D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_6D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_6D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_6D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:, :, :, :, :, :) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_6D_FourByteInt


   function nf90_get_var_7D_FourByteInt(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     integer (kind = FourByteInt), dimension(:, :, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_7D_FourByteInt
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
     integer, dimension(size(values))      :: defaultIntArray
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_7D_FourByteInt = &
          nf_get_varm_int(ncid, varid, localStart, localCount, localStride, localMap, defaultIntArray)
     else if(present(stride)) then
       nf90_get_var_7D_FourByteInt = &
          nf_get_vars_int(ncid, varid, localStart, localCount, localStride, defaultIntArray)
     else
       nf90_get_var_7D_FourByteInt = &
          nf_get_vara_int(ncid, varid, localStart, localCount, defaultIntArray)
     end if
     values(:, :, :, :, :, :, :) = reshape(defaultIntArray(:), shape(values))
   end function nf90_get_var_7D_FourByteInt


   function nf90_get_var_1D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_1D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_1D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_1D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_1D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_1D_FourByteReal


   function nf90_get_var_2D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_2D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_2D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_2D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_2D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_2D_FourByteReal


   function nf90_get_var_3D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_3D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_3D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_3D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_3D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_3D_FourByteReal


   function nf90_get_var_4D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_4D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_4D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_4D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_4D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_4D_FourByteReal


   function nf90_get_var_5D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_5D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_5D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_5D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_5D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_5D_FourByteReal


   function nf90_get_var_6D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_6D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_6D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_6D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_6D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_6D_FourByteReal


   function nf90_get_var_7D_FourByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = FourByteReal), dimension(:, :, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_7D_FourByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_7D_FourByteReal = &
          nf_get_varm_real(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_7D_FourByteReal = &
          nf_get_vars_real(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_7D_FourByteReal = &
          nf_get_vara_real(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_7D_FourByteReal


   function nf90_get_var_1D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_1D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_1D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_1D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_1D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_1D_EightByteReal


   function nf90_get_var_2D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_2D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_2D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_2D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_2D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_2D_EightByteReal


   function nf90_get_var_3D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_3D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_3D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_3D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_3D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_3D_EightByteReal


   function nf90_get_var_4D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_4D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_4D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_4D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_4D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_4D_EightByteReal


   function nf90_get_var_5D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_5D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_5D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_5D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_5D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_5D_EightByteReal


   function nf90_get_var_6D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_6D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_6D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_6D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_6D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_6D_EightByteReal


   function nf90_get_var_7D_EightByteReal(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     real (kind = EightByteReal), dimension(:, :, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_7D_EightByteReal
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_7D_EightByteReal = &
          nf_get_varm_double(ncid, varid, localStart, localCount, localStride, localMap, values)
     else if(present(stride)) then
       nf90_get_var_7D_EightByteReal = &
          nf_get_vars_double(ncid, varid, localStart, localCount, localStride, values)
     else
       nf90_get_var_7D_EightByteReal = &
          nf_get_vara_double(ncid, varid, localStart, localCount, values)
     end if
   end function nf90_get_var_7D_EightByteReal


