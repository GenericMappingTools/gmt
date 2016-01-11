/*	$Id$ */

/*! . */
int GMT_initconf(struct GMT_CTRL *GMT) {
	int i, error = 0;
	double const pt = 1.0/72.0;	/* points to inch */

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

	/* MAP_ANNOT_OFFSET_PRIMARY, MAP_ANNOT_OFFSET_SECONDARY */
	GMT->current.setting.map_annot_offset[GMT_PRIMARY] = GMT->current.setting.map_annot_offset[GMT_SECONDARY] = 5 * pt; /* 5p */
	/* MAP_ANNOT_OBLIQUE */
	GMT->current.setting.map_annot_oblique = 1;
	/* MAP_ANNOT_MIN_ANGLE */
	GMT->current.setting.map_annot_min_angle = 20;
	/* MAP_ANNOT_MIN_SPACING */
	GMT->current.setting.map_annot_min_spacing = 0; /* 0p */
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
	GMT->current.setting.map_frame_width = 5 * pt; /* 5p */
	/* MAP_GRID_CROSS_SIZE_PRIMARY, MAP_GRID_CROSS_SIZE_SECONDARY */
	GMT->current.setting.map_grid_cross_size[GMT_PRIMARY] = GMT->current.setting.map_grid_cross_size[GMT_SECONDARY] = 0; /* 0p */
	/* MAP_GRID_PEN_PRIMARY */
	error += GMT_getpen (GMT, "default,black", &GMT->current.setting.map_grid_pen[GMT_PRIMARY]);
	/* MAP_GRID_PEN_SECONDARY */
	error += GMT_getpen (GMT, "thinner,black", &GMT->current.setting.map_grid_pen[GMT_SECONDARY]);
	/* MAP_LABEL_OFFSET */
	GMT->current.setting.map_label_offset = 8 * pt;	/* 8p */
	/* MAP_LINE_STEP */
	GMT->current.setting.map_line_step = 0.75 * pt;	/* 0.75p */
	/* MAP_LOGO */
	GMT->current.setting.map_logo = false;
	/* MAP_LOGO_POS */
	GMT->current.setting.map_logo_justify = PSL_BL;	/* BL */
	GMT->current.setting.map_logo_pos[GMT_X] = GMT->current.setting.map_logo_pos[GMT_Y] = -54 * pt;	/* -54p */
	/* MAP_ORIGIN_X, MAP_ORIGIN_Y */
	GMT->current.setting.map_origin[GMT_X] = GMT->current.setting.map_origin[GMT_Y] = 1;	/* 1i */
	/* MAP_POLAR_CAP */
	GMT->current.setting.map_polar_cap[0] = 85;
	GMT->current.setting.map_polar_cap[1] = 90;
	/* MAP_SCALE_HEIGHT */
	GMT->current.setting.map_scale_height = 5 * pt;	/* 5p */
	/* MAP_TICK_LENGTH_PRIMARY */
	GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER] = 5 * pt;	/* 5p */
	GMT->current.setting.map_tick_length[GMT_TICK_UPPER] = 2.5 * pt;	/* 2.5p */
	/* MAP_TICK_LENGTH_SECONDARY */
	GMT->current.setting.map_tick_length[GMT_ANNOT_LOWER] = 15 * pt;	/* 15p */
	GMT->current.setting.map_tick_length[GMT_TICK_LOWER] = 3.75 * pt;	/* 3.75p */
	/* MAP_TICK_PEN_PRIMARY */
	error += GMT_getpen (GMT, "thinner,black", &GMT->current.setting.map_tick_pen[GMT_PRIMARY]);
	/* MAP_TICK_PEN_SECONDARY */
	error += GMT_getpen (GMT, "thinner,black", &GMT->current.setting.map_tick_pen[GMT_SECONDARY]);
	/* MAP_TITLE_OFFSET */
	GMT->current.setting.map_title_offset = 14 * pt;	/* 14p */
	/* MAP_VECTOR_SHAPE */
	GMT->current.setting.map_vector_shape = 0;

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
	if (GMT->PSL) {	/* Only when using PSL in this session */
#ifdef HAVE_ZLIB
		GMT->PSL->internal.compress = PSL_DEFLATE;
		GMT->PSL->internal.deflate_level = 5;
#else
		/* Silently fall back to LZW compression when ZLIB not available */
		GMT->PSL->internal.compress = PSL_LZW;
#endif
		/* PS_LINE_CAP */
		GMT->PSL->internal.line_cap = PSL_BUTT_CAP;
		/* PS_LINE_JOIN */
		GMT->PSL->internal.line_join = PSL_MITER_JOIN;
		/* PS_MITER_LIMIT */
		GMT->PSL->internal.miter_limit = 35;
	}
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
	if (GMT->PSL) GMT->PSL->internal.comments = 0;	/* Only when using PSL in this session */

		/* IO group */

	/* IO_COL_SEPARATOR */
	strcpy (GMT->current.setting.io_col_separator, "\t");
	/* IO_GRIDFILE_FORMAT */
	strcpy (GMT->current.setting.io_gridfile_format, "nf");
	/* IO_GRIDFILE_SHORTHAND */
	GMT->current.setting.io_gridfile_shorthand = false;
	/* IO_HEADER */
	GMT->current.setting.io_header[GMT_IN] = GMT->current.setting.io_header[GMT_OUT] = false;
	/* IO_N_HEADER_RECS */
	GMT->current.setting.io_n_header_items = 0;
	/* IO_NAN_RECORDS (pass) */
	GMT->current.setting.io_nan_records = true;
	/* IO_NC4_CHUNK_SIZE (auto) */
	GMT->current.setting.io_nc4_chunksize[0] = k_netcdf_io_chunked_auto;
	/* IO_NC4_DEFLATION_LEVEL */
	GMT->current.setting.io_nc4_deflation_level = 3;
	/* IO_LONLAT_TOGGLE */
	GMT->current.setting.io_lonlat_toggle[GMT_IN] = false;
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
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: Unrecognized value during gmtdefaults initialization.\n");
	return 0;
}
