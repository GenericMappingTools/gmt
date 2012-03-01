   function nf90_put_var_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *),             intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_text

     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride
 
     ! Set local arguments to default values
     localStart (:)  = 1
     localCount (1)  = len(values); localCount (2:) = 1
     localStride(:)  = 1
          
     if(present(start))  localStart (:size(start) ) = start(:)
     if(present(count))  localCount (:size(count) ) = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)

     nf90_put_var_text = nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values)
   end function nf90_put_var_text

   function nf90_get_var_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *),             intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_text
 
     integer, dimension(nf90_max_var_dims) :: localIndex, textDimIDs
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride 
     integer                               :: counter, stringLength
 
     ! Set local arguments to default values
     localStart (:)  = 1
     localCount (1)  = len(values); localCount (2:) = 1
     localStride(:)  = 1
     
     if(present(start))  localStart (:size(start) ) = start(:)
     if(present(count))  localCount (:size(count) ) = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)

     nf90_get_var_text = nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values)
   end function nf90_get_var_text


   function nf90_put_var_1D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_1D_text
 
     integer, parameter                    :: numDims = 1
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_1D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1))
     else
       nf90_put_var_1D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1))
     end if
   end function nf90_put_var_1D_text


   function nf90_put_var_2D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_2D_text
 
     integer, parameter                    :: numDims = 2
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_2D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1))
     else
       nf90_put_var_2D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1))
     end if
   end function nf90_put_var_2D_text


   function nf90_put_var_3D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_3D_text
 
     integer, parameter                    :: numDims = 3
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_3D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1))
     else
       nf90_put_var_3D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1))
     end if
   end function nf90_put_var_3D_text


   function nf90_put_var_4D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_4D_text
 
     integer, parameter                    :: numDims = 4
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_4D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1))
     else
       nf90_put_var_4D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1))
     end if
   end function nf90_put_var_4D_text


   function nf90_put_var_5D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_5D_text
 
     integer, parameter                    :: numDims = 5
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1, 1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_5D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1,1))
     else
       nf90_put_var_5D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1,1))
     end if
   end function nf90_put_var_5D_text


   function nf90_put_var_6D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_6D_text
 
     integer, parameter                    :: numDims = 6
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: counter

     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1, 1, 1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_6D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1,1,1))
     else
       nf90_put_var_6D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1,1,1))
     end if
   end function nf90_put_var_6D_text


   function nf90_put_var_7D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :, :, :, :), &
                                      intent( in) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_put_var_7D_text
 
     integer, parameter                  :: numDims = 7
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount ( :numDims+1) = (/ len(values(1, 1, 1, 1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_put_var_7D_text = &
          nf_put_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1,1,1,1))
     else
       nf90_put_var_7D_text = &
          nf_put_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1,1,1,1))
     end if
   end function nf90_put_var_7D_text


   function nf90_get_var_1D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_1D_text
 
     integer, parameter                  :: numDims = 1
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1/)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_1D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1))
     else
       nf90_get_var_1D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1))
     end if
   end function nf90_get_var_1D_text


   function nf90_get_var_2D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_2D_text
 
     integer, parameter                  :: numDims = 2
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_2D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1))
     else
       nf90_get_var_2D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1))
     end if
   end function nf90_get_var_2D_text


   function nf90_get_var_3D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_3D_text
 
     integer, parameter                  :: numDims = 3
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_3D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1))
     else
       nf90_get_var_3D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1))
     end if
   end function nf90_get_var_3D_text


   function nf90_get_var_4D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_4D_text
 
     integer, parameter                  :: numDims = 4
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_4D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1))
     else
       nf90_get_var_4D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1))
     end if
   end function nf90_get_var_4D_text


   function nf90_get_var_5D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_5D_text
 
     integer, parameter                  :: numDims = 5
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1, 1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_5D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1,1))
     else
       nf90_get_var_5D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1,1))
     end if
   end function nf90_get_var_5D_text


   function nf90_get_var_6D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_6D_text
 
     integer, parameter                  :: numDims = 6
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1, 1, 1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_6D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1,1,1))
     else
       nf90_get_var_6D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1,1,1))
     end if
   end function nf90_get_var_6D_text


   function nf90_get_var_7D_text(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     character (len = *), dimension(:, :, :, :, :, :, :), &
                                      intent(out) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: nf90_get_var_7D_text
 
     integer, parameter                  :: numDims = 7
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                             :: counter
 
     ! Set local arguments to default values
     localStart (:         ) = 1
     localCount (:numDims+1) = (/ len(values(1, 1, 1, 1, 1, 1, 1)), shape(values) /)
     localCount (numDims+2:) = 0
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start))  = start(:)
     if(present(count))  localCount (:size(count))  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       nf90_get_var_7D_text = &
          nf_get_varm_text(ncid, varid, localStart, localCount, localStride, localMap, values(1,1,1,1,1,1,1))
     else
       nf90_get_var_7D_text = &
          nf_get_vars_text(ncid, varid, localStart, localCount, localStride, values(1,1,1,1,1,1,1))
     end if
   end function nf90_get_var_7D_text
