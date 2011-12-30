/*
 *	$Id$
 */

#define COASTLIB
#include"wvs.h"

#define sqr(x) ((x)*(x))
#define F (D2R * 0.5 * 1.0e-6)

int Douglas_Peucker (double x_source[], double y_source[], int n_source, double band, int index[]);
int Douglas_Peucker_i (int x_source[], int y_source[], int n_source, double band, int index[]);

/* Stack-based Douglas Peucker line simplification routine */
/* returned value is the number of output points */

int Douglas_Peucker (double x_source[], double y_source[], int n_source, double band, int index[])
/* x/y_source	Input coordinates, n_source of them */
/* band;		tolerance in km */
/* index[]	output co-ordinates indices */
{
	int	n_stack, n_dest, start, end, i, sig;
	int	*sig_start, *sig_end;	/* indices of start&end of working section */

	double dev_sqr, max_dev_sqr, band_sqr;
	double  x12, y12, d12, x13, y13, d13, x23, y23, d23;

        /* check for simple cases */

        if ( n_source < 3 )     /* one or two points */
        {
                for ( i = 0; i < n_source; i++) index[i] = i;
                return (n_source);
        }


        /* more complex case. initialise stack */

 	sig_start = (int *) GMT_memory (NULL, n_source, sizeof (int), "Douglas_Peucker_i");
	sig_end   = (int *) GMT_memory (NULL, n_source, sizeof (int), "Douglas_Peucker_i");
	
 	band *= 360.0 / (2.0 * M_PI * 6371.007181);	/* Now in degrees */
	band_sqr = sqr(band);

	n_dest = 0;

        sig_start[0] = 0;
        sig_end[0] = n_source-1;

        n_stack = 1;

        /* while the stack is not empty  ... */

        while ( n_stack > 0 )
        {
                /* ... pop the top-most entries off the stacks */

                start = sig_start[n_stack-1];
                end = sig_end[n_stack-1];

                n_stack--;

                if ( end - start > 1 )  /* any intermediate points ? */
                {
                        /* ... yes, so find most deviant intermediate point to
                               either side of line joining start & end points */

                x12 = x_source[end] - x_source[start];
                if (fabs (x12) > 180.0) x12 = 360.0 - fabs (x12);
                y12 = y_source[end] - y_source[start];
		x12 *= cosd (0.5 * (y_source[end] + y_source[start]));
		d12 = sqr(x12) + sqr(y12);

                for ( i = start + 1, sig = start, max_dev_sqr = -1.0; i <
end; i++ )
                        {
                                x13 = x_source[i] - x_source[start];
              			if (fabs (x13) > 180.0) x13 = 360.0 - fabs (x13);
				y13 = y_source[i] - y_source[start];

				x23 = x_source[i] - x_source[end];
               			if (fabs (x23) > 180.0) x23 = 360.0 - fabs (x23);
				y23 = y_source[i] - y_source[end];
                                
                                x13 *= cosd (0.5 * (y_source[i] + y_source[end]));
                                x23 *= cosd (0.5 * (y_source[i] + y_source[end]));
                                
                                d13 = sqr(x13) + sqr(y13);
                                d23 = sqr(x23) + sqr(y23);

                                if ( d13 >= ( d12 + d23 ) )
                                        dev_sqr = d23;
                                else if ( d23 >= ( d12 + d13 ) )
                                        dev_sqr = d13;
                                else
                                        dev_sqr =  sqr( x13 * y12 - y13 *
x12 ) / d12;

                                if ( dev_sqr > max_dev_sqr  )
                                {
                                        sig = i;
                                        max_dev_sqr = dev_sqr;
                                }
                        }

                        if ( max_dev_sqr < band_sqr )   /* is there a sig.
intermediate point ? */
                        {
                                /* ... no, so transfer current start point */

                                index[n_dest] = start;
                                n_dest++;
                        }
                        else
                        {
                                /* ... yes, so push two sub-sections on
stack for further processing */

                                n_stack++;

                                sig_start[n_stack-1] = sig;
                                sig_end[n_stack-1] = end;

                                n_stack++;

                                sig_start[n_stack-1] = start;
                                sig_end[n_stack-1] = sig;
                        }
                }
                else
                {
                        /* ... no intermediate points, so transfer current
start point */

                        index[n_dest] = start;
                        n_dest++;
                }
        }


        /* transfer last point */

        index[n_dest] = n_source-1;
        n_dest++;

	free (sig_start);
	free (sig_end);
	
        return(n_dest);
}

int Douglas_Peucker_i (int x_source[], int y_source[], int n_source, double band, int index[])
/* x/y_source	Input coordinates, n_source of them */
/* band;		tolerance in km */
/* index[]	output co-ordinates indices */
{
	int	n_stack, n_dest, start, end, i, sig;
	int	*sig_start, *sig_end;	/* indices of start&end of working section */

	double dev_sqr, max_dev_sqr, band_sqr;
	double  x12, y12, d12, x13, y13, d13, x23, y23, d23;

        /* check for simple cases */

        if ( n_source < 3 ) return(0);    /* one or two points */

        /* more complex case. initialise stack */

 	sig_start = (int *) GMT_memory (NULL, n_source, sizeof (int), "Douglas_Peucker_i");
	sig_end   = (int *) GMT_memory (NULL, n_source, sizeof (int), "Douglas_Peucker_i");
	
	band *= 360.0 / (2.0 * M_PI * 6371.007181);	/* Now in degrees */
	band_sqr = sqr(band);
		
	n_dest = 0;

        sig_start[0] = 0;
        sig_end[0] = n_source-1;

        n_stack = 1;

        /* while the stack is not empty  ... */

        while ( n_stack > 0 )
        {
                /* ... pop the top-most entries off the stacks */

                start = sig_start[n_stack-1];
                end = sig_end[n_stack-1];

                n_stack--;

                if ( end - start > 1 )  /* any intermediate points ? */
                {
                        /* ... yes, so find most deviant intermediate point to
                               either side of line joining start & end points */

                x12 = 1.0e-6 * (x_source[end] - x_source[start]);
                if (fabs (x12) > 180.0) x12 = 360.0 - fabs (x12);
                y12 = 1.0e-6 * (y_source[end] - y_source[start]);
		x12 *= cos (F * (y_source[end] + y_source[start]));
		d12 = sqr(x12) + sqr(y12);

                for ( i = start + 1, sig = start, max_dev_sqr = -1.0; i <
end; i++ )
                        {
                                x13 = 1.0e-6 * (x_source[i] - x_source[start]);
                                y13 = 1.0e-6 * (y_source[i] - y_source[start]);
				if (fabs (x13) > 180.0) x13 = 360.0 - fabs (x13);
                                x13 *= cos (F * (y_source[i] + y_source[start]));

                                x23 = 1.0e-6 * (x_source[i] - x_source[end]);
                                y23 = 1.0e-6 * (y_source[i] - y_source[end]);
				if (fabs (x23) > 180.0) x23 = 360.0 - fabs (x23);
                                x23 *= cos (F * (y_source[i] + y_source[end]));
                                
                                d13 = sqr(x13) + sqr(y13);
                                d23 = sqr(x23) + sqr(y23);

                                if ( d13 >= ( d12 + d23 ) )
                                        dev_sqr = d23;
                                else if ( d23 >= ( d12 + d13 ) )
                                        dev_sqr = d13;
                                else
                                        dev_sqr =  sqr( x13 * y12 - y13 *
x12 ) / d12;

                                if ( dev_sqr > max_dev_sqr  )
                                {
                                        sig = i;
                                        max_dev_sqr = dev_sqr;
                                }
                        }

                        if ( max_dev_sqr < band_sqr )   /* is there a sig.
intermediate point ? */
                        {
                                /* ... no, so transfer current start point */

                                index[n_dest] = start;
                                n_dest++;
                        }
                        else
                        {
                                /* ... yes, so push two sub-sections on
stack for further processing */

                                n_stack++;

                                sig_start[n_stack-1] = sig;
                                sig_end[n_stack-1] = end;

                                n_stack++;

                                sig_start[n_stack-1] = start;
                                sig_end[n_stack-1] = sig;
                        }
                }
                else
                {
                        /* ... no intermediate points, so transfer current
start point */

                        index[n_dest] = start;
                        n_dest++;
                }
        }


        /* transfer last point */

        index[n_dest] = n_source-1;
        n_dest++;

	free (sig_start);
	free (sig_end);
	
        return (n_dest);
}
