Modules (Classic Mode)
======================

.. note::

   Looking for the *modern mode* modules like ``begin`` and ``figure``? See the
   :doc:`equivalent page for modern mode <modules>`.

This is a list of all GMT "classic mode" core and supplemental modules and their uses,
as well as some utility scripts.
These modules are fully compatible with GMT 4 and 5.
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
   isogmt
   supplements/img/img2google

.. toctree::
    :hidden:

    blockmean
    blockmedian
    blockmode
    dimfilter
    docs
    filter1d
    fitcircle
    gmt2kml
    gmtconnect
    gmtconvert
    gmtdefaults
    grdgdal
    gmtget
    gmtinfo
    gmtlogo-classic
    gmtmath
    gmtregress
    gmtselect
    gmtset
    gmtsimplify
    gmtspatial
    gmtvector
    gmtwhich
    grd2cpt
    grd2kml
    grd2xyz
    grdblend
    grdclip
    grdcontour-classic
    grdconvert
    grdcut
    grdedit
    grdfft
    grdfill
    grdfilter
    grdgradient
    grdhisteq
    grdimage-classic
    grdinfo
    grdinterpolate
    grdlandmask
    grdmask
    grdmath
    grdpaste
    grdproject
    grdsample
    grdtrack
    grdtrend
    grdvector-classic
    grdview-classic
    grdvolume
    greenspline
    kml2gmt
    makecpt
    mapproject
    nearneighbor
    project
    psbasemap
    psclip
    pscoast
    pscontour
    psconvert
    psevents
    pshistogram
    psimage
    pslegend
    psmask
    psrose
    psscale
    pssolar
    psternary
    pstext
    pswiggle
    psxy
    psxyz
    sample1d
    spectrum1d
    sph2grd
    sphdistance
    sphinterpolate
    sphtriangulate
    splitxyz
    surface
    trend1d
    trend2d
    triangulate
    xyz2grd

.. toctree::
    :hidden:

    supplements/geodesy/earthtide
    supplements/geodesy/gpsgridder
    supplements/geodesy/psvelo
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
    supplements/mgd77/mgd77track-classic
    supplements/potential/gmtflexure
    supplements/potential/gmtgravmag3d
    supplements/potential/gravfft
    supplements/potential/grdflexure
    supplements/potential/grdgravmag3d
    supplements/potential/grdredpol
    supplements/potential/grdseamount
    supplements/potential/talwani2d
    supplements/potential/talwani3d
    supplements/segy/pssegyz
    supplements/segy/pssegy
    supplements/segy/segy2grd
    supplements/seis/pscoupe
    supplements/seis/psmeca
    supplements/seis/pspolar
    supplements/seis/pssac
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

    - :doc:`blockmean`
    - :doc:`blockmedian`
    - :doc:`blockmode`
    - :doc:`dimfilter`
    - :doc:`docs`
    - :doc:`filter1d`
    - :doc:`fitcircle`
    - :doc:`gmt2kml`
    - :doc:`gmtconnect`
    - :doc:`gmtconvert`
    - :doc:`gmtdefaults`
    - :doc:`grdgdal`
    - :doc:`gmtget`
    - :doc:`gmtinfo`
    - :doc:`gmtlogo-classic`
    - :doc:`gmtmath`
    - :doc:`gmtregress`
    - :doc:`gmtselect`
    - :doc:`gmtset`
    - :doc:`gmtsimplify`
    - :doc:`gmtspatial`
    - :doc:`gmtvector`
    - :doc:`gmtwhich`
    - :doc:`grd2cpt`
    - :doc:`grd2kml`
    - :doc:`grd2xyz`
    - :doc:`grdblend`
    - :doc:`grdclip`
    - :doc:`grdcontour-classic`
    - :doc:`grdconvert`
    - :doc:`grdcut`
    - :doc:`grdedit`
    - :doc:`grdfft`
    - :doc:`grdfill`
    - :doc:`grdfilter`
    - :doc:`grdgradient`
    - :doc:`grdhisteq`
    - :doc:`grdimage-classic`
    - :doc:`grdinfo`
    - :doc:`grdinterpolate`
    - :doc:`grdlandmask`
    - :doc:`grdmask`
    - :doc:`grdmath`
    - :doc:`grdpaste`
    - :doc:`grdproject`
    - :doc:`grdsample`
    - :doc:`grdtrack`
    - :doc:`grdtrend`
    - :doc:`grdvector-classic`
    - :doc:`grdview-classic`
    - :doc:`grdvolume`
    - :doc:`greenspline`
    - :doc:`kml2gmt`
    - :doc:`makecpt`
    - :doc:`mapproject`
    - :doc:`nearneighbor`
    - :doc:`project`
    - :doc:`psbasemap`
    - :doc:`psclip`
    - :doc:`pscoast`
    - :doc:`pscontour`
    - :doc:`psconvert`
    - :doc:`psevents`
    - :doc:`pshistogram`
    - :doc:`psimage`
    - :doc:`pslegend`
    - :doc:`psmask`
    - :doc:`psrose`
    - :doc:`psscale`
    - :doc:`pssolar`
    - :doc:`psternary`
    - :doc:`pstext`
    - :doc:`pswiggle`
    - :doc:`psxy`
    - :doc:`psxyz`
    - :doc:`sample1d`
    - :doc:`spectrum1d`
    - :doc:`sph2grd`
    - :doc:`sphdistance`
    - :doc:`sphinterpolate`
    - :doc:`sphtriangulate`
    - :doc:`splitxyz`
    - :doc:`surface`
    - :doc:`trend1d`
    - :doc:`trend2d`
    - :doc:`triangulate`
    - :doc:`xyz2grd`

Supplemental Modules
--------------------

.. hlist::
    :columns: 6

    - :doc:`/supplements/geodesy/earthtide`
    - :doc:`/supplements/geodesy/gpsgridder`
    - :doc:`/supplements/geodesy/psvelo`
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
    - :doc:`/supplements/mgd77/mgd77track-classic`
    - :doc:`/supplements/potential/gmtflexure`
    - :doc:`/supplements/potential/gmtgravmag3d`
    - :doc:`/supplements/potential/gravfft`
    - :doc:`/supplements/potential/grdflexure`
    - :doc:`/supplements/potential/grdgravmag3d`
    - :doc:`/supplements/potential/grdredpol`
    - :doc:`/supplements/potential/grdseamount`
    - :doc:`/supplements/potential/talwani2d`
    - :doc:`/supplements/potential/talwani3d`
    - :doc:`/supplements/segy/pssegy`
    - :doc:`/supplements/segy/pssegyz`
    - :doc:`/supplements/segy/segy2grd`
    - :doc:`/supplements/seis/pscoupe`
    - :doc:`/supplements/seis/psmeca`
    - :doc:`/supplements/seis/pspolar`
    - :doc:`/supplements/seis/pssac`
    - :doc:`/supplements/spotter/backtracker`
    - :doc:`/supplements/spotter/gmtpmodeler`
    - :doc:`/supplements/spotter/grdpmodeler`
    - :doc:`/supplements/spotter/grdrotater`
    - :doc:`/supplements/spotter/grdspotter`
    - :doc:`/supplements/spotter/hotspotter`
    - :doc:`/supplements/spotter/originater`
    - :doc:`/supplements/spotter/polespotter`
    - :doc:`/supplements/spotter/rotconverter`
    - :doc:`/supplements/spotter/rotsmoother`
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
    - :doc:`isogmt`
    - :doc:`supplements/img/img2google`

.. include:: module_core_purpose.rst_
.. include:: supplements/module_supplements_purpose.rst_

Plotting
--------

+-------------------------------+-----------------------+
| :doc:`gmtlogo-classic`        | |gmtlogo_purpose|     |
+-------------------------------+-----------------------+
| :doc:`grdcontour-classic`     | |grdcontour_purpose|  |
+-------------------------------+-----------------------+
| :doc:`grdimage-classic`       | |grdimage_purpose|    |
+-------------------------------+-----------------------+
| :doc:`grdvector-classic`      | |grdvector_purpose|   |
+-------------------------------+-----------------------+
| :doc:`grdview-classic`        | |grdview_purpose|     |
+-------------------------------+-----------------------+
| :doc:`psbasemap`              | |psbasemap_purpose|   |
+-------------------------------+-----------------------+
| :doc:`psclip`                 | |psclip_purpose|      |
+-------------------------------+-----------------------+
| :doc:`pscoast`                | |pscoast_purpose|     |
+-------------------------------+-----------------------+
| :doc:`pscontour`              | |pscontour_purpose|   |
+-------------------------------+-----------------------+
| :doc:`psevents`               | |psevents_purpose|    |
+-------------------------------+-----------------------+
| :doc:`pshistogram`            | |pshistogram_purpose| |
+-------------------------------+-----------------------+
| :doc:`psimage`                | |psimage_purpose|     |
+-------------------------------+-----------------------+
| :doc:`pslegend`               | |pslegend_purpose|    |
+-------------------------------+-----------------------+
| :doc:`psmask`                 | |psmask_purpose|      |
+-------------------------------+-----------------------+
| :doc:`psrose`                 | |psrose_purpose|      |
+-------------------------------+-----------------------+
| :doc:`psscale`                | |psscale_purpose|     |
+-------------------------------+-----------------------+
| :doc:`pssolar`                | |pssolar_purpose|     |
+-------------------------------+-----------------------+
| :doc:`psternary`              | |psternary_purpose|   |
+-------------------------------+-----------------------+
| :doc:`pstext`                 | |pstext_purpose|      |
+-------------------------------+-----------------------+
| :doc:`pswiggle`               | |pswiggle_purpose|    |
+-------------------------------+-----------------------+
| :doc:`psxy`                   | |psxy_purpose|        |
+-------------------------------+-----------------------+
| :doc:`psxyz`                  | |psxyz_purpose|       |
+-------------------------------+-----------------------+

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
| :doc:`gmtsimplify`    | |gmtsimplify_purpose| |
+-----------------------+-----------------------+
| :doc:`grdsample`      | |grdsample_purpose|   |
+-----------------------+-----------------------+
| :doc:`grdtrack`       | |grdtrack_purpose|    |
+-----------------------+-----------------------+
| :doc:`sample1d`       | |sample1d_purpose|    |
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
| :doc:`gmtinfo`        | |gmtinfo_purpose|     |
+-----------------------+-----------------------+
| :doc:`gmtset`         | |gmtset_purpose|      |
+-----------------------+-----------------------+
| :doc:`grdinfo`        | |grdinfo_purpose|     |
+-----------------------+-----------------------+

Mathematical operations on tables or grids
------------------------------------------

+-----------------------+---------------------------+
| :doc:`gmtmath`        | |gmtmath_purpose|         |
+-----------------------+---------------------------+
| :doc:`makecpt`        | |makecpt_purpose|         |
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
| :doc:`gmtconnect`     | |gmtconnect_purpose|  |
+-----------------------+-----------------------+
| :doc:`gmtconvert`     | |gmtconvert_purpose|  |
+-----------------------+-----------------------+
| :doc:`gmtselect`      | |gmtselect_purpose|   |
+-----------------------+-----------------------+
| :doc:`gmtspatial`     | |gmtspatial_purpose|  |
+-----------------------+-----------------------+
| :doc:`gmtvector`      | |gmtvector_purpose|   |
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
| :doc:`splitxyz`       | |splitxyz_purpose|    |
+-----------------------+-----------------------+
| :doc:`xyz2grd`        | |xyz2grd_purpose|     |
+-----------------------+-----------------------+

Trends in 1-D and 2-D data
--------------------------

+-----------------------+-----------------------+
| :doc:`fitcircle`      | |fitcircle_purpose|   |
+-----------------------+-----------------------+
| :doc:`gmtregress`     | |gmtregress_purpose|  |
+-----------------------+-----------------------+
| :doc:`grdtrend`       | |grdtrend_purpose|    |
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
| :doc:`grdvolume`      | |grdvolume_purpose|   |
+-----------------------+-----------------------+

Miscellaneous
-------------

+-----------------------+-----------------------+
| :doc:`docs`           | |docs_purpose|        |
+-----------------------+-----------------------+
| :doc:`gmt2kml`        | |gmt2kml_purpose|     |
+-----------------------+-----------------------+
| :doc:`grdgdal`        | |grdgdal_purpose|     |
+-----------------------+-----------------------+
| :doc:`gmtwhich`       | |gmtwhich_purpose|    |
+-----------------------+-----------------------+
| :doc:`gmtwhich`       | |gmtwhich_purpose|    |
+-----------------------+-----------------------+
| :doc:`kml2gmt`        | |kml2gmt_purpose|     |
+-----------------------+-----------------------+
| :doc:`psconvert`      | |psconvert_purpose|   |
+-----------------------+-----------------------+

geodesy
-------

+--------------------------------------------+----------------------+
| :doc:`/supplements/geodesy/earthtide`      | |earthtide_purpose|  |
+--------------------------------------------+----------------------+
| :doc:`/supplements/geodesy/gpsgridder`     | |gpsgridder_purpose| |
+--------------------------------------------+----------------------+
| :doc:`/supplements/geodesy/psvelo`         | |psvelo_purpose|     |
+--------------------------------------------+----------------------+

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

+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77convert`        | |mgd77convert_purpose| |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77header`         | |mgd77header_purpose|  |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77info`           | |mgd77info_purpose|    |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77list`           | |mgd77list_purpose|    |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77magref`         | |mgd77magref_purpose|  |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77manage`         | |mgd77manage_purpose|  |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77path`           | |mgd77path_purpose|    |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77sniffer`        | |mgd77sniffer_purpose| |
+-----------------------------------------------+------------------------+
| :doc:`/supplements/mgd77/mgd77track-classic`  | |mgd77track_purpose|   |
+-----------------------------------------------+------------------------+

potential
---------

+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gmtflexure`   | |gmtflexure_purpose|     |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gmtgravmag3d` | |gmtgravmag3d_purpose|   |
+--------------------------------------------+--------------------------+
| :doc:`/supplements/potential/gravfft`      | |gravfft_purpose|        |
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
| :doc:`/supplements/segy/pssegyz`  | |pssegyz_purpose|     |
+-----------------------------------+-----------------------+
| :doc:`/supplements/segy/pssegy`   | |pssegy_purpose|      |
+-----------------------------------+-----------------------+
| :doc:`/supplements/segy/segy2grd` | |segy2grd_purpose|    |
+-----------------------------------+-----------------------+

seis
----

+----------------------------------+--------------------+
| :doc:`/supplements/seis/pscoupe` | |pscoupe_purpose|  |
+----------------------------------+--------------------+
| :doc:`/supplements/seis/psmeca`  | |psmeca_purpose|   |
+----------------------------------+--------------------+
| :doc:`/supplements/seis/pspolar` | |pspolar_purpose|  |
+----------------------------------+--------------------+
| :doc:`/supplements/seis/pssac`   | |pssac_purpose|    |
+----------------------------------+--------------------+

spotter
-------

+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/backtracker`  | |backtracker_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/gmtpmodeler`  | |gmtpmodeler_purpose|  |
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
| :doc:`/supplements/spotter/polespotter`  | |polespotter_purpose|  |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/rotconverter` | |rotconverter_purpose| |
+------------------------------------------+------------------------+
| :doc:`/supplements/spotter/rotsmoother`  | |rotsmoother_purpose|  |
+------------------------------------------+------------------------+

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
