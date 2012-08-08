/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         utparse
#define yylex           utlex
#define yyerror         uterror
#define yylval          utlval
#define yychar          utchar
#define yydebug         utdebug
#define yynerrs         utnerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "parser.y"

/*
 * Copyright 2008, 2009 University Corporation for Atmospheric Research
 *
 * This file is part of the UDUNITS-2 package.  See the file LICENSE
 * in the top-level source-directory of the package for copying and
 * redistribution conditions.
 */
/*
 * bison(1)-based parser for decoding formatted unit specifications.
 *
 * This module is thread-compatible but not thread-safe.  Multi-threaded
 * access must be externally synchronized.
 */

/*LINTLIBRARY*/

#ifndef	_XOPEN_SOURCE
#   define _XOPEN_SOURCE 500
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "udunits2.h"

static ut_unit*		_finalUnit;	/* fully-parsed specification */
static ut_system*	_unitSystem;	/* The unit-system to use */
static char*		_errorMessage;	/* last error-message */
static ut_encoding	_encoding;	/* encoding of string to be parsed */
static int		_restartScanner;/* restart scanner? */


/*
 * Removes leading and trailing whitespace from a string.
 *
 * Arguments:
 *	string		NUL-terminated string.  Will be modified if it
 *                      contains whitespace.
 *	encoding	The character-encoding of "string".
 * Returns:
 *      "string"
 */
char*
ut_trim(
    char* const	        string,
    const ut_encoding	encoding)
{
    static const char*	asciiSpace = " \t\n\r\f\v";
    static const char*	latin1Space = " \t\n\r\f\v\xa0";	/* add NBSP */
    const char*		whiteSpace;
    char*		start;
    char*		stop;
    size_t              len;

    whiteSpace =
	encoding == UT_LATIN1
	    ? latin1Space
	    : asciiSpace;

    start = string + strspn(string, whiteSpace);

    for (stop = start + strlen(start); stop > start; --stop)
	 if (strchr(whiteSpace, stop[-1]) == NULL)
	    break;

    len = stop - start;

    (void)memmove(string, start, len);

    string[len] = 0;

    ut_set_status(UT_SUCCESS);

    return start;
}


/*
 *  YACC error routine:
 */
void
uterror(
    char        	*s)
{
    static char*	nomem = "uterror(): out of memory";

    if (_errorMessage != NULL && _errorMessage != nomem)
	free(_errorMessage);

    _errorMessage = strdup(s);

    if (_errorMessage == NULL)
	_errorMessage = nomem;
}


/* Line 189 of yacc.c  */
#line 183 "parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ERR = 258,
     SHIFT = 259,
     MULTIPLY = 260,
     DIVIDE = 261,
     INT = 262,
     EXPONENT = 263,
     REAL = 264,
     ID = 265,
     DATE = 266,
     CLOCK = 267,
     TIMESTAMP = 268,
     LOGREF = 269
   };
#endif
/* Tokens.  */
#define ERR 258
#define SHIFT 259
#define MULTIPLY 260
#define DIVIDE 261
#define INT 262
#define EXPONENT 263
#define REAL 264
#define ID 265
#define DATE 266
#define CLOCK 267
#define TIMESTAMP 268
#define LOGREF 269




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 103 "parser.y"

    char*	id;			/* identifier */
    ut_unit*	unit;			/* "unit" structure */
    double	rval;			/* floating-point numerical value */
    long	ival;			/* integer numerical value */



/* Line 214 of yacc.c  */
#line 256 "parser.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 268 "parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  15
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   95

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  17
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  8
/* YYNRULES -- Number of rules.  */
#define YYNRULES  37
/* YYNRULES -- Number of states.  */
#define YYNSTATES  45

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   269

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      15,    16,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    10,    14,    18,    22,
      26,    28,    31,    34,    38,    42,    46,    50,    52,    55,
      58,    61,    63,    67,    71,    75,    79,    81,    83,    85,
      87,    90,    94,    98,   102,   104,   107,   110
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      18,     0,    -1,    -1,    19,    -1,     1,    -1,    20,    -1,
      20,     4,     9,    -1,    20,     4,     7,    -1,    20,     4,
      24,    -1,    20,     4,     1,    -1,    21,    -1,    20,    21,
      -1,    20,     1,    -1,    20,     5,    21,    -1,    20,     5,
       1,    -1,    20,     6,    21,    -1,    20,     6,     1,    -1,
      22,    -1,    22,     7,    -1,    22,     8,    -1,    22,     1,
      -1,    10,    -1,    15,    19,    16,    -1,    15,    19,     1,
      -1,    14,    20,    16,    -1,    14,    20,     1,    -1,    23,
      -1,     7,    -1,     9,    -1,    11,    -1,    11,    12,    -1,
      11,    12,    12,    -1,    11,    12,     7,    -1,    11,    12,
      10,    -1,    13,    -1,    13,    12,    -1,    13,     7,    -1,
      13,    10,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   133,   133,   137,   141,   146,   149,   155,   161,   167,
     175,   178,   185,   191,   198,   204,   211,   219,   222,   228,
     234,   242,   295,   298,   304,   310,   316,   322,   325,   330,
     333,   336,   339,   352,   371,   374,   377,   390
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ERR", "SHIFT", "MULTIPLY", "DIVIDE",
  "INT", "EXPONENT", "REAL", "ID", "DATE", "CLOCK", "TIMESTAMP", "LOGREF",
  "'('", "')'", "$accept", "unit_spec", "shift_exp", "product_exp",
  "power_exp", "basic_exp", "number", "timestamp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    17,    18,    18,    18,    19,    19,    19,    19,    19,
      20,    20,    20,    20,    20,    20,    20,    21,    21,    21,
      21,    22,    22,    22,    22,    22,    22,    23,    23,    24,
      24,    24,    24,    24,    24,    24,    24,    24
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     1,     3,     3,     3,     3,
       1,     2,     2,     3,     3,     3,     3,     1,     2,     2,
       2,     1,     3,     3,     3,     3,     1,     1,     1,     1,
       2,     3,     3,     3,     1,     2,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     4,    27,    28,    21,     0,     0,     0,     3,     0,
      10,     0,    26,     0,     0,     1,    12,     0,     0,     0,
      11,    20,    18,    19,    12,    24,    23,    22,     9,     7,
       6,    29,    34,     8,    14,    13,    16,    15,    30,    36,
      37,    35,    32,    33,    31
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    12,    33
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -10
static const yytype_int8 yypact[] =
{
      40,   -10,   -10,   -10,   -10,    80,    80,     2,   -10,    17,
     -10,     0,   -10,    29,    12,   -10,   -10,    70,    50,    60,
     -10,   -10,   -10,   -10,    48,   -10,   -10,   -10,   -10,   -10,
     -10,    -9,    56,   -10,   -10,   -10,   -10,   -10,    81,   -10,
     -10,   -10,   -10,   -10,   -10
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -10,   -10,     5,     7,    67,   -10,   -10,   -10
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -26
static const yytype_int8 yytable[] =
{
     -17,    21,    15,    38,   -17,   -17,   -17,    22,    23,   -17,
     -17,    14,    13,    26,   -17,   -17,   -17,    -5,    16,     0,
       0,    17,    18,    19,     2,     0,     3,     4,    27,     0,
      24,     5,     6,    -5,    18,    19,     2,     0,     3,     4,
      -2,     1,     0,     5,     6,    25,     0,     2,   -25,     3,
       4,    34,   -25,     0,     5,     6,   -25,     2,     0,     3,
       4,    36,     0,    39,     5,     6,    40,     2,    41,     3,
       4,    28,     0,     0,     5,     6,    20,    29,     0,    30,
      20,    31,     0,    32,     0,    35,    37,     2,    42,     3,
       4,    43,     0,    44,     5,     6
};

static const yytype_int8 yycheck[] =
{
       0,     1,     0,    12,     4,     5,     6,     7,     8,     9,
      10,     6,     5,     1,    14,    15,    16,     0,     1,    -1,
      -1,     4,     5,     6,     7,    -1,     9,    10,    16,    -1,
       1,    14,    15,    16,     5,     6,     7,    -1,     9,    10,
       0,     1,    -1,    14,    15,    16,    -1,     7,     0,     9,
      10,     1,     4,    -1,    14,    15,     8,     7,    -1,     9,
      10,     1,    -1,     7,    14,    15,    10,     7,    12,     9,
      10,     1,    -1,    -1,    14,    15,     9,     7,    -1,     9,
      13,    11,    -1,    13,    -1,    18,    19,     7,     7,     9,
      10,    10,    -1,    12,    14,    15
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     7,     9,    10,    14,    15,    18,    19,    20,
      21,    22,    23,    20,    19,     0,     1,     4,     5,     6,
      21,     1,     7,     8,     1,    16,     1,    16,     1,     7,
       9,    11,    13,    24,     1,    21,     1,    21,    12,     7,
      10,    12,     7,    10,    12
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1464 of yacc.c  */
#line 133 "parser.y"
    {
		    _finalUnit = ut_get_dimensionless_unit_one(_unitSystem);
		    YYACCEPT;
		}
    break;

  case 3:

/* Line 1464 of yacc.c  */
#line 137 "parser.y"
    {
		    _finalUnit = (yyvsp[(1) - (1)].unit);
		    YYACCEPT;
		}
    break;

  case 4:

/* Line 1464 of yacc.c  */
#line 141 "parser.y"
    {
		    YYABORT;
		}
    break;

  case 5:

/* Line 1464 of yacc.c  */
#line 146 "parser.y"
    {
		    (yyval.unit) = (yyvsp[(1) - (1)].unit);
		}
    break;

  case 6:

/* Line 1464 of yacc.c  */
#line 149 "parser.y"
    {
		    (yyval.unit) = ut_offset((yyvsp[(1) - (3)].unit), (yyvsp[(3) - (3)].rval));
		    ut_free((yyvsp[(1) - (3)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 155 "parser.y"
    {
		    (yyval.unit) = ut_offset((yyvsp[(1) - (3)].unit), (yyvsp[(3) - (3)].ival));
		    ut_free((yyvsp[(1) - (3)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 161 "parser.y"
    {
		    (yyval.unit) = ut_offset_by_time((yyvsp[(1) - (3)].unit), (yyvsp[(3) - (3)].rval));
		    ut_free((yyvsp[(1) - (3)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 167 "parser.y"
    {
		    ut_status	prev = ut_get_status();
		    ut_free((yyvsp[(1) - (3)].unit));
		    ut_set_status(prev);
		    YYERROR;
		}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 175 "parser.y"
    {
		    (yyval.unit) = (yyvsp[(1) - (1)].unit);
		}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 178 "parser.y"
    {
		    (yyval.unit) = ut_multiply((yyvsp[(1) - (2)].unit), (yyvsp[(2) - (2)].unit));
		    ut_free((yyvsp[(1) - (2)].unit));
		    ut_free((yyvsp[(2) - (2)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 185 "parser.y"
    {
		    ut_status	prev = ut_get_status();
		    ut_free((yyvsp[(1) - (2)].unit));
		    ut_set_status(prev);
		    YYERROR;
		}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 191 "parser.y"
    {
		    (yyval.unit) = ut_multiply((yyvsp[(1) - (3)].unit), (yyvsp[(3) - (3)].unit));
		    ut_free((yyvsp[(1) - (3)].unit));
		    ut_free((yyvsp[(3) - (3)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 198 "parser.y"
    {
		    ut_status	prev = ut_get_status();
		    ut_free((yyvsp[(1) - (3)].unit));
		    ut_set_status(prev);
		    YYERROR;
		}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 204 "parser.y"
    {
		    (yyval.unit) = ut_divide((yyvsp[(1) - (3)].unit), (yyvsp[(3) - (3)].unit));
		    ut_free((yyvsp[(1) - (3)].unit));
		    ut_free((yyvsp[(3) - (3)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 211 "parser.y"
    {
		    ut_status	prev = ut_get_status();
		    ut_free((yyvsp[(1) - (3)].unit));
		    ut_set_status(prev);
		    YYERROR;
		}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 219 "parser.y"
    {
		    (yyval.unit) = (yyvsp[(1) - (1)].unit);
		}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 222 "parser.y"
    {
		    (yyval.unit) = ut_raise((yyvsp[(1) - (2)].unit), (yyvsp[(2) - (2)].ival));
		    ut_free((yyvsp[(1) - (2)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 228 "parser.y"
    {
		    (yyval.unit) = ut_raise((yyvsp[(1) - (2)].unit), (yyvsp[(2) - (2)].ival));
		    ut_free((yyvsp[(1) - (2)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 234 "parser.y"
    {
		    ut_status	prev = ut_get_status();
		    ut_free((yyvsp[(1) - (2)].unit));
		    ut_set_status(prev);
		    YYERROR;
		}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 242 "parser.y"
    {
		    double	prefix = 1;
		    ut_unit*	unit = NULL;
		    char*	cp = (yyvsp[(1) - (1)].id);
		    int		symbolPrefixSeen = 0;

		    while (*cp) {
			size_t	nchar;
			double	value;

			unit = ut_get_unit_by_name(_unitSystem, cp);

			if (unit != NULL)
			    break;

			unit = ut_get_unit_by_symbol(_unitSystem, cp);

			if (unit != NULL)
			    break;

			if (utGetPrefixByName(_unitSystem, cp, &value, &nchar)
				== UT_SUCCESS) {
			    prefix *= value;
			    cp += nchar;
			}
			else {
			    if (!symbolPrefixSeen &&
				    utGetPrefixBySymbol(_unitSystem, cp, &value,
					&nchar) == UT_SUCCESS) {
				symbolPrefixSeen = 1;
				prefix *= value;
				cp += nchar;
			    }
			    else {
				break;
			    }
			}
		    }

		    free((yyvsp[(1) - (1)].id));

		    if (unit == NULL) {
			ut_set_status(UT_UNKNOWN);
			YYERROR;
		    }

		    (yyval.unit) = ut_scale(prefix, unit);

		    ut_free(unit);

		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 295 "parser.y"
    {
		    (yyval.unit) = (yyvsp[(2) - (3)].unit);
		}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 298 "parser.y"
    {
		    ut_status	status = ut_get_status();
		    ut_free((yyvsp[(2) - (3)].unit));
		    ut_set_status(status);
		    YYERROR;
		}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 304 "parser.y"
    {
		    (yyval.unit) = ut_log((yyvsp[(1) - (3)].rval), (yyvsp[(2) - (3)].unit));
		    ut_free((yyvsp[(2) - (3)].unit));
		    if ((yyval.unit) == NULL)
			YYERROR;
		}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 310 "parser.y"
    {
		    ut_status	status = ut_get_status();
		    ut_free((yyvsp[(2) - (3)].unit));
		    ut_set_status(status);
		    YYERROR;
		}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 316 "parser.y"
    {
		    (yyval.unit) = ut_scale((yyvsp[(1) - (1)].rval),
                        ut_get_dimensionless_unit_one(_unitSystem));
		}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 322 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (1)].ival);
		}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 325 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (1)].rval);
		}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 330 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (1)].rval);
		}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 333 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (2)].rval) + (yyvsp[(2) - (2)].rval);
		}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 336 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (3)].rval) + ((yyvsp[(2) - (3)].rval) - (yyvsp[(3) - (3)].rval));
		}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 339 "parser.y"
    {
		    int	mag = (yyvsp[(3) - (3)].ival) >= 0 ? (yyvsp[(3) - (3)].ival) : -(yyvsp[(3) - (3)].ival);
		    if (mag <= 24) {
			(yyval.rval) = (yyvsp[(1) - (3)].rval) + ((yyvsp[(2) - (3)].rval) - ut_encode_clock((yyvsp[(3) - (3)].ival), 0, 0));
		    }
		    else if (mag >= 100 && mag <= 2400) {
			(yyval.rval) = (yyvsp[(1) - (3)].rval) + ((yyvsp[(2) - (3)].rval) - ut_encode_clock((yyvsp[(3) - (3)].ival)/100, (yyvsp[(3) - (3)].ival)%100, 0));
		    }
		    else {
			ut_set_status(UT_SYNTAX);
			YYERROR;
		    }
		}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 352 "parser.y"
    {
		    int	error = 0;

		    if (strcasecmp((yyvsp[(3) - (3)].id), "UTC") != 0 &&
			    strcasecmp((yyvsp[(3) - (3)].id), "GMT") != 0 &&
			    strcasecmp((yyvsp[(3) - (3)].id), "Z") != 0) {
			ut_set_status(UT_UNKNOWN);
			error = 1;
		    }

		    free((yyvsp[(3) - (3)].id));

		    if (!error) {
			(yyval.rval) = (yyvsp[(1) - (3)].rval) + (yyvsp[(2) - (3)].rval);
		    }
		    else {
			YYERROR;
		    }
		}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 371 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (1)].rval);
		}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 374 "parser.y"
    {
		    (yyval.rval) = (yyvsp[(1) - (2)].rval) - (yyvsp[(2) - (2)].rval);
		}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 377 "parser.y"
    {
		    int	mag = (yyvsp[(2) - (2)].ival) >= 0 ? (yyvsp[(2) - (2)].ival) : -(yyvsp[(2) - (2)].ival);
		    if (mag <= 24) {
			(yyval.rval) = (yyvsp[(1) - (2)].rval) - ut_encode_clock((yyvsp[(2) - (2)].ival), 0, 0);
		    }
		    else if (mag >= 100 && mag <= 2400) {
			(yyval.rval) = (yyvsp[(1) - (2)].rval) - ut_encode_clock((yyvsp[(2) - (2)].ival)/100, (yyvsp[(2) - (2)].ival)%100, 0);
		    }
		    else {
			ut_set_status(UT_SYNTAX);
			YYERROR;
		    }
		}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 390 "parser.y"
    {
		    int	error = 0;

		    if (strcasecmp((yyvsp[(2) - (2)].id), "UTC") != 0 &&
			    strcasecmp((yyvsp[(2) - (2)].id), "GMT") != 0 &&
			    strcasecmp((yyvsp[(2) - (2)].id), "Z") != 0) {
			ut_set_status(UT_UNKNOWN);
			error = 1;
		    }

		    free((yyvsp[(2) - (2)].id));

		    if (!error) {
			(yyval.rval) = (yyvsp[(1) - (2)].rval);
		    }
		    else {
			YYERROR;
		    }
		}
    break;



/* Line 1464 of yacc.c  */
#line 1991 "parser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1684 of yacc.c  */
#line 411 "parser.y"


#define yymaxdepth	utmaxdepth
#define yylval		utlval
#define yychar		utchar
#define yypact		utpact
#define yyr1		utr1
#define yyr2		utr2
#define yydef		utdef
#define yychk		utchk
#define yypgo		utpgo
#define yyact		utact
#define yyexca		utexca
#define yyerrflag	uterrflag
#define yynerrs		utnerrs
#define yyps		utps
#define yypv		utpv
#define yys		uts
#define yy_yys		utyys
#define yystate		utstate
#define yytmp		uttmp
#define yyv		utv
#define yy_yyv		utyyv
#define yyval		utval
#define yylloc		utlloc
#define yyreds		utreds
#define yytoks		uttoks
#define yylhs		utyylhs
#define yylen		utyylen
#define yydefred	utyydefred
#define yydgoto		utyydgoto
#define yysindex	utyysindex
#define yyrindex	utyyrindex
#define yygindex	utyygindex
#define yytable		utyytable
#define yycheck		utyycheck
#define yyname		utyyname
#define yyrule		utyyrule

#include "scanner.c"


/*
 * Converts a string in the Latin-1 character set (ISO 8859-1) to the UTF-8
 * character set.
 *
 * Arguments:
 *      latin1String    Pointer to the string to be converted.  May be freed
 *                      upon return.
 * Returns:
 *      NULL            Failure.  ut_handle_error_message() was called.
 *      else            Pointer to UTF-8 representation of "string".  Must not
 *                      be freed.  Subsequent calls may overwrite.
 */
static const char*
latin1ToUtf8(
    const char* const   latin1String)
{
    static char*                utf8String = NULL;
    static size_t               bufSize = 0;
    size_t                      size;
    const unsigned char*        in;
    unsigned char*              out;

    assert(latin1String != NULL);

    size = 2 * strlen(latin1String) + 1;

    if (size > bufSize) {
        char*   buf = realloc(utf8String, size);

        if (buf != NULL) {
            utf8String = buf;
            bufSize = size;
        }
        else {
            ut_handle_error_message("Couldn't allocate %ld-byte buffer: %s",
                (unsigned long)size, strerror(errno));

            return NULL;
        }
    }

    for (in = (const unsigned char*)latin1String,
            out = (unsigned char*)utf8String; *in; ++in) {
#       define IS_ASCII(c) (((c) & 0x80) == 0)

        if (IS_ASCII(*in)) {
            *out++ = *in;
        }
        else {
            *out++ = 0xC0 | ((0xC0 & *in) >> 6);
            *out++ = 0x80 | (0x3F & *in);
        }
    }

    *out = 0;

    return utf8String;
}


/*
 * Returns the binary representation of a unit corresponding to a string
 * representation.
 *
 * Arguments:
 *	system		Pointer to the unit-system in which the parsing will
 *			occur.
 *	string		The string to be parsed (e.g., "millimeters").  There
 *			should be no leading or trailing whitespace in the
 *			string.  See ut_trim().
 *	encoding	The encoding of "string".
 * Returns:
 *	NULL		Failure.  "ut_get_status()" will be one of
 *			    UT_BAD_ARG		"system" or "string" is NULL.
 *			    UT_SYNTAX		"string" contained a syntax
 *						error.
 *			    UT_UNKNOWN		"string" contained an unknown
 *						identifier.
 *			    UT_OS		Operating-system failure.  See
 *						"errno".
 *	else		Pointer to the unit corresponding to "string".
 */
ut_unit*
ut_parse(
    const ut_system* const	system,
    const char* const		string,
    ut_encoding			encoding)
{
    ut_unit*	unit = NULL;		/* failure */

    if (system == NULL || string == NULL) {
	ut_set_status(UT_BAD_ARG);
    }
    else {
        const char*     utf8String;

        if (encoding != UT_LATIN1) {
            utf8String = string;
        }
        else {
            utf8String = latin1ToUtf8(string);
            encoding = UT_UTF8;

            if (utf8String == NULL)
                ut_set_status(UT_OS);
        }

        if (utf8String != NULL) {
            YY_BUFFER_STATE	buf = ut_scan_string(utf8String);

            _unitSystem = (ut_system*)system;
            _encoding = encoding;
            _restartScanner = 1;

#if YYDEBUG
            utdebug = 0;
            ut_flex_debug = 0;
#endif

            _finalUnit = NULL;

            if (utparse() == 0) {
                int     status;
                int	n = yy_c_buf_p  - buf->yy_ch_buf;

                if (n >= strlen(utf8String)) {
                    unit = _finalUnit;	/* success */
                    status = UT_SUCCESS;
                }
                else {
                    /*
                     * Parsing terminated before the end of the string.
                     */
                    ut_free(_finalUnit);
                    status = UT_SYNTAX;
                }

                ut_set_status(status);
            }

            ut_delete_buffer(buf);
        }                               /* utf8String != NULL */
    }                                   /* valid arguments */

    return unit;
}

