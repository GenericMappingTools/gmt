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
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         dapparse
#define yylex           daplex
#define yyerror         daperror
#define yylval          daplval
#define yychar          dapchar
#define yydebug         dapdebug
#define yynerrs         dapnerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 11 "dap.y"

#include "config.h"
#include "dapparselex.h"
int dapdebug = 0;


/* Line 189 of yacc.c  */
#line 87 "dap.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
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
     SCAN_ALIAS = 258,
     SCAN_ARRAY = 259,
     SCAN_ATTR = 260,
     SCAN_BYTE = 261,
     SCAN_CODE = 262,
     SCAN_DATASET = 263,
     SCAN_DATA = 264,
     SCAN_ERROR = 265,
     SCAN_FLOAT32 = 266,
     SCAN_FLOAT64 = 267,
     SCAN_GRID = 268,
     SCAN_INT16 = 269,
     SCAN_INT32 = 270,
     SCAN_MAPS = 271,
     SCAN_MESSAGE = 272,
     SCAN_SEQUENCE = 273,
     SCAN_STRING = 274,
     SCAN_STRUCTURE = 275,
     SCAN_UINT16 = 276,
     SCAN_UINT32 = 277,
     SCAN_URL = 278,
     SCAN_PTYPE = 279,
     SCAN_PROG = 280,
     WORD_WORD = 281,
     WORD_STRING = 282
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 156 "dap.tab.c"

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
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   361

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  106
/* YYNRULES -- Number of states.  */
#define YYNSTATES  201

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   282

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    35,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    31,    30,
       2,    34,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    32,     2,    33,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    28,     2,    29,     2,     2,     2,     2,
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
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,    10,    13,    16,    18,    20,    22,
      24,    30,    31,    34,    39,    47,    54,    66,    68,    70,
      72,    74,    76,    78,    80,    82,    84,    86,    87,    90,
      94,    99,   105,   107,   109,   111,   113,   117,   119,   120,
     123,   126,   131,   136,   141,   146,   151,   156,   161,   166,
     171,   176,   178,   180,   184,   186,   190,   192,   196,   198,
     202,   204,   208,   210,   214,   216,   220,   222,   226,   228,
     232,   234,   236,   238,   242,   250,   251,   256,   257,   262,
     263,   268,   269,   274,   276,   278,   280,   282,   284,   286,
     288,   290,   292,   294,   296,   298,   300,   302,   304,   306,
     308,   310,   312,   314,   316,   318,   320
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      37,     0,    -1,    38,    41,    -1,    38,    41,     9,    -1,
      39,    49,    -1,    40,    64,    -1,     1,    -1,     8,    -1,
       5,    -1,    10,    -1,    28,    42,    29,    47,    30,    -1,
      -1,    42,    43,    -1,    44,    48,    45,    30,    -1,    20,
      28,    42,    29,    48,    45,    30,    -1,    18,    28,    42,
      29,    48,    30,    -1,    13,    28,     4,    31,    43,    16,
      31,    42,    29,    48,    30,    -1,     1,    -1,     6,    -1,
      14,    -1,    21,    -1,    15,    -1,    22,    -1,    11,    -1,
      12,    -1,    23,    -1,    19,    -1,    -1,    45,    46,    -1,
      32,    26,    33,    -1,    32,    34,    26,    33,    -1,    32,
      69,    34,    26,    33,    -1,     1,    -1,    48,    -1,     1,
      -1,    69,    -1,    28,    50,    29,    -1,     1,    -1,    -1,
      50,    51,    -1,    63,    30,    -1,     6,    69,    52,    30,
      -1,    14,    69,    53,    30,    -1,    21,    69,    54,    30,
      -1,    15,    69,    55,    30,    -1,    22,    69,    56,    30,
      -1,    11,    69,    57,    30,    -1,    12,    69,    58,    30,
      -1,    19,    69,    59,    30,    -1,    23,    69,    60,    30,
      -1,    69,    28,    50,    29,    -1,     1,    -1,    26,    -1,
      52,    35,    26,    -1,    26,    -1,    53,    35,    26,    -1,
      26,    -1,    54,    35,    26,    -1,    26,    -1,    55,    35,
      26,    -1,    26,    -1,    56,    35,    26,    -1,    26,    -1,
      57,    35,    26,    -1,    26,    -1,    58,    35,    26,    -1,
      62,    -1,    59,    35,    62,    -1,    61,    -1,    60,    35,
      61,    -1,    69,    -1,    69,    -1,    27,    -1,     3,    26,
      26,    -1,    28,    65,    66,    67,    68,    29,    30,    -1,
      -1,     7,    34,    26,    30,    -1,    -1,    17,    34,    26,
      30,    -1,    -1,    24,    34,    26,    30,    -1,    -1,    25,
      34,    26,    30,    -1,    26,    -1,     3,    -1,     4,    -1,
       5,    -1,     6,    -1,     8,    -1,     9,    -1,    10,    -1,
      11,    -1,    12,    -1,    13,    -1,    14,    -1,    15,    -1,
      16,    -1,    18,    -1,    19,    -1,    20,    -1,    21,    -1,
      22,    -1,    23,    -1,     7,    -1,    17,    -1,    25,    -1,
      24,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    53,    53,    54,    55,    56,    57,    61,    65,    69,
      74,    80,    81,    87,    89,    91,    93,    96,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   114,   115,   119,
     120,   121,   122,   127,   128,   132,   135,   136,   141,   142,
     146,   147,   149,   151,   153,   155,   157,   159,   161,   163,
     165,   166,   171,   172,   176,   177,   181,   182,   186,   187,
     191,   192,   195,   196,   199,   200,   203,   204,   208,   209,
     213,   217,   218,   229,   233,   237,   237,   238,   238,   239,
     239,   240,   240,   246,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SCAN_ALIAS", "SCAN_ARRAY", "SCAN_ATTR",
  "SCAN_BYTE", "SCAN_CODE", "SCAN_DATASET", "SCAN_DATA", "SCAN_ERROR",
  "SCAN_FLOAT32", "SCAN_FLOAT64", "SCAN_GRID", "SCAN_INT16", "SCAN_INT32",
  "SCAN_MAPS", "SCAN_MESSAGE", "SCAN_SEQUENCE", "SCAN_STRING",
  "SCAN_STRUCTURE", "SCAN_UINT16", "SCAN_UINT32", "SCAN_URL", "SCAN_PTYPE",
  "SCAN_PROG", "WORD_WORD", "WORD_STRING", "'{'", "'}'", "';'", "':'",
  "'['", "']'", "'='", "','", "$accept", "start", "dataset", "attr", "err",
  "datasetbody", "declarations", "declaration", "base_type", "array_decls",
  "array_decl", "datasetname", "var_name", "attributebody", "attr_list",
  "attribute", "bytes", "int16", "uint16", "int32", "uint32", "float32",
  "float64", "strs", "urls", "url", "str_or_id", "alias", "errorbody",
  "errorcode", "errormsg", "errorptype", "errorprog", "name", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   123,   125,
      59,    58,    91,    93,    61,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    36,    37,    37,    37,    37,    37,    38,    39,    40,
      41,    42,    42,    43,    43,    43,    43,    43,    44,    44,
      44,    44,    44,    44,    44,    44,    44,    45,    45,    46,
      46,    46,    46,    47,    47,    48,    49,    49,    50,    50,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    52,    52,    53,    53,    54,    54,    55,    55,
      56,    56,    57,    57,    58,    58,    59,    59,    60,    60,
      61,    62,    62,    63,    64,    65,    65,    66,    66,    67,
      67,    68,    68,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    69
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     2,     2,     1,     1,     1,     1,
       5,     0,     2,     4,     7,     6,    11,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     2,     3,
       4,     5,     1,     1,     1,     1,     3,     1,     0,     2,
       2,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     1,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     1,     1,     3,     7,     0,     4,     0,     4,     0,
       4,     0,     4,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     6,     8,     7,     9,     0,     0,     0,     0,     1,
      11,     2,    37,    38,     4,    75,     5,     0,     3,     0,
       0,    77,    17,    18,    23,    24,     0,    19,    21,     0,
      26,     0,    20,    22,    25,     0,    12,     0,    51,    84,
      85,    86,    87,   103,    88,    89,    90,    91,    92,    93,
      94,    95,    96,   104,    97,    98,    99,   100,   101,   102,
     106,   105,    83,    36,    39,     0,     0,     0,     0,    79,
       0,    11,    11,    34,    84,    87,    91,    92,    94,    95,
      98,   100,   101,   102,     0,    33,    35,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    40,    38,
       0,     0,     0,    81,     0,     0,     0,    10,     0,    73,
      52,     0,    62,     0,    64,     0,    54,     0,    58,     0,
      72,     0,    66,    71,    56,     0,    60,     0,     0,    68,
      70,     0,    76,     0,     0,     0,     0,     0,     0,     0,
      32,    13,     0,    28,    41,     0,    46,     0,    47,     0,
      42,     0,    44,     0,    48,     0,    43,     0,    45,     0,
      49,     0,    50,    78,     0,     0,     0,     0,     0,    27,
      83,     0,     0,    53,    63,    65,    55,    59,    67,    57,
      61,    69,    80,     0,    74,     0,    15,     0,    29,     0,
       0,    82,    11,    14,    30,     0,     0,    31,     0,     0,
      16
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,     7,     8,    11,    17,    36,    37,   108,
     143,    84,    85,    14,    19,    64,   111,   117,   125,   119,
     127,   113,   115,   121,   128,   129,   122,    65,    16,    21,
      69,   103,   136,    86
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -70
static const yytype_int16 yypact[] =
{
       5,   -70,   -70,   -70,   -70,     8,    -1,     3,     0,   -70,
     -70,    10,   -70,   -70,   -70,    28,   -70,    69,   -70,   159,
      34,    57,   -70,   -70,   -70,   -70,    50,   -70,   -70,    58,
     -70,    65,   -70,   -70,   -70,   263,   -70,   312,   -70,    59,
     -70,   -70,   312,   -70,   -70,   -70,   -70,   312,   312,   -70,
     312,   312,   -70,   -70,   -70,   312,   -70,   312,   312,   312,
     -70,   -70,   -70,   -70,   -70,    64,    67,    70,    63,    75,
      96,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,   -70,
     -70,   -70,   -70,   -70,    73,   -70,   -70,   -70,    78,    79,
      80,    81,    82,    83,   287,    84,    85,   312,   -70,   -70,
      86,    88,    87,    90,    89,   127,   212,   -70,     4,   -70,
     -70,   -23,   -70,   -21,   -70,   -19,   -70,   -13,   -70,   -12,
     -70,   -10,   -70,   -70,   -70,    -9,   -70,    36,    42,   -70,
     -70,   186,   -70,    92,    91,    93,    97,   338,   312,   312,
     -70,   -70,    39,   -70,   -70,    99,   -70,   103,   -70,   104,
     -70,   105,   -70,   106,   -70,   287,   -70,   108,   -70,   109,
     -70,   312,   -70,   -70,   114,   110,   121,   102,   122,   -70,
     120,   128,   123,   -70,   -70,   -70,   -70,   -70,   -70,   -70,
     -70,   -70,   -70,   125,   -70,   155,   -70,    37,   -70,   126,
     132,   -70,   -70,   -70,   -70,   181,   236,   -70,   312,   187,
     -70
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -70,   -70,   -70,   -70,   -70,   -70,   -69,   -25,   -70,   -50,
     -70,   -70,   -37,   -70,   117,   -70,   -70,   -70,   -70,   -70,
     -70,   -70,   -70,   -70,   -70,    60,    74,   -70,   -70,   -70,
     -70,   -70,   -70,   -18
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      87,    66,   105,   106,    12,   140,     1,   144,     9,   146,
       2,   148,   145,     3,   147,     4,   149,   150,   152,    18,
     154,   156,   151,   153,    89,   155,   157,    10,    15,    90,
      91,    13,    92,    93,   141,    20,   142,    94,   140,    95,
      96,    97,    74,    40,    41,    75,    43,    44,    45,    46,
      76,    77,    49,    78,    79,    52,    53,    54,    80,    56,
      81,    82,    83,    60,    61,   170,   158,   193,    67,   142,
      22,   159,   160,   171,    68,    23,   123,   161,    70,   130,
      24,    25,    26,    27,    28,    88,    71,    29,    30,    31,
      32,    33,    34,    72,    98,    99,   100,   101,    35,   102,
     104,   168,   169,   107,   109,   110,   112,   114,   116,   118,
     124,   126,   167,    66,   133,   135,   132,   164,   185,   187,
     137,   134,   163,   196,   172,   173,   166,   165,    22,   174,
     175,   176,   177,    23,   179,   180,   183,   123,    24,    25,
      26,    27,    28,   130,   182,    29,    30,    31,    32,    33,
      34,   184,   186,   188,   189,   191,   138,   190,   195,   194,
      38,   199,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,   192,    38,    63,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    22,   197,   162,   131,   200,    23,     0,
       0,   181,     0,    24,    25,    26,    27,    28,     0,   178,
      29,    30,    31,    32,    33,    34,     0,    22,     0,     0,
       0,   139,    23,     0,     0,     0,     0,    24,    25,    26,
      27,    28,     0,     0,    29,    30,    31,    32,    33,    34,
       0,     0,     0,     0,    73,   198,    74,    40,    41,    75,
      43,    44,    45,    46,    76,    77,    49,    78,    79,    52,
      53,    54,    80,    56,    81,    82,    83,    60,    61,    62,
      74,    40,    41,    75,    43,    44,    45,    46,    76,    77,
      49,    78,    79,    52,    53,    54,    80,    56,    81,    82,
      83,    60,    61,    62,   120,    74,    40,    41,    75,    43,
      44,    45,    46,    76,    77,    49,    78,    79,    52,    53,
      54,    80,    56,    81,    82,    83,    60,    61,    62,    22,
       0,     0,     0,     0,    23,     0,     0,     0,     0,    24,
      25,    26,    27,    28,     0,     0,    29,    30,    31,    32,
      33,    34
};

static const yytype_int16 yycheck[] =
{
      37,    19,    71,    72,     1,     1,     1,    30,     0,    30,
       5,    30,    35,     8,    35,    10,    35,    30,    30,     9,
      30,    30,    35,    35,    42,    35,    35,    28,    28,    47,
      48,    28,    50,    51,    30,     7,    32,    55,     1,    57,
      58,    59,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    30,    30,    34,    32,
       1,    35,    30,    34,    17,     6,    94,    35,    28,    97,
      11,    12,    13,    14,    15,    26,    28,    18,    19,    20,
      21,    22,    23,    28,    30,    28,    26,    34,    29,    24,
       4,   138,   139,    30,    26,    26,    26,    26,    26,    26,
      26,    26,   137,   131,    26,    25,    30,    26,    16,   169,
      31,    34,    30,   192,   142,    26,    29,    34,     1,    26,
      26,    26,    26,     6,    26,    26,    26,   155,    11,    12,
      13,    14,    15,   161,    30,    18,    19,    20,    21,    22,
      23,    30,    30,    33,    26,    30,    29,    34,    26,    33,
       1,   198,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    31,     1,    29,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     1,    33,    29,    99,    30,     6,    -1,
      -1,   161,    -1,    11,    12,    13,    14,    15,    -1,   155,
      18,    19,    20,    21,    22,    23,    -1,     1,    -1,    -1,
      -1,    29,     6,    -1,    -1,    -1,    -1,    11,    12,    13,
      14,    15,    -1,    -1,    18,    19,    20,    21,    22,    23,
      -1,    -1,    -1,    -1,     1,    29,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,     1,
      -1,    -1,    -1,    -1,     6,    -1,    -1,    -1,    -1,    11,
      12,    13,    14,    15,    -1,    -1,    18,    19,    20,    21,
      22,    23
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     5,     8,    10,    37,    38,    39,    40,     0,
      28,    41,     1,    28,    49,    28,    64,    42,     9,    50,
       7,    65,     1,     6,    11,    12,    13,    14,    15,    18,
      19,    20,    21,    22,    23,    29,    43,    44,     1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    29,    51,    63,    69,    34,    17,    66,
      28,    28,    28,     1,     3,     6,    11,    12,    14,    15,
      19,    21,    22,    23,    47,    48,    69,    48,    26,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    30,    28,
      26,    34,    24,    67,     4,    42,    42,    30,    45,    26,
      26,    52,    26,    57,    26,    58,    26,    53,    26,    55,
      27,    59,    62,    69,    26,    54,    26,    56,    60,    61,
      69,    50,    30,    26,    34,    25,    68,    31,    29,    29,
       1,    30,    32,    46,    30,    35,    30,    35,    30,    35,
      30,    35,    30,    35,    30,    35,    30,    35,    30,    35,
      30,    35,    29,    30,    26,    34,    29,    43,    48,    48,
      26,    34,    69,    26,    26,    26,    26,    26,    62,    26,
      26,    61,    30,    26,    30,    16,    30,    45,    33,    26,
      34,    30,    31,    30,    33,    26,    42,    33,    29,    48,
      30
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
      yyerror (parsestate, YY_("syntax error: cannot back up")); \
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
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, parsestate)
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
		  Type, Value, parsestate); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, DAPparsestate* parsestate)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parsestate)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    DAPparsestate* parsestate;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (parsestate);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, DAPparsestate* parsestate)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parsestate)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    DAPparsestate* parsestate;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parsestate);
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, DAPparsestate* parsestate)
#else
static void
yy_reduce_print (yyvsp, yyrule, parsestate)
    YYSTYPE *yyvsp;
    int yyrule;
    DAPparsestate* parsestate;
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
		       		       , parsestate);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, parsestate); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, DAPparsestate* parsestate)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parsestate)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    DAPparsestate* parsestate;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (parsestate);

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
int yyparse (DAPparsestate* parsestate);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





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
yyparse (DAPparsestate* parsestate)
#else
int
yyparse (parsestate)
    DAPparsestate* parsestate;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

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
        case 6:

/* Line 1464 of yacc.c  */
#line 57 "dap.y"
    {dap_unrecognizedresponse(parsestate); YYABORT;;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 62 "dap.y"
    {dap_tagparse(parsestate,SCAN_DATASET);;}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 66 "dap.y"
    {dap_tagparse(parsestate,SCAN_ATTR);;}
    break;

  case 9:

/* Line 1464 of yacc.c  */
#line 70 "dap.y"
    {dap_tagparse(parsestate,SCAN_ERROR);;}
    break;

  case 10:

/* Line 1464 of yacc.c  */
#line 75 "dap.y"
    {dap_datasetbody(parsestate,(yyvsp[(4) - (5)]),(yyvsp[(2) - (5)]));;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 80 "dap.y"
    {(yyval)=dap_declarations(parsestate,null,null);;}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 81 "dap.y"
    {(yyval)=dap_declarations(parsestate,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 13:

/* Line 1464 of yacc.c  */
#line 88 "dap.y"
    {(yyval)=dap_makebase(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 14:

/* Line 1464 of yacc.c  */
#line 90 "dap.y"
    {if(((yyval)=dap_makestructure(parsestate,(yyvsp[(5) - (7)]),(yyvsp[(6) - (7)]),(yyvsp[(3) - (7)])))==null) {YYABORT;};}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 92 "dap.y"
    {if(((yyval)=dap_makesequence(parsestate,(yyvsp[(5) - (6)]),(yyvsp[(3) - (6)])))==null) {YYABORT;};}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 95 "dap.y"
    {if(((yyval)=dap_makegrid(parsestate,(yyvsp[(10) - (11)]),(yyvsp[(5) - (11)]),(yyvsp[(8) - (11)])))==null) {YYABORT;};}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 97 "dap.y"
    {daperror(parsestate,"Unrecognized type"); YYABORT;;}
    break;

  case 18:

/* Line 1464 of yacc.c  */
#line 102 "dap.y"
    {(yyval)=(Object)SCAN_BYTE;;}
    break;

  case 19:

/* Line 1464 of yacc.c  */
#line 103 "dap.y"
    {(yyval)=(Object)SCAN_INT16;;}
    break;

  case 20:

/* Line 1464 of yacc.c  */
#line 104 "dap.y"
    {(yyval)=(Object)SCAN_UINT16;;}
    break;

  case 21:

/* Line 1464 of yacc.c  */
#line 105 "dap.y"
    {(yyval)=(Object)SCAN_INT32;;}
    break;

  case 22:

/* Line 1464 of yacc.c  */
#line 106 "dap.y"
    {(yyval)=(Object)SCAN_UINT32;;}
    break;

  case 23:

/* Line 1464 of yacc.c  */
#line 107 "dap.y"
    {(yyval)=(Object)SCAN_FLOAT32;;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 108 "dap.y"
    {(yyval)=(Object)SCAN_FLOAT64;;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 109 "dap.y"
    {(yyval)=(Object)SCAN_URL;;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 110 "dap.y"
    {(yyval)=(Object)SCAN_STRING;;}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 114 "dap.y"
    {(yyval)=dap_arraydecls(parsestate,null,null);;}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 115 "dap.y"
    {(yyval)=dap_arraydecls(parsestate,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 119 "dap.y"
    {(yyval)=dap_arraydecl(parsestate,null,(yyvsp[(2) - (3)]));;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 120 "dap.y"
    {(yyval)=dap_arraydecl(parsestate,null,(yyvsp[(3) - (4)]));;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 121 "dap.y"
    {(yyval)=dap_arraydecl(parsestate,(yyvsp[(2) - (5)]),(yyvsp[(4) - (5)]));;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 123 "dap.y"
    {daperror(parsestate,"Illegal dimension declaration"); YYABORT;;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 127 "dap.y"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 129 "dap.y"
    {daperror(parsestate,"Illegal dataset declaration"); YYABORT;;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 132 "dap.y"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 135 "dap.y"
    {dap_attributebody(parsestate,(yyvsp[(2) - (3)]));;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 137 "dap.y"
    {daperror(parsestate,"Illegal DAS body"); YYABORT;;}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 141 "dap.y"
    {(yyval)=dap_attrlist(parsestate,null,null);;}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 142 "dap.y"
    {(yyval)=dap_attrlist(parsestate,(yyvsp[(1) - (2)]),(yyvsp[(2) - (2)]));;}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 146 "dap.y"
    {(yyval)=null;;}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 148 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_BYTE);;}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 150 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_INT16);;}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 152 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_UINT16);;}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 154 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_INT32);;}
    break;

  case 45:

/* Line 1464 of yacc.c  */
#line 156 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_UINT32);;}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 158 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_FLOAT32);;}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 160 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_FLOAT64);;}
    break;

  case 48:

/* Line 1464 of yacc.c  */
#line 162 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_STRING);;}
    break;

  case 49:

/* Line 1464 of yacc.c  */
#line 164 "dap.y"
    {(yyval)=dap_attribute(parsestate,(yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),(Object)SCAN_URL);;}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 165 "dap.y"
    {(yyval)=dap_attrset(parsestate,(yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]));;}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 167 "dap.y"
    {daperror(parsestate,"Illegal attribute"); YYABORT;;}
    break;

  case 52:

/* Line 1464 of yacc.c  */
#line 171 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_BYTE);;}
    break;

  case 53:

/* Line 1464 of yacc.c  */
#line 173 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_BYTE);;}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 176 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_INT16);;}
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 178 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_INT16);;}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 181 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_UINT16);;}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 183 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_UINT16);;}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 186 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_INT32);;}
    break;

  case 59:

/* Line 1464 of yacc.c  */
#line 188 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_INT32);;}
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 191 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_UINT32);;}
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 192 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_UINT32);;}
    break;

  case 62:

/* Line 1464 of yacc.c  */
#line 195 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_FLOAT32);;}
    break;

  case 63:

/* Line 1464 of yacc.c  */
#line 196 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_FLOAT32);;}
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 199 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_FLOAT64);;}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 200 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_FLOAT64);;}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 203 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_STRING);;}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 204 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_STRING);;}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 208 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,null,(yyvsp[(1) - (1)]),(Object)SCAN_URL);;}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 209 "dap.y"
    {(yyval)=dap_attrvalue(parsestate,(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),(Object)SCAN_URL);;}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 213 "dap.y"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 217 "dap.y"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 218 "dap.y"
    {(yyval)=(yyvsp[(1) - (1)]);;}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 229 "dap.y"
    {(yyval)=(yyvsp[(2) - (3)]); (yyval)=(yyvsp[(3) - (3)]); (yyval)=null;;}
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 234 "dap.y"
    {dap_errorbody(parsestate,(yyvsp[(2) - (7)]),(yyvsp[(3) - (7)]),(yyvsp[(4) - (7)]),(yyvsp[(5) - (7)]));;}
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 237 "dap.y"
    {(yyval)=null;;}
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 237 "dap.y"
    {(yyval)=(yyvsp[(3) - (4)]);;}
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 238 "dap.y"
    {(yyval)=null;;}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 238 "dap.y"
    {(yyval)=(yyvsp[(3) - (4)]);;}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 239 "dap.y"
    {(yyval)=null;;}
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 239 "dap.y"
    {(yyval)=(yyvsp[(3) - (4)]);;}
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 240 "dap.y"
    {(yyval)=null;;}
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 240 "dap.y"
    {(yyval)=(yyvsp[(3) - (4)]);;}
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 246 "dap.y"
    {(yyval)=dapdecode(parsestate->lexstate,(yyvsp[(1) - (1)]));;}
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 247 "dap.y"
    {(yyval)=strdup("alias");;}
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 248 "dap.y"
    {(yyval)=strdup("array");;}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 249 "dap.y"
    {(yyval)=strdup("attributes");;}
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 250 "dap.y"
    {(yyval)=strdup("byte");;}
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 251 "dap.y"
    {(yyval)=strdup("dataset");;}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 252 "dap.y"
    {(yyval)=strdup("data");;}
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 253 "dap.y"
    {(yyval)=strdup("error");;}
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 254 "dap.y"
    {(yyval)=strdup("float32");;}
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 255 "dap.y"
    {(yyval)=strdup("float64");;}
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 256 "dap.y"
    {(yyval)=strdup("grid");;}
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 257 "dap.y"
    {(yyval)=strdup("int16");;}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 258 "dap.y"
    {(yyval)=strdup("int32");;}
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 259 "dap.y"
    {(yyval)=strdup("maps");;}
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 260 "dap.y"
    {(yyval)=strdup("sequence");;}
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 261 "dap.y"
    {(yyval)=strdup("string");;}
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 262 "dap.y"
    {(yyval)=strdup("structure");;}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 263 "dap.y"
    {(yyval)=strdup("uint16");;}
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 264 "dap.y"
    {(yyval)=strdup("uint32");;}
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 265 "dap.y"
    {(yyval)=strdup("url");;}
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 266 "dap.y"
    {(yyval)=strdup("code");;}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 267 "dap.y"
    {(yyval)=strdup("message");;}
    break;

  case 105:

/* Line 1464 of yacc.c  */
#line 268 "dap.y"
    {(yyval)=strdup("program");;}
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 269 "dap.y"
    {(yyval)=strdup("program_type");;}
    break;



/* Line 1464 of yacc.c  */
#line 2280 "dap.tab.c"
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
      yyerror (parsestate, YY_("syntax error"));
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
	    yyerror (parsestate, yymsg);
	  }
	else
	  {
	    yyerror (parsestate, YY_("syntax error"));
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
		      yytoken, &yylval, parsestate);
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
		  yystos[yystate], yyvsp, parsestate);
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
  yyerror (parsestate, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, parsestate);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, parsestate);
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
#line 272 "dap.y"


