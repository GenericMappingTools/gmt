/* -------------------------------------------------------------------
 *	$Id: mgd77snifferdefaults.h,v 1.2 2006-02-21 02:14:24 mtchandl Exp $	
 *
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
 * Abbrev,     min,     max,  delta,  maxdt,  maxds,  maxdz,  maxArea*/
{   "drt",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{    "tz",     -13,      12,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{  "year",    1939,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{ "month",       1,      12,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "day",       1,      31,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{  "hour",       0,      24,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "min",       0,      60,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "lat",     -90,      90,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "lon",    -180,     180,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "ptc",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "twt",       0,      15,     .1,  .0013,      1,    .15,    FALSE},
{ "depth",       0,   11000,     50,      1,   1000,    104,     4500},
{   "bcc",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "btc",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{  "mtf1",   19000,   72000,    200,    .32,    200,     40,    FALSE},
{  "mtf2",   19000,   72000,    200,    .32,    200,     40,    FALSE},
{   "mag",    -800,     800,     10,    .29,    200,     38,    FALSE},
{ "msens",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{  "diur",    -100,     100,      2,    .05,     20,      3,    FALSE},
{   "msd",   -1000,   10000,     10,    .33,   1000,     10,    FALSE},
{  "gobs",  975900,  986000,      5,   .043,    100,    3.6,    FALSE},
{   "eot",    -150,     150,      2,   .033,    100,    2.4,    FALSE},
{   "faa",    -250,     250,      5,   .045,    100,    4.8,     2500},
{   "nqc",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{    "id",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{   "sln",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE},
{  "sspn",   FALSE,   FALSE,  FALSE,  FALSE,  FALSE,  FALSE,    FALSE}
