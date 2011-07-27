/*
 *	$Id$
 */
#define COASTLIB 1
#include "wvs.h"

/* These two are local native versions */

int pol_readheader (struct GMT3_POLY *h, FILE *fp)
{
	int n;
	n = fread ((void *)h, sizeof (struct GMT3_POLY), 1, fp);
	return (n);
}

int pol_writeheader (struct GMT3_POLY *h, FILE *fp)
{
	int n;
	n = fwrite ((void *)h, sizeof (struct GMT3_POLY), 1, fp);
	return (n);
}

int pol_fread (struct LONGPAIR *p, size_t n_items, FILE *fp)
{
	int n;

	n = fread ((void *)p, sizeof (struct LONGPAIR), n_items, fp);
	return (n);
}

int pol_fwrite (struct LONGPAIR *p, size_t n_items, FILE *fp)
{
	int n;
	n = fwrite ((void *)p, sizeof (struct LONGPAIR), n_items, fp);
	return (n);
}

/* Down here lies the ones we use to build the GSHHS distribution files
 * which must be BIGENDIAN.  Only polygon_to_gshhs.c uses these directly
*/

#ifndef WORDS_BIGENDIAN
void swab_polheader (struct GMT3_POLY *h);
void swab_polpoints (struct LONGPAIR *p, int n);
#endif

int pol_readheader2 (struct GMT3_POLY *h, FILE *fp)
{
	int n;
	n = fread ((void *)h, sizeof (struct GMT3_POLY), 1, fp);
#ifndef WORDS_BIGENDIAN
	swab_polheader (h);
#endif
	return (n);
}

int pol_writeheader2 (struct GMT3_POLY *h, FILE *fp)
{
	int n;
	struct GMT3_POLY *use_h;
#ifndef WORDS_BIGENDIAN
	struct GMT3_POLY tmp_h;
	tmp_h = *h;
	swab_polheader (&tmp_h);
	use_h = &tmp_h;
#else
	use_h = h;
#endif
	n = fwrite ((void *)use_h, sizeof (struct GMT3_POLY), 1, fp);
	return (n);
}

int pol_fread2 (struct LONGPAIR *p, size_t n_items, FILE *fp)
{
	int n;

	n = fread ((void *)p, sizeof (struct LONGPAIR), n_items, fp);
#ifndef WORDS_BIGENDIAN
	swab_polpoints (p, n_items);
#endif
	return (n);
}

int pol_fwrite2 (struct LONGPAIR *p, size_t n_items, FILE *fp)
{
	int n;
#ifndef WORDS_BIGENDIAN
	swab_polpoints (p, n_items);
#endif
	n = fwrite ((void *)p, sizeof (struct LONGPAIR), n_items, fp);
	return (n);
}

#ifndef WORDS_BIGENDIAN
void swab_polheader (struct GMT3_POLY *h)
{
	unsigned int *i, j;

	h->id = GMT_swab4 (h->id);
	h->n = GMT_swab4 (h->n);
	h->greenwich = GMT_swab4 (h->greenwich);
	h->level = GMT_swab4 (h->level);
	h->datelon = GMT_swab4 (h->datelon);
	h->source = GMT_swab4 (h->source);
	h->parent = GMT_swab4 (h->parent);
	h->ancestor = GMT_swab4 (h->ancestor);
	h->river = GMT_swab4 (h->river);
	i = (unsigned int *)&h->west;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->east;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->south;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->north;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->area;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
	i = (unsigned int *)&h->area_res;
	j = GMT_swab4 (i[0]);
	i[0] = GMT_swab4 (i[1]);
	i[1] = j;
}

void swab_polpoints (struct LONGPAIR *p, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		p[i].x = GMT_swab4 (p[i].x);
		p[i].y = GMT_swab4 (p[i].y);
	}
}
#endif
