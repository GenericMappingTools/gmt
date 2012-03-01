/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * lex(1) specification for tokens for the Unidata units package, UDUNITS2.
 */

%option noyywrap

%{

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

%}

space			[ \t\r\f\v]
int			[0-9]+
int_period		{int}\.
period_int		\.{int}
int_period_int		{int}\.{int}
mantissa		{int_period}|{period_int}|{int_period_int}
real_exp		[eE][+-]?[0-9]+
real			[+-]?({int}{real_exp}|{mantissa}{real_exp}?)
year			[+-]?[0-9]{1,4}
month			0?[1-9]|1[0-2]
day			0?[1-9]|[1-2][0-9]|30|31
hour			[+-]?[0-1]?[0-9]|2[0-3]
minute			[0-5]?[0-9]
second			({minute}|60)(\.[0-9]*)?
middot                  \xc2\xb7
utf8_exp_digit	        \xc2(\xb9|\xb2|\xb3)|\xe2\x81(\xb0|[\xb4-\xb9])
utf8_exp_sign		\xe2\x81\xba|\xe2\x81\xbb
utf8_exponent		{utf8_exp_sign}?{utf8_exp_digit}+
nbsp                    \xc2\xa0
shy                     \xc2\xad
degree                  \xc2\xb0
mu                      \xc2\xb5
blk1                    \xc3([\x80-\x96])
blk2                    \xc3([\x98-\xB6])
blk3                    \xc3([\xB8-\xBF])
latin1		        {nbsp}|{shy}|{degree}|{mu}|{blk1}|{blk2}|{blk3}
utf8_cont               [\x80-\xbf]
utf8_2bytes             [\xc8-\xdf]{utf8_cont}
utf8_3bytes             [\xe0-\xef]{utf8_cont}{utf8_cont}
letter  		[_a-zA-Z]|{latin1}|{utf8_2bytes}|{utf8_3bytes}
alphanum		{letter}|[0-9]
id			%|'|\"|{letter}({alphanum}*{letter})?
broken_date		{year}-{month}(-{day})?
packed_date		{year}({month}{day}?)?
broken_clock		{hour}:{minute}(:{second})?
packed_clock		{hour}({minute}{second}?)?
broken_timestamp	{broken_date}({space}+{broken_clock})?
packed_timestamp	{packed_date}T{packed_clock}?
logref			\({space}*[Rr][Ee](:{space})?{space}*
after                   [Aa][Ff][Tt][Ee][Rr]
from                    [Ff][Rr][Oo][Mm]
since                   [Ss][Ii][Nn][Cc][Ee]
ref                     [Rr][Ee][Ff]
per                     [Pp][Ee][Rr]

%Start		id_seen

%%
    if (_restartScanner) {
	BEGIN(INITIAL);
	_restartScanner = 0;
    }

{space}*(@|{after}|{from}|{since}|{ref}){space}* {
    BEGIN(INITIAL);
    return SHIFT;
}

{space}*({per}|"/"){space}* {
    BEGIN(INITIAL);
    return DIVIDE;
}

"-"|"."|"*"|{middot}|{space}+ {
    BEGIN(INITIAL);
    return MULTIPLY;
}

("^"|"**")[+-]?{int} {
    int		status;

    if (sscanf(yytext, "%*[*^]%ld", &yylval.ival) != 1) {
        ut_handle_error_message("Invalid integer\n", stderr);

	status	= ERR;
    }
    else {
	status	= EXPONENT;
    }

    return status;
}

{utf8_exponent} {
    int		status = EXPONENT;
    int		exponent = 0;
    int		sign = 1;
    char*	cp = yytext;

    if (strncmp(cp, "\xe2\x81\xba", 3) == 0) {
	cp += 3;
    }
    else if (strncmp(cp, "\xe2\x81\xbb", 3) == 0) {
	sign = -1;
	cp += 3;
    }

    while (cp < yytext + yyleng) {
	int	j;
	static struct {
	    const char*	string;
	    const int	len;
	} utf8_exponents[] = {
	    {"\xe2\x81\xb0", 3},        /* 0 */
	    {"\xc2\xb9",     2},        /* 1 */
	    {"\xc2\xb2",     2},        /* 2 */
	    {"\xc2\xb3",     2},        /* 3 */
	    {"\xe2\x81\xb4", 3},        /* 4 */
	    {"\xe2\x81\xb5", 3},        /* 5 */
	    {"\xe2\x81\xb6", 3},        /* 6 */
	    {"\xe2\x81\xb7", 3},        /* 7 */
	    {"\xe2\x81\xb8", 3},        /* 8 */
	    {"\xe2\x81\xb9", 3},        /* 9 */
	};

	exponent *= 10;

	for (j = 0; j < 10; j++) {
	    int	len = utf8_exponents[j].len;

	    if (strncmp(cp, utf8_exponents[j].string, len) == 0) {
		exponent += j;
		cp += len;
		break;
	    }
	}

	if (j >= 10) {
	    status = ERR;
	    break;
	}
    }

    if (status == EXPONENT)
	yylval.ival = sign * exponent;

    BEGIN(INITIAL);
    return status;
}

{broken_date}(T|{space}*) {
    int		year;
    int		month;
    int		day = 1;

    (void) sscanf((char*)yytext, "%d-%d-%d", &year, &month, &day);

    yylval.rval	= ut_encode_date(year, month, day);

    BEGIN(INITIAL);
    return DATE;
}

{broken_clock}{space}* {
    int		hour;
    int		minute;
    double	second	= 0.0;

    (void) sscanf((char*)yytext, "%d:%d:%lf", &hour, &minute, &second);

    yylval.rval	= ut_encode_clock(hour, minute, second);

    BEGIN(INITIAL);
    return CLOCK;
}

{packed_timestamp}{space}* {
    int		year;
    int		month = 1;
    int		day = 1;
    int		hour = 0;
    int		minute = 0;
    double	second	= 0.0;

    (void) sscanf((char*)yytext, "%4d%2d%2dT%2d%2d%lf", &year, &month,
        &day, &hour, &minute, &second);

    yylval.rval	= ut_encode_time(year, month, day, hour, minute, second);

    BEGIN(INITIAL);
    return TIMESTAMP;
}

<INITIAL>{real} {
    int		status;

    errno	= 0;
    yylval.rval = atof((char*)yytext);

    if (errno == 0) {
	status	= REAL;
    } else {
        ut_handle_error_message("Invalid real\n", stderr);

	status	= ERR;
    }

    return status;
}

[+-]?{int} {
    int		status;

    errno	= 0;
    yylval.ival = atol((char*)yytext);

    if (errno == 0) {
	status	= INT;
    } else {
        ut_handle_error_message("Invalid integer\n", stderr);

	status	= ERR;
    }

    return status;
}

(log|lg){space}*{logref} {
    yylval.rval = 10;
    return LOGREF;
}

ln{space}*{logref} {
    yylval.rval = M_E;
    return LOGREF;
}

lb{space}*{logref} {
    yylval.rval = 2;
    return LOGREF;
}

{id} {
    yylval.id = strdup((char*)yytext);

    BEGIN(id_seen);
    return ID;
}

. {
    BEGIN(INITIAL);
    return yytext[0];
}

%%
