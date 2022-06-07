struct  GRAVMAG_XY {
	double  x, y;
};

double gaussian(double rad, double half_width) {
	double y, gauss_ct = -0.5 / (half_width * half_width);
	y = exp(rad * rad * gauss_ct);
	return (y);
}

void helper_fun(struct GMTGRAVMAG3D_CTRL *Ctrl, struct GRAVMAG_XY *ellipse[2], int m, int j1, int j2, int j3, int i1, int i2, int i3, double z1, double z2, double z3) {
	/* j1, j2 & j2 can only be 0 ot 1 */
	Ctrl->raw_mesh[m].t1[0] =  ellipse[j1][i1].x;
	Ctrl->raw_mesh[m].t1[1] = -ellipse[j1][i1].y;
	Ctrl->raw_mesh[m].t1[2] = -z1;		/* -1 because Z is positive down in Okabe */

	Ctrl->raw_mesh[m].t2[0] =  ellipse[j2][i2].x;
	Ctrl->raw_mesh[m].t2[1] = -ellipse[j2][i2].y;
	Ctrl->raw_mesh[m].t2[2] = -z2;

	Ctrl->raw_mesh[m].t3[0] =  ellipse[j3][i3].x;
	Ctrl->raw_mesh[m].t3[1] = -ellipse[j3][i3].y;
	Ctrl->raw_mesh[m].t3[2] = -z3;
}

int prism(struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl, int nb) {
	double a, b, c, z_c, x0, y0, z_top, z_bot;
	int i_tri = Ctrl->n_raw_triang;

	a   = Ctrl->M.params[PRISM][nb][0];		b  = Ctrl->M.params[PRISM][nb][1];		c  = Ctrl->M.params[PRISM][nb][2];
	z_c = Ctrl->M.params[PRISM][nb][3];		x0 = Ctrl->M.params[PRISM][nb][4];		y0 = Ctrl->M.params[PRISM][nb][5];

	Ctrl->raw_mesh = gmt_M_memory (GMT, Ctrl->raw_mesh, Ctrl->n_raw_triang + 12, struct GMTGRAVMAG3D_RAW);
	z_top = -(z_c + c);		z_bot = -z_c ;		/* -1 because Z is positive down in Okabe */

	/* vertex of top rectangle */
		/* first triangle */
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_top;
	Ctrl->raw_mesh[i_tri].t2[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_top;
		/* second triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_top;
	Ctrl->raw_mesh[i_tri].t2[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_top;
	/* vertex of east rectangle */
		/* first triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] = a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] = a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_top;
		/* second triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] = a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] = a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_bot;
	/* vertex of north rectangle */
		/* first triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_top;
		/* second triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_bot;
	/* vertex of west rectangle */
		/* first triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_top;
		/* second triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_bot;
	/* vertex of south rectangle */
		/* first triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_top;
		/* second triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_top;
	Ctrl->raw_mesh[i_tri].t3[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_bot;
	/* vertex of bottom rectangle */
		/* first triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t3[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_bot;
		/* second triangle */
	i_tri++;
	Ctrl->raw_mesh[i_tri].t1[0] = -a/2 + x0;	Ctrl->raw_mesh[i_tri].t1[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t1[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t2[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t2[1] =  b/2 - y0;	Ctrl->raw_mesh[i_tri].t2[2] = z_bot;
	Ctrl->raw_mesh[i_tri].t3[0] =  a/2 + x0;	Ctrl->raw_mesh[i_tri].t3[1] = -b/2 - y0;	Ctrl->raw_mesh[i_tri].t3[2] = z_bot;

	Ctrl->n_raw_triang += 12;
	return 0;
}

int five_psoid(struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl, int body_type, int nb, bool cone, bool piram, bool sino, bool hemi) {
/*	Constructs either a sphere, ellipsoid, cone, pyramid, or a bell */
/*	as a union of triangular facets. Returns number of triangles. */
	int i, j, j1, k, l, m, m1, m2, n = 0, n_tri, i_tri, npts_circ, n_slices, n_sigmas = 2;
	bool first = true;
	double a, b, c, z_c, x0, y0, z_top, z_bot;
	double dfi, d_sli, ai0, ai1, bi0, bi1, zi0, zi1;
	double d_tet, half_width_x, dx, dy, rad_x, rad_y;
	struct GRAVMAG_XY *ellipse[2];
	
	i_tri = Ctrl->n_raw_triang;		/* Start over any previous raw triang collection */

	/* Fish the these parameters from Ctrl struct */
	switch (body_type) {
		case BELL:
			n_sigmas  = Ctrl->M.params[body_type][nb][6];
			npts_circ = Ctrl->M.params[body_type][nb][7];
			n_slices  = Ctrl->M.params[body_type][nb][8];
			break;
		case ELLIPSOID:
			npts_circ = Ctrl->M.params[body_type][nb][6];
			n_slices  = Ctrl->M.params[body_type][nb][7];
			break;
		case CONE:
			npts_circ = Ctrl->M.params[body_type][nb][4];
			n_slices  = Ctrl->n_slices;
			break;
		case SPHERE:
			npts_circ = Ctrl->M.params[body_type][nb][4];
			n_slices  = Ctrl->M.params[body_type][nb][5];
			break;
		default:
			npts_circ = Ctrl->npts_circ;
			n_slices  = Ctrl->n_slices;
			break;
	}

	a  = Ctrl->M.params[body_type][nb][0];		b   = Ctrl->M.params[body_type][nb][1];
	c  = Ctrl->M.params[body_type][nb][2];		z_c = Ctrl->M.params[body_type][nb][3];
	x0 = Ctrl->M.params[body_type][nb][4];		y0  = Ctrl->M.params[body_type][nb][5];
	z_top = z_c + c;	z_bot = z_c;

	n_tri = (hemi) ? 2 * npts_circ * n_slices : 2 * (npts_circ * (n_slices*2 - 1));
	ellipse[0] = (struct GRAVMAG_XY *) calloc((size_t) (npts_circ+1), sizeof(struct GRAVMAG_XY));
	ellipse[1] = (struct GRAVMAG_XY *) calloc((size_t) (npts_circ+1), sizeof(struct GRAVMAG_XY));

	Ctrl->n_raw_triang += n_tri;
	Ctrl->raw_mesh = gmt_M_memory(GMT, Ctrl->raw_mesh, Ctrl->n_raw_triang, struct GMTGRAVMAG3D_RAW);

	dfi = (TWO_PI / npts_circ);		d_tet = (M_PI_2 / n_slices);
	d_sli = c / n_slices;
	half_width_x = 0.5 * a;
	dx = n_sigmas * a / n_slices;	dy = n_sigmas * b / n_slices; 

	for (j = 0; j < Ctrl->n_slices; j++) {
		j1 = j + 1;
		if (cone || piram) {
			ai0 = j * a / n_slices;		bi0 = j * b / n_slices;
			zi0 = z_top - j * d_sli;
			ai1 = j1 * a / n_slices;		bi1 = j1 * b / n_slices;
			zi1 = z_top - j1 * d_sli;
		}
		else if (sino) { /* Bell shaped volume */
			rad_x = j * dx;		rad_y = j * dy;
			ai0 = rad_x;		bi0 = rad_y;
			zi0 = z_bot + c * gaussian(rad_x, half_width_x); 
			rad_x = j1 * dx;	rad_y = j1 * dy;
			ai1 = rad_x;		bi1 = rad_y;
			zi1 = z_bot + c * gaussian(rad_x, half_width_x);
		}
		else { /* Ellipsoide or Sphere */
			ai0 = a*cos(M_PI_2-j*d_tet);	bi0 = b*cos(M_PI_2-j*d_tet);
			zi0 = z_top - c * (1. - sqrt(1. - (ai0/a)*(ai0/a)));
			ai1 = a*cos(M_PI_2-j1*d_tet);	bi1 = b*cos(M_PI_2-j1*d_tet);
			zi1 = z_top - c * (1. - sqrt(1. - (ai1/a)*(ai1/a)));
		}
		for (i = 0; i < npts_circ; i++) { /* compute slice j */
			ellipse[0][i].x = x0 + ai0 * cos (i*dfi);
			ellipse[0][i].y = y0 + bi0 * sin (i*dfi);
			ellipse[1][i].x = x0 + ai1 * cos (i*dfi);
			ellipse[1][i].y = y0 + bi1 * sin (i*dfi);
		}
		ellipse[0][npts_circ].x = ellipse[0][0].x; /* close slice "contour" */
		ellipse[0][npts_circ].y = ellipse[0][0].y;
		ellipse[1][npts_circ].x = ellipse[1][0].x;
		ellipse[1][npts_circ].y = ellipse[1][0].y;

		/* Calculates vertex of triangles in slice j */
		i = 0;
		if (first) {
			for (m = 0; m < npts_circ ; m++) {
				helper_fun(Ctrl, ellipse, m + i_tri, 0, 1, 1, i, i+1, i, zi0, zi1, zi1);
				i++;
			}
		}
		else {
			for (m = (j-1)*npts_circ + i_tri; m < j*npts_circ + i_tri; m++) {
				/* First triangle */
				m1 = 2 * m + npts_circ;		m2 = 2 * m + 1 + npts_circ;
				helper_fun(Ctrl, ellipse, m1, 0, 1, 1, i, i+1, i, zi0, zi1, zi1);
				/* Second triangle */
				helper_fun(Ctrl, ellipse, m2, 0, 0, 1, i, i+1, i+1, zi0, zi0, zi1);
				i++;
			}
		}
		first = false;
	}
	/* First half is ready. Now, either close it and return or construct the other half by simetry */
	if (cone || piram || sino || hemi) { /* close the base and return */
		if (sino && fabs ((zi1 - z_c) / z_c) > 0.01) { 
			/* bell's last slice is 1% far from base, so we add a vertical wall */ 
			z_top = zi1;	/* update z_top value */
			for (n = 0; n < npts_circ; n++) {
				/* First triangle */
				j = npts_circ * (n_slices * 2 - 1) + 2 * (n + i_tri);
				helper_fun(Ctrl, ellipse, j, 1, 1, 1, n, n+1, n, z_top, z_top, z_bot);
				/* Second triangle */
				j1 = npts_circ * (n_slices * 2 - 1) + 2 * (n + i_tri) + 1;
				helper_fun(Ctrl, ellipse, j1, 1, 1, 1, n+1, n+1, n, z_top, z_top, z_bot);
			}
		}
		else if (sino) /* slightly change base depth to force a closed volume */
			z_c = zi1;

		i = npts_circ * (n_slices*2 - 1) + 2 * n;
		for (k = i, l = 0; k < i+npts_circ; k++, l++) {
			helper_fun(Ctrl, ellipse, k + i_tri, 1, 1, 1, 0, l, l+1, z_c, z_c, z_c);
			Ctrl->raw_mesh[k + i_tri].t1[0] = x0;		/* These two fell of the general case of the helper_fun() */
			Ctrl->raw_mesh[k + i_tri].t1[1] = y0;
		}
		free((void *)ellipse[0]);
		free((void *)ellipse[1]);
		return (k);
	}
	n_tri = npts_circ * (n_slices*2 - 1);
	for (j = n_tri-1, i = n_tri; j >= 0; j--, i++) {
		Ctrl->raw_mesh[i+i_tri].t1[0] = Ctrl->raw_mesh[j+i_tri].t1[0];
		Ctrl->raw_mesh[i+i_tri].t1[1] = Ctrl->raw_mesh[j+i_tri].t1[1];
		Ctrl->raw_mesh[i+i_tri].t1[2] = z_c + (z_c - Ctrl->raw_mesh[j+i_tri].t1[2]);

		Ctrl->raw_mesh[i+i_tri].t2[0] = Ctrl->raw_mesh[j+i_tri].t2[0];
		Ctrl->raw_mesh[i+i_tri].t2[1] = Ctrl->raw_mesh[j+i_tri].t2[1];
		Ctrl->raw_mesh[i+i_tri].t2[2] = z_c + (z_c - Ctrl->raw_mesh[j+i_tri].t2[2]);

		Ctrl->raw_mesh[i+i_tri].t3[0] = Ctrl->raw_mesh[j+i_tri].t3[0];
		Ctrl->raw_mesh[i+i_tri].t3[1] = Ctrl->raw_mesh[j+i_tri].t3[1];
		Ctrl->raw_mesh[i+i_tri].t3[2] = z_c + (z_c - Ctrl->raw_mesh[j+i_tri].t3[2]);
	}
	free ((void *)ellipse[0]);
	free ((void *)ellipse[1]);
	n_tri = 2 * (npts_circ * (n_slices*2 - 1));
	return (n_tri);
}

int cilindro (struct GMT_CTRL *GMT, struct GMTGRAVMAG3D_CTRL *Ctrl, int nb) {
	double z_top, z_bot, dfi, rad_c, height_c, z_c, x0, y0;
	int i, j, j1, n_tri, i_tri, npts_circ;
	struct GRAVMAG_XY *circ;

	i_tri = Ctrl->n_raw_triang;		/* Start over any previous raw triang collection */

	rad_c = Ctrl->M.params[CYLINDER][nb][0];		height_c   = Ctrl->M.params[CYLINDER][nb][1];
	z_c   = Ctrl->M.params[CYLINDER][nb][2];
	x0    = Ctrl->M.params[CYLINDER][nb][3];		y0  = Ctrl->M.params[CYLINDER][nb][4];
	npts_circ = Ctrl->M.params[CYLINDER][nb][5];
	z_top = -(z_c + height_c);		z_bot = -z_c ;

	n_tri = Ctrl->npts_circ * 4;
	circ = (struct GRAVMAG_XY *) calloc((size_t) (Ctrl->npts_circ+1), sizeof(struct GRAVMAG_XY));

	Ctrl->n_raw_triang += n_tri;
	Ctrl->raw_mesh = gmt_M_memory(GMT, Ctrl->raw_mesh, Ctrl->n_raw_triang, struct GMTGRAVMAG3D_RAW);

	dfi = (TWO_PI / npts_circ);

	for (i = 0; i < npts_circ; i++) { /* compute circle */
		circ[i].x = x0 + rad_c * cos (i*dfi);
		circ[i].y = y0 + rad_c * sin (i*dfi);
	}
	circ[npts_circ].x = circ[0].x;	circ[npts_circ].y = circ[0].y;

	for (i = 0; i < Ctrl->npts_circ; i++) { /* Calculates vertex of top circle */
		Ctrl->raw_mesh[i+i_tri].t1[0] = x0;
		Ctrl->raw_mesh[i+i_tri].t1[1] = -y0;
		Ctrl->raw_mesh[i+i_tri].t1[2] = z_top;
		
		Ctrl->raw_mesh[i+i_tri].t2[0] = circ[i+1].x;
		Ctrl->raw_mesh[i+i_tri].t2[1] = -circ[i+1].y;
		Ctrl->raw_mesh[i+i_tri].t2[2] = z_top;
		
		Ctrl->raw_mesh[i+i_tri].t3[0] = circ[i].x;
		Ctrl->raw_mesh[i+i_tri].t3[1] = -circ[i].y;
		Ctrl->raw_mesh[i+i_tri].t3[2] = z_top;
	}
	for (i = 0; i < npts_circ; i++) { /* Computes vertex of side rectangle, where each one is decomposed into two triangles */
		/* First triangle */
		j = 2 * (i + i_tri) + npts_circ;
		Ctrl->raw_mesh[j].t1[0] = circ[i].x;	Ctrl->raw_mesh[j].t1[1] = -circ[i].y;	Ctrl->raw_mesh[j].t1[2] = z_top;
		Ctrl->raw_mesh[j].t2[0] = circ[i+1].x;	Ctrl->raw_mesh[j].t2[1] = -circ[i+1].y;	Ctrl->raw_mesh[j].t2[2] = z_top;
		Ctrl->raw_mesh[j].t3[0] = circ[i].x;	Ctrl->raw_mesh[j].t3[1] = -circ[i].y;	Ctrl->raw_mesh[j].t3[2] = z_bot;
		/* Second triangle */
		j1 = 2 * (i + i_tri) + npts_circ + 1;
		Ctrl->raw_mesh[j1].t1[0] = circ[i+1].x;	Ctrl->raw_mesh[j1].t1[1] = -circ[i+1].y;	Ctrl->raw_mesh[j1].t1[2] = z_top;
		Ctrl->raw_mesh[j1].t2[0] = circ[i+1].x;	Ctrl->raw_mesh[j1].t2[1] = -circ[i+1].y;	Ctrl->raw_mesh[j1].t2[2] = z_bot;
		Ctrl->raw_mesh[j1].t3[0] = circ[i].x;	Ctrl->raw_mesh[j1].t3[1] = -circ[i].y;		Ctrl->raw_mesh[j1].t3[2] = z_bot;
	}
	for (i = 0; i < npts_circ; i++) { /* Now the vertex of bottom circle */
		j = (i + i_tri) + 3 * npts_circ;
		Ctrl->raw_mesh[j].t1[0] = x0;			Ctrl->raw_mesh[j].t1[1] = -y0;			Ctrl->raw_mesh[j].t1[2] = z_bot;
		Ctrl->raw_mesh[j].t2[0] = circ[i].x;	Ctrl->raw_mesh[j].t2[1] = -circ[i].y;	Ctrl->raw_mesh[j].t2[2] = z_bot;
		Ctrl->raw_mesh[j].t3[0] = circ[i+1].x;	Ctrl->raw_mesh[j].t3[1] = -circ[i+1].y;	Ctrl->raw_mesh[j].t3[2] = z_bot;
	}
	free ((void *)circ);
	return n_tri;
}
