:orphan:

Modules
=======

.. note::
   Looking for the *classic mode* modules like ``psxy`` and ``pscoast``? See the
   :doc:`equivalent page for classic mode <modules-classic>`.

This is a list of all GMT core and supplemental modules and their uses,
as well as some utility scripts.
All modules are requested via a call to the :doc:`gmt` program.

.. Add core and supplemental modules to hidden toctrees to
.. suppress "document isn't included in any toctree" warnings

.. toctree::
   :hidden:

   gmt
   gmt-config
   gmt5syntax
   gmt_shell_functions.sh
   gmtswitch
   supplements/img/img2google

.. toctree::
   :hidden:

   basemap
   batch
   begin
   blockmean
   blockmedian
   blockmode
   clear
   clip
   coast
   colorbar
   contour
   dimfilter
   docs
   end
   events
   figure
   filter1d
   fitcircle
   gmt2kml
   gmtbinstats
   gmtconnect
   gmtconvert
   gmtdefaults
   gmtget
   gmtinfo
   gmtlogo
   gmtmath
   gmtregress
   gmtselect
   gmtset
   gmtsimplify
   gmtspatial
   gmtsplit
   gmtvector
   gmtwhich
   grd2cpt
   grd2kml
   grd2xyz
   grdblend
   grdclip
   grdcontour
   grdconvert
   grdcut
   grdedit
   grdfft
   grdfill
   grdfilter
   grdgdal
   grdgradient
   grdhisteq
   grdimage
   grdinfo
   grdinterpolate
   grdlandmask
   grdmask
   grdmath
   grdmix
   grdpaste
   grdproject
   grdsample
   grdselect
   grdtrack
   grdtrend
   grdvector
   grdview
   grdvolume
   greenspline
   histogram
   image
   inset
   kml2gmt
   legend
   makecpt
   mapproject
   mask
   movie
   nearneighbor
   plot
   plot3d
   project
   psconvert
   rose
   sample1d
   solar
   spectrum1d
   sph2grd
   sphdistance
   sphinterpolate
   sphtriangulate
   subplot
   surface
   ternary
   text
   trend1d
   trend2d
   triangulate
   wiggle
   xyz2grd

.. toctree::
   :hidden:

   supplements/geodesy/earthtide
   supplements/geodesy/gpsgridder
   supplements/geodesy/velo
   supplements/gshhg/gshhg
   supplements/img/img2grd
   supplements/mgd77/mgd77convert
   supplements/mgd77/mgd77header
   supplements/mgd77/mgd77info
   supplements/mgd77/mgd77list
   supplements/mgd77/mgd77magref
   supplements/mgd77/mgd77manage
   supplements/mgd77/mgd77path
   supplements/mgd77/mgd77sniffer
   supplements/mgd77/mgd77track
   supplements/potential/gmtflexure
   supplements/potential/gmtgravmag3d
   supplements/potential/gravfft
   supplements/potential/gravprisms
   supplements/potential/grdflexure
   supplements/potential/grdgravmag3d
   supplements/potential/grdredpol
   supplements/potential/grdseamount
   supplements/potential/talwani2d
   supplements/potential/talwani3d
   supplements/segy/segyz
   supplements/segy/segy
   supplements/segy/segy2grd
   supplements/seis/coupe
   supplements/seis/meca
   supplements/seis/polar
   supplements/seis/sac
   supplements/seis/grdshake
   supplements/seis/grdvs30
   supplements/spotter/backtracker
   supplements/spotter/gmtpmodeler
   supplements/spotter/grdpmodeler
   supplements/spotter/grdrotater
   supplements/spotter/grdspotter
   supplements/spotter/hotspotter
   supplements/spotter/originater
   supplements/spotter/polespotter
   supplements/spotter/rotconverter
   supplements/spotter/rotsmoother
   supplements/windbarbs/barb
   supplements/windbarbs/grdbarb
   supplements/x2sys/x2sys_binlist
   supplements/x2sys/x2sys_cross
   supplements/x2sys/x2sys_datalist
   supplements/x2sys/x2sys_get
   supplements/x2sys/x2sys_init
   supplements/x2sys/x2sys_list
   supplements/x2sys/x2sys_merge
   supplements/x2sys/x2sys_put
   supplements/x2sys/x2sys_report
   supplements/x2sys/x2sys_solve

Program
-------

- :doc:`gmt`

Core Modules
------------

.. hlist::
    :columns: 6

    - :doc:`basemap`
    - :doc:`batch`
    - :doc:`begin`
    - :doc:`gmtbinstats`
    - :doc:`blockmean`
    - :doc:`blockmedian`
    - :doc:`blockmode`
    - :doc:`clear`
    - :doc:`clip`
    - :doc:`coast`
    - :doc:`colorbar`
    - :doc:`gmtconnect`
    - :doc:`contour`
    - :doc:`gmtconvert`
    - :doc:`gmtdefaults`
    - :doc:`dimfilter`
    - :doc:`docs`
    - :doc:`end`
    - :doc:`events`
    - :doc:`figure`
    - :doc:`filter1d`
    - :doc:`fitcircle`
    - :doc:`gmtget`
    - :doc:`gmt2kml`
    - :doc:`grd2cpt`
    - :doc:`grd2kml`
    - :doc:`grd2xyz`
    - :doc:`grdblend`
    - :doc:`grdclip`
    - :doc:`grdcontour`
    - :doc:`grdconvert`
    - :doc:`grdcut`
    - :doc:`grdedit`
    - :doc:`grdfft`
    - :doc:`grdfill`
    - :doc:`grdfilter`
    - :doc:`grdgdal`
    - :doc:`grdgradient`
    - :doc:`grdhisteq`
    - :doc:`grdimage`
    - :doc:`grdinfo`
    - :doc:`grdinterpolate`
    - :doc:`grdlandmask`
    - :doc:`grdmask`
    - :doc:`grdmath`
    - :doc:`grdmix`
    - :doc:`grdpaste`
    - :doc:`grdproject`
    - :doc:`grdsample`
    - :doc:`grdselect`
    - :doc:`grdtrack`
    - :doc:`grdtrend`
    - :doc:`grdvector`
    - :doc:`grdview`
    - :doc:`grdvolume`
    - :doc:`greenspline`
    - :doc:`histogram`
    - :doc:`image`
    - :doc:`gmtinfo`
    - :doc:`inset`
    - :doc:`kml2gmt`
    - :doc:`legend`
    - :doc:`gmtlogo`
    - :doc:`makecpt`
    - :doc:`mapproject`
    - :doc:`mask`
    - :doc:`gmtmath`
    - :doc:`movie`
    - :doc:`nearneighbor`
    - :doc:`plot`
    - :doc:`plot3d`
    - :doc:`project`
    - :doc:`psconvert`
    - :doc:`gmtregress`
    - :doc:`rose`
    - :doc:`sample1d`
    - :doc:`gmtselect`
    - :doc:`gmtset`
    - :doc:`gmtsimplify`
    - :doc:`solar`
    - :doc:`gmtspatial`
    - :doc:`spectrum1d`
    - :doc:`sph2grd`
    - :doc:`sphdistance`
    - :doc:`sphinterpolate`
    - :doc:`sphtriangulate`
    - :doc:`gmtsplit`
    - :doc:`subplot`
    - :doc:`surface`
    - :doc:`ternary`
    - :doc:`text`
    - :doc:`trend1d`
    - :doc:`trend2d`
    - :doc:`triangulate`
    - :doc:`gmtvector`
    - :doc:`gmtwhich`
    - :doc:`wiggle`
    - :doc:`xyz2grd`

Supplemental Modules
--------------------

.. hlist::
    :columns: 6

    - :doc:`/supplements/geodesy/earthtide`
    - :doc:`/supplements/geodesy/gpsgridder`
    - :doc:`/supplements/geodesy/velo`
    - :doc:`/supplements/gsfml/fzanalyzer`
    - :doc:`/supplements/gsfml/fzblender`
    - :doc:`/supplements/gsfml/fzinformer`
    - :doc:`/supplements/gsfml/fzmapper`
    - :doc:`/supplements/gsfml/fzmodeler`
    - :doc:`/supplements/gsfml/fzprofiler`
    - :doc:`/supplements/gsfml/mlconverter`
    - :doc:`/supplements/gshhg/gshhg`
    - :doc:`/supplements/img/img2grd`
    - :doc:`/supplements/mgd77/mgd77convert`
    - :doc:`/supplements/mgd77/mgd77header`
    - :doc:`/supplements/mgd77/mgd77info`
    - :doc:`/supplements/mgd77/mgd77list`
    - :doc:`/supplements/mgd77/mgd77magref`
    - :doc:`/supplements/mgd77/mgd77manage`
    - :doc:`/supplements/mgd77/mgd77path`
    - :doc:`/supplements/mgd77/mgd77sniffer`
    - :doc:`/supplements/mgd77/mgd77track`
    - :doc:`/supplements/potential/gmtflexure`
    - :doc:`/supplements/potential/gravfft`
    - :doc:`/supplements/potential/gmtgravmag3d`
    - :doc:`/supplements/potential/gravprisms`
    - :doc:`/supplements/potential/grdflexure`
    - :doc:`/supplements/potential/grdgravmag3d`
    - :doc:`/supplements/potential/grdredpol`
    - :doc:`/supplements/potential/grdseamount`
    - :doc:`/supplements/potential/talwani2d`
    - :doc:`/supplements/potential/talwani3d`
    - :doc:`/supplements/segy/segy2grd`
    - :doc:`/supplements/segy/segy`
    - :doc:`/supplements/segy/segyz`
    - :doc:`/supplements/seis/coupe`
    - :doc:`/supplements/seis/meca`
    - :doc:`/supplements/seis/polar`
    - :doc:`/supplements/seis/sac`
    - :doc:`/supplements/spotter/backtracker`
    - :doc:`/supplements/spotter/grdpmodeler`
    - :doc:`/supplements/spotter/grdrotater`
    - :doc:`/supplements/spotter/grdspotter`
    - :doc:`/supplements/spotter/hotspotter`
    - :doc:`/supplements/spotter/originater`
    - :doc:`/supplements/spotter/gmtpmodeler`
    - :doc:`/supplements/spotter/polespotter`
    - :doc:`/supplements/spotter/rotconverter`
    - :doc:`/supplements/spotter/rotsmoother`
    - :doc:`/supplements/windbarbs/barb`
    - :doc:`/supplements/windbarbs/grdbarb`
    - :doc:`/supplements/x2sys/x2sys_binlist`
    - :doc:`/supplements/x2sys/x2sys_cross`
    - :doc:`/supplements/x2sys/x2sys_datalist`
    - :doc:`/supplements/x2sys/x2sys_get`
    - :doc:`/supplements/x2sys/x2sys_init`
    - :doc:`/supplements/x2sys/x2sys_list`
    - :doc:`/supplements/x2sys/x2sys_merge`
    - :doc:`/supplements/x2sys/x2sys_put`
    - :doc:`/supplements/x2sys/x2sys_report`
    - :doc:`/supplements/x2sys/x2sys_solve`

Utility Scripts
---------------

.. hlist::
    :columns: 6

    - :doc:`gmt-config`
    - :doc:`gmt5syntax`
    - :doc:`gmt_shell_functions.sh`
    - :doc:`gmtswitch`
    - :doc:`supplements/img/img2google`


.. include:: module_core_purpose.rst_
.. include:: supplements/module_supplements_purpose.rst_

Session management
------------------

+-----------------------+-------------------+
| :doc:`begin`          | |begin_purpose|   |
+-----------------------+-------------------+
| :doc:`clear`          | |clear_purpose|   |
+-----------------------+-------------------+
| :doc:`docs`           | |docs_purpose|    |
+-----------------------+-------------------+
| :doc:`end`            | |end_purpose|     |
+-----------------------+-------------------+
| :doc:`figure`         | |figure_purpose|  |
+-----------------------+-------------------+
| :doc:`inset`          | |inset_purpose|   |
+-----------------------+-------------------+
| :doc:`subplot`        | |subplot_purpose| |
+-----------------------+-------------------+

Plotting
--------

+-----------------------+-----------------------+
| :doc:`basemap`        | |basemap_purpose|     |
+-----------------------+-----------------------+
| :doc:`clip`           | |clip_purpose|        |
+-----------------------+-----------------------+
| :doc:`coast`          | |coast_purpose|       |
+-----------------------+-----------------------+
| :doc:`colorbar`       | |colorbar_purpose|    |
+-----------------------+-----------------------+
| :doc:`contour`        | |contour_purpose|     |
+-----------------------+-----------------------+
| :doc:`events`         | |events_purpose|      |
+-----------------------+-----------------------+
| :doc:`grdcontour`     | |grdcontour_purpose|  |
+-----------------------+-----------------------+
| :doc:`grdimage`       | |grdimage_purpose|    |
+-----------------------+-----------------------+
| :doc:`grdvector`      | |grdvector_purpose|   |
+-----------------------+-----------------------+
| :doc:`grdview`        | |grdview_purpose|     |
+-----------------------+-----------------------+
| :doc:`histogram`      | |histogram_purpose|   |
+-----------------------+-----------------------+
| :doc:`image`          | |image_purpose|       |
+-----------------------+-----------------------+
| :doc:`legend`         | |legend_purpose|      |
+-----------------------+-----------------------+
| :doc:`gmtlogo`        | |gmtlogo_purpose|     |
+-----------------------+-----------------------+
| :doc:`mask`           | |mask_purpose|        |
+-----------------------+-----------------------+
| :doc:`plot`           | |plot_purpose|        |
+-----------------------+-----------------------+
| :doc:`plot3d`         | |plot3d_purpose|      |
+-----------------------+-----------------------+
| :doc:`rose`           | |rose_purpose|        |
+-----------------------+-----------------------+
| :doc:`solar`          | |solar_purpose|       |
+-----------------------+-----------------------+
| :doc:`ternary`        | |ternary_purpose|     |
+-----------------------+-----------------------+
| :doc:`text`           | |text_purpose|        |
+-----------------------+-----------------------+
| :doc:`wiggle`         | |wiggle_purpose|      |
+-----------------------+-----------------------+

Filtering
---------

+-----------------------+-----------------------+
| :doc:`blockmean`      | |blockmean_purpose|   |
+-----------------------+-----------------------+
| :doc:`blockmedian`    | |blockmedian_purpose| |
+-----------------------+-----------------------+
| :doc:`blockmode`      | |blockmode_purpose|   |
+-----------------------+-----------------------+
| :doc:`dimfilter`      | |dimfilter_purpose|   |
+-----------------------+-----------------------+
| :doc:`filter1d`       | |filter1d_purpose|    |
+-----------------------+-----------------------+
| :doc:`grdfilter`      | |grdfilter_purpose|   |
+-----------------------+-----------------------+

Gridding
--------

+-----------------------+---------------------------+
| :doc:`greenspline`    | |greenspline_purpose|     |
+-----------------------+---------------------------+
| :doc:`nearneighbor`   | |nearneighbor_purpose|    |
+-----------------------+---------------------------+
| :doc:`sphinterpolate` | |sphinterpolate_purpose|  |
+-----------------------+---------------------------+
| :doc:`surface`        | |surface_purpose|         |
+-----------------------+---------------------------+
| :doc:`triangulate`    | |triangulate_purpose|     |
+-----------------------+---------------------------+
| :doc:`grdinterpolate` | |grdinterpolate_purpose|  |
+-----------------------+---------------------------+

Sampling of 1-D and 2-D data
----------------------------

+-----------------------+-----------------------+
| :doc:`grdsample`      | |grdsample_purpose|   |
+-----------------------+-----------------------+
| :doc:`grdtrack`       | |grdtrack_purpose|    |
+-----------------------+-----------------------+
| :doc:`sample1d`       | |sample1d_purpose|    |
+-----------------------+-----------------------+
| :doc:`gmtsimplify`    | |gmtsimplify_purpose| |
+-----------------------+-----------------------+

Projection and map-transformation
---------------------------------

+-----------------------+-----------------------+
| :doc:`grdproject`     | |grdproject_purpose|  |
+-----------------------+-----------------------+
| :doc:`mapproject`     | |mapproject_purpose|  |
+-----------------------+-----------------------+
| :doc:`project`        | |project_purpose|     |
+-----------------------+-----------------------+

Information retrieval
---------------------

+-----------------------+-----------------------+
| :doc:`gmtdefaults`    | |gmtdefaults_purpose| |
+-----------------------+-----------------------+
| :doc:`gmtget`         | |gmtget_purpose|      |
+-----------------------+-----------------------+
| :doc:`grdinfo`        | |grdinfo_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdselect`      | |grdselect_purpose|   |
+-----------------------+-----------------------+
| :doc:`gmtinfo`        | |gmtinfo_purpose|     |
+-----------------------+-----------------------+
| :doc:`gmtset`         | |gmtset_purpose|      |
+-----------------------+-----------------------+

Mathematical operations on tables or grids
------------------------------------------

+-----------------------+---------------------------+
| :doc:`makecpt`        | |makecpt_purpose|         |
+-----------------------+---------------------------+
| :doc:`gmtmath`        | |gmtmath_purpose|         |
+-----------------------+---------------------------+
| :doc:`spectrum1d`     | |spectrum1d_purpose|      |
+-----------------------+---------------------------+
| :doc:`sph2grd`        | |sph2grd_purpose|         |
+-----------------------+---------------------------+
| :doc:`sphdistance`    | |sphdistance_purpose|     |
+-----------------------+---------------------------+
| :doc:`sphtriangulate` | |sphtriangulate_purpose|  |
+-----------------------+---------------------------+

Convert or extract subsets of data
----------------------------------

+-----------------------+-----------------------+
| :doc:`gmtbinstats`    | |gmtbinstats_purpose| |
+-----------------------+-----------------------+
| :doc:`gmtconnect`     | |gmtconnect_purpose|  |
+-----------------------+-----------------------+
| :doc:`gmtconvert`     | |gmtconvert_purpose|  |
+-----------------------+-----------------------+
| :doc:`grd2kml`        | |grd2kml_purpose|     |
+-----------------------+-----------------------+
| :doc:`grd2xyz`        | |grd2xyz_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdblend`       | |grdblend_purpose|    |
+-----------------------+-----------------------+
| :doc:`grdconvert`     | |grdconvert_purpose|  |
+-----------------------+-----------------------+
| :doc:`grdcut`         | |grdcut_purpose|      |
+-----------------------+-----------------------+
| :doc:`grdpaste`       | |grdpaste_purpose|    |
+-----------------------+-----------------------+
| :doc:`gmtselect`      | |gmtselect_purpose|   |
+-----------------------+-----------------------+
| :doc:`gmtspatial`     | |gmtspatial_purpose|  |
+-----------------------+-----------------------+
| :doc:`gmtsplit`       | |gmtsplit_purpose|    |
+-----------------------+-----------------------+
| :doc:`gmtvector`      | |gmtvector_purpose|   |
+-----------------------+-----------------------+
| :doc:`xyz2grd`        | |xyz2grd_purpose|     |
+-----------------------+-----------------------+

Trends in 1-D and 2-D data
--------------------------

+-----------------------+-----------------------+
| :doc:`fitcircle`      | |fitcircle_purpose|   |
+-----------------------+-----------------------+
| :doc:`grdtrend`       | |grdtrend_purpose|    |
+-----------------------+-----------------------+
| :doc:`gmtregress`     | |gmtregress_purpose|  |
+-----------------------+-----------------------+
| :doc:`trend1d`        | |trend1d_purpose|     |
+-----------------------+-----------------------+
| :doc:`trend2d`        | |trend2d_purpose|     |
+-----------------------+-----------------------+

Grid operations
---------------

+-----------------------+-----------------------+
| :doc:`grd2cpt`        | |grd2cpt_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdclip`        | |grdclip_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdedit`        | |grdedit_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdfft`         | |grdfft_purpose|      |
+-----------------------+-----------------------+
| :doc:`grdfill`        | |grdfill_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdgradient`    | |grdgradient_purpose| |
+-----------------------+-----------------------+
| :doc:`grdhisteq`      | |grdhisteq_purpose|   |
+-----------------------+-----------------------+
| :doc:`grdlandmask`    | |grdlandmask_purpose| |
+-----------------------+-----------------------+
| :doc:`grdmask`        | |grdmask_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdmath`        | |grdmath_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdmix`         | |grdmix_purpose|      |
+-----------------------+-----------------------+
| :doc:`grdvolume`      | |grdvolume_purpose|   |
+-----------------------+-----------------------+

Miscellaneous
-------------

+-----------------------+-----------------------+
| :doc:`batch`          | |batch_purpose|       |
+-----------------------+-----------------------+
| :doc:`gmt2kml`        | |gmt2kml_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdgdal`        | |grdgdal_purpose|     |
+-----------------------+-----------------------+
| :doc:`kml2gmt`        | |kml2gmt_purpose|     |
+-----------------------+-----------------------+
| :doc:`movie`          | |movie_purpose|       |
+-----------------------+-----------------------+
| :doc:`psconvert`      | |psconvert_purpose|   |
+-----------------------+-----------------------+
| :doc:`gmtwhich`       | |gmtwhich_purpose|    |
+-----------------------+-----------------------+

geodesy
-------

+--------------------------------------------+----------------------+
| :doc:`/supplements/geodesy/earthtide`      | |earthtide_purpose|  |
+--------------------------------------------+----------------------+
| :doc:`/supplements/geodesy/gpsgridder`     | |gpsgridder_purpose| |
+--------------------------------------------+----------------------+
| :doc:`/supplements/geodesy/velo`           | |velo_purpose|       |
+--------------------------------------------+----------------------+

GSFML
-----

+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/fzanalyzer`  | |fzanalyzer_purpose|  |
+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/fzblender`   | |fzblender_purpose|   |
+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/fzinformer`  | |fzinformer_purpose|  |
+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/fzmapper`    | |fzmapper_purpose|    |
+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/fzmodeler`   | |fzmodeler_purpose|   |
+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/fzprofiler`  | |fzprofiler_purpose|  |
+---------------------------------------+-----------------------+
| :doc:`/supplements/gsfml/mlconverter` | |mlconverter_purpose| |
+---------------------------------------+-----------------------+

GSHHG
-----

+---------------------------------+-----------------+
| :doc:`/supplements/gshhg/gshhg` | |gshhg_purpose| |
+---------------------------------+-----------------+

IMG
---

+----------------------------------------+-------------------+
| :doc:`/supplements/img/img2grd`        | |img2grd_purpose| |
+----------------------------------------+-------------------+

MGD77
-----

+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77convert`   | |mgd77convert_purpose| |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77header`    | |mgd77header_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77info`      | |mgd77info_purpose|    |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77list`      | |mgd77list_purpose|    |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77magref`    | |mgd77magref_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77manage`    | |mgd77manage_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77path`      | |mgd77path_purpose|    |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77sniffer`   | |mgd77sniffer_purpose| |
+------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77track`     | |mgd77track_purpose|   |
+------------------------------------------+------------------------+

potential
---------

+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gmtflexure`   | |gmtflexure_purpose|     |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gravfft`      | |gravfft_purpose|        |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gmtgravmag3d` | |gmtgravmag3d_purpose|   |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gravprisms`   | |gravprisms_purpose|     |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/grdflexure`   | |grdflexure_purpose|     |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/grdgravmag3d` | |grdgravmag3d_purpose|   |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/grdredpol`    | |grdredpol_purpose|      |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/grdseamount`  | |grdseamount_purpose|    |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/talwani2d`    | |talwani2d_purpose|      |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/talwani3d`    | |talwani3d_purpose|      |
+--------------------------------------------+--------------------------+

SEGY
----

+-----------------------------------+-----------------------+
| :doc:`/supplements/segy/segyz`    | |segyz_purpose|       |
+-----------------------------------+-----------------------+
| :doc:`/supplements/segy/segy`     | |segy_purpose|        |
+-----------------------------------+-----------------------+
| :doc:`/supplements/segy/segy2grd` | |segy2grd_purpose|    |
+-----------------------------------+-----------------------+

seis
----

+-----------------------------------+--------------------+
| :doc:`/supplements/seis/coupe`    | |coupe_purpose|    |
+-----------------------------------+--------------------+
| :doc:`/supplements/seis/meca`     | |meca_purpose|     |
+-----------------------------------+--------------------+
| :doc:`/supplements/seis/polar`    | |polar_purpose|    |
+-----------------------------------+--------------------+
| :doc:`/supplements/seis/sac`      | |sac_purpose|      |
+-----------------------------------+--------------------+
| :doc:`/supplements/seis/grdshake` | |grdshake_purpose| |
+-----------------------------------+--------------------+
| :doc:`/supplements/seis/grdvs30`  | |grdvs30_purpose|  |
+-----------------------------------+--------------------+

spotter
-------

+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/backtracker`  | |backtracker_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/grdpmodeler`  | |grdpmodeler_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/grdrotater`   | |grdrotater_purpose|   |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/grdspotter`   | |grdspotter_purpose|   |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/hotspotter`   | |hotspotter_purpose|   |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/originater`   | |originater_purpose|   |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/gmtpmodeler`  | |gmtpmodeler_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/polespotter`  | |polespotter_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/rotconverter` | |rotconverter_purpose| |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/rotsmoother`  | |rotsmoother_purpose|  |
+------------------------------------------+------------------------+

windbarbs
---------

+----------------------------------------+-------------------+
| :doc:`/supplements/windbarbs/barb`     | |barb_purpose|    |
+----------------------------------------+-------------------+
| :doc:`/supplements/windbarbs/grdbarb`  | |grdbarb_purpose| |
+----------------------------------------+-------------------+

x2sys
-----

+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_binlist`  | |x2sys_binlist_purpose|    |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_cross`    | |x2sys_cross_purpose|      |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_datalist` | |x2sys_datalist_purpose|   |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_get`      | |x2sys_get_purpose|        |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_init`     | |x2sys_init_purpose|       |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_list`     | |x2sys_list_purpose|       |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_merge`    | |x2sys_merge_purpose|      |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_put`      | |x2sys_put_purpose|        |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_report`   | |x2sys_report_purpose|     |
+------------------------------------------+----------------------------+
| :doc:`/supplements/x2sys/x2sys_solve`    | |x2sys_solve_purpose|      |
+------------------------------------------+----------------------------+
