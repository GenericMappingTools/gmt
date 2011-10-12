/*	$Id$	*/
#ifndef SEGY_H
#define SEGY_H

/* This is the header for the PASSCAL SEGY trace data.  
 * From PASSCAL code base which is in the public domain
 *
 * The PASSCAL SEGY trace format is a modified form of the SEG-Y
 * trace format.  The modification comes is because we use some of
 * the unspecified header words to store information pertinent to 
 * the PASSCAL data.  The data values for each trace are preceded
 * by a 240 byte header. This format is given below.  All integer 
 * values are stored with the most significant byte first.  Data
 * values are either 16 0r 32 bit integers depending on byte 206
 * of the header, the field named "data_form".
 * 
 * Up to byte 180, the definitions follow the SEGY definition laid down
 * by Barry et al.
 * 
 * SEGYHEAD is now typedef'ed 
 *
 * Reading bytes  directly into this header will allow access to
 * all of the fields.  The number in the comment is the byte offset
 * into the segy file.  An "X" in the comment indicates that field
 * is NEVER set.  Fields that are set to default values contain that
 * value and follow a ":" in the comment ("grep : segy.h" will spit
 * all default fields out).  Two pairs of fields exist to cover an
 * inherited limitation; sampleLength/num_samps and deltaSample/samp_rate.
 * When the value is too large to fit in a int16_t, sampleLength or
 * deltaSample become flags and require their int32_t counterparts,
 * num_samps and samp_rate, to contain that value.
 */
typedef struct SegyHead {   /*  Offset Description  */
   int32_t  lineSeq;           /*   0 Sequence numbers within line */
   int32_t  reelSeq;           /*   4 Sequence numbers within reel */
   int32_t  event_number;      /*   8 Original field record number or trigger number */
   int32_t  channel_number;    /*  12 Trace channel number within the original field record */
   int32_t  energySourcePt;    /*  16 X */
   int32_t  cdpEns;            /*  20 X */
   int32_t  traceInEnsemble;   /*  24 X */
   int16_t traceID;           /*  28 Trace identification code: seismic data = 1 */
   int16_t vertSum;           /*  30 X */
   int16_t horSum;            /*  32 X */
   int16_t dataUse;           /*  34 X */
   int32_t  sourceToRecDist;   /*  36 X */
   int32_t  recElevation;      /*  40 X */
   int32_t  sourceSurfaceElevation; /*  44 X */
   int32_t  sourceDepth;       /*  48 X */
   int32_t  datumElevRec;      /*  52 X */
   int32_t  datumElemSource;   /*  56 X */
   int32_t  recWaterDepth;     /*  60 X */
   int32_t  sourceWaterDepth;  /*  64 X */
   int16_t elevationScale;    /*  68 Elevation Scaler: scale = 1 */
   int16_t coordScale;        /*  70 Coordinate Scaler: scale = 1 */
   int32_t  sourceLongOrX;     /*  72 X */
   int32_t  sourceLatOrY;      /*  76 X */
   int32_t  recLongOrX;        /*  80 X */
   int32_t  recLatOrY;         /*  84 X */
   int16_t coordUnits;        /*  88 Coordinate Units:  = 2 (Lat/Long) */
   int16_t weatheringVelocity;/*  90 X */
   int16_t subWeatheringVelocity; /*  92 X */
   int16_t sourceUpholeTime;  /*  94 X */
   int16_t recUpholeTime;     /*  96 X */
   int16_t sourceStaticCor;   /*  98 X */
   int16_t recStaticCor;      /* 100 X */
   int16_t totalStatic;       /* 102 Total Static in MILLISECS added to Trace Start Time */
   int16_t lagTimeA;          /* 104 X */
   int16_t lagTimeB;          /* 106 X */
   int16_t delay;             /* 108 X */
   int16_t muteStart;         /* 110 X */
   int16_t muteEnd;           /* 112 X */
   int16_t sampleLength;      /* 114 Number of samples in this trace (unless == 32767) */
   int16_t deltaSample;       /* 116 Sampling interval in MICROSECONDS (unless == 1) */
   int16_t gainType;          /* 118 Gain Type: 1 = Fixed Gain */
   int16_t gainConst;         /* 120 Gain of amplifier */
   int16_t initialGain;       /* 122 X */
   int16_t correlated;        /* 124 X */
   int16_t sweepStart;        /* 126 X */
   int16_t sweepEnd;          /* 128 X */
   int16_t sweepLength;       /* 130 X */
   int16_t sweepType;         /* 132 X */
   int16_t sweepTaperAtStart; /* 134 X */
   int16_t sweepTaperAtEnd;   /* 136 X */
   int16_t taperType;         /* 138 X */
   int16_t aliasFreq;         /* 140 X */
   int16_t aliasSlope;        /* 142 X */
   int16_t notchFreq;         /* 144 X */
   int16_t notchSlope;        /* 146 X */
   int16_t lowCutFreq;        /* 148 X */
   int16_t hiCutFreq;         /* 150 X */
   int16_t lowCutSlope;       /* 152 X */
   int16_t hiCutSlope;        /* 154 X */
   int16_t year;              /* 156 year of Start of trace */
   int16_t day;               /* 158 day of year at Start of trace */
   int16_t hour;              /* 160 hour of day at Start of trace */
   int16_t minute;            /* 162 minute of hour at Start of trace */
   int16_t second;            /* 164 second of minute at Start of trace */
   int16_t timeBasisCode;     /* 166 Time basis code: 2 = GMT */
   int16_t traceWeightingFactor; /* 168 X */
   int16_t phoneRollPos1;     /* 170 X */
   int16_t phoneFirstTrace;   /* 172 X */
   int16_t phoneLastTrace;    /* 174 X */
   int16_t gapSize;           /* 176 X */
   int16_t taperOvertravel;   /* 178 X */
   char  station_name[6];   /* 180 Station Name code (5 chars + \0) */
   char  sensor_serial[8];  /* 186 Sensor Serial code (7 chars + \0) */
   char  channel_name[4];   /* 194 Channel Name code (3 chars + \0) */
   char  extrash[2];        /* 198 Extra unassigned bytes (2 chars) */
   int32_t  samp_rate;         /* 200 Sample interval in MICROSECS as a 32 bit integer */
   int16_t data_form;         /* 204 Data Format flag: 0=16 bit, 1=32 bit integer */
   int16_t m_secs;            /* 206 MILLISECONDS of seconds of Start of trace */
   int16_t trigyear;          /* 208 year of Trigger time */
   int16_t trigday;           /* 210 day of year at Trigger time */
   int16_t trighour;          /* 212 hour of day at Trigger time */
   int16_t trigminute;        /* 214 minute of hour at Trigger time */
   int16_t trigsecond;        /* 216 second of minute at Trigger time */
   int16_t trigmills;         /* 218 MILLISECONDS of seconds of Trigger time */
   float scale_fac;         /* 220 Scale Factor (IEEE 32 bit float) */
   int16_t inst_no;           /* 224 Instrument Serial Number */
   int16_t not_to_be_used;    /* 226 X */
   uint32_t  num_samps; /* 228 Number of Samples as a 32 bit integer
                             * (when sampleLength == 32767) */
   int32_t  max;               /* 232 Maximum value in Counts */
   int32_t  min;               /* 236 Minimum value in Counts */
} SEGYHEAD;                 /* end of segy trace header */
#endif                      /* SEGY_H        */
