/*	$Id$ */

/*! . */
int def_us_local(struct GMT_CTRL *GMT) {

	/* GMT Time language file for US (english) mode [US] */

	/* Month record */ 
	strcpy (GMT->current.language.month_name[0][0], "January"); strcpy (GMT->current.language.month_name[1][0], "Jan");
	strcpy (GMT->current.language.month_name[2][0], "J");       strcpy (GMT->current.language.month_name[3][0], "JAN");
	strcpy (GMT->current.language.month_name[0][1], "February");strcpy (GMT->current.language.month_name[1][1], "Feb");
	strcpy (GMT->current.language.month_name[2][1], "F");       strcpy (GMT->current.language.month_name[3][1], "FEB");
	strcpy (GMT->current.language.month_name[0][2], "March");   strcpy (GMT->current.language.month_name[1][2], "Mar");
	strcpy (GMT->current.language.month_name[2][2], "M");       strcpy (GMT->current.language.month_name[3][2], "MAR");
	strcpy (GMT->current.language.month_name[0][3], "April");   strcpy (GMT->current.language.month_name[1][3], "Apr");
	strcpy (GMT->current.language.month_name[2][3], "A");       strcpy (GMT->current.language.month_name[3][3], "APR");
	strcpy (GMT->current.language.month_name[0][4], "May");     strcpy (GMT->current.language.month_name[1][4], "May");
	strcpy (GMT->current.language.month_name[2][4], "M");       strcpy (GMT->current.language.month_name[3][4], "MAY");
	strcpy (GMT->current.language.month_name[0][5], "June");    strcpy (GMT->current.language.month_name[1][5], "Jun");
	strcpy (GMT->current.language.month_name[2][5], "J");       strcpy (GMT->current.language.month_name[3][5], "JUN");
	strcpy (GMT->current.language.month_name[0][6], "July");    strcpy (GMT->current.language.month_name[1][6], "Jul");
	strcpy (GMT->current.language.month_name[2][6], "J");       strcpy (GMT->current.language.month_name[3][6], "JUL");
	strcpy (GMT->current.language.month_name[0][7], "August");  strcpy (GMT->current.language.month_name[1][7], "Aug");
	strcpy (GMT->current.language.month_name[2][7], "A");       strcpy (GMT->current.language.month_name[3][7], "AUG");
	strcpy (GMT->current.language.month_name[0][8], "September");strcpy(GMT->current.language.month_name[1][8], "Sep");
	strcpy (GMT->current.language.month_name[2][8], "S");       strcpy (GMT->current.language.month_name[3][8], "SEP");
	strcpy (GMT->current.language.month_name[0][9], "October"); strcpy (GMT->current.language.month_name[1][9], "Oct");
	strcpy (GMT->current.language.month_name[2][9], "O");       strcpy (GMT->current.language.month_name[3][9], "OCT");
	strcpy (GMT->current.language.month_name[0][10],"November");strcpy (GMT->current.language.month_name[1][10],"Nov");
	strcpy (GMT->current.language.month_name[2][10],"N");       strcpy (GMT->current.language.month_name[3][10],"NOV");
	strcpy (GMT->current.language.month_name[0][11],"December");strcpy (GMT->current.language.month_name[1][11],"Dec");
	strcpy (GMT->current.language.month_name[2][11],"D");       strcpy (GMT->current.language.month_name[3][11],"DEC");

	/* Week name record */ 
	strcpy (GMT->current.language.week_name[0], "Week");        strcpy (GMT->current.language.week_name[1], "Wk");
	strcpy (GMT->current.language.week_name[2], "W");

	/* Weekday record */ 
	strcpy (GMT->current.language.day_name[0][0], "Sunday");   strcpy (GMT->current.language.day_name[1][0], "Sun");
	strcpy (GMT->current.language.day_name[2][0], "S");
	strcpy (GMT->current.language.day_name[0][1], "Monday");   strcpy (GMT->current.language.day_name[1][1], "Mon");
	strcpy (GMT->current.language.day_name[2][1], "M");
	strcpy (GMT->current.language.day_name[0][2], "Tuesday");  strcpy (GMT->current.language.day_name[1][2], "Tue");
	strcpy (GMT->current.language.day_name[2][2], "T");
	strcpy (GMT->current.language.day_name[0][3], "Wednesday");strcpy (GMT->current.language.day_name[1][3], "Wed");
	strcpy (GMT->current.language.day_name[2][3], "W");
	strcpy (GMT->current.language.day_name[0][4], "Thursday"); strcpy (GMT->current.language.day_name[1][4], "Thu");
	strcpy (GMT->current.language.day_name[2][4], "T");
	strcpy (GMT->current.language.day_name[0][5], "Friday");   strcpy (GMT->current.language.day_name[1][5], "Fri");
	strcpy (GMT->current.language.day_name[2][5], "F");
	strcpy (GMT->current.language.day_name[0][6], "Saturday"); strcpy (GMT->current.language.day_name[1][6], "Sat");
	strcpy (GMT->current.language.day_name[2][6], "S");

	/* Compass name record */
	strcpy (GMT->current.language.cardinal_name[0][0], "West"); strcpy (GMT->current.language.cardinal_name[1][0], "W");
	strcpy (GMT->current.language.cardinal_name[2][0], "W");
	strcpy (GMT->current.language.cardinal_name[0][1], "East"); strcpy (GMT->current.language.cardinal_name[1][1], "E");
	strcpy (GMT->current.language.cardinal_name[2][1], "E");
	strcpy (GMT->current.language.cardinal_name[0][2], "South"); strcpy (GMT->current.language.cardinal_name[1][2], "S");
	strcpy (GMT->current.language.cardinal_name[2][2], "S");
	strcpy (GMT->current.language.cardinal_name[0][3], "North"); strcpy (GMT->current.language.cardinal_name[1][3], "N");
	strcpy (GMT->current.language.cardinal_name[2][3], "N");

	return 0;
}
