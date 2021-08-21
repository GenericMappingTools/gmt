		/* Common parameters for the -S option in meca and coupe */
		bool active;
		bool no_label;	/* True of font size is set to zero */
		bool read;	/* True if no scale given; must be first column after the required ones */
		bool fixed;	/* Use fixed symbol size (+m) */
		bool linear;	/* Scale symbols size linearly with moment (+l) */
		unsigned int readmode;
		unsigned int plotmode;
		unsigned int n_cols;
		double scale;
		double angle;
		double reference;
		int justify;
		double offset[2];
		struct GMT_FONT font;
