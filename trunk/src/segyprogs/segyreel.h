/* $Id: segyreel.h,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $ */
/* SEGY REEL HEADER VARIABLES */
typedef struct SEGYReel_header {
    long        job;
    long        line;
    long        reel;

    short       num_traces;     /* number of traces in this file */
    short       num_aux;        /* number of auxiliary traces  */
    short       sr;             /* sample rate (microseconds) this data */
    short       fldsr;          /* field sample rate */
    short       nsamp;          /* number of samples this data */
    short       fsamp;          /* number of field samples DOES NOT APPLY FOR THIS DATA */

    short       dsfc;           /* data sample format code 1 = IBM FP (taken from segy2sierra.c) */
    short       mfold;          /* number of CDP traces   = 0 here */
    short       sort;           /* sort code 1=as recorded */

    short       vcode;          /* part of STANDARD SEGY */
    short       sweep_start;    /* part of STANDARD SEGY */
    short       sweep_end;      /* part of STANDARD SEGY */
    short       sweep_len;      /* part of STANDARD SEGY */
    short       sweep_type;     /* part of STANDARD SEGY */
    short       sweep_chan;     /* part of STANDARD SEGY */

    short       taper_start;    /* part of STANDARD SEGY */
    short       taper_end;      /* part of STANDARD SEGY */
    short       taper_type;     /* part of STANDARD SEGY */
    short       correlated;     /* part of STANDARD SEGY */
    short       bin_gain_recov; /* part of STANDARD SEGY */
    short       amp_gain_recov; /* part of STANDARD SEGY */

    short       measure;        /* 1 = meters */
    short       polarity;       /* 1 = reversed up=neg */

    short       vibe_polarity;          /* part of STANDARD SEGY */
    short       num_trace_per_file;     /* LDS */
    short       mean_amp;               /* LDS */
    short       attribute;              /* LDS */
    short       domain;                 /* LDS */
    short       instrument_type;        /* LDS */

    short       creation_year;          /* USGS */
    short       creation_month;         /* USGS */
    short       creation_day_of_month;  /* USGS */
    short       dummy[162];


} SEGYREEL;
