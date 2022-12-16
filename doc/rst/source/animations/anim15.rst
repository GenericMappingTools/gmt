.. _anim15:

(15) Animating 2-D coupled gridding via SVD
===========================================

Gridding via elastically coupled Green's splines via :doc:`/supplements/geodesy/gpsgridder`
is similar to :doc:`/greenspline` gridding in that a SVD-based solution allows for the
use of a subset of the eigenvalues. While a good first step is to use 25% of them, this
animation shows the solutions for all choices of eigenvalues and tracks the reduction
of misfit (both total and separately for the east and north components.)  The movie
shares the same data and setup as one of our test scripts (gpsgridder1.sh) but has been
"weaponized" to do it via animation.  Note as we include the contributions from the
tiniest eigenvalues we rapidly "improve" the misfit while adding spurious variations to
the solution. The moral is to not try to fit the data exactly. 

.. literalinclude:: /_verbatim/anim15.txt
   :language: bash

..  youtube:: Pvvc4vb8G4Y
    :width: 100%
