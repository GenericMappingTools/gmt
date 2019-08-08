.. _supplemental_modules_classic:

GMT Supplemental Modules (Classic Mode)
=======================================

This is a list of GMT "classic mode" supplemental modules and their respective documentation.
These modules are fully compatible with GMT 4 and 5.

.. note::

   Looking for the *modern mode* modules like ``meca`` and ``velo``? See the
   :ref:`equivalent page for modern mode <supplemental_modules>`.

geodesy
-------

+--------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/geodesy/earthtide`      | Compute grids or time-series of solid Earth tides                                 |
+--------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/geodesy/gpsgridder`     | Interpolate GPS velocity vectors using Green's functions for a thin elastic sheet |
+--------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/geodesy/psvelo`         | Plot velocity vectors, crosses, and wedges on maps                                |
+--------------------------------------------+-----------------------------------------------------------------------------------+

GSHHG
-----

+---------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/gshhg/gshhg` | Extract data tables from binary GSHHS or WDBII data files |
+---------------------------------+-----------------------------------------------------------+

IMG
---

+----------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/img/img2grd`        | Extract a subset from an img file in Mercator or Geographic format                |
+----------------------------------------+-----------------------------------------------------------------------------------+

MGD77
-----

+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77convert`   | Convert MGD77 data to other file formats                                          |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77header`    | Create MGD77 headers from A77 files                                               |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77info`      | Extract information about MGD77 files                                             |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77list`      | Extract data from MGD77 files                                                     |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77magref`    | Evaluate the IGRF or CM4 magnetic field models                                    |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77manage`    | Manage the content of MGD77+ files                                                |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77path`      | Return paths to MGD77 cruises and directories                                     |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77sniffer`   | Along-track quality control of MGD77 cruises                                      |
+------------------------------------------+-----------------------------------------------------------------------------------+
| :doc:`/supplements/mgd77/mgd77track`     | Plot track-line map of MGD77 cruises                                              |
+------------------------------------------+-----------------------------------------------------------------------------------+

potential
---------

+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/gmtflexure`   | Compute flexural deformation of 2-D loads, forces, and bending moments                 |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/gmtgravmag3d` | Compute the gravity/magnetic anomaly of a 3-D body by the method of Okabe              |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/gravfft`      | Spectral calculations of gravity, isostasy, admittance, and coherence for grids        |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdflexure`   | Compute flexural deformation of 3-D surfaces for various rheologies                    |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdgravmag3d` | Computes the gravity effect of one (or two) grids by the method of Okabe               |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdredpol`    | Compute the Continuous Reduction To the Pole, AKA differential RTP                     |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/grdseamount`  | Create synthetic seamounts (Gaussian, parabolic, cone or disc, circular or elliptical) |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/talwani2d`    | Compute geopotential anomalies over 2-D bodies by the method of Talwani                |
+--------------------------------------------+----------------------------------------------------------------------------------------+
| :doc:`/supplements/potential/talwani3d`    | Compute geopotential anomalies over 3-D bodies by the method of Talwani                |
+--------------------------------------------+----------------------------------------------------------------------------------------+

SEGY
----

+-----------------------------------+------------------------------------+
| :doc:`/supplements/segy/pssegyz`  | Plot a SEGY file on a map in 3-D   |
+-----------------------------------+------------------------------------+
| :doc:`/supplements/segy/pssegy`   | Plot a SEGY file on a map          |
+-----------------------------------+------------------------------------+
| :doc:`/supplements/segy/segy2grd` | Converting SEGY data to a GMT grid |
+-----------------------------------+------------------------------------+

seis
----

+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/pscoupe` | Plot cross-sections of focal mechanisms                   |
+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/psmeca`  | Plot focal mechanisms on maps                             |
+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/pspolar` | Plot polarities on the inferior focal half-sphere on maps |
+----------------------------------+-----------------------------------------------------------+
| :doc:`/supplements/seis/pssac`   | Plot seismograms in SAC format on maps                    |
+----------------------------------+-----------------------------------------------------------+

spotter
-------

+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/backtracker`  | Generate forward and backward flowlines and hotspot tracks              |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/gmtpmodeler`  | Evaluate a plate motion model at given locations                        |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/grdpmodeler`  | Evaluate a plate motion model on a geographic grid                      |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/grdrotater`   | Finite rotation reconstruction of geographic grid                       |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/grdspotter`   | Create CVA image from a gravity or topography grid                      |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/hotspotter`   | Create CVA image from seamount locations                                |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/originater`   | Associate seamounts with nearest hotspot point sources                  |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/polespotter`  | Find stage poles given fracture zones and abyssal hills                 |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/rotconverter` | Manipulate total reconstruction and stage rotations                     |
+------------------------------------------+-------------------------------------------------------------------------+
| :doc:`/supplements/spotter/rotsmoother`  | Get mean rotations and covariance matrices from set of finite rotations |
+------------------------------------------+-------------------------------------------------------------------------+

x2sys
-----

+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_binlist`  | Create bin index listing from track data files                     |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_cross`    | Calculate crossovers between track data files                      |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_datalist` | Extract content of track data files                                |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_get`      | Get track listing from track index database                        |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_init`     | Initialize a new x2sys track database                              |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_list`     | Extract subset from crossover data base                            |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_merge`    | Merge an updated COEs table (smaller) into the main table (bigger) |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_put`      | Update track index database from track bin file                    |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_report`   | Report statistics from crossover data base                         |
+------------------------------------------+--------------------------------------------------------------------+
| :doc:`/supplements/x2sys/x2sys_solve`    | Determine least-squares systematic correction from crossovers      |
+------------------------------------------+--------------------------------------------------------------------+
