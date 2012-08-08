/* -*-Mode: C;-*-
 * Module:      CDMS time conversion and arithmetic routines
 *
 * Copyright:	1995, Regents of the University of California
 *		This software may not be distributed to others without
 *		permission of the author.
 *
 * Author:      Bob Drach, Lawrence Livermore National Laboratory
 *              drach@llnl.gov
 *
 * Version:     $Id$
 *
 * Revision History:
 *
 * $Log: cdTimeConv.c,v $
 * Revision 1.1.1.1  2009/07/06 15:06:30  ed
 * added libcf to netcdf dist
 *
 * Revision 1.1  2008/06/30 16:07:44  ed
 * Beginning merge of calandar stuff.
 *
 * Revision 1.1.1.1  1997/12/09 18:57:40  drach
 * Copied from cirrus
 *
 * Revision 1.3  1996/09/09  18:28:33  drach
 * - Cleaned up minor compilation warnings
 *
 * Revision 1.2  1996/04/04  18:29:04  drach
 * - Added FORTRAN interface to time routines
 * - Added function cdParseDeltaTime
 *
 * Revision 1.1  1996/02/21  23:56:47  drach
 * - Overlayed cdtime routines in cdTimeConv.c:
 * - Added seconds, julian calendar, changed include to cdmsint.h for old
 *   time routines in timeArith.c and timeConv.c
 *
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include "cdmsint.h"

void cdComp2RelMixed(cdCompTime ct, cdUnitTime unit, cdCompTime basetime, double *reltime);
void cdRel2CompMixed(double reltime, cdUnitTime unit, cdCompTime basetime, cdCompTime *comptime);

#define CD_DEFAULT_BASEYEAR "1979"	     /* Default base year for relative time (no 'since' clause) */
#define VALCMP(a,b) ((a)<(b)?-1:(b)<(a)?1:0)

/* Validate the component time, return 0 if valid, 1 if not */
int
cdValidateTime(cdCalenType timetype, cdCompTime comptime)
{
	if(comptime.month<1 || comptime.month>12){
		cdError("Error on time conversion: invalid month = %hd\n",comptime.month);
		return 1;
	}
	if(comptime.day<1 || comptime.day>31){
		cdError("Error on time conversion: invalid day = %hd\n",comptime.day);
		return 1;
	}
	if(comptime.hour<0.0 || comptime.hour>24.0){
		cdError("Error on time conversion: invalid hour = %lf\n",comptime.hour);
		return 1;
	}
	return 0;
}

/* Trim trailing whitespace, up to n characters. */
/* If no whitespace up to the last character, set */
/* the last character to null, else set the first */
/* whitespace character to null. */
void
cdTrim(char* s, int n)
{
	char* c;

	if(s==NULL)
		return;
	for(c=s; *c && c<s+n-1 && !isspace(*c); c++);
	*c='\0';
	return;
}
					     /* Map to old timetypes */
int
cdToOldTimetype(cdCalenType newtype, CdTimeType* oldtype)
{
	switch(newtype){
	  case cdStandard:
		*oldtype = CdChron;
		break;
	  case cdJulian:
		*oldtype = CdJulianCal;
		break;
	  case cdNoLeap:
		*oldtype = CdChronNoLeap;
		break;
	  case cd360:
		*oldtype = CdChron360;
		break;
	  case cdClim:
		*oldtype = CdClim;
		break;
	  case cdClimLeap:
		*oldtype = CdClimLeap;
		break;
	  case cdClim360:
		*oldtype = CdClim360;
		break;
	  default:
		cdError("Error on relative units conversion, invalid timetype = %d",newtype);
		return 1;
	}
	return 0;
}

/* Parse relative units, returning the unit and base component time. */
/* Function returns 1 if error, 0 on success */
int
cdParseRelunits(cdCalenType timetype, char* relunits, cdUnitTime* unit, cdCompTime* base_comptime)
{
	char charunits[CD_MAX_RELUNITS];
	char basetime_1[CD_MAX_CHARTIME];
	char basetime_2[CD_MAX_CHARTIME];
	char basetime[CD_MAX_CHARTIME];
	int nconv;
					     /* Parse the relunits */
	nconv = sscanf(relunits,"%s since %s %s",charunits,basetime_1,basetime_2);
	if(nconv==EOF || nconv==0){
		cdError("Error on relative units conversion, string = %s\n",relunits);
		return 1;
	}

					     /* Get the units */
	cdTrim(charunits,CD_MAX_RELUNITS);
	if(!strncmp(charunits,"sec",3) || !strcmp(charunits,"s")){
		*unit = cdSecond;
	}
	else if(!strncmp(charunits,"min",3) || !strcmp(charunits,"mn")){
		*unit = cdMinute;
	}
	else if(!strncmp(charunits,"hour",4) || !strcmp(charunits,"hr")){
		*unit = cdHour;
	}
	else if(!strncmp(charunits,"day",3) || !strcmp(charunits,"dy")){
		*unit = cdDay;
	}
	else if(!strncmp(charunits,"week",4) || !strcmp(charunits,"wk")){
		*unit = cdWeek;
	}
	else if(!strncmp(charunits,"month",5) || !strcmp(charunits,"mo")){
		*unit = cdMonth;
	}
	else if(!strncmp(charunits,"season",6)){
		*unit = cdSeason;
	}
	else if(!strncmp(charunits,"year",4) || !strcmp(charunits,"yr")){
		if(!(timetype & cdStandardCal)){
			cdError("Error on relative units conversion: climatological units cannot be 'years'.\n");
			return 1;
		}
		*unit = cdYear;
	}
	else {
		cdError("Error on relative units conversion: invalid units = %s\n",charunits);
		return 1;
	}

					     /* Build the basetime, if any (default is 1979), */
					     /* or month 1 for climatological time. */
	if(nconv == 1){
		if(timetype & cdStandardCal)
			strcpy(basetime,CD_DEFAULT_BASEYEAR);
		else
			strcpy(basetime,"1");
	}
					     /* Convert the basetime to component, then epochal (hours since 1970) */
	else{
		if(nconv == 2){
			cdTrim(basetime_1,CD_MAX_CHARTIME);
			strcpy(basetime,basetime_1);
		}
		else{
			cdTrim(basetime_1,CD_MAX_CHARTIME);
			cdTrim(basetime_2,CD_MAX_CHARTIME);
			sprintf(basetime,"%s %s",basetime_1,basetime_2);
		}
	}

	cdChar2Comp(timetype, basetime, base_comptime);

	return 0;
}

/* Parse delta time. Return 0 if success, 1 on error. */
int
cdParseDeltaTime(cdCalenType timetype, char* deltaTime, double* value, cdUnitTime* unit){
	char charunits[CD_MAX_TIME_DELTA];
	int nconv;

	nconv = sscanf(deltaTime,"%lf %s",value,charunits);
	if(nconv==EOF || nconv==0){
		cdError("Error on delta time conversion, string = %s",deltaTime);
		return 1;
	}
	cdTrim(charunits,CD_MAX_TIME_DELTA);
	if(!strncmp(charunits,"sec",3) || !strcmp(charunits,"s")){
		*unit = cdSecond;
	}
	else if(!strncmp(charunits,"min",3) || !strcmp(charunits,"mn")){
		*unit = cdMinute;
	}
	else if(!strncmp(charunits,"hour",4) || !strcmp(charunits,"hr")){
		*unit = cdHour;
	}
	else if(!strncmp(charunits,"day",3) || !strcmp(charunits,"dy")){
		*unit = cdDay;
	}
	else if(!strncmp(charunits,"week",4) || !strcmp(charunits,"wk")){
		*unit = cdWeek;
	}
	else if(!strncmp(charunits,"month",5) || !strcmp(charunits,"mo")){
		*unit = cdMonth;
	}
	else if(!strncmp(charunits,"season",6)){
		*unit = cdSeason;
	}
	else if(!strncmp(charunits,"year",4) || !strcmp(charunits,"yr")){
		if(!(timetype & cdStandardCal)){
			cdError("Error on delta time conversion: climatological units cannot be 'years'.");
			return 1;
		}
		*unit = cdYear;
	}
	else {
		cdError("Error on delta time conversion: invalid units = %s",charunits);
		return 1;
	}
	return 0;
}

void
cdChar2Comp(cdCalenType timetype, char* chartime, cdCompTime* comptime)
{
	double sec;
	int ihr, imin, nconv;
	long year;
	short day;
	short month;

	comptime->year = CD_NULL_YEAR;
	comptime->month = CD_NULL_MONTH;
	comptime->day = CD_NULL_DAY;
	comptime->hour = CD_NULL_HOUR;
	
	if(timetype & cdStandardCal){
		nconv = sscanf(chartime,"%ld-%hd-%hd %d:%d:%lf",&year,&month,&day,&ihr,&imin,&sec);
		if(nconv==EOF || nconv==0){
			cdError("Error on character time conversion, string = %s\n",chartime);
			return;
		}
		if(nconv >= 1){
			comptime->year = year;
		}
		if(nconv >= 2){
			comptime->month = month;
		}
		if(nconv >= 3){
			comptime->day = day;
		}
		if(nconv >= 4){
			if(ihr<0 || ihr>23){
				cdError("Error on character time conversion: invalid hour = %d\n",ihr);
				return;
			}
			comptime->hour = (double)ihr;
		}
		if(nconv >= 5){
			if(imin<0 || imin>59){
				cdError("Error on character time conversion: invalid minute = %d\n",imin);
				return;
			}
			comptime->hour += (double)imin/60.;
		}
		if(nconv >= 6){
			if(sec<0.0 || sec>60.0){
				cdError("Error on character time conversion: invalid second = %lf\n",sec);
				return;
			}
			comptime->hour += sec/3600.;
		}
	}
	else{				     /* Climatological */
		nconv = sscanf(chartime,"%hd-%hd %d:%d:%lf",&month,&day,&ihr,&imin,&sec);
		if(nconv==EOF || nconv==0){
			cdError("Error on character time conversion, string = %s",chartime);
			return;
		}
		if(nconv >= 1){
			comptime->month = month;
		}
		if(nconv >= 2){
			comptime->day = day;
		}
		if(nconv >= 3){
			if(ihr<0 || ihr>23){
				cdError("Error on character time conversion: invalid hour = %d\n",ihr);
				return;
			}
			comptime->hour = (double)ihr;
		}
		if(nconv >= 4){
			if(imin<0 || imin>59){
				cdError("Error on character time conversion: invalid minute = %d\n",imin);
				return;
			}
			comptime->hour += (double)imin/60.;
		}
		if(nconv >= 5){
			if(sec<0.0 || sec>60.0){
				cdError("Error on character time conversion: invalid second = %lf\n",sec);
				return;
			}
			comptime->hour += sec/3600.;
		}
	}
	(void)cdValidateTime(timetype,*comptime);
	return;
}
void
cdChar2Rel(cdCalenType timetype, char* chartime, char* relunits, double* reltime)
{
	cdCompTime comptime;

	cdChar2Comp(timetype, chartime, &comptime);
	cdComp2Rel(timetype, comptime, relunits, reltime);
	return;
}
void
cdComp2Char(cdCalenType timetype, cdCompTime comptime, char* time)
{
	double dtmp, sec;
	int ihr, imin;
	int nskip;

	if(cdValidateTime(timetype,comptime))
		return;
	
	ihr = (int)comptime.hour;
	dtmp = 60.0 * (comptime.hour - (double)ihr);
	imin = (int)dtmp;
	sec = 60.0 * (dtmp - (double)imin);

	nskip = 0;
	if(sec == 0.0){
		if(imin == 0)
			nskip = 2;
		else
			nskip = 1;
	}

	if(timetype & cdStandardCal){
		if(nskip == 0)
			sprintf(time,"%ld-%hd-%hd %d:%d:%lf",comptime.year,comptime.month,comptime.day,ihr,imin,sec);
		else if(nskip == 1)
			sprintf(time,"%ld-%hd-%hd %d:%d",comptime.year,comptime.month,comptime.day,ihr,imin);
		else
			sprintf(time,"%ld-%hd-%hd %d:0",comptime.year,comptime.month,comptime.day,ihr);
	}
	else {				     /* Climatological */
		if(nskip == 0)
			sprintf(time,"%hd-%hd %d:%d:%lf",comptime.month,comptime.day,ihr,imin,sec);
		else if(nskip == 1)
			sprintf(time,"%hd-%hd %d:%d",comptime.month,comptime.day,ihr,imin);
		else
			sprintf(time,"%hd-%hd %d:0",comptime.month,comptime.day,ihr);
	}
	return;
}
void
cdComp2Rel(cdCalenType timetype, cdCompTime comptime, char* relunits, double* reltime)
{
	cdCompTime base_comptime;
	CdDeltaTime deltime;
	CdTime humantime;
	CdTimeType old_timetype;
	cdUnitTime unit;
	double base_etm, etm, delta;
	long ndel, hoursInYear;
	
					     /* Parse the relunits */
	if(cdParseRelunits(timetype, relunits, &unit, &base_comptime))
		return;

					     /* Handle mixed Julian/Gregorian calendar */
	if (timetype == cdMixed){
		switch(unit){
		case cdWeek: case cdDay: case cdHour: case cdMinute: case cdSecond:
			cdComp2RelMixed(comptime, unit, base_comptime, reltime);
			return;
		case cdYear: case cdSeason: case cdMonth:
			timetype = cdStandard;
			break;
		   default:
		      break;
		}
	}
	
					     /* Convert basetime to epochal */
	humantime.year = base_comptime.year;
	humantime.month = base_comptime.month;
	humantime.day = base_comptime.day;
	humantime.hour = base_comptime.hour;
	humantime.baseYear = 1970;
					     /* Map to old-style timetype */
	if(cdToOldTimetype(timetype,&old_timetype))
		return;
	humantime.timeType = old_timetype;
	Cdh2e(&humantime,&base_etm);

					     /* Map end time to epochal */
	humantime.year = comptime.year;
	humantime.month = comptime.month;
	humantime.day = comptime.day;
	humantime.hour = comptime.hour;
	Cdh2e(&humantime,&etm);
					     /* Calculate relative time value for months or hours */
	deltime.count = 1;
	deltime.units = (CdTimeUnit)unit;
	switch(unit){
	  case cdWeek: case cdDay: case cdHour: case cdMinute: case cdSecond:
		delta = etm - base_etm;
		if(!(timetype & cdStandardCal)){	/* Climatological time */
			hoursInYear = (timetype & cd365Days) ? 8760. : (timetype & cdHasLeap) ? 8784. : 8640.;
					     /* Normalize delta to interval [0,hoursInYear) */
			if(delta < 0.0 || delta >= hoursInYear)
				delta -= hoursInYear * floor(delta/hoursInYear);
		}
		break;
	  case cdYear: case cdSeason: case cdMonth:
		CdDivDelTime(base_etm, etm, deltime, old_timetype, 1970, &ndel);
		break;
	}

					     /* Convert to output units */
	switch(unit){
	  case cdSecond:
		*reltime = 3600.0 * delta;
		break;
	  case cdMinute:
		*reltime = 60.0 * delta;
		break;
	  case cdHour:
		*reltime = delta;
		break;
	  case cdDay:
		*reltime = delta/24.0;
		break;
	  case cdWeek:
		*reltime = delta/168.0;
		break;
	  case cdMonth: case cdSeason: case cdYear: /* Already in correct units */
		if(timetype & cdStandardCal)
			*reltime = (base_etm <= etm) ? (double)ndel : (double)(-ndel);
		else			     /* Climatological time is already normalized*/
			*reltime = (double)ndel;
		break;
	}

	return;
}
void
cdRel2Char(cdCalenType timetype, char* relunits, double reltime, char* chartime)
{
	cdCompTime comptime;

	cdRel2Comp(timetype, relunits, reltime, &comptime);
	cdComp2Char(timetype, comptime, chartime);

	return;
}
void
cdRel2Comp(cdCalenType timetype, char* relunits, double reltime, cdCompTime* comptime)
{
	CdDeltaTime deltime;
	CdTime humantime;
	CdTimeType old_timetype;
	cdCompTime base_comptime;
	cdUnitTime unit, baseunits;
	double base_etm, result_etm;
	double delta;
	long idelta;

					     /* Parse the relunits */
	if(cdParseRelunits(timetype, relunits, &unit, &base_comptime))
		return;

	if (timetype == cdMixed){
		switch(unit){
		case cdWeek: case cdDay: case cdHour: case cdMinute: case cdSecond:
			cdRel2CompMixed(reltime, unit, base_comptime, comptime);
			return;
		case cdYear: case cdSeason: case cdMonth:
			timetype = cdStandard;
			break;
		}
	}

	switch(unit){
	  case cdSecond:
		delta = reltime/3600.0;
		baseunits = cdHour;
		break;
	  case cdMinute:
		delta = reltime/60.0;
		baseunits = cdHour;
		break;
	  case cdHour:
		delta = reltime;
		baseunits = cdHour;
		break;
	  case cdDay:
		delta = 24.0 * reltime;
		baseunits = cdHour;
		break;
	  case cdWeek:
		delta = 168.0 * reltime;
		baseunits = cdHour;
		break;
	  case cdMonth:
		idelta = (long)(reltime + (reltime<0 ? -1.e-10 : 1.e-10));
		baseunits = cdMonth;
		break;
	  case cdSeason:
		idelta = (long)(3.0 * reltime + (reltime<0 ? -1.e-10 : 1.e-10));
		baseunits = cdMonth;
		break;
	  case cdYear:
		idelta = (long)(12 * reltime + (reltime<0 ? -1.e-10 : 1.e-10));
		baseunits = cdMonth;
		break;
	}

	deltime.count = 1;
	deltime.units = (CdTimeUnit)baseunits;

	humantime.year = base_comptime.year;
	humantime.month = base_comptime.month;
	humantime.day = base_comptime.day;
	humantime.hour = base_comptime.hour;
	humantime.baseYear = 1970;
					     /* Map to old-style timetype */
	if(cdToOldTimetype(timetype,&old_timetype))
		return;
	humantime.timeType = old_timetype;

	Cdh2e(&humantime,&base_etm);
					     /* If months, seasons, or years, */
	if(baseunits == cdMonth){

					     /* Calculate new epochal time from integer months. */
					     /* Convert back to human, then comptime. */
					     /* For zero reltime, just return the basetime*/
		if(reltime != 0.0){
			CdAddDelTime(base_etm,idelta,deltime,old_timetype,1970,&result_etm);
			Cde2h(result_etm, old_timetype, 1970, &humantime);
		}
	}
					     /* Calculate new epochal time. */
					     /* Convert back to human, then comptime. */
	else{
		Cde2h(base_etm+delta, old_timetype, 1970, &humantime);
		
	}
	comptime->year = humantime.year;
	comptime->month = humantime.month;
	comptime->day = humantime.day;
	comptime->hour = humantime.hour;

	return;
}
void
cdRel2Rel(cdCalenType timetype, char* relunits, double reltime, char* outunits, double* outtime)
{
	cdCompTime comptime;

	cdRel2Comp(timetype, relunits, reltime, &comptime);
	cdComp2Rel(timetype, comptime, outunits, outtime);
	return;
}

/* ----------------------------------------------------------------------------------------------*/
/* Absolute time routines */

					     /* Parse absolute time units, returning */
					     /* the unit and format components. */
					     /* For example, "day as %Y%m%d.f" returns */
					     /* a unit of cdDay, and components [cdYear,cdMonth, */
					     /* cdDay,cdFraction]. ncompon is the number of components. */
					     /* Returns 1 on success, 0 on error. */
int
cdParseAbsunits(char *absunits, cdUnitTime *unit, int *ncompon, cdUnitTime compon[]){
	int nconv;
	char charunits[CD_MAX_ABSUNITS];
	char format[CD_MAX_ABSUNITS];
	char *c;
	int iform;

	nconv = sscanf(absunits,"%s as %s",charunits,format);
	if(nconv==EOF || nconv<2){
		cdError("Error on absolute units conversion, string = %s\n",absunits);
		return 1;
	}
	cdTrim(charunits,CD_MAX_ABSUNITS);
	if(!strncmp(charunits,"hour",4)){
		*unit = cdHour;
	}
	else if(!strncmp(charunits,"day",3)){
		*unit = cdDay;
	}
	else if(!strncmp(charunits,"calendar_month",14)){
		*unit = cdMonth;
	}
	else if(!strncmp(charunits,"calendar_year",13)){
		*unit = cdYear;
	}
	else if(!strncmp(charunits,"min",3)){
		*unit = cdMinute;
	}
	else if(!strncmp(charunits,"sec",3)){
		*unit = cdSecond;
	}
	else {
		cdError("Error on absolute units conversion: invalid units = %s\n",charunits);
		return 1;
	}

					     /* Parse the format */
	for(c=format, iform=0; *c && iform<CD_MAX_ABS_COMPON; c++){
		if(*c=='.')
			continue;
		else if(*c=='%'){
			c++;
			switch(*c){
			case 'Y':
				compon[iform++]=cdYear;
				break;
			case 'm':
				compon[iform++]=cdMonth;
				break;
			case 'd':
				compon[iform++]=cdDay;
				break;
			case 'H':
				compon[iform++]=cdHour;
				break;
			case 'M':
				compon[iform++]=cdMinute;
				break;
			case 'S':
				compon[iform++]=cdSecond;
				break;
			case 'f':
				compon[iform++]=cdFraction;
				break;
			default:
				cdError("Error on absolute units conversion: invalid format = %s\n",format);
				return 1;
				
			}
		}
		else {
			cdError("Error on absolute units conversion: invalid format = %s\n",format);
			return 1;
		}
	}
	*ncompon=iform;
	return 0;
}

					     /* Convert absolute time to component time. */
					     /* frac is the fractional part, or 0.0 if */
					     /* abstimetype is cdInt. */
					     /* */
					     /* Note: for formats which incorporate the */
					     /* diurnal phase, the fractional part is */
					     /* incorporated into comptime->hour. */
					     /* Return 0 on success, 1 on failure. */
int 
cdAbs2Comp(char *absunits, void *abstime, cdType abstimetype, cdCompTime *comptime, double *frac){
	cdUnitTime unit;
	int ncompon;
	cdUnitTime compon[CD_MAX_ABS_COMPON];
	double dabstime, fraction;
	long iabstime;
	int iform;

					     /* Parse the absunits */
	if (cdParseAbsunits(absunits, &unit, &ncompon, compon)==1)
		return 1;

					     /* Break the time into integer and fractional parts */
	switch(abstimetype){
	case cdFloat:
		dabstime = (double)(*(float *)abstime);
		iabstime = (long)dabstime;
		fraction = dabstime-(double)iabstime;
		break;
	case cdDouble:
		dabstime = *(double *)abstime;
		iabstime = (long)dabstime;
		fraction = dabstime-(double)iabstime;
		break;
	case cdInt:
		iabstime = (long)(*(int *)abstime);
		dabstime = (double)iabstime;
		fraction = 0.0;
		break;
	case cdLong:
		iabstime = *(long *)abstime;
		dabstime = (double)iabstime;
		fraction = 0.0;
		break;
	default:
		cdError("Error converting absolute to component time: invalid datatype = %d\n",abstimetype);
		return 1;
	}

					     /* Extract the components */
	comptime->year = 0;
	comptime->month = comptime->day = 1;
	comptime->hour = 0.0;
	for(iform=ncompon-1; iform>=0; iform--){
		switch(compon[iform]){
		case cdYear:
			comptime->year = iabstime%10000;
			iabstime /= 10000;
			break;
		case cdMonth:
			comptime->month = iabstime%100;
			iabstime /= 100;
			break;
		case cdDay:
			comptime->day = iabstime%100;
			iabstime /= 100;
			break;
		case cdHour:
			comptime->hour = dabstime;
			break;
		case cdMinute:
			comptime->hour = dabstime/60.0;
			break;
		case cdSecond:
			comptime->hour = dabstime/3600.0;
			break;
		case cdFraction:
			if (unit==cdDay)
				comptime->hour = 24.0*fraction;
			break;
		}
	}
	*frac = fraction;
	return 0;
}

					     /* Convert component to absolute time. */
					     /* absunits is of the form "<unit> as <format>". */
					     /* abstimetype is either cdFloat, cdDouble, or cdInt. */
					     /* frac, the fractional part, is only used */
					     /* if unit is cdMonth or cdYear, and */
					     /* abstimetype is cdFloat or cdDouble. */
					     /* Otherwise, it is derived from comptime.hour. */
					     /* abstime is the absolute time returned. */
					     /* The function returns 0 on success, 1 on failure. */

int
cdComp2Abs(cdCompTime comptime, char *absunits, cdType abstimetype, double frac, void *abstime){
	cdUnitTime unit;
	int ncompon;
	cdUnitTime compon[CD_MAX_ABS_COMPON];
	double dabstime;
	long iabstime;
	int iform;

					     /* Parse the absunits */
	if (cdParseAbsunits(absunits, &unit, &ncompon, compon)==1)
		return 1;

					     /* Set the absolute time */
	iabstime = 0;
	dabstime = 0.0;
	for(iform=0; iform<ncompon; iform++){
		switch(compon[iform]){
		case cdYear:
			iabstime = iabstime*10000+comptime.year;
			dabstime = (double)iabstime;
			break;
		case cdMonth:
			iabstime = iabstime*100+comptime.month;
			dabstime = (double)iabstime;
			break;
		case cdDay:
			iabstime = iabstime*100+comptime.day;
			dabstime = (double)iabstime;
			break;
		case cdHour:
			dabstime = comptime.hour;
			iabstime = (long)dabstime;
			break;
		case cdMinute:
			dabstime = 60.0*comptime.hour;
			iabstime = (long)dabstime;
			break;
		case cdSecond:
			dabstime = 3600.0*comptime.hour;
			iabstime = (long)dabstime;
			break;
		case cdFraction:
			if (unit==cdDay)
				dabstime += comptime.hour/24.0;
			else if(unit==cdYear)
				dabstime += frac;
			else if(unit==cdMonth)
				dabstime += frac;
			break;
		}
	}

					     /* Cast to the specified datatype */
	switch(abstimetype){
	case cdFloat:
		*(float *)abstime = (float)dabstime;
		break;
	case cdDouble:
		*(double *)abstime = dabstime;
		break;
	case cdInt:
		*(int *)abstime = (int)iabstime;
		break;
	case cdLong:
		*(long *)abstime = iabstime;
		break;
	default:
		cdError("Error converting component to absolute time: invalid datatype = %d\n",abstimetype);
		return 1;
	}
	return 0;
}

					     /* Return 1 if the time and units are a valid relative */
					     /* time, 0 if not. If comptime is non-null, return the */
					     /* components */
int
cdDecodeRelativeTime(cdCalenType timetype, char* units, double time, cdCompTime* comptime){
	int saveOpts, saveOccurred, err;
	cdCompTime ctime, *pctime;

	pctime = (comptime ? comptime : &ctime);

	saveOpts = cuErrOpts;
	saveOccurred = cuErrorOccurred;
	cuErrOpts = 0;			     /* Turn off error reporting */

					     /* If it's absolute, it's not relative */
	if (cdDecodeAbsoluteTime(units, &time, cdDouble, 0, 0)){
		cuErrOpts = saveOpts;
		return 0;
	}
	cuErrorOccurred = 0;
	cdRel2Comp(timetype, units, time, pctime);

	err = (cuErrorOccurred==0);
	cuErrOpts = saveOpts;
	cuErrorOccurred = saveOccurred;
	return err;
}

					     /* Return 1 if the time and units are a valid absolute */
					     /* time, 0 if not.*/
int
cdDecodeAbsoluteTime(char* units, void* time, cdType abstimetype, cdCompTime* comptime, double* fraction){
	int saveOpts, err;
	cdCompTime ctime, *pctime;
	double frac, *pfrac;

	pctime = (comptime ? comptime : &ctime);
	pfrac = (fraction ? fraction : &frac);

	saveOpts = cuErrOpts;
	cuErrOpts = 0;			     /* Turn off error reporting */
	err = cdAbs2Comp(units, time, abstimetype, pctime, pfrac);
	
	cuErrOpts = saveOpts;
	return (err==0);
}
/* ----------------------------------------------------------------------------------------------*/
/* Mixed Julian/Gregorian calendar routines */

					     /* Return value expressed in hours. */
double
cdToHours(double value, cdUnitTime unit){

	double result;

	switch(unit){
	case cdSecond:
		result = value/3600.0;
		break;
	case cdMinute:
		result = value/60.0;
		break;
	case cdHour:
		result = value;
		break;
	case cdDay:
		result = 24.0 * value;
		break;
	case cdWeek:
		result = 168.0 * value;
		break;
	}
	return result;
}
					     /* Value is in hours. Translate to units. */
double cdFromHours(double value, cdUnitTime unit){
	double result;

	switch(unit){
	case cdSecond:
		result = value * 3600.0;
		break;
	case cdMinute:
		result = value * 60.0;
		break;
	case cdHour:
		result = value;
		break;
	case cdDay:
		result = value/24.0;
		break;
	case cdWeek:
		result = value/168.0;
		break;
	}
	return result;
}

					     /* Add (value,unit) to comptime. */
					     /* value is in hours. */
					     /* calendar is anything but cdMixed. */
void
cdCompAdd(cdCompTime comptime, double value, cdCalenType calendar, cdCompTime *result){

	double reltime;

	cdComp2Rel(calendar, comptime, "hours", &reltime);
	reltime += value;
	cdRel2Comp(calendar, "hours", reltime, result);
	return;
}

					     /* ca - cb in Julian calendar */
					     /* Result is in hours. */
double
cdDiffJulian(cdCompTime ca, cdCompTime cb){

	double rela, relb;

	cdComp2Rel(cdJulian, ca, "hours", &rela);
	cdComp2Rel(cdJulian, cb, "hours", &relb);
	return (rela - relb);
}

					     /* ca - cb in Gregorian calendar */
					     /* Result is in hours. */
double
cdDiffGregorian(cdCompTime ca, cdCompTime cb){

	double rela, relb;

	cdComp2Rel(cdStandard, ca, "hours", &rela);
	cdComp2Rel(cdStandard, cb, "hours", &relb);
	return (rela - relb);
}

					     /* Return -1, 0, 1 as ca is less than, equal to, */
					     /* or greater than cb, respectively. */
int cdCompCompare(cdCompTime ca, cdCompTime cb){

	int test;

	if ((test = VALCMP(ca.year, cb.year)))
		return test;
	else if ((test = VALCMP(ca.month, cb.month)))
		return test;
	else if ((test = VALCMP(ca.day, cb.day)))
		return test;
	else
		return (test = VALCMP(ca.hour, cb.hour));
}

					     /* ca - cb in mixed Julian/Gregorian calendar. */
					     /* Result is in hours. */
double
cdDiffMixed(cdCompTime ca, cdCompTime cb){

	static cdCompTime ZA = {1582, 10, 5, 0.0};
	static cdCompTime ZB = {1582, 10, 15, 0.0};
	double result;

	if (cdCompCompare(cb, ZB) == -1){
		if (cdCompCompare(ca, ZB) == -1) {
			result = cdDiffJulian(ca, cb);
		}
		else {
			result = cdDiffGregorian(ca, ZB) + cdDiffJulian(ZA, cb);
		}
	}
	else {
		if (cdCompCompare(ca, ZB) == -1){
			result = cdDiffJulian(ca, ZA) + cdDiffGregorian(ZB, cb);
		}
		else {
			result = cdDiffGregorian(ca, cb);
		}
	}
	return result;
}

					     /* Add value in hours to ct, */
					     /* in the mixed Julian/Gregorian calendar. */
void
cdCompAddMixed(cdCompTime ct, double value, cdCompTime *result){

	static cdCompTime ZA = {1582, 10, 5, 0.0};
	static cdCompTime ZB = {1582, 10, 15, 0.0};
	double xj, xg;

	if (cdCompCompare(ct, ZB) == -1){
		xj = cdDiffJulian(ZA, ct);
		if (value <= xj){
			cdCompAdd(ct, value, cdJulian, result);
		}
		else {
			cdCompAdd(ZB, value-xj, cdStandard, result);
		}
	}
	else {
		xg = cdDiffGregorian(ZB, ct);
		if (value > xg){
			cdCompAdd(ct, value, cdStandard, result);
		}
		else {
			cdCompAdd(ZA, value-xg, cdJulian, result);
		}
	}
	return;
}

					     /* Convert ct to relunits (unit, basetime) */
					     /* in the mixed Julian/Gregorian calendar. */
					     /* unit is anything but year, season, month. unit and basetime are */
					     /* from the parsed relunits. Return result in reltime. */
void
cdComp2RelMixed(cdCompTime ct, cdUnitTime unit, cdCompTime basetime, double *reltime){

	double hourdiff;

	hourdiff = cdDiffMixed(ct, basetime);
	*reltime = cdFromHours(hourdiff, unit);
	return;
}

					     /* Convert relative time (reltime, unit, basetime) to comptime in the */
					     /* mixed Julian/Gregorian calendar. unit is anything but */
					     /* year, season, month. unit and basetime are */
					     /* from the parsed relunits. Return result in comptime. */
void
cdRel2CompMixed(double reltime, cdUnitTime unit, cdCompTime basetime, cdCompTime *comptime){

	reltime = cdToHours(reltime, unit);
	cdCompAddMixed(basetime, reltime, comptime);
	return;
}
