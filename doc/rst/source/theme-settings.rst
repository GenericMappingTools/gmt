##########
GMT Themes
##########

GMT offers 3 build-in themes (classic, modern, and minimal). Classic theme is the
default for classic mode and modern theme is the default for modern mode. You can
also create and use your own themes by compiling files of desired settings and
placing them in your GMT user themes directory (usually ~/.gmt/themes) and name
them *theme*.conf. The GMT_THEME parameter (see :doc:`gmt.conf`) is
used to set the current theme. The table below lists the default settings for the
classic, modern, and minimal themes.

Default settings for build-in themes
------------------------------------
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| Parameter                 | Classic                         | Modern                          | Minimal                         |
+===========================+=================================+=================================+=================================+
| FONT_ANNOT_PRIMARY        | 12p,Helvetica,black             | auto,Helvetica,black            | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_ANNOT_SECONDARY      | 14p,Helvetica,black             | auto,Helvetica,black            | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_HEADING              | 32p,Helvetica,black             | auto,Helvetica-Bold,black       | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_LABEL                | 16p,Helvetica,black             | auto,Helvetica,black            | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_LOGO                 | 8p,Helvetica,black              | auto,Helvetica,black            | auto,Helvetica,black            |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_SUBTITLE             | 18p,Helvetica,black             | auto,Helvetica-Bold,black       | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_TAG                  | 20p,Helvetica,black             | auto,Helvetica,black            | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FONT_TITLE                | 24p,Helvetica,black             | auto,Helvetica-Bold,black       | auto,AvantGarde-Book,black      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| FORMAT_GEO_MAP            | ddd:mm:ssF                      | ddd:mm:ssF                      | ddd:mm:ssF                      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_ANNOT_OFFSET_PRIMARY  | 5p                              | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_ANNOT_OFFSET_SECONDARY| 5p                              | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_FRAME_AXES            | WESNZ                           | WrStZ                           | WrStZ                           |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_FRAME_PEN             | thicker,black                   | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_FRAME_TYPE            | fancy                           | fancy                           | plain                           |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_FRAME_WIDTH           | 5p                              | :ref:`auto <auto-scaling>`      | n/a                             |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_GRID_PEN_PRIMARY      | default,black                   | :ref:`auto <auto-scaling>`      | thinner, lightgrey              |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_GRID_PEN_SECONDARY    | thinner,black                   | :ref:`auto <auto-scaling>`      | thinnest, lightgrey             |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_HEADING_OFFSET        | 18p                             | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_LABEL_OFFSET          | 8p                              | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_TICK_LENGTH_PRIMARY   | 5p/2.5p                         | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_TICK_LENGTH_SECONDARY | 15p/3.75p                       | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_TICK_PEN_PRIMARY      | default,black                   | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_TICK_PEN_SECONDARY    | thinner,black                   | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_TITLE_OFFSET          | 14p                             | :ref:`auto <auto-scaling>`      | :ref:`auto <auto-scaling>`      |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+
| MAP_VECTOR_SHAPE          | 0                               | 0.5                             | 0.5                             |
+---------------------------+---------------------------------+---------------------------------+---------------------------------+