/* -------------------------------------------------------------------
 *	$Id$	
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *    Copyright (c) 2004-2015 by P. Wessel and M. T. Chandler
 *	File:	mgd77snifferdefaults.h
 *
 *	Include file for mgd77sniffer
 *
 *	Authors:
 *		Michael Chandler and Paul Wessel
 *		School of Ocean and Earth Science and Technology
 *		University of Hawaii
 * 
 *	Date:	23-Feb-2004
 * 
 * ------------------------------------------------------------------
 * Abbrev,     min,     max,  binwidth,  maxdt,  maxds,  maxdz,  binmin, binmax, maxArea*/
{   "drt",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{    "tz",     -13,      12,     false,  false,  false,  false,   false,  false,  false},
{  "year",    1939,   false,     false,  false,  false,  false,   false,  false,  false},
{ "month",       1,      12,     false,  false,  false,  false,   false,  false,  false},
{   "day",       1,      31,     false,  false,  false,  false,   false,  false,  false},
{  "hour",       0,      24,     false,  false,  false,  false,   false,  false,  false},
{   "min",       0,      60,     false,  false,  false,  false,   false,  false,  false},
{   "lat",     -90,      90,     false,  false,  false,  false,   false,  false,  false},
{   "lon",    -180,     180,     false,  false,  false,  false,   false,  false,  false},
{   "ptc",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{   "twt",       0,      15,        .1,  .0013,      1,    .15,   false,  false,  false},
{ "depth",       0,   11000,        50,      1,   1000,    104, -100000, 100000, 100000},
{   "bcc",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{   "btc",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{  "mtf1",   19000,   72000,       200,    .32,    200,     40,   false,  false,  false},
{  "mtf2",   19000,   72000,       200,    .32,    200,     40,   false,  false,  false},
{   "mag",   -1500,    1500,        10,    .29,    200,     38,  -10000,  10000,  false},
{ "msens",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{  "diur",    -100,     100,         2,    .05,     20,      3,   false,  false,  false},
{   "msd",   -1000,    1100,        10,    .33,    100,     10,   false,  false,  false},
{  "gobs",  977600,  983800,         5,   .043,    100,    3.6,   false,  false,  false},
{   "eot",    -150,     150,         2,   .033,    100,    2.4,   false,  false,  false},
{   "faa",    -400,     550,         5,   .045,    100,    4.8,   -1000,   1000,  50000},
{   "nqc",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{    "id",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{   "sln",   false,   false,     false,  false,  false,  false,   false,  false,  false},
{  "sspn",   false,   false,     false,  false,  false,  false,   false,  false,  false}
