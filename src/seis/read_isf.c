/* From https://www.isc.ac.uk/standards/isf/download/isf_c.tar */

#include "isf_head.h"

float select_mag(int mag_c, float *mags, char **mag_t);

int read_event_data(FILE *fp, char *line, int *yyyy, int *mm, int *dd, int *hh, int *mi, 
                int *ss, int *msec, char *timfix, float *stime, float *sdobs,
                float *lat, float *lon, char *epifix, float *smaj, float *smin,
                int *strike, float *depth, char *depfix, float *sdepth,
                int *ndef, int *nsta, int *gap, float *mindist, float *maxdist,
                char *antype, char *loctype, char *etype, char *author, char *origid,
                float *lats, float *lons, float *rms, float *depths, int *years,
		int *months, int *days, int *hours, int *minutes, int *idx_min_rms) {

	int	done = false, event_c = 0; 
	float	gap_min = 360;
	*idx_min_rms = 0; 	/* I'm currently using GAP instead of RMS */

	while (!done && (fgets (line, ISF_LINE_LEN, fp) != NULL)) {
		if (!read_origin(line,yyyy,mm,dd,hh,mi,ss,msec,timfix,stime,sdobs,
				lat,lon,epifix,smaj,smin,strike,depth,depfix,sdepth,ndef,
				nsta,gap,mindist,maxdist,antype,loctype,etype,author,origid)) {
			lats[event_c] = *lat;	lons[event_c] = *lon;
			rms[event_c] = *sdobs;	depths[event_c] = *depth;
			years[event_c] = *yyyy;	months[event_c] = *mm;
			days[event_c] = *dd;	hours[event_c] = *hh;
			minutes[event_c] = *mi;
			if (*gap != ISF_NULL && *gap < gap_min) {
				gap_min = *gap;
				*idx_min_rms = event_c;
			}
			event_c++;
		}
		else
			done = true;
	}
	return event_c;
}

int read_mags(FILE *fp, char *line, char *magtype, char* magind, float* mag, float* magerr,
		int* nsta, char* author, char* origid, char **mag_t, float *mags) {

	int	done = false, mag_c = 0; 

	while (!done && (fgets (line, ISF_LINE_LEN, fp) != NULL)) {
		if(!read_netmag(line,magtype,magind,mag,magerr,nsta,author,origid)) {
			sscanf (magtype, "%s",mag_t[mag_c]);
			mags[mag_c] = *mag;
			mag_c++;
		}
		else
			done = true;
	}
	return(mag_c);
}

float select_mag(int mag_c, float *mags, char **mag_t) {
	/* Select among the following magnitudes in this orther */ 
	int	i;
	float	mag;
	for (i = 0; i < mag_c; i++) {
		gmt_str_toupper (mag_t[i]);
		if (!strncmp(mag_t[i],"MW",2)) {
			mag = mags[i];
			break;
		}
		else if (!strncmp(mag_t[i],"MB",2)) {
			mag = mags[i];
			break;
		}
		else if (!strncmp(mag_t[i],"MS",2)) {
			mag = mags[i];
			break;
		}
		else if (!strncmp(mag_t[i],"MD",2)) {
			mag = mags[i];
			break;
		}
		else if (!strncmp(mag_t[i],"ML",2)) {
			mag = mags[i];
			break;
		}
		else	/* A magnitude not in this list */
			mag = mags[0];
	}
	return (mag);
}

/*  Parses a line asuming it to be an event title line.
    Requires event ID to be present but allows lines with no region.

    Returns 0 if the line is a properly formatted event ID line.
    Returns 20 and writes a diagnostic to isf_error on error.
*/
int read_event_id(char *line, char *evid, char *region) {
    char substr[ISF_LINE_LEN];

    /* Chars 1-5: must be the word 'Event'. Char 6: must be a space. */
    if (strncmp(line,"Event ",6) && strncmp(line,"EVENT ",6)){
        sprintf(isf_error,"not an event title line: %s",line); 
        return 20;
    }

    /* Chars 7-14: event ID */
    if (!partline(evid,line,6,8)){
        sprintf(isf_error,"missing evid: %s",line); 
        return 20;
    }
    if (check_whole(evid)){
        sprintf(isf_error,"bad evid: %s",line); 
        return 20;
    }

    /* Not quite right but lots of people hit CR after evid */
    if (strlen(line) < 15) return 0;

    /* Char 15: must be a space */
    if (line[14] != ' ' ){
        sprintf(isf_error,"bad format, char 15: %s",line); 
        return 20;
    }

    /* Chars 16-80: geographic region if there */
    partline(region,line,15,65);

    /* Check for extra characters after char 80.  */
    if (partline(substr,line,80,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }  

    return 0;
}


/*  Tests a line to discover if it is an origin header line.

    Returns 0 if the line is a properly formatted origin header line.
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_origin_head(char *line) {
    char substr[ISF_LINE_LEN];
    char head[] = "   Date       Time        Err   RMS Latitude Longitude  Smaj  Smin  Az Depth   Err Ndef Nsta Gap  mdist  Mdist Qual   Author      OrigID";
    int headlen = 136;

    if (strncmp(line,head,headlen) != 0){
        sprintf(isf_error,"not an origin header: %s",line); 
        return 20;
    }

    /* Check for extra characters after char 136.  */
    if (partline(substr,line,headlen,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}


/*  Parses a line asuming it to be an origin line.
    Values are asigned to variables, the pointers to which have been sent
    as arguments. If an optional parameter is not given then the
    corresponding variable will have ISF_NULL assigned to it.

    Returns 0 if the line is a properly formatted origin line.
    Returns 20 and writes a diagnostic to isf_error on error.
*/
int read_origin(char *line, int *yyyy, int *mm, int *dd, int *hh, int *mi, 
                int *ss, int *msec, char *timfix, float *stime, float *sdobs,
                float *lat, float *lon, char *epifix, float *smaj, float *smin,
                int *strike, float *depth, char *depfix, float *sdepth,
                int *ndef, int *nsta, int *gap, float *mindist,
                float *maxdist, char *antype, char *loctype, char *etype,
                char *author, char *origid) {

    char substr[ISF_LINE_LEN];

    /* Chars 1-4: year. */
    if (!partline(substr,line,0,4)){
        sprintf(isf_error,"missing year: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad year: %s",line); 
        return 20;
    }
    *yyyy = atoi(substr);

    /* Char 5: '/' character. */
    if (line[4] != '/'){
        sprintf(isf_error,"bad date: %s",line); 
        return 20;
    }

    /* Chars 6-7: month. */
    if (!partline(substr,line,5,2)){
        sprintf(isf_error,"missing month: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad month: %s",line); 
        return 20;
    }
    *mm = atoi(substr);

    /* Char 8: '/' character. */
    if (line[7] != '/'){
        sprintf(isf_error,"bad date: %s",line); 
        return 20;
    }

    /* Chars 9-10: day. */
    if (!partline(substr,line,8,2)){
        sprintf(isf_error,"missing day: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad day: %s",line); 
        return 20;
    }
    *dd = atoi(substr);

    /* Char 11: space. */
    if (line[10] != ' '){
        sprintf(isf_error,"bad date: %s",line); 
        return 20;
    }

    /* Chars 12,13: hour. */
    if (!partline(substr,line,11,2)){
        sprintf(isf_error,"missing hour: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad hour: %s",line); 
        return 20;
    }
    *hh = atoi(substr);

    /* Char 14: ':' character. */
    if (line[13] != ':'){
        sprintf(isf_error,"bad date: %s",line); 
        return 20;
    }

    /* Chars 15,16: minute. */
    if (!partline(substr,line,14,2)){
        sprintf(isf_error,"missing minute: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad minute: %s",line); 
        return 20;
    }
    *mi = atoi(substr);

    /* Char 17: ':' character. */
    if (line[16] != ':'){
        sprintf(isf_error,"bad date: %s",line); 
        return 20;
    }

   /* Chars 18,19: integral second. */
    if (!partline(substr,line,17,2)){
        sprintf(isf_error,"missing second: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad second: %s",line); 
        return 20;
    }
    *ss = atoi(substr);

    /* Char 20-22: msec or spaces. */
    /* Allow decimal place with no numbers after it. */
    if (partline(substr,line,20,2)){
        /* Char 20: '.' character */
        if (line[19] != '.'){
            sprintf(isf_error,"bad date: %s",line); 
            return 20;
        }
        /* Chars 21,22: 10s of msec. */
        if (!isdigit(line[20])){
            sprintf(isf_error,"bad date: %s",line); 
            return 20;
        }
        *msec = (line[20]-'0')*100;

        if (isdigit(line[21])){
               *msec += (line[21]-'0')*10;
        }
        else if (line[21] != ' ') {
            sprintf(isf_error,"bad date: %s",line); 
            return 20;
        }
    }
    else {
        /* Char 20: '.' character or space */
        if (line[19] != '.' && line[19] != ' '){
            sprintf(isf_error,"bad date: %s",line); 
            return 20;
        }
        *msec = ISF_NULL;
    }

    /* Char 23: timfix - either f or space. */
    if (line[22] == ' ' || line[22] == 'f'){
         *timfix = line[22];
    }
    else {
        sprintf(isf_error,"bad timfix: %s",line); 
        return 20;
    }

    /* Char 24: space. */
    if (line[23] != ' '){
        sprintf(isf_error,"bad format, char 24: %s",line); 
        return 20;
    }

    /* Chars 25-29: origin time error - float if anything. */
    if (partline(substr,line,24,5)){
       if (check_float(substr)){
            sprintf(isf_error,"bad stime: %s",line); 
            return 20;
        }
        *stime = (float)atof(substr);
    }
    else {
        *stime = ISF_NULL;
    }

    /* Char 30: space. */
    if (line[29] != ' '){
        sprintf(isf_error,"bad format, char 30: %s",line); 
        return 20;
    }

    /* Chars 31-35: rms (sdobs) - float if anything. */
    if (partline(substr,line,30,5)){
       if (check_float(substr)){
            sprintf(isf_error,"bad sdobs: %s",line); 
            return 20;
        }
        *sdobs = (float)atof(substr);
    }
    else {
        *sdobs = ISF_NULL;
    }

    /* Char 36: space. */
    if (line[35] != ' '){
        sprintf(isf_error,"bad format, char 36: %s",line); 
        return 20;
    }

    /* Chars 37-44: lattitude - must be there. */
    if (!partline(substr,line,36,8)){
        sprintf(isf_error,"missing lattitude: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad lattitude: %s",line); 
        return 20;
    }
    *lat = (float)atof(substr);

    /* Char 45: space. */
    if (line[44] != ' '){
        sprintf(isf_error,"bad format, char 45: %s",line); 
        return 20;
    }

   /* Chars 46-54: longitude - must be float. */
    if (!partline(substr,line,45,9)){
        sprintf(isf_error,"missing longitude: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad longitude: %s",line); 
        return 20;
    }
    *lon = (float)atof(substr);

    /* Char 55: epifix - either f or space. */
    if (line[54] == ' ' || line[54] == 'f'){
         *epifix = line[54];
    }
    else {
        sprintf(isf_error,"bad epifix: %s",line); 
        return 20;
    }

    /* Chars 56-60: semi-major axis of error ellipse - float if there. */
    /* This is departure from format but smaj < smin is daft. */
    if (partline(substr,line,55,5)){
       if (check_float(substr)){
            sprintf(isf_error,"bad smaj: %s",line); 
            return 20;
        }
        *smaj = (float)atof(substr);
    }
    else {
        *smaj = ISF_NULL;
    }

    /* Char 61: space. */
    if (line[60] != ' '){
        sprintf(isf_error,"bad format, char 61: %s",line); 
        return 20;
    }

    /* Chars 62-66: semi-minor axis of error ellipse - float if there. */
    if (partline(substr,line,61,5)){
       if (check_float(substr)){
            sprintf(isf_error,"bad smin: %s",line); 
            return 20;
        }
        *smin = (float)atof(substr);
    }
    else {
        *smin = ISF_NULL;
    }

    /* Char 67: space. */
    if (line[66] != ' '){
        sprintf(isf_error,"bad format, char 67: %s",line); 
        return 20;
    }

    /* Chars 68-70: strike - integer if there. */
    if (partline(substr,line,67,3)){
       if (check_int(substr)){
            sprintf(isf_error,"bad strike: %s",line); 
            return 20;
        }
        *strike = atoi(substr);
    }
    else {
        *strike = ISF_NULL;
    }

    /* Char 71: space. */
    if (line[70] != ' '){
        sprintf(isf_error,"bad format, char 71: %s",line); 
        return 20;
    }

    /* Chars 72-76: depth - float if there. */
    if (partline(substr,line,71,5)){
       if (check_float(substr)){
            sprintf(isf_error,"bad depth: %s",line); 
            return 20;
        }
        *depth = (float)atof(substr);
    }
    else {
        *depth = ISF_NULL;
    }

    /* Char 77: depfix - either f,d, or space. */
    if (line[76] == ' ' || line[76] == 'f' || line[76] == 'd'){
         *depfix = line[76];
    }
    else {
        sprintf(isf_error,"bad depfix: %s",line); 
        return 20;
    }

    /* Char 78: space. */
    if (line[77] != ' '){
        sprintf(isf_error,"bad format, char 78: %s",line); 
        return 20;
    }

    /* Chars 79-82: depth error - float if there. */
    if (partline(substr,line,78,4)){
       if (check_float(substr)){
            sprintf(isf_error,"bad sdepth: %s",line); 
            return 20;
        }
        *sdepth = (float)atof(substr);
    }
    else {
        *sdepth = ISF_NULL;
    }

    /* Char 83: space. */
    if (line[82] != ' '){
        sprintf(isf_error,"bad format, char 83: %s",line); 
        return 20;
    }

    /* Chars 84-87: ndef - integer if there. */
    if (partline(substr,line,83,4)){
       if (check_int(substr)){
            sprintf(isf_error,"bad ndef: %s",line); 
            return 20;
        }
        *ndef = atoi(substr);
    }
    else {
        *ndef = ISF_NULL;
    }

    /* Char 88: space. */
    if (line[87] != ' '){
        sprintf(isf_error,"bad format, char 88: %s",line); 
        return 20;
    }

    /* Chars 89-92: nsta - integer if there. */
    if (partline(substr,line,88,4)){
       if (check_int(substr)){
            sprintf(isf_error,"bad nsta: %s",line); 
            return 20;
        }
        *nsta = atoi(substr);
    }
    else {
        *nsta = ISF_NULL;
    }

    /* Char 93: space. */
    if (line[92] != ' '){
        sprintf(isf_error,"bad format, char 93: %s",line); 
        return 20;
    }

    /* Chars 94-96: gap - integer if there */
    if (partline(substr,line,93,3)){
       if (check_int(substr)){
            sprintf(isf_error,"bad gap: %s",line); 
            return 20;
        }
        *gap = atoi(substr);
    }
    else {
        *gap = ISF_NULL;
    }

    /* Char 97: space. */
    if (line[96] != ' '){
        sprintf(isf_error,"bad format, char 97: %s",line); 
        return 20;
    }

    /* Chars 98-103: minimum distance - float if there. */
    if (partline(substr,line,97,6)){
       if (check_float(substr)){
            sprintf(isf_error,"bad mindist: %s",line); 
            return 20;
        }
        *mindist = (float)atof(substr);
    }
    else {
        *mindist = ISF_NULL;
    }

    /* Char 104: space. */
    if (line[103] != ' '){
        sprintf(isf_error,"bad format, char 104: %s",line); 
        return 20;
    }

    /* Chars 105-110: maximum distance - float if there. */
    if (partline(substr,line,104,6)){
       if (check_float(substr)){
            sprintf(isf_error,"bad maxdist: %s",line); 
            return 20;
        }
        *maxdist = (float)atof(substr);
    }
    else {
        *maxdist = ISF_NULL;
    }

    /* Char 111: space. */
    if (line[110] != ' '){
        sprintf(isf_error,"bad format, char 111: %s",line); 
        return 20;
    }

    /* Char 112: analysis type - either space, a, m, or g. */
    if (line[111] == ' ' || line[111] == 'a' || line[111] == 'm' || \
        line[111] =='g' ){
         *antype = line[111];
    }
    else {
        sprintf(isf_error,"bad antype: %s",line); 
        return 20;
    }

    /* Char 113: space. */
    if (line[112] != ' '){
        sprintf(isf_error,"bad format, char 113: %s",line); 
        return 20;
    }

    /* Char 114: location method - either space, i, p, g, or o. */
    if (line[113] == ' ' || line[113] == 'i' || line[113] == 'p' || \
        line[113] =='g' || line[113] == 'o'){
         *loctype = line[113];
    }
    else {
        sprintf(isf_error,"bad loctype: %s",line); 
        return 20;
    }

    /* Char 115: space. */
    if (line[114] != ' '){
        sprintf(isf_error,"bad format, char 115: %s",line); 
        return 20;
    }

    /* Chars 116-117: event type, any characters allowed but must be there. */
    if (!partline(etype,line,115,2)){
          strcpy(etype,"");
    }
    else if (strlen(etype) != 2){
        sprintf(isf_error,"bad etype: %s",line); 
        return 20;
    }

    /* Char 118: space. */
    if (line[117] != ' '){
        sprintf(isf_error,"bad format, char 118: %s",line); 
        return 20;
    }

    /* Chars 119-127: author, any characters allowed but must be there. */
    if (!partline(author,line,118,9)){
        sprintf(isf_error,"missing author: %s",line); 
        return 20;
    }
    if (check_whole(author)){
        sprintf(isf_error,"bad author: %s",line); 
        return 20;
    }

    /* Char 128: space */
    if (line[127] != ' '){
        sprintf(isf_error,"bad format, char 128: %s",line); 
        return 20;
    }

    /* Chars 129-136: origin ID, any characters allowed but must be there. */
    if (!partline(origid,line,128,8)){
        sprintf(isf_error,"missing origid: %s",line); 
        return 20;
    }
    if (check_whole(origid)){
        sprintf(isf_error,"bad origid: %s",line); 
        return 20;
    }

    /* Check for extra stuff after char 136. */
    if (partline(substr,line,136,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    } 
    return 0;
}

/*  To check that a line is a good magnitude block header line. 

    Returns 0 if the line is a magnitude block header.
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_netmag_head(char *line) {
    char substr[ISF_LINE_LEN];

    char head[] = "Magnitude  Err Nsta Author      OrigID";
    int headlen = 38;

    if (strncmp(line,head,headlen) != 0){
        sprintf(isf_error,"not a netmag header: %s",line); 
        return 20;
    }
    if (partline(substr,line,headlen,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}

/*  Parses a line assuming that it is a magnitude sub-block data line.
    Values are asigned to variables, the pointers to which have been sent
    as arguments. If an optional parameter is not given then the
    corresponding variable will have ISF_NULL assigned to it.
                                      
    Returns 0 if the line is a properly formatted magnitude line,
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_netmag( char *line, char *magtype, char* magind, float* mag,
                    float* magerr, int* nsta, char* author, char* origid)
{
    char substr[ISF_LINE_LEN];

    /* Chars 1-5: magnitude type, any characters allowed but must be there. */
    if (!partline(magtype,line,0,5)){
        sprintf(isf_error,"missing magtype: %s",line); 
        return 20;
    }
    if (check_whole(magtype)){
        sprintf(isf_error,"bad magtype: %s",line); 
        return 20;
    }

    /* Char 6: less than or greater than indicator or space only. */
    if (line[5] == ' ' || line[5] == '<' || line[5] == '>'){
        *magind = line[5];
    }
    else {
        sprintf(isf_error,"bad magind: %s",line); 
        return 20;
    }

    /* Chars 7-10: magnitude, must be float. */
    if (!partline(substr,line,6,4)){
        sprintf(isf_error,"missing magnitude: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad magnitude: %s",line); 
        return 20;
    }
    *mag = (float)atof(substr);

    /* Char 11: must be a space. */
    if (line[10] != ' ' ){
        sprintf(isf_error,"bad format, char 11: %s",line); 
        return 20;
    }

    /* Chars 12-14: magnitude error, float if anything. */
    if (partline(substr,line,11,3)){
        if (check_float(substr)){
            sprintf(isf_error,"bad magnitude error: %s",line); 
            return 20;
        }
        *magerr = (float)atof(substr);
    }
    else {
        *magerr = ISF_NULL;
    }

    /* Char 15: must be a space. */
    if (line[14] != ' ' ){
        sprintf(isf_error,"bad format, char 15: %s",line); 
        return 20;
    }

    /* Chars 16-19: number of stations, integer if anything. */
    if (partline(substr,line,15,4)){
        if (check_float(substr)){
            sprintf(isf_error,"bad nsta: %s",line); 
            return 20;
        }
        *nsta = atoi(substr);
    }
    else {
        *nsta = ISF_NULL;
    }

    /* Char 20: must be a space. */
    if (line[19] != ' ' ){
        sprintf(isf_error,"bad format, char 20: %s",line); 
        return 20;
    }

    /* Chars 21-29: author, any characters allowed but must be there. */
    if (!partline(author,line,20,9)){
        sprintf(isf_error,"missing author: %s",line); 
        return 20;
    }
    if (check_whole(author)){
        sprintf(isf_error,"bad author: %s",line); 
        return 20;
    }

    /* Char 30: must be a space. */
    if (line[29] != ' ' ){
        sprintf(isf_error,"bad format, char 30: %s",line); 
        return 20;
    }

    /* Chars 31-38: origin ID, any characters allowed but must be there. */
    if (!partline(origid,line,30,8)){
        sprintf(isf_error,"missing origid: %s",line); 
        return 20;
    }
    if (check_whole(origid)){
        sprintf(isf_error,"bad origid: %s",line); 
        return 20;
    }

    /* Check for extra stuff after char 38. */
    if (partline(substr,line,38,0)){
       sprintf(isf_error,"extra characters at end: %s",line); 
       return 20;
    }    
    return 0;
}

/*  Parses a line to test whether it is a centroid origin label.

    Returns 0 if the line is a properly formatted centroid origin line.
    Returns 20 and writes a diagnostic to isf_error if not.
*/
int read_origin_centroid(char *line) {
    char substr[ISF_LINE_LEN];

    if (strncmp(line," (#CENTROID)",12)){
        sprintf(isf_error,"not a centroid comment: %s",line); 
        return 20;
    }
    if (partline(substr,line,13,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}

/*  Tests a line to discover if it is a first moment tensor header comment.

    Returns 0 if the line is a first moment tensor header line.
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_momten_head_1(char *line) {
    char substr[ISF_LINE_LEN];
    char head[] = " (#MOMTENS sc    M0 fCLVD    MRR    MTT    MPP    MRT    MTP    MPR NST1 NST2 Author   )";
    int headlen = 88;

    if (strncmp(line,head,headlen) != 0){
        sprintf(isf_error,"not a momten header: %s",line); 
        return 20;
    }  
    if (partline(substr,line,headlen,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}


/*  Tests a line to discover if it is a second moment tensor header comment.

    Returns 0 if the line is a second moment tensor header line.
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_momten_head_2(char *line) {
    char substr[ISF_LINE_LEN];
    char head[] = " (#             eM0 eCLVD    eRR    eTT    ePP    eRT    eTP    ePR NCO1 NCO2 Duration )";
    int headlen = 88;

    if (strncmp(line,head,headlen) != 0){
        sprintf(isf_error,"not a momten header2: %s",line); 
        return 20;
    }
    if (partline(substr,line,headlen,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }   
    return 0;
}

/*  Parses a line asuming it to be a first moment tensor data comment.
    Values are asigned to variables, the pointers to which have been sent
    as arguments. If an optional parameter is not given then the
    corresponding variable will have ISF_NULL assigned to it.

    Returns 0 if the line is a properly formatted first moment tensor data line.
    Returns 20 and writes a diagnostic to isf_error on error.
*/
int read_momten_line_1(char *line, int *scale_factor, float *scalar_moment,
                       float *fclvd, float *mrr, float *mtt, float *mpp,
                       float *mrt, float *mtp, float *mpr, int *nsta1,
                       int *nsta2, char *author)
{
    char substr[ISF_LINE_LEN];

    /* Chars 1-11: should be the string  ' (#        ' */
    if (strncmp(line," (#        ",11) != 0){
        sprintf(isf_error,"not a moment tensor line: %s",line); 
        return 20;
    }

    /* Chars 12,13: scale factor - integer */
    if (!partline(substr,line,11,2)){
        sprintf(isf_error,"missing scale factor: %s",line); 
        return 20;
    }
    if (check_int(substr)){
        sprintf(isf_error,"bad scale factor: %s",line); 
        return 20;
    }
    *scale_factor = atoi(substr);

    /* Char 14: must be a space */
    if (line[13] != ' ' ){
        sprintf(isf_error,"bad format, char 14: %s",line); 
        return 20;
    }

    /* Chars 15-19: scalar seismic moment - must be float. */
    if (!partline(substr,line,14,5)){
        sprintf(isf_error,"missing moment: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad moment: %s",line); 
        return 20;
    }
    *scalar_moment = (float)atof(substr);

    /* Char 20: must be a space */
    if (line[19] != ' ' ){
        sprintf(isf_error,"bad format, char 20: %s",line); 
        return 20;
    }

    /* Chars 21-25: fCLVD, float if anything */
    if (partline(substr,line,20,5)){
        if (check_float(substr)){
            sprintf(isf_error,"bad fclvd: %s",line); 
            return 20;
        }
        *fclvd = (float)atof(substr);
    }
    else {
        *fclvd = ISF_NULL;
    }

    /* Char 26: must be a space */
    if (line[25] != ' ' ){
        sprintf(isf_error,"bad format, char 26: %s",line); 
        return 20;
    }

    /* Chars 27-32: radial-radial element, float if anything */
    if (partline(substr,line,26,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mrr: %s",line); 
            return 20;
        }
        *mrr = (float)atof(substr);
    }
    else {
        *mrr = ISF_NULL;
    }

    /* Char 33: must be a space */
    if (line[32] != ' ' ){
        sprintf(isf_error,"bad format, char 33: %s",line); 
        return 20;
    }

    /* Chars 34-39: theta-theta element, float if anything */
    if (partline(substr,line,33,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mtt: %s",line); 
            return 20;
        }
        *mtt = (float)atof(substr);
    }
    else {
        *mtt = ISF_NULL;
    }

    /* Char 40: must be a space */
    if (line[39] != ' ' ){
        sprintf(isf_error,"bad format, char 40: %s",line); 
        return 20;
    }

    /* Chars 41-46: phi-phi element, float if anything */
    if (partline(substr,line,40,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mpp: %s",line); 
            return 20;
        }
        *mpp = (float)atof(substr);
    }
    else {
        *mpp = ISF_NULL;
    }

    /* Char 47: must be a space */
    if (line[46] != ' ' ){
        sprintf(isf_error,"bad format, char 47: %s",line); 
        return 20;
    }

    /* Chars 48-53: radial-theta element, float if anything */
    if (partline(substr,line,47,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mrt: %s",line); 
            return 20;
        }
        *mrt = (float)atof(substr);
    }
    else {
        *mrt = ISF_NULL;
    }

    /* Char 54: must be a space */
    if (line[53] != ' ' ){
        sprintf(isf_error,"bad format, char 54: %s",line); 
        return 20;
    }

    /* Chars 55-60: theta-phi element, float if anything */
    if (partline(substr,line,54,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mtp: %s",line); 
            return 20;
        }
        *mtp = (float)atof(substr);
    }
    else {
        *mtp = ISF_NULL;
    }

    /* Char 61: must be a space */
    if (line[60] != ' ' ){
        sprintf(isf_error,"bad format, char 61: %s",line); 
        return 20;
    }

    /* Chars 62-67: phi-radial element, float if anything */
    if (partline(substr,line,61,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mpr: %s",line); 
            return 20;
        }
        *mpr = (float)atof(substr);
    }
    else {
        *mpr = ISF_NULL;
    }

    /* Char 68: must be a space */
    if (line[67] != ' ' ){
        sprintf(isf_error,"bad format, char 68: %s",line); 
        return 20;
    }

    /* Chars 69-72: nsta1, int if anything */
    if (partline(substr,line,68,4)){
        if (check_int(substr)){
            sprintf(isf_error,"bad nsta1: %s",line); 
            return 20;
        }
        *nsta1 = atoi(substr);
    }
    else {
        *nsta1 = ISF_NULL;
    }

    /* Char 73: must be a space */
    if (line[72] != ' ' ){
        sprintf(isf_error,"bad format, char 73: %s",line); 
        return 20;
    }

    /* Chars 74-77: nsta2, int if anything */
    if (partline(substr,line,73,4)){
        if (check_int(substr)){
            sprintf(isf_error,"bad nsta2: %s",line); 
            return 20;
        }
        *nsta2 = atoi(substr);
    }
    else {
        *nsta2 = ISF_NULL;
    }

    /* Char 78: must be a space */
    if (line[77] != ' ' ){
        sprintf(isf_error,"bad format, char 78: %s",line); 
        return 20;
    }

    /* Chars 79-87: author, any characters allowed but must be there. */
    if (!partline(author,line,78,9)){
        sprintf(isf_error,"missing author: %s",line); 
        return 20;
    }
    if (check_whole(author)){
        sprintf(isf_error,"bad author: %s",line); 
        return 20;
    }

    /* Check for extra characters - could be close bracket somewhere. */
    if (partline(substr,line,87,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}

/*  Parses a line asuming it to be a second moment tensor data comment.
    Values are asigned to variables, the pointers to which have been sent
    as arguments. If an optional parameter is not given then the
    corresponding variable will have ISF_NULL assigned to it.

    Returns 0 if the line is properly formatted second moment tensor data.
    Returns 20 and writes a diagnostic to isf_error on error.
*/
int read_momten_line_2(char *line, float *scalar_moment_unc, float *fclvd_unc,
                       float *mrr_unc, float *mtt_unc, float *mpp_unc,
                       float *mrt_unc, float *mtp_unc, float *mpr_unc,
                       int *ncomp1, int *ncomp2, float *duration)
{
    char substr[ISF_LINE_LEN];

    /* Chars 1-14: should be the string  ' (#           '. */
    if (strncmp(line," (#           ",14) != 0){
        sprintf(isf_error,"not a moment tensor line: %s",line); 
        return 20;
    }

    /* Chars 15-19: uncertainty in scalar seismic moment - float if there. */
    if (partline(substr,line,14,5)){
        if (check_float(substr)){
            sprintf(isf_error,"bad scalar_moment_unc: %s",line); 
            return 20;
        }
        *scalar_moment_unc = (float)atof(substr);
    }
    else {
        *scalar_moment_unc = ISF_NULL;
    }

    /* Char 20: must be a space. */
    if (line[19] != ' ' ){
        sprintf(isf_error,"bad format, char 20: %s",line); 
        return 20;
    }

    /* Chars 21-25: uncertainty in fCLVD, float if anything. */
    if (partline(substr,line,20,5)){
        if (check_float(substr)){
            sprintf(isf_error,"bad fclvd_unc: %s",line); 
            return 20;
        }
        *fclvd_unc = (float)atof(substr);
    }
    else {
        *fclvd_unc = ISF_NULL;
    }

    /* Char 26: must be a space. */
    if (line[25] != ' ' ){
        sprintf(isf_error,"bad format, char 26: %s",line); 
        return 20;
    }

    /* Chars 27-32: uncertainty in radial-radial element, float if anything. */
    if (partline(substr,line,26,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mrr_unc: %s",line); 
            return 20;
        }
        *mrr_unc = (float)atof(substr);
    }
    else {
        *mrr_unc = ISF_NULL;
    }

    /* Char 33: must be a space. */
    if (line[32] != ' ' ){
        sprintf(isf_error,"bad format, char 33: %s",line); 
        return 20;
    }

    /* Chars 34-39: uncertainty in theta-theta element, float if anything. */
    if (partline(substr,line,33,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mtt_unc: %s",line); 
            return 20;
        }
        *mtt_unc = (float)atof(substr);
    }
    else {
        *mtt_unc = ISF_NULL;
    }

    /* Char 40: must be a space */
    if (line[39] != ' ' ){
        sprintf(isf_error,"bad format, char 40: %s",line); 
        return 20;
    }

    /* Chars 41-46: uncertainty in phi-phi element, float if anything. */
    if (partline(substr,line,40,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mpp_unc: %s",line); 
            return 20;
        }
        *mpp_unc = (float)atof(substr);
    }
    else {
        *mpp_unc = ISF_NULL;
    }

    /* Char 47: must be a space */
    if (line[46] != ' ' ){
        sprintf(isf_error,"bad format, char 47: %s",line); 
        return 20;
    }

    /* Chars 48-53: uncertainty in radial-theta element, float if anything. */
    if (partline(substr,line,47,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mrt_unc: %s",line); 
            return 20;
        }
        *mrt_unc = (float)atof(substr);
    }
    else {
        *mrt_unc = ISF_NULL;
    }

    /* Char 54: must be a space. */
    if (line[53] != ' ' ){
        sprintf(isf_error,"bad format: %s",line); 
        return 20;
    }

    /* Chars 55-60: uncertainty in theta-phi element, float if anything. */
    if (partline(substr,line,54,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mtp_unc: %s",line); 
            return 20;
        }
        *mtp_unc = (float)atof(substr);
    }
    else {
        *mtp_unc = ISF_NULL;
    }

    /* Char 61: must be a space. */
    if (line[60] != ' ' ){
        sprintf(isf_error,"bad format, char 61: %s",line); 
        return 20;
    }

    /* Chars 62-67: uncertainty in phi-radial element, float if anything. */
    if (partline(substr,line,61,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad mpr_unc: %s",line); 
            return 20;
        }
        *mpr_unc = (float)atof(substr);
    }
    else {
        *mpr_unc = ISF_NULL;
    }

    /* Char 68: must be a space. */
    if (line[67] != ' ' ){
        sprintf(isf_error,"bad format, char 68: %s",line); 
        return 20;
    }

    /* Chars 69-72: ncomp1, int if anything. */
    if (partline(substr,line,68,4)){
        if (check_int(substr)){
            sprintf(isf_error,"bad ncomp1: %s",line); 
            return 20;
        }
        *ncomp1 = atoi(substr);
    }
    else {
        *ncomp1 = ISF_NULL;
    }

    /* Char 73: must be a space */
    if (line[72] != ' ' ){
        sprintf(isf_error,"bad format, char 73: %s",line); 
        return 20;
    }

    /* Chars 74-77: ncomp2, int if anything. */
    if (partline(substr,line,73,4)){
        if (check_int(substr)){
            sprintf(isf_error,"bad ncomp2: %s",line); 
            return 20;
        }
        *ncomp2 = atoi(substr);
    }
    else {
        *ncomp2 = ISF_NULL;
    }

    /* Char 78: must be a space. */
    if (line[77] != ' ' ){
        sprintf(isf_error,"bad format, char 78: %s",line); 
        return 20;
    }

    /* Chars 79-86: duration, float if anything. */
    if (partline(substr,line,78,8)){
        if (check_float(substr)){
            sprintf(isf_error,"bad duration: %s",line); 
            return 20;
        }
        *duration = (float)atof(substr);
    }
    else {
        *duration = ISF_NULL;
    }

    /* Check for extra stuff - not including close bracket. */
    if (partline(substr,line,86,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}

/*    Tests a line to discover if it is a fault plane header comment.

    Returns 0 if the line is a fault plane header.
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_fault_plane_head(char *line)
{
    char substr[ISF_LINE_LEN];
    char head[] = " (#FAULT_PLANE Typ Strike   Dip    Rake  NP  NS Plane Author   )";
    int headlen = 64;

    if (strncmp(line,head,headlen) != 0){
        sprintf(isf_error,"not a fault plane header: %s",line); 
        return 20;
    }
    if (partline(substr,line,headlen,0)){
       sprintf(isf_error,"extra characters at end: %s",line); 
       return 20;
    } 
    return 0;
}

/*  Parses a line asuming it to be a fault plane data comment.
    Could be first or second plane, the only difference is whether
    author field is expected or not.
    Values are asigned to variables, the pointers to which have been sent
    as arguments. If an optional parameter is not given then the
    corresponding variable will have ISF_NULL assigned to it.

    Returns 0 if the line is a properly formatted fault plane data line.
    Returns 20 and writes a diagnostic to isf_error on error.
*/
int read_fault_plane (char *line, char *f_type, float *strike, float *dip,
                       float *rake, int *np, int *ns, char *f_plane, char *author)
{
    char substr[ISF_LINE_LEN];
    int line_num;

    /* Chars 1-3: the strings  ' (#' or ' (+', */
    /* depending on whether this is the first or second plane given. */
    /* Chars 4-15: spaces. */
    if (strncmp(line," (#            ",15) == 0){
        line_num = 1;
    }
    else if (strncmp(line," (+            ",15) == 0){
        line_num = 2;
    }
    else {
        sprintf(isf_error,"not a fault plane line: %s",line); 
        return 20;
    }

    /* Chars 16-18: fault plane solution type. */
    if (partline(f_type,line,15,3)){
        if (strcmp(f_type,"FM") && strcmp(f_type,"BB") && strcmp(f_type,"BDC")){
            sprintf(isf_error,"bad f_type: %s",line); 
            return 20;
        }
    }
    else {
        strcpy(f_type,"");
    }

    /* Char 19: must be a space */
    if (line[18] != ' ' ){
        sprintf(isf_error,"bad format, char 19: %s",line); 
        return 20;
    }

    /* Chars 20-25: strike, must be float. */
    if (!partline(substr,line,19,6)){
        sprintf(isf_error,"missing strike: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad strike: %s",line); 
        return 20;
    }
    *strike = (float)atof(substr);

    /* Char 26: must be a space. */
    if (line[25] != ' ' ){
        sprintf(isf_error,"bad format, char 26: %s",line); 
        return 20;
    }

    /* Chars 27-31: dip, must be float. */
    if (!partline(substr,line,26,5)){
        sprintf(isf_error,"missing dip: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad dip: %s",line); 
        return 20;
    }
    *dip = (float)atof(substr);

    /* Char 32: must be a space. */
    if (line[31] != ' ' ){
        sprintf(isf_error,"bad format, char 32: %s",line); 
        return 20;
    }

    /* Chars 33-39: rake, float - need not be there if both planes given. */
    if (partline(substr,line,32,7)){
        if (check_float(substr)){
            sprintf(isf_error,"bad rake: %s",line); 
            return 20;
        }
        *rake = (float)atof(substr);
    }
    else {
        *rake = ISF_NULL;
    }

    /* Char 40: must be a space. */
    if (line[39] != ' ' ){
        sprintf(isf_error,"bad format, char 40: %s",line); 
        return 20;
    }

    /* Chars 41-43: np, int if there. */
    if (partline(substr,line,40,3)){
        if (check_int(substr)){
            sprintf(isf_error,"bad np: %s",line); 
            return 20;
        }
        *np = atoi(substr);
    }
    else {
        *np = ISF_NULL;
    }

    /* Char 44: must be a space. */
    if (line[43] != ' ' ){
        sprintf(isf_error,"bad format, char 44: %s",line); 
        return 20;
    }

    /* Chars 45-47: ns, int if there. */
    if (partline(substr,line,44,3)){
        if (check_int(substr)){
            sprintf(isf_error,"bad ns: %s",line); 
            return 20;
        }
        *ns = atoi(substr);
    }
    else {
        *ns = ISF_NULL;
    }

    /* Char 48: must be a space. */
    if (line[47] != ' ' ){
        sprintf(isf_error,"bad format, char 48: %s",line); 
        return 20;
    }

    /* Chars 49-53: plane identification. */
    if (partline(f_plane,line,48,5)){
        if (strcmp(f_plane,"FAULT") != 0 && strcmp(f_plane,"AUXIL") != 0){
        sprintf(isf_error,"bad f_plane: %s",line); 
            return 20;
        }
    }
    else {
        strcpy(f_plane,"");
    }

    /* Chars 54-63: First plane has author, don't read for second plane. */
    if (line_num==1){

        /* Char 54: must be a space */
        if (line[53] != ' ' ){
            sprintf(isf_error,"bad format, char 54: %s",line); 
            return 20;
        }

        /* Chars 55-63: author, any characters allowed but must be there */
        if (!partline(author,line,54,9)){
            sprintf(isf_error,"missing author: %s",line); 
            return 20;
        }
        if (check_whole(author)){
            sprintf(isf_error,"bad author: %s",line); 
            return 20;
        }
    }

    /* Check for extra stuff - not including close bracket */
    if (partline(substr,line,63,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}

/*  Tests a line to discover if it is a principal axes header comment.

    Returns 0 if the line is a principal axes header.
    Returns 20 and writes a diagnostic to isf_error otherwise.
*/
int read_axes_head(char *line) {
    char substr[ISF_LINE_LEN];
    char head[] = " (#PRINAX sc  T_val T_azim  T_pl  B_val B_azim  B_pl  P_val P_azim  P_pl Author   )";
    int headlen = 83;

    if (strncmp(line,head,headlen) != 0){
        sprintf(isf_error,"not an axes header: %s",line); 
        return 20;
    }
    if (partline(substr,line,headlen,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    } 
    return 0;
}

/*  Parses a line asuming it to be a principal axes data comment.
    Values are asigned to variables, the pointers to which have been sent
    as arguments. If an optional parameter is not given then the
    corresponding variable will have ISF_NULL assigned to it.

    Returns 0 if the line is a properly formatted principal axes data line.
    Returns 20 and writes a diagnostic to isf_error on error.
*/
int read_axes(char *line, int *scale_factor, float *t_val, float *t_azim,
              float *t_pl, float *b_val, float *b_azim, float *b_pl,
              float *p_val, float *p_azim, float *p_pl, char *author)
{

    char substr[ISF_LINE_LEN];

    /* Chars 1-10: should be the string  ' (#       '. */
    if (strncmp(line," (#       ",10) != 0){
        sprintf(isf_error,"not an axes line: %s",line); 
        return 20;
    }

    /* Chars 11,12: scale factor - int if there. */
    if (partline(substr,line,10,2)){
        if (check_int(substr)){
            sprintf(isf_error,"bad scale factor: %s",line); 
            return 20;
        }
        *scale_factor = atoi(substr);
    }
    else {
        *scale_factor = ISF_NULL;
    }

    /* Char 13: must be a space. */
    if (line[12] != ' ' ){
        sprintf(isf_error,"bad format, char 13: %s",line); 
        return 20;
    }

    /* Chars 14-19: t value - float if there. */
    if (partline(substr,line,13,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad t_val: %s",line); 
            return 20;
        }
        *t_val = (float)atof(substr);
    }
    else {
        *t_val = ISF_NULL;
    }

    /* Char 20: must be a space. */
    if (line[19] != ' ' ){
        sprintf(isf_error,"bad format, char 20: %s",line); 
        return 20;
    }

    /* Chars 21-26: t azim, must be float. */
    if (!partline(substr,line,20,6)){
        sprintf(isf_error,"missing t_azim: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad t_azim: %s",line); 
        return 20;
    }
    *t_azim = (float)atof(substr);

    /* Char 27: must be a space. */
    if (line[26] != ' ' ){
        sprintf(isf_error,"bad format, char 27: %s",line); 
        return 20;
    }

    /* Chars 28-32: t plunge, must be float. */
    if (!partline(substr,line,27,5)){
        sprintf(isf_error,"missing t_pl: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad t_pl: %s",line); 
        return 20;
    }
    *t_pl = (float)atof(substr);

    /* Char 33: must be a space. */
    if (line[32] != ' ' ){
        sprintf(isf_error,"bad format, char 33: %s",line); 
        return 20;
    }

    /* Chars 34-39: b value - float if there. */
    if (partline(substr,line,33,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad b_val: %s",line); 
            return 20;
        }
        *b_val = (float)atof(substr);
    }
    else {
        *b_val = ISF_NULL;
    }

    /* Char 40: must be a space. */
    if (line[39] != ' ' ){
        sprintf(isf_error,"bad format, char 40: %s",line); 
        return 20;
    }

    /* Chars 41-46: b azim, must be float. */
    if (!partline(substr,line,40,6)){
        sprintf(isf_error,"missing b_azim: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad b_azim: %s",line); 
        return 20;
    }
    *b_azim = (float)atof(substr);

    /* Char 47: must be a space. */
    if (line[46] != ' ' ){
        sprintf(isf_error,"bad format, char 47: %s",line); 
        return 20;
    }

    /* Chars 48-52: b plunge, must be float. */
    if (!partline(substr,line,47,5)){
        sprintf(isf_error,"missing b_pl: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad b_pl: %s",line); 
        return 20;
    }
    *b_pl = (float)atof(substr);


    /* Char 53: must be a space. */
    if (line[52] != ' ' ){
        sprintf(isf_error,"bad format, char 53: %s",line); 
        return 20;
    }

    /* Chars 54-59: p value - float if there. */
    if (partline(substr,line,53,6)){
        if (check_float(substr)){
            sprintf(isf_error,"bad p_val: %s",line); 
            return 20;
        }
        *p_val = (float)atof(substr);
    }
    else {
        *p_val = ISF_NULL;
    }

    /* Char 60: must be a space. */
    if (line[59] != ' ' ){
        sprintf(isf_error,"bad format, char 60: %s",line); 
        return 20;
    }

    /* Chars 61-66: p azim, must be float. */
    if (!partline(substr,line,60,6)){
        sprintf(isf_error,"missing p_azim: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad p_azim: %s",line); 
        return 20;
    }
    *p_azim = (float)atof(substr);

    /* Char 67: must be a space. */
    if (line[66] != ' ' ){
        sprintf(isf_error,"bad format, char 67: %s",line); 
        return 20;
    }

    /* Chars 68-72: p plunge, must be float */
    if (!partline(substr,line,67,5)){
        sprintf(isf_error,"missing p_pl: %s",line); 
        return 20;
    }
    if (check_float(substr)){
        sprintf(isf_error,"bad p_pl: %s",line); 
        return 20;
    }
    *p_pl = (float)atof(substr);

    /* Char 73: must be a space. */
    if (line[72] != ' ' ){
        sprintf(isf_error,"bad format, char 73: %s",line); 
        return 20;
    }

    /* Chars 74-82: author, any characters allowed but must be there. */
    if (!partline(author,line,73,9)){
        sprintf(isf_error,"missing author: %s",line); 
        return 20;
    }
    if (check_whole(author)){
        sprintf(isf_error,"bad author: %s",line); 
        return 20;
    }

    /* Check for extra stuff - not including close bracket. */
    if (partline(substr,line,82,0)){
        sprintf(isf_error,"extra characters at end: %s",line); 
        return 20;
    }
    return 0;
}

/* Get a substring, removing leading and trailing white space.
   Expects a string, an offset from the start of the string, and a maximum 
   length for the resulting substring. If this length is 0 it will take up
   to the end of the input string.

   Need to allow for ')' to come after the required field at the end of a 
   comment.  Discard ')' at end of a string  as long as there's no '(' 
   before it anywhere.

   Returns the length of the resulting substring.
*/
int partline(char *part, char *line, int offset, int numchars) {
    int i,j,len;
    int bracket=0;

    len = strlen(line);
    if (len < offset) return 0;

    if (numchars == 0) numchars = len-offset;

    for (i=offset, j=0; i < offset+numchars; i++){
        if ( j == 0 && (line[i] == ' ' || line[i] == '\t')) continue;
        if ( line[i] == '\0' || line[i] == '\n') break;
        part[j++] = line[i];
        if ( line[i] == '(' ) bracket=1;
    }
    if (!bracket){
        while (--j != -1 && (part[j]==' ' || part[j]=='\t' || part[j]==')' ));
        part[++j]='\0';
    }
    else if (j){
        while (part[--j] == ' ' || part[j] == '\t');
        part[++j]='\0';
    }
    return j;
}

/*  To check that a string has no spaces in it.
    Returns 0 if there are no spaces or 1 if there is a space.
*/
int check_whole(char *substr) {
    int i;
    for (i=0; i < strlen(substr); i++){
        if (substr[i] == ' ' || substr[i] == '\t') return 1;
    }
    return 0;
}

/*  To check that a string contains only sign/number characters and so 
    is suitable for atoi - atoi itself does no checking.
    Returns 0 if OK,  1 if not.
*/
int check_int(char *substr) {
    int i;
    for (i=0; i < strlen(substr); i++){
        if (isdigit(substr[i])) continue;
        if (i==0)
            if (substr[i] == '+' || substr[i] == '-') continue;
        return 1;
    }
    return 0;
}


/*  To check that a string contains only sign/number/point characters and so 
    is suitable for atof - atof itself does no checking.
    Returns 0 if OK,  1 if not.
*/
int check_float(char *substr) {
    int i;
    for (i=0; i < strlen(substr); i++){
        if (isdigit(substr[i]) || substr[i] == '.') continue;
        if (i==0)
            if (substr[i] == '+' || substr[i] == '-') continue;
        return 1;
    }
    return 0;
}

