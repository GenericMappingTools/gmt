struct GMT_COMMON {
	/* Holds current selections for the family of common GMT options */
	struct GMT_B_OPT {	/* [1]  -B<params> */
		BOOLEAN active;
		short int processed;
	} B;	
	struct GMT_H_OPT {	/* [2]  -H[i][<nrecs>] */
		BOOLEAN active[2];
		short int processed;
		int n_header_records;
	} H;	
	struct GMT_H_OPT {	/* [3-4]  -J<params> */
		BOOLEAN active;
		short int processed;
		int projection;
		double par[6];
	} J;		
	struct GMT_K_OPT {	/* [5]  -K */
		BOOLEAN active;
		short int processed;
	} K;	
	struct GMT_O_OPT {	/* [6]  -O */
		BOOLEAN active;
		short int processed;
	} K;
	struct GMT_P_OPT {	/* [7]  -P */
		BOOLEAN active;
		short int processed;
	} K;
	struct GMT_R_OPT {	/* [8]  -Rw/e/s/n[/z0/z1][r] */
		BOOLEAN active;
		BOOLEAN corners;
		short int processed;
		double x_min, x_max, y_min, y_max, z_min, z_max;
	} R;
	struct GMT_U_OPT {	/* [9]  -U */
		BOOLEAN active;
		short int processed;
		double x, y;
		char *label;	
	} U;
	struct GMT_V_OPT {	/* [10]  -V */
		BOOLEAN active;
		short int processed;
	} V;
	struct GMT_X_OPT {	/* [11]  -X */
		BOOLEAN active;
		double off;
		char mode;	/* r, a, or c */
	} X;
	struct GMT_X_OPT {	/* [12] -Y */
		BOOLEAN active;
		short int processed;
		double off;
		char mode;	/* r, a, or c */
	} Y;
	struct GMT_c_OPT {	/* [13]  -c */
		BOOLEAN active;
		short int processed;
		int copies;
	} c;
	struct GMT_t_OPT {	/* [14]  -:[i|o] */
		BOOLEAN active;
		BOOLEAN toggle[2];
		short int processed;
	} t;
	struct GMT_b_OPT {	/* -b[i|o][<n>][s|S|d|D] */
		BOOLEAN active;
		BOOLEAN binary[2];
		BOOLEAN precision[2];
		BOOLEAN swab[2];
		short int processed;
		int ncol[2];
	} b;
	struct GMT_f_OPT {	/* [15]  -f[i|o]<col>|<colrange>[t|T|g],.. */
		BOOLEAN active;
		short int processed;
		char col_type[2][BUFSIZ];
	} f;
};
