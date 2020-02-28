GMT Supplemental Packages
=========================

These packages are for the most part written and supported by us, but
there are some exceptions. They provide extensions of GMT that are
needed for particular rather than general applications.
Questions or bug reports for this software
should be addressed to the person(s) listed in the ``README`` file associated with
the particular program. It is not guaranteed that these programs are
fully ANSI-C, Y2K, or POSIX compliant, or that they necessarily will
install smoothly on all platforms, but most do. Note that the data sets
some of these programs work on are not distributed with these packages;
they must be obtained separately. The contents of the supplemental
archive may change without notice; at this writing it contains these directories:

gshhg: GSHHG data extractor
---------------------------

This package contains :doc:`gshhg </supplements/gshhg/gshhg>` which you
can use to extract shoreline polygons from the Global Self-consistent
Hierarchical High-resolution Shorelines (GSHHG) available separately
from or the (GSHHG is the polygon data base from which the
GMT coastlines derive). The package is maintained by Paul Wessel.

img: gridded altimetry extractor
--------------------------------

This package consists of the program
:doc:`img2grd </supplements/img/img2grd>` to extract subsets of the
global gravity and predicted topography solutions derived from satellite
altimetry [23]_. The package is maintained by Walter Smith and Paul Wessel.

geodesy: Geodesy
----------------

This package contains the programs
:doc:`earthtide </supplements/geodesy/earthtide>` for computing solid Earth tides,
:doc:`gpsgridder </supplements/geodesy/gpsgridder>` for gridding GPS velocity components, and
:doc:`velo </supplements/geodesy/velo>` for plotting error ellipses, velocity arrows, and rotational wedges.
The velo program was developed by Kurt Feigl and Genevieve
Patau and is now maintained by the GMT team. The earthtide program is a translation to C of a
program written by Dennis Milbert and distributed with his permission.

mgd77: MGD77 extractor and plotting tools
-----------------------------------------

This package currently holds the programs
:doc:`mgd77convert </supplements/mgd77/mgd77convert>`,
:doc:`mgd77header </supplements/mgd77/mgd77header>`,
:doc:`mgd77info </supplements/mgd77/mgd77info>`,
:doc:`mgd77list </supplements/mgd77/mgd77list>`,
:doc:`mgd77magref </supplements/mgd77/mgd77magref>`,
:doc:`mgd77manage </supplements/mgd77/mgd77manage>`,
:doc:`mgd77path </supplements/mgd77/mgd77path>`,
:doc:`mgd77sniffer </supplements/mgd77/mgd77sniffer>`, and
:doc:`mgd77track </supplements/mgd77/mgd77track>` which can be used to
extract information or data values from or plot marine geophysical data
files in the ASCII MGD77 or netCDF MGD77+ formats [24]_). This package
has replaced the old **mgg** package. The package is maintained by Paul Wessel and Mike Chandler.

potential: Geopotential tools
-----------------------------

At the moment, this package contains the programs
:doc:`gravfft </supplements/potential/gravfft>`, which performs gravity,
isostasy, and admittance calculation for grids,
:doc:`flexure </supplements/potential/gmtflexure>` and
:doc:`grdflexure </supplements/potential/grdflexure>`
which calculates flexural deformation for profiles and grids, respectively,
:doc:`grdredpol </supplements/potential/grdredpol>`, which compute the
continuous reduction to the pole, AKA differential RTP for magnetic
data, :doc:`grdseamount </supplements/potential/grdseamount>`, which computes
synthetic bathymetry over various seamount shapes, and
:doc:`gravmag3d </supplements/potential/gmtgravmag3d>` and
:doc:`grdgravmag3d </supplements/potential/grdgravmag3d>`,
which computes the gravity or
magnetic anomaly of a body by the method of Okabe [25]_, and
:doc:`talwani2d </supplements/potential/talwani2d>` and
:doc:`talwani3d </supplements/potential/talwani3d>` and
which uses the methods of Talwani to compute various geopotential components
from 2-D [26]_ or 3-D [27]_ bodies.
The package is maintained by Joaquim Luis and Paul Wessel.

seis: Seismology
----------------

This package contains the programs
:doc:`coupe </supplements/seis/coupe>`,
:doc:`meca </supplements/seis/meca>`,
:doc:`polar </supplements/seis/polar>`, and
:doc:`sac </supplements/seis/sac>` which are used by seismologists
for plotting focal mechanisms (including cross-sections
and polarities) and SAC files.
The coupe, meca, and polar were developed by Kurt Feigl and Genevieve
Patau, while Dongdong Tian added sac; the package is now maintained by the GMT team.

segy: plotting SEGY seismic data
--------------------------------

This package contains programs to plot SEGY seismic data files using the
GMT mapping transformations and postscript library.
:doc:`segy </supplements/segy/segy>` generates a 2-D plot (x:location
and y:time/depth) while :doc:`segyz </supplements/segy/segyz>`
generates a 3-D plot (x and y: location coordinates, z: time/depth).
Locations may be read from predefined or arbitrary portions of each
trace header. Finally, :doc:`segy2grd </supplements/segy/segy2grd>` can
convert SEGY data to a GMT grid file. The package is maintained by Tim Henstock [28]_.

spotter: backtracking and hotspotting
-------------------------------------

This package contains the plate tectonic programs
:doc:`backtracker </supplements/spotter/backtracker>`, which you can use to
move geologic markers forward or backward in time,
:doc:`grdpmodeler </supplements/spotter/grdpmodeler>` which evaluates
predictions of a plate motion model on a grid,
:doc:`grdrotater </supplements/spotter/grdrotater>` which rotates entire
grids using a finite rotation,
:doc:`hotspotter </supplements/spotter/hotspotter>` which generates CVA
grids based on seamount locations and a set of absolute plate motion
stage poles (:doc:`grdspotter </supplements/spotter/grdspotter>` does the
same using a bathymetry grid instead of seamount locations),
:doc:`originater </supplements/spotter/originater>`, which associates
seamounts with the most likely hotspot origins,
:doc:`polespotter </supplements/spotter/polespotter>`, which determines
likely stage pole locations from seafloor fabric, and
:doc:`rotconverter </supplements/spotter/rotconverter>` which does various
operations involving finite rotations on a sphere. The package is
maintained by Paul Wessel.

x2sys: track crossover error estimation
---------------------------------------

This package contains the tools
:doc:`x2sys_datalist </supplements/x2sys/x2sys_datalist>`, which allows
you to extract data from almost any binary or ASCII data file, and
:doc:`x2sys_cross </supplements/x2sys/x2sys_cross>` which determines
crossover locations and errors generated by one or several geospatial
tracks. Newly added are the tools
:doc:`x2sys_init </supplements/x2sys/x2sys_init>`,
:doc:`x2sys_binlist </supplements/x2sys/x2sys_binlist>`,
:doc:`x2sys_get </supplements/x2sys/x2sys_get>`,
:doc:`x2sys_list </supplements/x2sys/x2sys_list>`,
:doc:`x2sys_put </supplements/x2sys/x2sys_put>`,
:doc:`x2sys_report </supplements/x2sys/x2sys_report>`,
:doc:`x2sys_solve </supplements/x2sys/x2sys_solve>` and
:doc:`x2sys_merge </supplements/x2sys/x2sys_merge>` which extends the
track-management system employed by the mgg supplement to generic track
data of any format. This package represents a new generation of tools
and replaces the old **x_system** package. The package is maintained by
Paul Wessel.

.. [23]
   For data bases, see `<http://topex.ucsd.edu/marine_grav/mar_grav.html>`_.

.. [24]
   The ASCII MGD77 data are available on CD-ROM from NCEI (`<http://www.ncei.noaa.gov/>`_).

.. [25]
   Okabe, M., 1979, Analytical expressions for gravity anomalies due to
   polyhedral bodies and translation into magnetic anomalies,
   *Geophysics, 44*, 730–741.

.. [26]
   Talwani, M., J. L. Worzel, and M. Landisman (1959), Rapid gravity computations
   for two-dimensional bodies with application to the Mendocino submarine fracture zone,
   *J. Geophys. Res., 64*, 49–59.

.. [27]
   Talwani, M., and M. Ewing (1960), Rapid computation of gravitational attraction of
   three-dimensional bodies of arbitrary shape, *Geophysics, 25*, 203–225.

.. [28]
   `Timothy J. Henstock <http://www.southampton.ac.uk/oes/research/staff/then.page>`_,
   University of Southampton
