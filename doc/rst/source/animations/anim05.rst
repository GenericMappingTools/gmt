.. _anim05:

(5) Control spline gridding via eigenvalues
-------------------------------------------

Our next animation performs gridding via Green's splines that allows to the selection of an approximate fit
by ignoring the smaller eigenvalues.  However, it is not easy to know how many
eigenvalues to include.  This animation illustrates gridding of bathymetry data
(black squares) from ship data. We show the earth_relief_01m for comparison in
the left panel and the gridded result in the right panel as a function of the
number of eigenvalues included.  The middle panel shows the incremental values
added as new eigenvalues are included.  Note the data distribution is such that
we have a good coverage in the north with larger gaps in the south - the solution
quality reflects this disparity.  We also report the RMS misfit between the model
and the data as the solution builds incrementally.

.. literalinclude:: /_verbatim/anim05.txt
   :language: bash

..  youtube:: xxxxxxxxxx
    :width: 100%
