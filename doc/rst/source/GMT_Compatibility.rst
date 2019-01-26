Incompatibilities between GMT 5 and GMT 4
=========================================

As features are added and bugs are discovered, it is occasionally
necessary to break the established syntax of a GMT program option, such as when
the intent of the option is non-unique due to a modifier key being the
same as a distance unit indicator. Other times we see a greatly improved
commonality across similar options by making minor adjustments. However,
we are aware that such changes may cause grief and trouble with
established scripts and the habits of many GMT users. To alleviate this
situation we have introduced a configuration that allows GMT to tolerate and process
obsolete program syntax (to the extent possible). To activate you must
make sure :ref:`GMT_COMPATIBILITY <GMT_COMPATIBILITY>` is set to 4 in your gmt.conf file.
When not running in compatibility mode any obsolete syntax will be considered as
errors. We recommend that users with prior GMT 4 experience run
GMT 5 in compatibility mode, heed the warnings about obsolete syntax, and
correct their scripts or habits accordingly. When this transition has been
successfully navigated it is better to turn compatibility mode off and leave
the past behind. Occasionally, users will supply an ancient
GMT 3 syntax which may have worked in GMT 4 but is not honored in GMT 5.

Here are a list of known incompatibilities that are correctly processed
with a warning under compatibility mode:

*  GMT **default names**: We have
   organized the default parameters logically by group and renamed several to
   be easier to remember and to group. Old and new names can be found in
   Table :ref:`obsolete <tbl-obsoletedefs>`.
   In addition, a few defaults are no longer recognized,
   such as N_COPIES, PS_COPIES, DOTS_PR_INCH, GMT_CPTDIR, PS_DPI, and PS_EPS,
   TRANSPARENCY. This also means the old common option **-c** for specifying
   PostScript copies is no longer available.

*  **Units**: The unit abbreviation for arc seconds is finally **s**
   instead of **c**, with the same change for upper case in some clock
   format statements.

*  **Contour labels**: The modifiers **+k**\ *fontcolor* and
   **+s**\ *fontsize* are obsolete, now being part of **+f**\ *font*.

*  **Ellipsoids**: Assigning :ref:`PROJ_ELLIPSOID <PROJ_ELLIPSOID>` a file name is
   deprecated, use comma-separated parameters :math:`a, f^{-1}` instead.

*  **Custom symbol macros:** Circle macro symbol **C** is deprecated; use **c** instead.

*  **Map scale**: Used by :doc:`basemap`
   and others. Here, the unit **m** is deprecated; use **M** for statute miles.

*  **3-D perspective**: Some programs used a combination of **-E**,
   **-Z** to set up a 3-D perspective view, but these options were not
   universal. The new 3-D perspective in
   GMT 5 means you instead use the
   common option **-p** to configure the 3-D projection.

*  **Pixel vs. gridline registration:** Some programs used to have a
   local **-F** to turn on pixel registration; now this is a common
   option **-r**.

*  **Table file headers**: For consistency with other common i/o options
   we now use **-h** instead of **-H**.

*  **Segment headers**: These are now automatically detected and hence
   there is no longer a **-m** (or the older **-M** option).

*  **Front symbol**: The syntax for the front symbol has changed from
   **-Sf**\ *spacing/size*\ [**+d**][**+t**][:\ *offset*] to
   **-Sf**\ *spacing*\ [/*size*][**+r+l**][**+f+t+s+c+b**][\ **+o**\ *offset*].

*  **Vector symbol**: With the introduction of geo-vectors there are
   three kinds of vectors that can be drawn: Cartesian (straight)
   vectors with **-Sv** or **-SV**, geo-vectors (great circles) with
   **-S=**, and circular vectors with **-Sm**. These are all composed of
   a line (controlled by pen settings) and 0--2 arrow heads (control by
   fill and outline settings). Many modifiers common to all arrows have
   been introduced using the **+key**\ [*arg*] format. The *size* of a
   vector refers to the length of its head; all other quantities are
   given via modifiers (which have sensible default values). In
   particular, giving size as *vectorwidth/headlength/headwidth* is
   deprecated. See the :doc:`plot` man page for
   a clear description of all modifiers.

*  :doc:`blockmean`: The **-S** and **-Sz**
   options are deprecated; use **-Ss** instead.

*  :doc:`filter1d`: The **-N**\ *ncol/tcol*
   option is deprecated; use **-N**\ *tcol* instead as we automatically
   determine the number of columns in the file.

*  :doc:`gmt2kml`: The **-L** option no longer expects column numbers,
   just the column names.  This allows the extra columns to contain text
   strings but means users have to supply the data columns in the order
   specified by **-L**.

*  :doc:`gmtconvert`: **-F** is
   deprecated; use common option **-o** instead.

*  :doc:`gmtdefaults`: **-L** is
   deprecated; this is now the default behavior.

*  :doc:`gmtmath`: **-F** is deprecated; use
   common option **-o** instead.

*  :doc:`gmtselect`: **-Cf** is deprecated;
   use common specification format **-C-** instead. Also,
   **-N**...\ **o** is deprecated; use **-E** instead.

*  :doc:`grd2xyz`: **-E** is deprecated as
   the ESRI ASCII exchange format is now detected automatically.

*  :doc:`grdcontour`: **-m** is deprecated
   as segment headers are handled automatically.

*  :doc:`grdfft`: **-M** is deprecated; use
   common option **-fg** instead.

*  :doc:`grdgradient`: **-L** is
   deprecated; use common option **-n** instead. Also, **-M** is
   deprecated; use common option **-fg** instead.

*  :doc:`grdlandmask`: **-N**...\ **o**
   is deprecated; use **-E** instead.

*  :doc:`grdimage`: **-S** is deprecated;
   use **-n**\ *mode*\ [**+a**][\ **+t**\ *threshold*] instead.

*  :doc:`grdmath`: LDIST and PDIST now return
   distances in spherical degrees; while in
   GMT 4 it returned km; use
   DEG2KM for conversion, if needed.

*  :doc:`grdproject`: **-S** is
   deprecated; use **-n**\ *mode*\ [**+a**\ ][\ **+t**\ *threshold*]
   instead. Also, **-N** is deprecated; use **-D** instead.

*  :doc:`grdsample`: **-Q** is deprecated;
   use **-n**\ *mode*\ [**+a**][\ **+t**\ *threshold*] instead. Also,
   **-L** is deprecated; use common option **-n** instead, and
   **-N**\ *nx/ny* is deprecated; use **-I**\ *nx*\ **+n**\ /*ny*\ **+n** instead.

*  :doc:`grdtrack`: **-Q** is deprecated;
   use **-n**\ *mode*\ [**+a**][\ **+t**\ *threshold*] instead. Also,
   **-L** is deprecated; use common option **-n** instead, and **-S** is
   deprecated; use common option **-sa** instead.

*  :doc:`grdvector`: **-E** is deprecated;
   use the vector modifier **+jc** as well as the general vector
   specifications discussed earlier.

*  :doc:`grdview`: **-L** is deprecated; use common option **-n** instead.

*  :doc:`nearneighbor`: **-L** is
   deprecated; use common option **-n** instead.

*  :doc:`project`: **-D** is deprecated; use **-**\ **-**\ :ref:`FORMAT_GEO_OUT <FORMAT_GEO_OUT>` instead.

*  :doc:`basemap`: **-G** is deprecated;
   specify canvas color via **-B** modifier **+g**\ *color*.

*  :doc:`coast`: **-m** is deprecated and
   have reverted to **-M** for selecting data output instead of plotting.

*  :doc:`contour`: **-T**\ *indexfile* is deprecated; use **-Q**\ *indexfile*.

*  :doc:`histogram`: **-T**\ *col* is
   deprecated; use common option **-i** instead.

*  :doc:`legend`: Paragraph text header flag > is deprecated; use P instead.

*  :doc:`mask`: **-D**...\ **+n**\ *min* is deprecated; use **-Q** instead.

*  :doc:`rose`: Old vector specifications in
   Option **-M** are deprecated; see new explanations.

*  :doc:`text`: **-m** is deprecated; use
   **-M** to indicate paragraph mode. Also, **-S** is deprecated as
   fonts attributes are now specified via the font itself.

*  :doc:`wiggle`: **-D** is deprecated;
   use common option **-g** to indicate data gaps. Also, **-N** is
   deprecated as all fills are set via the **-G** option.

*  :doc:`plot`: Old vector specifications in
   Option **-S** are deprecated; see new explanations.

*  :doc:`plot3d`: Old vector specifications in
   Option **-S** are deprecated; see new explanations.

*  :doc:`splitxyz`: **-G** is deprecated;
   use common option **-g** to indicate data gaps. Also, **-M** is
   deprecated; use common option **-fg** instead.

*  :doc:`triangulate`: **-m** is
   deprecated; use **-M** to output triangle vertices.

*  :doc:`xyz2grd`: **-E** is deprecated as
   the ESRI ASCII exchange format is one of our recognized formats.
   Also, **-A** (no arguments) is deprecated; use **-Az** instead.

*  grdraster: Now in the main GMT core.  The
   **H**\ *skip* field in ``grdraster.info`` is no longer expected as we automatically
   determine if a raster has a GMT header. Also, to output
   *x,y,z* triplets instead of writing a grid now requires **-T**.

*  :doc:`img2grd <supplements/img/img2grd>`: **-m**\ *inc* is
   deprecated; use **-I**\ *inc* instead.

*  :doc:`velo <supplements/meca/velo>`: Old vector
   specifications are deprecated; see new explanations.

*  :doc:`mgd77convert <supplements/mgd77/mgd77convert>`:
   **-4** is deprecated; use **-D** instead.

*  :doc:`mgd77list <supplements/mgd77/mgd77list>`: The unit
   **m** is deprecated; use **M** for statute miles.

*  :doc:`mgd77manage <supplements/mgd77/mgd77manage>`: The
   unit **m** is deprecated; use **M** for statute miles. The **-Q** is
   deprecated; use **-n**\ *mode*\ [**+a**][\ **+t**\ *threshold*] instead

*  :doc:`mgd77path <supplements/mgd77/mgd77path>`: **-P** is
   deprecated (clashes with
   GMT common options); use **-A** instead.

*  :doc:`backtracker <supplements/spotter/backtracker>`:
   **-C** is deprecated as stage vs. finite rotations are detected
   automatically.

*  :doc:`grdrotater <supplements/spotter/grdrotater>`:
   **-C** is deprecated as stage vs. finite rotations are detected
   automatically. Also, **-T**\ *lon/lat/angle* is now set via
   **-e**\ *lon/lat/angle*.

*  :doc:`grdspotter <supplements/spotter/grdspotter>`:
   **-C** is deprecated as stage vs. finite rotations are detected
   automatically.

*  :doc:`hotspotter <supplements/spotter/hotspotter>`: **-C**
   is deprecated as stage vs. finite rotations are detected
   automatically.

*  :doc:`originater <supplements/spotter/originater>`:
   **-C** is deprecated as stage vs. finite rotations are detected
   automatically.

*  :doc:`rotconverter <supplements/spotter/rotconverter>`:
   **-Ff** selection is deprecated, use **-Ft** instead.

*  :doc:`x2sys_datalist <supplements/x2sys/x2sys_datalist>`:
   The unit **m** is deprecated; use **M** for statute miles.

.. _tbl-obsoletedefs:

+---------------------------------+-----------------------------------------+
| **Old Name**                    | **New Name**                            |
+=================================+=========================================+
| **ANNOT_FONT_PRIMARY**          | **FONT_ANNOT_PRIMARY**                  |
+---------------------------------+-----------------------------------------+
| **ANNOT_FONT_SECONDARY**        | **FONT_ANNOT_SECONDARY**                |
+---------------------------------+-----------------------------------------+
| **ANNOT_FONT_SIZE_PRIMARY**     | **FONT_ANNOT_PRIMARY**                  |
+---------------------------------+-----------------------------------------+
| **ANNOT_FONT_SIZE_SECONDARY**   | **FONT_ANNOT_SECONDARY**                |
+---------------------------------+-----------------------------------------+
| **ANNOT_MIN_ANGLE**             | **MAP_ANNOT_MIN_SPACING**               |
+---------------------------------+-----------------------------------------+
| **ANNOT_OFFSET_PRIMARY**        | **MAP_ANNOT_OFFSET_PRIMARY**            |
+---------------------------------+-----------------------------------------+
| **ANNOT_OFFSET_SECONDARY**      | **MAP_ANNOT_OFFSET_SECONDARY**          |
+---------------------------------+-----------------------------------------+
| **BASEMAP_AXES**                | **MAP_FRAME_AXES**                      |
+---------------------------------+-----------------------------------------+
| **BASEMAP_FRAME_RGB**           | **MAP_DEFAULT_PEN**                     |
+---------------------------------+-----------------------------------------+
| **BASEMAP_TYPE**                | **MAP_FRAME_TYPE**                      |
+---------------------------------+-----------------------------------------+
| **CHAR_ENCODING**               | **PS_CHAR_ENCODING**                    |
+---------------------------------+-----------------------------------------+
| **D_FORMAT**                    | **FORMAT_FLOAT_OUT**                    |
+---------------------------------+-----------------------------------------+
| **DEGREE_SYMBOL**               | **MAP_DEGREE_SYMBOL**                   |
+---------------------------------+-----------------------------------------+
| **ELLIPSOID**                   | **PROJ_ELLIPSOID**                      |
+---------------------------------+-----------------------------------------+
| **FIELD_DELIMITER**             | **IO_COL_SEPARATOR**                    |
+---------------------------------+-----------------------------------------+
| **FRAME_PEN**                   | **MAP_FRAME_PEN**                       |
+---------------------------------+-----------------------------------------+
| **FRAME_WIDTH**                 | **MAP_FRAME_WIDTH**                     |
+---------------------------------+-----------------------------------------+
| **GLOBAL_X_SCALE**              | **PS_SCALE_X**                          |
+---------------------------------+-----------------------------------------+
| **GLOBAL_Y_SCALE**              | **PS_SCALE_Y**                          |
+---------------------------------+-----------------------------------------+
| **GRID_CROSS_SIZE_PRIMARY**     | **MAP_GRID_CROSS_SIZE_PRIMARY**         |
+---------------------------------+-----------------------------------------+
| **GRID_CROSS_SIZE_SECONDARY**   | **MAP_GRID_CROSS_SIZE_SECONDARY**       |
+---------------------------------+-----------------------------------------+
| **GRID_PEN_PRIMARY**            | **MAP_GRID_PEN_PRIMARY**                |
+---------------------------------+-----------------------------------------+
| **GRID_PEN_SECONDARY**          | **MAP_GRID_PEN_SECONDARY**              |
+---------------------------------+-----------------------------------------+
| **GRIDFILE_FORMAT**             | **IO_GRIDFILE_FORMAT**                  |
+---------------------------------+-----------------------------------------+
| **GRIDFILE_SHORTHAND**          | **IO_GRIDFILE_SHORTHAND**               |
+---------------------------------+-----------------------------------------+
| **HEADER_FONT_SIZE**            | **FONT_TITLE**                          |
+---------------------------------+-----------------------------------------+
| **HEADER_FONT**                 | **FONT_TITLE**                          |
+---------------------------------+-----------------------------------------+
| **HEADER_OFFSET**               | **MAP_TITLE_OFFSET**                    |
+---------------------------------+-----------------------------------------+
| **HISTORY**                     | **GMT_HISTORY**                         |
+---------------------------------+-----------------------------------------+
| **HSV_MAX_SATURATION**          | **COLOR_HSV_MAX_S**                     |
+---------------------------------+-----------------------------------------+
| **HSV_MAX_VALUE**               | **COLOR_HSV_MAX_V**                     |
+---------------------------------+-----------------------------------------+
| **HSV_MIN_SATURATION**          | **COLOR_HSV_MIN_S**                     |
+---------------------------------+-----------------------------------------+
| **HSV_MIN_VALUE**               | **COLOR_HSV_MIN_V**                     |
+---------------------------------+-----------------------------------------+
| **INPUT_CLOCK_FORMAT**          | **FORMAT_CLOCK_IN**                     |
+---------------------------------+-----------------------------------------+
| **INPUT_DATE_FORMAT**           | **FORMAT_DATE_IN**                      |
+---------------------------------+-----------------------------------------+
| **INTERPOLANT**                 | **GMT_INTERPOLANT**                     |
+---------------------------------+-----------------------------------------+
| **LABEL_FONT**                  | **FONT_LABEL**                          |
+---------------------------------+-----------------------------------------+
| **LABEL_OFFSET**                | **MAP_LABEL_OFFSET**                    |
+---------------------------------+-----------------------------------------+
| **LINE_STEP**                   | **MAP_LINE_STEP**                       |
+---------------------------------+-----------------------------------------+
| **MAP_SCALE_FACTOR**            | **PROJ_SCALE_FACTOR**                   |
+---------------------------------+-----------------------------------------+
| **MEASURE_UNIT**                | **PROJ_LENGTH_UNIT**                    |
+---------------------------------+-----------------------------------------+
| **NAN_RECORDS**                 | **IO_NAN_RECORDS**                      |
+---------------------------------+-----------------------------------------+
| **OBLIQUE_ANNOTATION**          | **MAP_ANNOT_OBLIQUE**                   |
+---------------------------------+-----------------------------------------+
| **OUTPUT_CLOCK_FORMAT**         | **FORMAT_CLOCK_OUT**                    |
+---------------------------------+-----------------------------------------+
| **OUTPUT_DATE_FORMAT**          | **FORMAT_DATE_OUT**                     |
+---------------------------------+-----------------------------------------+
| **OUTPUT_DEGREE_FORMAT**        | **FORMAT_GEO_OUT**                      |
+---------------------------------+-----------------------------------------+
| **PAGE_COLOR**                  | **PS_PAGE_COLOR**                       |
+---------------------------------+-----------------------------------------+
| **PAGE_ORIENTATION**            | **PS_PAGE_ORIENTATION**                 |
+---------------------------------+-----------------------------------------+
| **PAPER_MEDIA**                 | **PS_MEDIA**                            |
+---------------------------------+-----------------------------------------+
| **PLOT_CLOCK_FORMAT**           | **FORMAT_CLOCK_MAP**                    |
+---------------------------------+-----------------------------------------+
| **PLOT_DATE_FORMAT**            | **FORMAT_DATE_MAP**                     |
+---------------------------------+-----------------------------------------+
| **PLOT_DEGREE_FORMAT**          | **FORMAT_GEO_MAP**                      |
+---------------------------------+-----------------------------------------+
| **POLAR_CAP**                   | **MAP_POLAR_CAP**                       |
+---------------------------------+-----------------------------------------+
| **PS_COLOR**                    | **COLOR_HSV_MAX_V**                     |
+---------------------------------+-----------------------------------------+
| **TICK_LENGTH**                 | **MAP_TICK_LENGTH_PRIMARY\|SECONDARY**  |
+---------------------------------+-----------------------------------------+
| **TICK_PEN**                    | **MAP_TICK_PEN_PRIMARY\|SECONDARY**     |
+---------------------------------+-----------------------------------------+
| **TIME_FORMAT_PRIMARY**         | **FORMAT_TIME_PRIMARY_MAP**             |
+---------------------------------+-----------------------------------------+
| **TIME_FORMAT_SECONDARY**       | **FORMAT_TIME_SECONDARY_MAP**           |
+---------------------------------+-----------------------------------------+
| **UNIX_TIME_FORMAT**            | **FORMAT_TIME_STAMP**                   |
+---------------------------------+-----------------------------------------+
| **UNIX_TIME_POS**               | **MAP_LOGO_POS**                        |
+---------------------------------+-----------------------------------------+
| **UNIX_TIME**                   | **MAP_LOGO**                            |
+---------------------------------+-----------------------------------------+
| **VECTOR_SHAPE**                | **MAP_VECTOR_SHAPE**                    |
+---------------------------------+-----------------------------------------+
| **VERBOSE**                     | **GMT_VERBOSE**                         |
+---------------------------------+-----------------------------------------+
| **WANT_LEAP_SECONDS**           | **TIME_LEAP_SECONDS**                   |
+---------------------------------+-----------------------------------------+
| **X_ORIGIN**                    | **MAP_ORIGIN_X**                        |
+---------------------------------+-----------------------------------------+
| **XY_TOGGLE**                   | **IO_LONLAT_TOGGLE**                    |
+---------------------------------+-----------------------------------------+
| **Y_AXIS_TYPE**                 | **MAP_ANNOT_ORTHO**                     |
+---------------------------------+-----------------------------------------+
| **Y_ORIGIN**                    | **MAP_ORIGIN_Y**                        |
+---------------------------------+-----------------------------------------+
| **Y2K_OFFSET_YEAR**             | **TIME_Y2K_OFFSET_YEAR**                |
+---------------------------------+-----------------------------------------+

Note: While **TIME_LEAP_SECONDS** is a recognized keyword it is
currently not implemented and has no effect.  We reserve the right
to enable this feature in the future.
