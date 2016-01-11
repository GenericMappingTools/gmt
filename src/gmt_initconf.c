/*	$Id$ */

/*! . */
int GMT_initconf(struct GMT_CTRL *GMT) {
	int i, error = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, *size;
	double dval, inc[2];

		/* FORMAT group */

	/* FORMAT_CLOCK_IN */
	strcpy(GMT->current.setting.format_clock_in, "hh:mm:ss");
	gmt_clock_C_format (GMT, GMT->current.setting.format_clock_in, &GMT->current.io.clock_input, 0);
	/* FORMAT_DATE_IN */
	strcpy (GMT->current.setting.format_date_in, "yyyy-mm-dd");
	gmt_date_C_format (GMT, GMT->current.setting.format_date_in, &GMT->current.io.date_input, 0);
	/* FORMAT_CLOCK_OUT */
	strcpy (GMT->current.setting.format_clock_out, "hh:mm:ss");
	gmt_clock_C_format (GMT, GMT->current.setting.format_clock_out, &GMT->current.io.clock_output, 1);
	/* FORMAT_DATE_OUT */
	strcpy (GMT->current.setting.format_date_out, "yyyy-mm-dd");
	gmt_date_C_format (GMT, GMT->current.setting.format_date_out, &GMT->current.io.date_output, 1);
	/* FORMAT_GEO_OUT */
	strcpy (GMT->current.setting.format_geo_out, "D");
	gmt_geo_C_format (GMT);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of GMT_begin */
	/* FORMAT_CLOCK_MAP */
	strcpy (GMT->current.setting.format_clock_map, "hh:mm:ss");
	gmt_clock_C_format (GMT, GMT->current.setting.format_clock_map, &GMT->current.plot.calclock.clock, 2);
	/* FORMAT_DATE_MAP */
	strcpy (GMT->current.setting.format_date_map, "yyyy-mm-dd");
	gmt_date_C_format (GMT, GMT->current.setting.format_date_map, &GMT->current.plot.calclock.date, 2);
	/* FORMAT_GEO_MAP */
	strcpy (GMT->current.setting.format_geo_map, "ddd:mm:ss");
	gmt_plot_C_format (GMT);	/* Can fail if FORMAT_FLOAT_OUT not yet set, but is repeated at the end of GMT_begin */
	/* FORMAT_TIME_PRIMARY_MAP */
	strcpy (GMT->current.setting.format_time[GMT_PRIMARY], "full");
	/* FORMAT_TIME_SECONDARY_MAP */
	strcpy (GMT->current.setting.format_time[GMT_SECONDARY], "full");
	/* FORMAT_FLOAT_OUT */
	strcpy (GMT->current.setting.format_float_out, "%.12g");
	/* FORMAT_FLOAT_MAP */
	strcpy (GMT->current.setting.format_float_map, "%.12g");
	/* FORMAT_TIME_STAMP */
	strcpy (GMT->current.setting.format_time_stamp, "%Y %b %d %H:%M:%S");

		/* FONT group */

	/* FONT_ANNOT_PRIMARY */
	error += GMT_getfont (GMT, "12p,Helvetica,black", &GMT->current.setting.font_annot[GMT_PRIMARY]);
	/* FONT_ANNOT_SECONDARY */
	error += GMT_getfont (GMT, "14p,Helvetica,black", &GMT->current.setting.font_annot[GMT_SECONDARY]);
	/* FONT_TITLE */
	error += GMT_getfont (GMT, "24p,Helvetica,black", &GMT->current.setting.font_title);
	/* FONT_LABEL */
	error += GMT_getfont (GMT, "16p,Helvetica,black", &GMT->current.setting.font_label);
	/* FONT_LOGO */
	error += GMT_getfont (GMT, "8p,Helvetica,black", &GMT->current.setting.font_logo);

		/* MAP group */

	/* MAP_ANNOT_OFFSET_PRIMARY */
	size = strdup("5p");	/* We need to use a variable otherwise GMT_to_inch crashes on Release version on Windows */
	GMT->current.setting.map_annot_offset[GMT_PRIMARY] = GMT_to_inch (GMT, size);
	/* MAP_ANNOT_OFFSET_SECONDARY */
	GMT->current.setting.map_annot_offset[GMT_SECONDARY] = GMT_to_inch (GMT, size);
	/* MAP_ANNOT_OBLIQUE */
	GMT->current.setting.map_annot_oblique = 1;
	/* MAP_ANNOT_MIN_ANGLE */
	GMT->current.setting.map_annot_min_angle = 20;
	/* MAP_ANNOT_MIN_SPACING */
	size[0] = '0';		/* size = "0p" */
	GMT->current.setting.map_annot_min_spacing = GMT_to_inch (GMT, size);
	/* MAP_ANNOT_ORTHO */
	strcpy (GMT->current.setting.map_annot_ortho, "we");
	/* MAP_DEGREE_SYMBOL (ring) */
	GMT->current.setting.map_degree_symbol = gmt_ring;
	/* MAP_FRAME_AXES */
	strcpy (GMT->current.setting.map_frame_axes, "WESNZ");
	for (i = 0; i < 5; i++) GMT->current.map.frame.side[i] = 0;	/* Unset default settings */
	GMT->current.map.frame.draw_box = false;
	error += gmt5_decode_wesnz (GMT, "WESNZ", false);
	/* MAP_DEFAULT_PEN */
	error += GMT_getpen (GMT, "default,black", &GMT->current.setting.map_default_pen);
	/* MAP_FRAME_PEN */
	error += GMT_getpen (GMT, "thicker,black", &GMT->current.setting.map_frame_pen);
	/* MAP_FRAME_TYPE (fancy) */
	GMT->current.setting.map_frame_type = GMT_IS_FANCY;
	/* MAP_FRAME_WIDTH */
	size[0] = '5';		/* size = "5p" */
	dval = GMT_to_inch (GMT, size);
	GMT->current.setting.map_frame_width = dval;
	/* MAP_GRID_CROSS_SIZE_PRIMARY */
	size[0] = '0';		/* size = "0p" */
	dval = GMT_to_inch (GMT, size);
	GMT->current.setting.map_grid_cross_size[GMT_PRIMARY] = dval;
	/* MAP_GRID_CROSS_SIZE_SECONDARY */
	dval = GMT_to_inch (GMT, size);
	GMT->current.setting.map_grid_cross_size[GMT_SECONDARY] = dval;
	/* MAP_GRID_PEN_PRIMARY */
	error += GMT_getpen (GMT, "default,black", &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
	/* MAP_GRID_PEN_SECONDARY */
	error += GMT_getpen (GMT, "thinner,black", &GMT->current.setting.map_grid_pen[GMT_SECONDARY]);
	/* MAP_LABEL_OFFSET */
	size[0] = '8';		/* size = "8p" */
	GMT->current.setting.map_label_offset = GMT_to_inch (GMT, size);
	/* MAP_LINE_STEP */
	free(size);		size = strdup("0.75p");
	GMT->current.setting.map_line_step = GMT_to_inch (GMT, size);
	/* MAP_LOGO */
	error += gmt_true_false_or_error ("false", &GMT->current.setting.map_logo);
	/* MAP_LOGO_POS */
	sscanf ("BL/-54p/-54p", "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
	GMT->current.setting.map_logo_justify = GMT_just_decode (GMT, txt_a, PSL_NO_DEF);
	GMT->current.setting.map_logo_pos[GMT_X] = GMT_to_inch (GMT, txt_b);
	GMT->current.setting.map_logo_pos[GMT_Y] = GMT_to_inch (GMT, txt_c);
	/* MAP_ORIGIN_X */
	free(size);		size = strdup("1i");
	GMT->current.setting.map_origin[GMT_X] = GMT_to_inch (GMT, size);
	/* MAP_ORIGIN_Y */
	GMT->current.setting.map_origin[GMT_Y] = GMT_to_inch (GMT, size);
	/* MAP_POLAR_CAP */
	i = sscanf ("85/90", "%[^/]/%s", txt_a, txt_b);
	error += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf (GMT, txt_a, GMT_IS_LAT, &GMT->current.setting.map_polar_cap[0]), txt_a);
	GMT_getinc (GMT, txt_b, inc);
	GMT->current.setting.map_polar_cap[1] = inc[GMT_X];
	/* MAP_SCALE_HEIGHT */
	free(size);		size = strdup("5p");
	GMT->current.setting.map_scale_height = GMT_to_inch (GMT, size);
	/* MAP_TICK_LENGTH_PRIMARY */
	sscanf ("5p/2.5p", "%[^/]/%s", txt_a, txt_b);
	GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = GMT_to_inch (GMT, txt_a);
	GMT->current.setting.map_tick_length[GMT_TICK_UPPER] = GMT_to_inch (GMT, txt_b);
	/* MAP_TICK_LENGTH_SECONDARY */
	sscanf ("15p/3.75p", "%[^/]/%s", txt_a, txt_b);
	GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = GMT_to_inch (GMT, txt_a);
	GMT->current.setting.map_tick_length[GMT_TICK_LOWER] = GMT_to_inch (GMT, txt_b);
	/* MAP_TICK_PEN_PRIMARY */
	error += GMT_getpen (GMT, "thinner,black", &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);
	/* MAP_TICK_PEN_SECONDARY */
	error += GMT_getpen (GMT, "thinner,black", &GMT->current.setting.map_tick_pen[GMT_SECONDARY]);
	/* MAP_TITLE_OFFSET */
	free(size);		size = strdup("14p");
	GMT->current.setting.map_title_offset = GMT_to_inch (GMT, size);
	/* MAP_VECTOR_SHAPE */
	GMT->current.setting.map_vector_shape = 0;

	free(size);

		/* COLOR group */

	/* COLOR_BACKGROUND */
	error += GMT_getrgb (GMT, "black", GMT->current.setting.color_patch[GMT_BGD]);
	/* COLOR_FOREGROUND */
	error += GMT_getrgb (GMT, "white", GMT->current.setting.color_patch[GMT_FGD]);
	/* COLOR_MODEL */
	GMT->current.setting.color_model = GMT_RGB;
	/* COLOR_NAN */
	error += GMT_getrgb (GMT, "127.5", GMT->current.setting.color_patch[GMT_NAN]);
	/* COLOR_HSV_MIN_S */
	GMT->current.setting.color_hsv_min_s = 1;
	/* COLOR_HSV_MAX_S */
	GMT->current.setting.color_hsv_max_s = 0.1;
	/* COLOR_HSV_MIN_V */
	GMT->current.setting.color_hsv_min_v = 0.3;
	/* COLOR_HSV_MAX_V */
	GMT->current.setting.color_hsv_max_v = 1;

		/* PS group */

	/* PS_CHAR_ENCODING */
	strcpy (GMT->current.setting.ps_encoding.name, "ISOLatin1+");
	gmt_load_encoding (GMT);
	/* PS_COLOR_MODEL */
	GMT->current.setting.ps_color_mode = PSL_RGB;
	/* PS_IMAGE_COMPRESS */
	if (!GMT->PSL) return (0);	/* Not using PSL in this session */
#ifdef HAVE_ZLIB
	GMT->PSL->internal.compress = PSL_DEFLATE;
	GMT->PSL->internal.deflate_level = 5;
#else
	/* Silently fall back to LZW compression when ZLIB not available */
	GMT->PSL->internal.compress = PSL_LZW;
#endif
	/* PS_LINE_CAP */
	if (!GMT->PSL) return (0);	/* Not using PSL in this session */
	GMT->PSL->internal.line_cap = PSL_BUTT_CAP;
	/* PS_LINE_JOIN */
	if (!GMT->PSL) return (0);	/* Not using PSL in this session */
	GMT->PSL->internal.line_join = PSL_MITER_JOIN;
	/* PS_MITER_LIMIT */
	if (!GMT->PSL) return (0);	/* Not using PSL in this session */
	GMT->PSL->internal.miter_limit = 35;
	/* PS_PAGE_COLOR */
	error += GMT_getrgb (GMT, "white", GMT->current.setting.ps_page_rgb);
	/* PS_PAGE_ORIENTATION */
	GMT->current.setting.ps_orientation = PSL_LANDSCAPE;
	/* PS_MEDIA */
	i = gmt_key_lookup ("a4", GMT_media_name, GMT_N_MEDIA);
	/* Use the specified standard format */
	GMT->current.setting.ps_media = i;
	GMT->current.setting.ps_page_size[0] = GMT_media[i].width;
	GMT->current.setting.ps_page_size[1] = GMT_media[i].height;
	/* PS_SCALE_X */
	GMT->current.setting.ps_magnify[GMT_X] = 1;
	/* PS_SCALE_Y */
	GMT->current.setting.ps_magnify[GMT_Y] = 1;
	/* PS_TRANSPARENCY */
	strcpy (GMT->current.setting.ps_transpmode, "Normal");
	/* PS_COMMENTS */
	if (!GMT->PSL) return (0);	/* Not using PSL in this session */
	GMT->PSL->internal.comments = 0;

		/* IO group */

	/* IO_COL_SEPARATOR */
	strcpy (GMT->current.setting.io_col_separator, "\t");
	/* IO_GRIDFILE_FORMAT */
	strcpy (GMT->current.setting.io_gridfile_format, "nf");
	/* IO_GRIDFILE_SHORTHAND */
	error += gmt_true_false_or_error ("false", &GMT->current.setting.io_gridfile_shorthand);
	/* IO_HEADER */
	error += gmt_true_false_or_error ("false", &GMT->current.setting.io_header[GMT_IN]);
	GMT->current.setting.io_header[GMT_OUT] = GMT->current.setting.io_header[GMT_IN];
	/* IO_N_HEADER_RECS */
	GMT->current.setting.io_n_header_items = 0;
	/* IO_NAN_RECORDS (pass) */
	GMT->current.setting.io_nan_records = true;
	/* IO_NC4_CHUNK_SIZE (auto) */
	GMT->current.setting.io_nc4_chunksize[0] = k_netcdf_io_chunked_auto;
	/* IO_NC4_DEFLATION_LEVEL */
	GMT->current.setting.io_nc4_deflation_level = 3;
	/* IO_LONLAT_TOGGLE */
	error += gmt_true_false_or_error ("false", &GMT->current.setting.io_lonlat_toggle[GMT_IN]);
	/* We got false/f/0 or true/t/1. Set outgoing setting to the same as the ingoing. */
	GMT->current.setting.io_lonlat_toggle[GMT_OUT] = GMT->current.setting.io_lonlat_toggle[GMT_IN];
	/* IO_SEGMENT_BINARY */
	GMT->current.setting.n_bin_header_cols = 2;
	/* IO_SEGMENT_MARKER */
	GMT->current.setting.io_seg_marker[GMT_OUT] = GMT->current.setting.io_seg_marker[GMT_IN] = '>';

		/* PROJ group */

	/* PROJ_AUX_LATITUDE (authalic) */
	GMT->current.setting.proj_aux_latitude = GMT_LATSWAP_G2A;
	/* PROJ_ELLIPSOID */
	GMT->current.setting.proj_ellipsoid = GMT_get_ellipsoid (GMT, "WGS-84");
	GMT_init_ellipsoid (GMT);	/* Set parameters depending on the ellipsoid */
	/* PROJ_DATUM (Not implemented yet) */
	/* PROJ_GEODESIC */
	GMT->current.setting.proj_geodesic = GMT_GEODESIC_VINCENTY;
	/* PROJ_LENGTH_UNIT */
	GMT->current.setting.proj_length_unit = GMT_CM;
	/* PROJ_MEAN_RADIUS */
	GMT->current.setting.proj_mean_radius = GMT_RADIUS_AUTHALIC;
	/* PROJ_SCALE_FACTOR (default) */
	GMT->current.setting.proj_scale_factor = -1.0;

		/* GMT group */

	/* GMT_COMPATIBILITY */
	GMT->current.setting.compatibility = 4;
	/* GMT_CUSTOM_LIBS (default to none) */
	/* GMT_EXPORT_TYPE */
	GMT->current.setting.export_type = GMT_DOUBLE;
	/* GMT_EXTRAPOLATE_VAL (NaN) */
	GMT->current.setting.extrapolate_val[0] = GMT_EXTRAPOLATE_NONE;
	/* GMT_FFT */
	GMT->current.setting.fft = k_fft_auto;
	/* GMT_HISTORY */
	GMT->current.setting.history = k_history_read;
	/* GMT_INTERPOLANT */
	GMT->current.setting.interpolant = GMT_SPLINE_AKIMA;
	/* GMT_TRIANGULATE */
	GMT->current.setting.triangulate = GMT_TRIANGLE_SHEWCHUK;
	/* GMT_VERBOSE (compat) */
	error += gmt_parse_V_option (GMT, 'c');

		/* DIR group */

	/* DIR_DATA (These are all empty) */
	/* DIR_DCW */
	/* DIR_GSHHG */

		/* TIME group */

	/* TIME_EPOCH */
	strcpy (GMT->current.setting.time_system.epoch, "1970-01-01T00:00:00");
	(void) GMT_init_time_system_structure (GMT, &GMT->current.setting.time_system);
	/* TIME_IS_INTERVAL */
	GMT->current.setting.time_is_interval = false;
	/* TIME_INTERVAL_FRACTION */
	GMT->current.setting.time_interval_fraction = 0.5;
	/* GMT_LANGUAGE */
	/* TIME_LANGUAGE --- SOME CONFUSION HERE. NO TIME_LANGUAGE IN gmt.conf */
	strcpy (GMT->current.setting.language, "us");
	gmt_get_language (GMT);	/* Load in names and abbreviations in chosen language */
	/* TIME_REPORT */
	GMT->current.setting.timer_mode = GMT_NO_TIMER;
	/* TIME_UNIT */
	GMT->current.setting.time_system.unit = 's';
	(void) GMT_init_time_system_structure (GMT, &GMT->current.setting.time_system);
	/* TIME_WEEK_START */
	GMT->current.setting.time_week_start = gmt_key_lookup ("Monday", GMT_weekdays, 7);
	/* TIME_Y2K_OFFSET_YEAR */
	GMT->current.setting.time_Y2K_offset_year = 1950;
	GMT->current.time.Y2K_fix.y2_cutoff = GMT->current.setting.time_Y2K_offset_year % 100;
	GMT->current.time.Y2K_fix.y100 = GMT->current.setting.time_Y2K_offset_year - GMT->current.time.Y2K_fix.y2_cutoff;
	GMT->current.time.Y2K_fix.y200 = GMT->current.time.Y2K_fix.y100 + 100;

	if (error)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized keyword or value during initialization.\n");
	return 0;
}
