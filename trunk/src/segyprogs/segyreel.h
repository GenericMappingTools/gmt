/* $Id$
 *
 * From PASSCAL code base which is in the public domain
 * SEGY REEL HEADER VARIABLES
 */
typedef struct SEGYReel_header {
    int32_t        job;
    int32_t        line;
    int32_t        reel;

    int16_t       num_traces;     /* number of traces in this file */
    int16_t       num_aux;        /* number of auxiliary traces  */
    uint16_t       sr;             /* sample rate (microseconds) this data */
    int16_t       fldsr;          /* field sample rate */
    int16_t       nsamp;          /* number of samples this data */
    int16_t       fsamp;          /* number of field samples DOES NOT APPLY FOR THIS DATA */

    int16_t       dsfc;           /* data sample format code 1 = IBM FP (taken from segy2sierra.c) */
    int16_t       mfold;          /* number of CDP traces   = 0 here */
    int16_t       sort;           /* sort code 1=as recorded */

    int16_t       vcode;          /* part of STANDARD SEGY */
    int16_t       sweep_start;    /* part of STANDARD SEGY */
    int16_t       sweep_end;      /* part of STANDARD SEGY */
    int16_t       sweep_len;      /* part of STANDARD SEGY */
    int16_t       sweep_type;     /* part of STANDARD SEGY */
    int16_t       sweep_chan;     /* part of STANDARD SEGY */

    int16_t       taper_start;    /* part of STANDARD SEGY */
    int16_t       taper_end;      /* part of STANDARD SEGY */
    int16_t       taper_type;     /* part of STANDARD SEGY */
    int16_t       correlated;     /* part of STANDARD SEGY */
    int16_t       bin_gain_recov; /* part of STANDARD SEGY */
    int16_t       amp_gain_recov; /* part of STANDARD SEGY */

    int16_t       measure;        /* 1 = meters */
    int16_t       polarity;       /* 1 = reversed up=neg */

    int16_t       vibe_polarity;          /* part of STANDARD SEGY */
    int16_t       num_trace_per_file;     /* LDS */
    int16_t       mean_amp;               /* LDS */
    int16_t       attribute;              /* LDS */
    int16_t       domain;                 /* LDS */
    int16_t       instrument_type;        /* LDS */

    int16_t       creation_year;          /* USGS */
    int16_t       creation_month;         /* USGS */
    int16_t       creation_day_of_month;  /* USGS */
    int16_t       dummy[162];


} SEGYREEL;
