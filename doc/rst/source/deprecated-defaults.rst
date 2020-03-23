Deprecated GMT Defaults Names
=============================

Since GMT 5, we have organized the GMT default parameters logically by group and
renamed several to be easier to remember and to group. Old and new names can be found in
the table below. In addition, a few defaults are no longer recognized, such as
**N_COPIES**, **PS_COPIES**, **DOTS_PR_INCH**, **GMT_CPTDIR**, **PS_DPI**,
and **PS_EPS**, **TRANSPARENCY**.

================================== ==========================================
**Old Name in GMT 4**              **New Name since GMT 5**
================================== ==========================================
**ANNOT_FONT_PRIMARY**             **FONT_ANNOT_PRIMARY**
**ANNOT_FONT_SECONDARY**           **FONT_ANNOT_SECONDARY**
**ANNOT_FONT_SIZE_PRIMARY**        **FONT_ANNOT_PRIMARY**
**ANNOT_FONT_SIZE_SECONDARY**      **FONT_ANNOT_SECONDARY**
**ANNOT_MIN_ANGLE**                **MAP_ANNOT_MIN_SPACING**
**ANNOT_OFFSET_PRIMARY**           **MAP_ANNOT_OFFSET_PRIMARY**
**ANNOT_OFFSET_SECONDARY**         **MAP_ANNOT_OFFSET_SECONDARY**
**BASEMAP_AXES**                   **MAP_FRAME_AXES**
**BASEMAP_FRAME_RGB**              **MAP_DEFAULT_PEN**
**BASEMAP_TYPE**                   **MAP_FRAME_TYPE**
**CHAR_ENCODING**                  **PS_CHAR_ENCODING**
**D_FORMAT**                       **FORMAT_FLOAT_OUT**
**DEGREE_SYMBOL**                  **MAP_DEGREE_SYMBOL**
**ELLIPSOID**                      **PROJ_ELLIPSOID**
**FIELD_DELIMITER**                **IO_COL_SEPARATOR**
**FRAME_PEN**                      **MAP_FRAME_PEN**
**FRAME_WIDTH**                    **MAP_FRAME_WIDTH**
**GLOBAL_X_SCALE**                 **PS_SCALE_X**
**GLOBAL_Y_SCALE**                 **PS_SCALE_Y**
**GRID_CROSS_SIZE_PRIMARY**        **MAP_GRID_CROSS_SIZE_PRIMARY**
**GRID_CROSS_SIZE_SECONDARY**      **MAP_GRID_CROSS_SIZE_SECONDARY**
**GRID_PEN_PRIMARY**               **MAP_GRID_PEN_PRIMARY**
**GRID_PEN_SECONDARY**             **MAP_GRID_PEN_SECONDARY**
**GRIDFILE_FORMAT**                **IO_GRIDFILE_FORMAT**
**GRIDFILE_SHORTHAND**             **IO_GRIDFILE_SHORTHAND**
**HEADER_FONT_SIZE**               **FONT_TITLE**
**HEADER_FONT**                    **FONT_TITLE**
**HEADER_OFFSET**                  **MAP_TITLE_OFFSET**
**HISTORY**                        **GMT_HISTORY**
**HSV_MAX_SATURATION**             **COLOR_HSV_MAX_S**
**HSV_MAX_VALUE**                  **COLOR_HSV_MAX_V**
**HSV_MIN_SATURATION**             **COLOR_HSV_MIN_S**
**HSV_MIN_VALUE**                  **COLOR_HSV_MIN_V**
**INPUT_CLOCK_FORMAT**             **FORMAT_CLOCK_IN**
**INPUT_DATE_FORMAT**              **FORMAT_DATE_IN**
**INTERPOLANT**                    **GMT_INTERPOLANT**
**LABEL_FONT**                     **FONT_LABEL**
**LABEL_OFFSET**                   **MAP_LABEL_OFFSET**
**LINE_STEP**                      **MAP_LINE_STEP**
**MAP_SCALE_FACTOR**               **PROJ_SCALE_FACTOR**
**MEASURE_UNIT**                   **PROJ_LENGTH_UNIT**
**NAN_RECORDS**                    **IO_NAN_RECORDS**
**OBLIQUE_ANNOTATION**             **MAP_ANNOT_OBLIQUE**
**OUTPUT_CLOCK_FORMAT**            **FORMAT_CLOCK_OUT**
**OUTPUT_DATE_FORMAT**             **FORMAT_DATE_OUT**
**OUTPUT_DEGREE_FORMAT**           **FORMAT_GEO_OUT**
**PAGE_COLOR**                     **PS_PAGE_COLOR**
**PAGE_ORIENTATION**               **PS_PAGE_ORIENTATION**
**PAPER_MEDIA**                    **PS_MEDIA**
**PLOT_CLOCK_FORMAT**              **FORMAT_CLOCK_MAP**
**PLOT_DATE_FORMAT**               **FORMAT_DATE_MAP**
**PLOT_DEGREE_FORMAT**             **FORMAT_GEO_MAP**
**POLAR_CAP**                      **MAP_POLAR_CAP**
**PS_COLOR**                       **COLOR_HSV_MAX_V**
**TICK_LENGTH**                    **MAP_TICK_LENGTH_PRIMARY\|SECONDARY**
**TICK_PEN**                       **MAP_TICK_PEN_PRIMARY\|SECONDARY**
**TIME_FORMAT_PRIMARY**            **FORMAT_TIME_PRIMARY_MAP**
**TIME_FORMAT_SECONDARY**          **FORMAT_TIME_SECONDARY_MAP**
**UNIX_TIME_FORMAT**               **FORMAT_TIME_STAMP**
**UNIX_TIME_POS**                  **MAP_LOGO_POS**
**UNIX_TIME**                      **MAP_LOGO**
**VECTOR_SHAPE**                   **MAP_VECTOR_SHAPE**
**VERBOSE**                        **GMT_VERBOSE**
**WANT_LEAP_SECONDS**              **TIME_LEAP_SECONDS**
**X_ORIGIN**                       **MAP_ORIGIN_X**
**XY_TOGGLE**                      **IO_LONLAT_TOGGLE**
**Y_AXIS_TYPE**                    **MAP_ANNOT_ORTHO**
**Y_ORIGIN**                       **MAP_ORIGIN_Y**
**Y2K_OFFSET_YEAR**                **TIME_Y2K_OFFSET_YEAR**
================================== ==========================================


**Note**: While **TIME_LEAP_SECONDS** is a recognized keyword it is
currently not implemented and has no effect.  We reserve the right
to enable this feature in the future.
