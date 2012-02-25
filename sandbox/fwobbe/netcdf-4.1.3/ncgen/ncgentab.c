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
#define yyparse         ncgparse
#define yylex           ncglex
#define yyerror         ncgerror
#define yylval          ncglval
#define yychar          ncgchar
#define yydebug         ncgdebug
#define yynerrs         ncgnerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 9 "ncgen.y"

/*
static char SccsId[] = "$Id$";
*/
#include        "includes.h"
#include        "offsets.h"

/* parser controls */
#define YY_NO_INPUT 1

/* True if string a equals string b*/
#define STREQ(a, b)     (*(a) == *(b) && strcmp((a), (b)) == 0)
#define VLENSIZE  (sizeof(nc_vlen_t))
#define MAXFLOATDIM 4294967295.0

/* mnemonic */
typedef enum Attrkind {ATTRVAR, ATTRGLOBAL, DONTKNOW} Attrkind;

typedef nc_vlen_t vlen_t;

/* We retain the old representation of the symbol list
   as a linked list.
*/
Symbol* symlist;

/* Track rootgroup separately*/
Symbol* rootgroup;

/* Track the group sequence */
static List* groupstack;

/* Provide a separate sequence for accumulating values
   during the parse.
*/
static List* stack;

/* track homogeneity of types for data lists*/
static nc_type consttype;

/* Misc. */
static int stackbase;
static int stacklen;
static int count;
static int opaqueid; /* counter for opaque constants*/
static int arrayuid; /* counter for pseudo-array types*/

char* primtypenames[PRIMNO] = {
"nat",
"byte", "char", "short",
"int", "float", "double",
"ubyte", "ushort", "uint",
"int64", "uint64",
"string"
};

/*Defined in ncgen.l*/
extern int lineno;              /* line number for error messages */
extern char* lextext;           /* name or string with escapes removed */

extern double double_val;       /* last double value read */
extern float float_val;         /* last float value read */
extern long long int64_val;         /* last int64 value read */
extern int int32_val;             /* last int32 value read */
extern short int16_val;         /* last short value read */
extern unsigned long long uint64_val;         /* last int64 value read */
extern unsigned int uint32_val;             /* last int32 value read */
extern unsigned short uint16_val;         /* last short value read */
extern char char_val;           /* last char value read */
extern signed char byte_val;    /* last byte value read */
extern unsigned char ubyte_val;    /* last byte value read */

/* Track definitions of dims, types, attributes, and vars*/
List* grpdefs;
List* dimdefs;
List* attdefs; /* variable-specific attributes*/
List* gattdefs; /* global attributes only*/
List* xattdefs; /* unknown attributes*/
List* typdefs;
List* vardefs;
List* condefs; /* non-dimension constants used in type defs*/
List* tmp;

/* Forward */
static Constant makeconstdata(nc_type);
static Constant makeenumconst(Symbol*);
static void addtogroup(Symbol*);
static Symbol* getunlimiteddim(void);
static void setunlimiteddim(Symbol* udim);
static Symbol* currentgroup(void);
static Symbol* createrootgroup(void);
static Symbol* creategroup(Symbol*);
static int dupobjectcheck(nc_class,Symbol*);
static void setpathcurrent(Symbol* sym);
static Symbol* makeattribute(Symbol*,Symbol*,Symbol*,Datalist*,Attrkind);
static Symbol* makeprimitivetype(nc_type i);
static Symbol* makespecial(int tag, Symbol* vsym, Symbol* tsym, void* data, int isconst);
static int containsfills(Datalist* list);
static void datalistextend(Datalist* dl, Constant* con);
static void vercheck(int ncid);

int yylex(void);

#ifndef NO_STDARG
static void yyerror(const char *fmt, ...);
#else
static void yyerror(fmt,va_alist) const char* fmt; va_dcl;
#endif

/* Extern */
extern int lex_init(void);



/* Line 189 of yacc.c  */
#line 194 "ncgen.tab.c"

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
     NC_UNLIMITED_K = 258,
     CHAR_K = 259,
     BYTE_K = 260,
     SHORT_K = 261,
     INT_K = 262,
     FLOAT_K = 263,
     DOUBLE_K = 264,
     UBYTE_K = 265,
     USHORT_K = 266,
     UINT_K = 267,
     INT64_K = 268,
     UINT64_K = 269,
     IDENT = 270,
     TERMSTRING = 271,
     CHAR_CONST = 272,
     BYTE_CONST = 273,
     SHORT_CONST = 274,
     INT_CONST = 275,
     INT64_CONST = 276,
     UBYTE_CONST = 277,
     USHORT_CONST = 278,
     UINT_CONST = 279,
     UINT64_CONST = 280,
     FLOAT_CONST = 281,
     DOUBLE_CONST = 282,
     DIMENSIONS = 283,
     VARIABLES = 284,
     NETCDF = 285,
     DATA = 286,
     TYPES = 287,
     COMPOUND = 288,
     ENUM = 289,
     OPAQUE = 290,
     OPAQUESTRING = 291,
     GROUP = 292,
     PATH = 293,
     FILLMARKER = 294,
     _FILLVALUE = 295,
     _FORMAT = 296,
     _STORAGE = 297,
     _CHUNKSIZES = 298,
     _DEFLATELEVEL = 299,
     _SHUFFLE = 300,
     _ENDIANNESS = 301,
     _NOFILL = 302,
     _FLETCHER32 = 303,
     DATASETID = 304
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 124 "ncgen.y"

Symbol* sym;
unsigned long  size; /* allow for zero size to indicate e.g. UNLIMITED*/
long           mark; /* track indices into the sequence*/
int            nctype; /* for tracking attribute list type*/
Datalist*      datalist;
Constant       constant;



/* Line 214 of yacc.c  */
#line 290 "ncgen.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 302 "ncgen.tab.c"

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
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   343

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  59
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  61
/* YYNRULES -- Number of rules.  */
#define YYNRULES  141
/* YYNRULES -- Number of states.  */
#define YYNSTATES  238

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   304

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      55,    56,    57,     2,    53,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    58,    52,
       2,    54,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,     2,    51,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     7,    12,    18,    19,    22,    23,    24,
      34,    35,    37,    40,    42,    45,    47,    49,    52,    55,
      58,    61,    64,    65,    67,    74,    76,    80,    84,    90,
      96,   102,   105,   109,   112,   114,   116,   118,   120,   122,
     124,   126,   128,   130,   132,   134,   135,   137,   140,   143,
     147,   149,   151,   153,   157,   161,   165,   169,   173,   175,
     176,   178,   181,   184,   188,   190,   192,   195,   197,   201,
     204,   205,   209,   211,   215,   217,   219,   223,   226,   227,
     231,   233,   237,   239,   241,   243,   245,   247,   249,   250,
     254,   259,   266,   272,   278,   285,   291,   297,   303,   309,
     315,   321,   327,   332,   334,   336,   337,   339,   342,   345,
     349,   353,   355,   357,   358,   360,   364,   366,   370,   372,
     374,   376,   378,   380,   382,   384,   386,   388,   390,   392,
     394,   396,   398,   400,   402,   406,   408,   410,   412,   414,
     416,   418
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      60,     0,    -1,    30,    49,    61,    -1,    50,    62,    63,
      51,    -1,   105,    67,    82,    88,   108,    -1,    -1,    63,
      64,    -1,    -1,    -1,    37,    15,    50,    65,    62,    63,
      66,    51,   105,    -1,    -1,    32,    -1,    32,    68,    -1,
      70,    -1,    68,    70,    -1,    15,    -1,    71,    -1,   106,
      52,    -1,    73,    72,    -1,    78,    72,    -1,    77,    72,
      -1,    76,    72,    -1,    -1,    52,    -1,    81,    34,    69,
      50,    74,    51,    -1,    75,    -1,    74,    53,    75,    -1,
      15,    54,   115,    -1,    35,    55,    20,    56,    69,    -1,
     103,    55,    57,    56,    69,    -1,    33,    69,    50,    79,
      51,    -1,    80,    52,    -1,    79,    80,    52,    -1,   103,
      97,    -1,     4,    -1,     5,    -1,     6,    -1,     7,    -1,
       8,    -1,     9,    -1,    10,    -1,    11,    -1,    12,    -1,
      13,    -1,    14,    -1,    -1,    28,    -1,    28,    83,    -1,
      84,    52,    -1,    83,    84,    52,    -1,    85,    -1,   106,
      -1,    86,    -1,    85,    53,    86,    -1,    87,    54,    24,
      -1,    87,    54,    20,    -1,    87,    54,    27,    -1,    87,
      54,     3,    -1,    15,    -1,    -1,    29,    -1,    29,    89,
      -1,    90,    52,    -1,    89,    90,    52,    -1,    91,    -1,
     106,    -1,   103,    92,    -1,    93,    -1,    92,    53,    93,
      -1,    15,    94,    -1,    -1,    55,    95,    56,    -1,    96,
      -1,    95,    53,    96,    -1,   107,    -1,    98,    -1,    97,
      53,    98,    -1,    15,    99,    -1,    -1,    55,   100,    56,
      -1,   101,    -1,   100,    53,   101,    -1,    24,    -1,    20,
      -1,   104,    -1,   104,    -1,   107,    -1,    81,    -1,    -1,
     106,    52,   105,    -1,    58,    15,    54,   111,    -1,   103,
     104,    58,    15,    54,   111,    -1,   104,    58,    15,    54,
     111,    -1,   104,    58,    40,    54,   111,    -1,   103,   104,
      58,    40,    54,   111,    -1,   104,    58,    42,    54,   118,
      -1,   104,    58,    43,    54,   116,    -1,   104,    58,    48,
      54,   119,    -1,   104,    58,    44,    54,   117,    -1,   104,
      58,    45,    54,   119,    -1,   104,    58,    46,    54,   118,
      -1,   104,    58,    47,    54,   119,    -1,    58,    41,    54,
     118,    -1,    15,    -1,    38,    -1,    -1,    31,    -1,    31,
     109,    -1,   110,    52,    -1,   109,   110,    52,    -1,   102,
      54,   111,    -1,   112,    -1,   113,    -1,    -1,   114,    -1,
     111,    53,   114,    -1,   115,    -1,    50,   111,    51,    -1,
      17,    -1,    18,    -1,    19,    -1,    20,    -1,    21,    -1,
      22,    -1,    23,    -1,    24,    -1,    25,    -1,    26,    -1,
      27,    -1,    16,    -1,    36,    -1,   107,    -1,    39,    -1,
     117,    -1,   116,    53,   117,    -1,    20,    -1,    24,    -1,
      21,    -1,    25,    -1,    16,    -1,   118,    -1,   117,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   199,   199,   205,   212,   219,   219,   222,   231,   221,
     236,   237,   238,   242,   242,   244,   254,   254,   257,   258,
     259,   260,   263,   263,   266,   296,   298,   315,   324,   336,
     350,   383,   384,   387,   401,   402,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   414,   415,   416,   419,   420,
     423,   423,   425,   426,   430,   434,   442,   452,   464,   476,
     477,   478,   481,   482,   485,   485,   487,   509,   513,   517,
     544,   545,   548,   549,   553,   567,   571,   576,   605,   606,
     610,   611,   616,   626,   646,   657,   668,   687,   694,   694,
     697,   699,   708,   719,   721,   723,   725,   727,   729,   731,
     733,   735,   737,   744,   750,   759,   760,   761,   764,   765,
     768,   772,   773,   777,   781,   782,   787,   788,   792,   793,
     794,   795,   796,   797,   798,   799,   800,   801,   802,   803,
     804,   805,   806,   810,   811,   815,   817,   819,   821,   826,
     830,   831
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NC_UNLIMITED_K", "CHAR_K", "BYTE_K",
  "SHORT_K", "INT_K", "FLOAT_K", "DOUBLE_K", "UBYTE_K", "USHORT_K",
  "UINT_K", "INT64_K", "UINT64_K", "IDENT", "TERMSTRING", "CHAR_CONST",
  "BYTE_CONST", "SHORT_CONST", "INT_CONST", "INT64_CONST", "UBYTE_CONST",
  "USHORT_CONST", "UINT_CONST", "UINT64_CONST", "FLOAT_CONST",
  "DOUBLE_CONST", "DIMENSIONS", "VARIABLES", "NETCDF", "DATA", "TYPES",
  "COMPOUND", "ENUM", "OPAQUE", "OPAQUESTRING", "GROUP", "PATH",
  "FILLMARKER", "_FILLVALUE", "_FORMAT", "_STORAGE", "_CHUNKSIZES",
  "_DEFLATELEVEL", "_SHUFFLE", "_ENDIANNESS", "_NOFILL", "_FLETCHER32",
  "DATASETID", "'{'", "'}'", "';'", "','", "'='", "'('", "')'", "'*'",
  "':'", "$accept", "ncdesc", "rootgroup", "groupbody", "subgrouplist",
  "namedgroup", "$@1", "$@2", "typesection", "typedecls", "typename",
  "type_or_attr_decl", "typedecl", "optsemicolon", "enumdecl",
  "enumidlist", "enumid", "opaquedecl", "vlendecl", "compounddecl",
  "fields", "field", "primtype", "dimsection", "dimdecls",
  "dim_or_attr_decl", "dimdeclist", "dimdecl", "dimd", "vasection",
  "vadecls", "vadecl_or_attr", "vardecl", "varlist", "varspec", "dimspec",
  "dimlist", "dimref", "fieldlist", "fieldspec", "fielddimspec",
  "fielddimlist", "fielddim", "varref", "typeref", "type_var_ref",
  "attrdecllist", "attrdecl", "path", "datasection", "datadecls",
  "datadecl", "datalist", "datalist0", "datalist1", "dataitem",
  "constdata", "intlist", "constint", "conststring", "constbool", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     123,   125,    59,    44,    61,    40,    41,    42,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    59,    60,    61,    62,    63,    63,    65,    66,    64,
      67,    67,    67,    68,    68,    69,    70,    70,    71,    71,
      71,    71,    72,    72,    73,    74,    74,    75,    76,    77,
      78,    79,    79,    80,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    81,    82,    82,    82,    83,    83,
      84,    84,    85,    85,    86,    86,    86,    86,    87,    88,
      88,    88,    89,    89,    90,    90,    91,    92,    92,    93,
      94,    94,    95,    95,    96,    97,    97,    98,    99,    99,
     100,   100,   101,   101,   102,   103,   104,   104,   105,   105,
     106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
     106,   106,   106,   107,   107,   108,   108,   108,   109,   109,
     110,   111,   111,   112,   113,   113,   114,   114,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   116,   116,   117,   117,   117,   117,   118,
     119,   119
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     4,     5,     0,     2,     0,     0,     9,
       0,     1,     2,     1,     2,     1,     1,     2,     2,     2,
       2,     2,     0,     1,     6,     1,     3,     3,     5,     5,
       5,     2,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     2,     2,     3,
       1,     1,     1,     3,     3,     3,     3,     3,     1,     0,
       1,     2,     2,     3,     1,     1,     2,     1,     3,     2,
       0,     3,     1,     3,     1,     1,     3,     2,     0,     3,
       1,     3,     1,     1,     1,     1,     1,     1,     0,     3,
       4,     6,     5,     5,     6,     5,     5,     5,     5,     5,
       5,     5,     4,     1,     1,     0,     1,     2,     2,     3,
       3,     1,     1,     0,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     3,     1,     1,     1,     1,     1,
       1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,    88,     2,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,   103,   104,
       0,     5,    87,     0,    85,    10,     0,    86,     0,     0,
       0,     0,     0,    11,    45,    88,   113,     0,     0,     3,
       6,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    12,    13,    16,    22,    22,    22,    22,
      87,     0,     0,    46,    59,    89,   129,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   130,   132,
     113,   131,    90,   111,   112,   114,   116,   139,   102,     0,
       0,     0,   113,   113,     0,     0,     0,     0,     0,     0,
       0,    15,     0,     0,    14,    23,    18,    21,    20,    19,
       0,     0,    17,   103,    47,     0,    50,    52,     0,    51,
      60,   105,     0,     0,     7,   113,   113,    92,    93,    95,
     135,   137,   136,   138,    96,   133,    98,   141,   140,    99,
     100,   101,    97,     0,     0,     0,     0,     0,    48,     0,
       0,    61,     0,    64,     0,    65,   106,     4,   117,   115,
      88,    91,    94,     0,     0,     0,     0,    85,     0,     0,
       0,    49,    58,    53,    57,    55,    54,    56,     0,    62,
      70,    66,    67,     0,    84,   107,     0,     5,   134,    30,
       0,    31,    78,    33,    75,    28,     0,     0,    25,    29,
      63,     0,    69,     0,   113,     0,   108,     8,    32,     0,
      77,     0,     0,    24,     0,     0,    72,    74,    70,    68,
     110,   109,     0,    83,    82,     0,    80,    76,    27,    26,
       0,    71,    88,     0,    79,    73,     9,    81
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     6,    21,    30,    40,   160,   222,    34,    53,
     102,    54,    55,   106,    56,   197,   198,    57,    58,    59,
     164,   165,    22,    64,   114,   115,   116,   117,   118,   121,
     151,   152,   153,   181,   182,   202,   215,   216,   193,   194,
     210,   225,   226,   183,    23,    24,    25,    26,    27,   157,
     185,   186,    82,    83,    84,    85,    86,   134,   137,   138,
     139
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -101
static const yytype_int16 yypact[] =
{
     -17,   -20,    23,   -19,  -101,   141,  -101,  -101,  -101,  -101,
    -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,
      -6,  -101,  -101,   268,    -1,    22,    11,  -101,    32,    33,
     -23,     9,   129,    70,    62,   141,   221,    79,    81,  -101,
    -101,    -4,    43,    44,    45,    46,    47,    48,    50,    52,
      53,    94,    55,    70,  -101,  -101,    59,    59,    59,    59,
      78,   197,    61,   176,    85,  -101,  -101,  -101,  -101,  -101,
    -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,  -101,
     221,  -101,    64,  -101,  -101,  -101,  -101,  -101,  -101,    65,
      66,    68,   221,   221,    79,     0,     0,    31,    79,    31,
      31,  -101,    69,    98,  -101,  -101,  -101,  -101,  -101,  -101,
      94,    72,  -101,    76,   176,    71,    73,  -101,    77,  -101,
     141,   102,    13,   221,  -101,   221,   221,    64,    64,  -101,
    -101,  -101,  -101,  -101,    82,  -101,  -101,  -101,  -101,  -101,
    -101,  -101,  -101,   268,    80,    88,    83,    89,  -101,   125,
      14,   141,    91,  -101,   280,  -101,   268,  -101,  -101,  -101,
     141,    64,    64,     0,   211,   105,   143,  -101,    94,   144,
      94,  -101,  -101,  -101,  -101,  -101,  -101,  -101,   108,  -101,
     -25,   109,  -101,   107,  -101,   268,   112,  -101,  -101,  -101,
     113,  -101,   111,   115,  -101,  -101,   138,    20,  -101,  -101,
    -101,     4,  -101,   178,   221,   142,  -101,   158,  -101,    -2,
    -101,   143,   304,  -101,   144,   -13,  -101,  -101,   145,  -101,
      64,  -101,   147,  -101,  -101,    -3,  -101,  -101,  -101,  -101,
       4,  -101,   141,    -2,  -101,  -101,  -101,  -101
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -101,  -101,  -101,    67,    26,  -101,  -101,  -101,  -101,  -101,
    -100,   175,  -101,     3,  -101,  -101,    15,  -101,  -101,  -101,
    -101,    86,   -21,  -101,  -101,   116,  -101,    84,  -101,  -101,
    -101,   100,  -101,  -101,    28,  -101,  -101,     2,  -101,    42,
    -101,  -101,    21,  -101,   -27,   -22,   -35,   -26,   -34,  -101,
    -101,   114,   -77,  -101,  -101,   132,    49,  -101,   -91,   -29,
     -55
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -104
static const yytype_int16 yytable[] =
{
      65,    31,    81,   122,   135,   136,    61,    62,    88,    28,
     145,    90,    60,     1,    38,   127,   128,   174,   223,    18,
     130,   131,   224,     4,   132,   133,    61,    62,    39,     3,
     201,     5,    60,  -103,   175,    29,    91,   119,   176,    31,
     230,   177,    19,   231,   141,   142,    81,    87,   161,   162,
     233,   130,   131,   234,    33,   132,   133,    32,    81,    81,
     107,   108,   109,    35,   158,   129,   123,    41,   195,   140,
     199,   213,   188,   214,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    36,    37,   119,    81,
      63,    81,    81,   154,   155,    87,    89,    92,    93,    94,
      95,    96,    97,    51,    98,    52,    99,   100,    19,   101,
     103,   105,   110,   112,   120,   124,   166,   123,   144,   143,
     125,   167,   126,   148,   154,   155,   149,   220,    20,   146,
     -58,   150,    31,   156,   184,   163,   168,   166,   169,   170,
     172,   171,   167,   179,    42,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,   191,   192,   196,
     200,   204,   203,   184,   206,   208,   209,   217,   211,    43,
      81,    44,    45,    46,    47,    48,    49,    50,    81,    19,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,   113,   212,   218,   221,    38,   217,   236,   232,    20,
     201,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,   207,    19,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,   187,   104,   229,
     147,   219,   235,   173,    20,    19,    18,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    19,
     190,   178,   111,   227,   237,   159,     0,    78,     0,    19,
      79,   228,   189,     0,     0,     0,     0,     0,     0,     0,
       0,    80,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,   180,     0,     0,     0,   205,
       0,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    19,    18,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,     0,     0,     0,     0,     0,     0,     0,     0,
      78,     0,    19,    79
};

static const yytype_int16 yycheck[] =
{
      35,    23,    36,    80,    95,    96,    33,    33,    37,    15,
     110,    15,    33,    30,    37,    92,    93,     3,    20,    15,
      20,    21,    24,     0,    24,    25,    53,    53,    51,    49,
      55,    50,    53,    58,    20,    41,    40,    63,    24,    61,
      53,    27,    38,    56,    99,   100,    80,    16,   125,   126,
      53,    20,    21,    56,    32,    24,    25,    58,    92,    93,
      57,    58,    59,    52,    51,    94,    53,    58,   168,    98,
     170,    51,   163,    53,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    54,    54,   114,   123,
      28,   125,   126,   120,   120,    16,    15,    54,    54,    54,
      54,    54,    54,    33,    54,    35,    54,    54,    38,    15,
      55,    52,    34,    52,    29,    50,   143,    53,    20,    50,
      54,   143,    54,    52,   151,   151,    53,   204,    58,    57,
      54,    54,   154,    31,   156,    53,    56,   164,    50,    56,
      15,    52,   164,    52,    15,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    52,    15,    15,
      52,    54,    53,   185,    52,    52,    55,   201,    53,    40,
     204,    42,    43,    44,    45,    46,    47,    48,   212,    38,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    54,    15,    52,    37,   230,   232,    51,    58,
      55,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,   187,    38,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,   160,    53,   214,
     114,   203,   230,   149,    58,    38,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    38,
     164,   151,    55,   211,   233,   123,    -1,    36,    -1,    38,
      39,   212,    51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    50,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    -1,    -1,    -1,   185,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    -1,    38,    39
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    30,    60,    49,     0,    50,    61,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    38,
      58,    62,    81,   103,   104,   105,   106,   107,    15,    41,
      63,   104,    58,    32,    67,    52,    54,    54,    37,    51,
      64,    58,    15,    40,    42,    43,    44,    45,    46,    47,
      48,    33,    35,    68,    70,    71,    73,    76,    77,    78,
      81,   103,   106,    28,    82,   105,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    36,    39,
      50,   107,   111,   112,   113,   114,   115,    16,   118,    15,
      15,    40,    54,    54,    54,    54,    54,    54,    54,    54,
      54,    15,    69,    55,    70,    52,    72,    72,    72,    72,
      34,    55,    52,    15,    83,    84,    85,    86,    87,   106,
      29,    88,   111,    53,    50,    54,    54,   111,   111,   118,
      20,    21,    24,    25,   116,   117,   117,   117,   118,   119,
     118,   119,   119,    50,    20,    69,    57,    84,    52,    53,
      54,    89,    90,    91,   103,   106,    31,   108,    51,   114,
      65,   111,   111,    53,    79,    80,   103,   104,    56,    50,
      56,    52,    15,    86,     3,    20,    24,    27,    90,    52,
      15,    92,    93,   102,   104,   109,   110,    62,   117,    51,
      80,    52,    15,    97,    98,    69,    15,    74,    75,    69,
      52,    55,    94,    53,    54,   110,    52,    63,    52,    55,
      99,    53,    54,    51,    53,    95,    96,   107,    15,    93,
     111,    52,    66,    20,    24,   100,   101,    98,   115,    75,
      53,    56,    51,    53,    56,    96,   105,   101
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
#line 202 "ncgen.y"
    {if (derror_count > 0) exit(6);;}
    break;

  case 7:

/* Line 1464 of yacc.c  */
#line 222 "ncgen.y"
    {
		Symbol* id = (yyvsp[(2) - (3)].sym);
                markcdf4("Group specification");
		if(creategroup(id) == NULL) 
                    yyerror("duplicate group declaration within parent group for %s",
                                id->name);
            ;}
    break;

  case 8:

/* Line 1464 of yacc.c  */
#line 231 "ncgen.y"
    {listpop(groupstack);;}
    break;

  case 11:

/* Line 1464 of yacc.c  */
#line 237 "ncgen.y"
    {;}
    break;

  case 12:

/* Line 1464 of yacc.c  */
#line 239 "ncgen.y"
    {markcdf4("Type specification");;}
    break;

  case 15:

/* Line 1464 of yacc.c  */
#line 245 "ncgen.y"
    { /* Use when defining a type */
              (yyvsp[(1) - (1)].sym)->objectclass = NC_TYPE;
              if(dupobjectcheck(NC_TYPE,(yyvsp[(1) - (1)].sym)))
                    yyerror("duplicate type declaration for %s",
                            (yyvsp[(1) - (1)].sym)->name);
              listpush(typdefs,(elem_t)(yyvsp[(1) - (1)].sym));
	    ;}
    break;

  case 16:

/* Line 1464 of yacc.c  */
#line 254 "ncgen.y"
    {;}
    break;

  case 17:

/* Line 1464 of yacc.c  */
#line 254 "ncgen.y"
    {;}
    break;

  case 24:

/* Line 1464 of yacc.c  */
#line 268 "ncgen.y"
    {
		int i;
                addtogroup((yyvsp[(3) - (6)].sym)); /* sets prefix*/
                (yyvsp[(3) - (6)].sym)->objectclass=NC_TYPE;
                (yyvsp[(3) - (6)].sym)->subclass=NC_ENUM;
                (yyvsp[(3) - (6)].sym)->typ.basetype=(yyvsp[(1) - (6)].sym);
                (yyvsp[(3) - (6)].sym)->typ.size = (yyvsp[(1) - (6)].sym)->typ.size;
                (yyvsp[(3) - (6)].sym)->typ.alignment = (yyvsp[(1) - (6)].sym)->typ.alignment;
                stackbase=(yyvsp[(5) - (6)].mark);
                stacklen=listlength(stack);
                (yyvsp[(3) - (6)].sym)->subnodes = listnew();
                /* Variety of field fixups*/
		/* 1. add in the enum values*/
		/* 2. make this type be their container*/
		/* 3. make constant names visible in the group*/
		/* 4. set field basetype to be same as enum basetype*/
                for(i=stackbase;i<stacklen;i++) {
                   Symbol* eid = (Symbol*)listget(stack,i);
		   assert(eid->subclass == NC_ECONST);
		   addtogroup(eid);
                   listpush((yyvsp[(3) - (6)].sym)->subnodes,(elem_t)eid);
                   eid->container = (yyvsp[(3) - (6)].sym);
		   eid->typ.basetype = (yyvsp[(3) - (6)].sym)->typ.basetype;
                }               
                listsetlength(stack,stackbase);/* remove stack nodes*/
              ;}
    break;

  case 25:

/* Line 1464 of yacc.c  */
#line 297 "ncgen.y"
    {(yyval.mark)=listlength(stack); listpush(stack,(elem_t)(yyvsp[(1) - (1)].sym));;}
    break;

  case 26:

/* Line 1464 of yacc.c  */
#line 299 "ncgen.y"
    {
		    int i;
		    (yyval.mark)=(yyvsp[(1) - (3)].mark);
		    /* check for duplicates*/
		    stackbase=(yyvsp[(1) - (3)].mark);
		    stacklen=listlength(stack);
		    for(i=stackbase;i<stacklen;i++) {
		      Symbol* elem = (Symbol*)listget(stack,i);
		      if(strcmp((yyvsp[(3) - (3)].sym)->name,elem->name)==0)
  	                yyerror("duplicate enum declaration for %s",
        	                 elem->name);
		    }    	    
		    listpush(stack,(elem_t)(yyvsp[(3) - (3)].sym));
		;}
    break;

  case 27:

/* Line 1464 of yacc.c  */
#line 316 "ncgen.y"
    {
            (yyvsp[(1) - (3)].sym)->objectclass=NC_TYPE;
            (yyvsp[(1) - (3)].sym)->subclass=NC_ECONST;
            (yyvsp[(1) - (3)].sym)->typ.econst=(yyvsp[(3) - (3)].constant);
	    (yyval.sym)=(yyvsp[(1) - (3)].sym);
        ;}
    break;

  case 28:

/* Line 1464 of yacc.c  */
#line 325 "ncgen.y"
    {
		    vercheck(NC_OPAQUE);
                    addtogroup((yyvsp[(5) - (5)].sym)); /*sets prefix*/
                    (yyvsp[(5) - (5)].sym)->objectclass=NC_TYPE;
                    (yyvsp[(5) - (5)].sym)->subclass=NC_OPAQUE;
                    (yyvsp[(5) - (5)].sym)->typ.typecode=NC_OPAQUE;
                    (yyvsp[(5) - (5)].sym)->typ.size=int32_val;
                    (yyvsp[(5) - (5)].sym)->typ.alignment=nctypealignment(NC_OPAQUE);
                ;}
    break;

  case 29:

/* Line 1464 of yacc.c  */
#line 337 "ncgen.y"
    {
                    Symbol* basetype = (yyvsp[(1) - (5)].sym);
		    vercheck(NC_VLEN);
                    addtogroup((yyvsp[(5) - (5)].sym)); /*sets prefix*/
                    (yyvsp[(5) - (5)].sym)->objectclass=NC_TYPE;
                    (yyvsp[(5) - (5)].sym)->subclass=NC_VLEN;
                    (yyvsp[(5) - (5)].sym)->typ.basetype=basetype;
                    (yyvsp[(5) - (5)].sym)->typ.typecode=NC_VLEN;
                    (yyvsp[(5) - (5)].sym)->typ.size=VLENSIZE;
                    (yyvsp[(5) - (5)].sym)->typ.alignment=nctypealignment(NC_VLEN);
                ;}
    break;

  case 30:

/* Line 1464 of yacc.c  */
#line 351 "ncgen.y"
    {
	    int i,j;
	    vercheck(NC_COMPOUND);
            addtogroup((yyvsp[(2) - (5)].sym));
	    /* check for duplicate field names*/
	    stackbase=(yyvsp[(4) - (5)].mark);
	    stacklen=listlength(stack);
	    for(i=stackbase;i<stacklen;i++) {
	      Symbol* elem1 = (Symbol*)listget(stack,i);
	      for(j=i+1;j<stacklen;j++) {
	          Symbol* elem2 = (Symbol*)listget(stack,j);
	          if(strcmp(elem1->name,elem2->name)==0) {
	            yyerror("duplicate field declaration for %s",elem1->name);
		  }
	      }
	    }
	    (yyvsp[(2) - (5)].sym)->objectclass=NC_TYPE;
            (yyvsp[(2) - (5)].sym)->subclass=NC_COMPOUND;
            (yyvsp[(2) - (5)].sym)->typ.basetype=NULL;
            (yyvsp[(2) - (5)].sym)->typ.typecode=NC_COMPOUND;
	    (yyvsp[(2) - (5)].sym)->subnodes = listnew();
	    /* Add in the fields*/
	    for(i=stackbase;i<stacklen;i++) {
	        Symbol* fsym = (Symbol*)listget(stack,i);
		fsym->container = (yyvsp[(2) - (5)].sym);
 	        listpush((yyvsp[(2) - (5)].sym)->subnodes,(elem_t)fsym);
	    }    	    
	    listsetlength(stack,stackbase);/* remove stack nodes*/
          ;}
    break;

  case 31:

/* Line 1464 of yacc.c  */
#line 383 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (2)].mark);;}
    break;

  case 32:

/* Line 1464 of yacc.c  */
#line 384 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark);;}
    break;

  case 33:

/* Line 1464 of yacc.c  */
#line 388 "ncgen.y"
    {
	    int i;
	    (yyval.mark)=(yyvsp[(2) - (2)].mark);
	    stackbase=(yyvsp[(2) - (2)].mark);
	    stacklen=listlength(stack);
	    /* process each field in the fieldlist*/
            for(i=stackbase;i<stacklen;i++) {
                Symbol* f = (Symbol*)listget(stack,i);
		f->typ.basetype = (yyvsp[(1) - (2)].sym);
            }
        ;}
    break;

  case 34:

/* Line 1464 of yacc.c  */
#line 401 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_CHAR]; ;}
    break;

  case 35:

/* Line 1464 of yacc.c  */
#line 402 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_BYTE]; ;}
    break;

  case 36:

/* Line 1464 of yacc.c  */
#line 403 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_SHORT]; ;}
    break;

  case 37:

/* Line 1464 of yacc.c  */
#line 404 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_INT]; ;}
    break;

  case 38:

/* Line 1464 of yacc.c  */
#line 405 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_FLOAT]; ;}
    break;

  case 39:

/* Line 1464 of yacc.c  */
#line 406 "ncgen.y"
    { (yyval.sym) = primsymbols[NC_DOUBLE]; ;}
    break;

  case 40:

/* Line 1464 of yacc.c  */
#line 407 "ncgen.y"
    { vercheck(NC_UBYTE); (yyval.sym) = primsymbols[NC_UBYTE]; ;}
    break;

  case 41:

/* Line 1464 of yacc.c  */
#line 408 "ncgen.y"
    { vercheck(NC_USHORT); (yyval.sym) = primsymbols[NC_USHORT]; ;}
    break;

  case 42:

/* Line 1464 of yacc.c  */
#line 409 "ncgen.y"
    { vercheck(NC_UINT); (yyval.sym) = primsymbols[NC_UINT]; ;}
    break;

  case 43:

/* Line 1464 of yacc.c  */
#line 410 "ncgen.y"
    { vercheck(NC_INT64); (yyval.sym) = primsymbols[NC_INT64]; ;}
    break;

  case 44:

/* Line 1464 of yacc.c  */
#line 411 "ncgen.y"
    { vercheck(NC_UINT64); (yyval.sym) = primsymbols[NC_UINT64]; ;}
    break;

  case 46:

/* Line 1464 of yacc.c  */
#line 415 "ncgen.y"
    {;}
    break;

  case 47:

/* Line 1464 of yacc.c  */
#line 416 "ncgen.y"
    {;}
    break;

  case 50:

/* Line 1464 of yacc.c  */
#line 423 "ncgen.y"
    {;}
    break;

  case 51:

/* Line 1464 of yacc.c  */
#line 423 "ncgen.y"
    {;}
    break;

  case 54:

/* Line 1464 of yacc.c  */
#line 431 "ncgen.y"
    {
		(yyvsp[(1) - (3)].sym)->dim.declsize = (size_t)uint32_val;
	      ;}
    break;

  case 55:

/* Line 1464 of yacc.c  */
#line 435 "ncgen.y"
    {
		if(int32_val <= 0) {
		    derror("dimension size must be positive");
		    YYABORT;
		}
		(yyvsp[(1) - (3)].sym)->dim.declsize = (size_t)int32_val;
	      ;}
    break;

  case 56:

/* Line 1464 of yacc.c  */
#line 443 "ncgen.y"
    { /* for rare case where 2^31 < dimsize < 2^32 */
                       if (double_val <= 0)
                         yyerror("dimension length must be positive");
                       if (double_val > MAXFLOATDIM)
                         yyerror("dimension too large");
                       if (double_val - (size_t) double_val > 0)
                         yyerror("dimension length must be an integer");
                       (yyvsp[(1) - (3)].sym)->dim.declsize = (size_t)double_val;
                   ;}
    break;

  case 57:

/* Line 1464 of yacc.c  */
#line 453 "ncgen.y"
    {
                       if(usingclassic) {
	  	         /* check for multiple UNLIMITED decls*/
                         if(getunlimiteddim() != NULL)
			    markcdf4("Type specification");
			 setunlimiteddim((yyvsp[(1) - (3)].sym));
		       }
		       (yyvsp[(1) - (3)].sym)->dim.declsize = NC_UNLIMITED;
		   ;}
    break;

  case 58:

/* Line 1464 of yacc.c  */
#line 465 "ncgen.y"
    { 
                     (yyvsp[(1) - (1)].sym)->objectclass=NC_DIM;
                     if(dupobjectcheck(NC_DIM,(yyvsp[(1) - (1)].sym)))
                        yyerror( "Duplicate dimension declaration for %s",
                                (yyvsp[(1) - (1)].sym)->name);
		     addtogroup((yyvsp[(1) - (1)].sym));
		     (yyval.sym)=(yyvsp[(1) - (1)].sym);
		     listpush(dimdefs,(elem_t)(yyvsp[(1) - (1)].sym));
                   ;}
    break;

  case 60:

/* Line 1464 of yacc.c  */
#line 477 "ncgen.y"
    {;}
    break;

  case 61:

/* Line 1464 of yacc.c  */
#line 478 "ncgen.y"
    {;}
    break;

  case 64:

/* Line 1464 of yacc.c  */
#line 485 "ncgen.y"
    {;}
    break;

  case 65:

/* Line 1464 of yacc.c  */
#line 485 "ncgen.y"
    {;}
    break;

  case 66:

/* Line 1464 of yacc.c  */
#line 488 "ncgen.y"
    {
		    int i;
		    stackbase=(yyvsp[(2) - (2)].mark);
		    stacklen=listlength(stack);
		    /* process each variable in the varlist*/
	            for(i=stackbase;i<stacklen;i++) {
	                Symbol* sym = (Symbol*)listget(stack,i);
			sym->objectclass = NC_VAR;
		        if(dupobjectcheck(NC_VAR,sym)) {
                            yyerror("Duplicate variable declaration for %s",
                                    sym->name);
			} else {
		  	    sym->typ.basetype = (yyvsp[(1) - (2)].sym);
	                    addtogroup(sym);
		            listpush(vardefs,(elem_t)sym);
			}
		    }
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		;}
    break;

  case 67:

/* Line 1464 of yacc.c  */
#line 510 "ncgen.y"
    {(yyval.mark)=listlength(stack);
                 listpush(stack,(elem_t)(yyvsp[(1) - (1)].sym));
		;}
    break;

  case 68:

/* Line 1464 of yacc.c  */
#line 514 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(elem_t)(yyvsp[(3) - (3)].sym));;}
    break;

  case 69:

/* Line 1464 of yacc.c  */
#line 518 "ncgen.y"
    {
		    int i;
		    Dimset dimset;
		    stacklen=listlength(stack);
		    stackbase=(yyvsp[(2) - (2)].mark);
		    count = stacklen - stackbase;
		    if(count >= NC_MAX_VAR_DIMS) {
			yyerror("%s has too many dimensions",(yyvsp[(1) - (2)].sym)->name);
			count = NC_MAX_VAR_DIMS - 1;
			stacklen = stackbase + count;
		    }
  	            dimset.ndims = count;
		    /* extract the actual dimensions*/
		    if(dimset.ndims > 0) {
		        for(i=0;i<count;i++) {
			    Symbol* dsym = (Symbol*)listget(stack,stackbase+i);
			    dimset.dimsyms[i] = dsym;
			}
			(yyvsp[(1) - (2)].sym)->typ.dimset = dimset;
		    }
		    (yyvsp[(1) - (2)].sym)->typ.basetype = NULL; /* not yet known*/
                    (yyvsp[(1) - (2)].sym)->objectclass=NC_VAR;
		    listsetlength(stack,stackbase);/* remove stack nodes*/
		    ;}
    break;

  case 70:

/* Line 1464 of yacc.c  */
#line 544 "ncgen.y"
    {(yyval.mark)=listlength(stack);;}
    break;

  case 71:

/* Line 1464 of yacc.c  */
#line 545 "ncgen.y"
    {(yyval.mark)=(yyvsp[(2) - (3)].mark);;}
    break;

  case 72:

/* Line 1464 of yacc.c  */
#line 548 "ncgen.y"
    {(yyval.mark)=listlength(stack); listpush(stack,(elem_t)(yyvsp[(1) - (1)].sym));;}
    break;

  case 73:

/* Line 1464 of yacc.c  */
#line 550 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(elem_t)(yyvsp[(3) - (3)].sym));;}
    break;

  case 74:

/* Line 1464 of yacc.c  */
#line 554 "ncgen.y"
    {Symbol* dimsym = (yyvsp[(1) - (1)].sym);
		dimsym->objectclass = NC_DIM;
		/* Find the actual dimension*/
		dimsym = locate(dimsym);
		if(dimsym == NULL) {
		    derror("Undefined or forward referenced dimension: %s",(yyvsp[(1) - (1)].sym)->name);
		    YYABORT;
		}
		(yyval.sym)=dimsym;
	    ;}
    break;

  case 75:

/* Line 1464 of yacc.c  */
#line 568 "ncgen.y"
    {(yyval.mark)=listlength(stack);
             listpush(stack,(elem_t)(yyvsp[(1) - (1)].sym));
	    ;}
    break;

  case 76:

/* Line 1464 of yacc.c  */
#line 572 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(elem_t)(yyvsp[(3) - (3)].sym));;}
    break;

  case 77:

/* Line 1464 of yacc.c  */
#line 577 "ncgen.y"
    {
		int i;
		Dimset dimset;
		stackbase=(yyvsp[(2) - (2)].mark);
		stacklen=listlength(stack);
		count = stacklen - stackbase;
		if(count >= NC_MAX_VAR_DIMS) {
		    yyerror("%s has too many dimensions",(yyvsp[(1) - (2)].sym)->name);
		    count = NC_MAX_VAR_DIMS - 1;
		    stacklen = stackbase + count;
		}
  	        dimset.ndims = count;
		if(count > 0) {
		    /* extract the actual dimensions*/
		    for(i=0;i<count;i++) {
		        Symbol* dsym = (Symbol*)listget(stack,stackbase+i);
		        dimset.dimsyms[i] = dsym;
		    }
		    (yyvsp[(1) - (2)].sym)->typ.dimset = dimset;
		}
		(yyvsp[(1) - (2)].sym)->typ.basetype = NULL; /* not yet known*/
                (yyvsp[(1) - (2)].sym)->objectclass=NC_TYPE;
                (yyvsp[(1) - (2)].sym)->subclass=NC_FIELD;
		listsetlength(stack,stackbase);/* remove stack nodes*/
		(yyval.sym) = (yyvsp[(1) - (2)].sym);
	    ;}
    break;

  case 78:

/* Line 1464 of yacc.c  */
#line 605 "ncgen.y"
    {(yyval.mark)=listlength(stack);;}
    break;

  case 79:

/* Line 1464 of yacc.c  */
#line 606 "ncgen.y"
    {(yyval.mark)=(yyvsp[(2) - (3)].mark);;}
    break;

  case 80:

/* Line 1464 of yacc.c  */
#line 610 "ncgen.y"
    {(yyval.mark)=listlength(stack); listpush(stack,(elem_t)(yyvsp[(1) - (1)].sym));;}
    break;

  case 81:

/* Line 1464 of yacc.c  */
#line 612 "ncgen.y"
    {(yyval.mark)=(yyvsp[(1) - (3)].mark); listpush(stack,(elem_t)(yyvsp[(3) - (3)].sym));;}
    break;

  case 82:

/* Line 1464 of yacc.c  */
#line 617 "ncgen.y"
    {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     sprintf(anon,"const%u",uint32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = uint32_val;
	    ;}
    break;

  case 83:

/* Line 1464 of yacc.c  */
#line 627 "ncgen.y"
    {  /* Anonymous integer dimension.
	         Can only occur in type definitions*/
	     char anon[32];
	     if(int32_val <= 0) {
		derror("field dimension must be positive");
		YYABORT;
	     }
	     sprintf(anon,"const%d",int32_val);
	     (yyval.sym) = install(anon);
	     (yyval.sym)->objectclass = NC_DIM;
	     (yyval.sym)->dim.isconstant = 1;
	     (yyval.sym)->dim.declsize = int32_val;
	    ;}
    break;

  case 84:

/* Line 1464 of yacc.c  */
#line 647 "ncgen.y"
    {Symbol* vsym = (yyvsp[(1) - (1)].sym);
		if(vsym->objectclass != NC_VAR) {
		    derror("Undefined or forward referenced variable: %s",vsym->name);
		    YYABORT;
		}
		(yyval.sym)=vsym;
	    ;}
    break;

  case 85:

/* Line 1464 of yacc.c  */
#line 658 "ncgen.y"
    {Symbol* tsym = (yyvsp[(1) - (1)].sym);
		if(tsym->objectclass != NC_TYPE) {
		    derror("Undefined or forward referenced type: %s",tsym->name);
		    YYABORT;
		}
		(yyval.sym)=tsym;
	    ;}
    break;

  case 86:

/* Line 1464 of yacc.c  */
#line 669 "ncgen.y"
    {Symbol* tvsym = (yyvsp[(1) - (1)].sym); Symbol* sym;
		/* disambiguate*/
		tvsym->objectclass = NC_VAR;
		sym = locate(tvsym);
		if(sym == NULL) {
		    tvsym->objectclass = NC_TYPE;
		    sym = locate(tvsym);
		    if(tvsym == NULL) {
		        derror("Undefined or forward referenced name: %s",(yyvsp[(1) - (1)].sym)->name);
		        YYABORT;
		    } else tvsym = sym;
		} else tvsym = sym;
		if(tvsym == NULL) {
		    derror("Undefined name: %s",(yyvsp[(1) - (1)].sym)->name);
		    YYABORT;
		}
		(yyval.sym)=tvsym;
	    ;}
    break;

  case 87:

/* Line 1464 of yacc.c  */
#line 687 "ncgen.y"
    {(yyval.sym)=(yyvsp[(1) - (1)].sym);;}
    break;

  case 88:

/* Line 1464 of yacc.c  */
#line 694 "ncgen.y"
    {;}
    break;

  case 89:

/* Line 1464 of yacc.c  */
#line 694 "ncgen.y"
    {;}
    break;

  case 90:

/* Line 1464 of yacc.c  */
#line 698 "ncgen.y"
    { (yyval.sym)=makeattribute((yyvsp[(2) - (4)].sym),NULL,NULL,(yyvsp[(4) - (4)].datalist),ATTRGLOBAL);;}
    break;

  case 91:

/* Line 1464 of yacc.c  */
#line 700 "ncgen.y"
    {Symbol* tsym = (yyvsp[(1) - (6)].sym); Symbol* vsym = (yyvsp[(2) - (6)].sym); Symbol* asym = (yyvsp[(4) - (6)].sym);
		if(vsym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,vsym,tsym,(yyvsp[(6) - (6)].datalist),ATTRVAR);
		} else {
		    derror("Doubly typed attribute: %s",asym->name);
		    YYABORT;
		}
	    ;}
    break;

  case 92:

/* Line 1464 of yacc.c  */
#line 709 "ncgen.y"
    {Symbol* sym = (yyvsp[(1) - (5)].sym); Symbol* asym = (yyvsp[(3) - (5)].sym);
		if(sym->objectclass == NC_VAR) {
		    (yyval.sym)=makeattribute(asym,sym,NULL,(yyvsp[(5) - (5)].datalist),ATTRVAR);
		} else if(sym->objectclass == NC_TYPE) {
		    (yyval.sym)=makeattribute(asym,NULL,sym,(yyvsp[(5) - (5)].datalist),ATTRGLOBAL);
		} else {
		    derror("Attribute prefix not a variable or type: %s",asym->name);
		    YYABORT;
		}
	    ;}
    break;

  case 93:

/* Line 1464 of yacc.c  */
#line 720 "ncgen.y"
    {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)(yyvsp[(5) - (5)].datalist),0);;}
    break;

  case 94:

/* Line 1464 of yacc.c  */
#line 722 "ncgen.y"
    {(yyval.sym) = makespecial(_FILLVALUE_FLAG,(yyvsp[(2) - (6)].sym),(yyvsp[(1) - (6)].sym),(void*)(yyvsp[(6) - (6)].datalist),0);;}
    break;

  case 95:

/* Line 1464 of yacc.c  */
#line 724 "ncgen.y"
    {(yyval.sym) = makespecial(_STORAGE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);;}
    break;

  case 96:

/* Line 1464 of yacc.c  */
#line 726 "ncgen.y"
    {(yyval.sym) = makespecial(_CHUNKSIZES_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)(yyvsp[(5) - (5)].datalist),0);;}
    break;

  case 97:

/* Line 1464 of yacc.c  */
#line 728 "ncgen.y"
    {(yyval.sym) = makespecial(_FLETCHER32_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);;}
    break;

  case 98:

/* Line 1464 of yacc.c  */
#line 730 "ncgen.y"
    {(yyval.sym) = makespecial(_DEFLATE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);;}
    break;

  case 99:

/* Line 1464 of yacc.c  */
#line 732 "ncgen.y"
    {(yyval.sym) = makespecial(_SHUFFLE_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);;}
    break;

  case 100:

/* Line 1464 of yacc.c  */
#line 734 "ncgen.y"
    {(yyval.sym) = makespecial(_ENDIAN_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);;}
    break;

  case 101:

/* Line 1464 of yacc.c  */
#line 736 "ncgen.y"
    {(yyval.sym) = makespecial(_NOFILL_FLAG,(yyvsp[(1) - (5)].sym),NULL,(void*)&(yyvsp[(5) - (5)].constant),1);;}
    break;

  case 102:

/* Line 1464 of yacc.c  */
#line 738 "ncgen.y"
    {
(yyval.sym) = makespecial(_FORMAT_FLAG,NULL,NULL,(void*)&(yyvsp[(4) - (4)].constant),1);
;}
    break;

  case 103:

/* Line 1464 of yacc.c  */
#line 745 "ncgen.y"
    {
	        (yyval.sym)=(yyvsp[(1) - (1)].sym);
                (yyvsp[(1) - (1)].sym)->is_ref=1;
                setpathcurrent((yyvsp[(1) - (1)].sym));
	    ;}
    break;

  case 104:

/* Line 1464 of yacc.c  */
#line 751 "ncgen.y"
    {
	        (yyval.sym)=(yyvsp[(1) - (1)].sym);
                (yyvsp[(1) - (1)].sym)->is_ref=1;
                (yyvsp[(1) - (1)].sym)->is_prefixed=1;
	        /* path is set in ncgen.l*/
	    ;}
    break;

  case 106:

/* Line 1464 of yacc.c  */
#line 760 "ncgen.y"
    {;}
    break;

  case 107:

/* Line 1464 of yacc.c  */
#line 761 "ncgen.y"
    {;}
    break;

  case 110:

/* Line 1464 of yacc.c  */
#line 769 "ncgen.y"
    {(yyvsp[(1) - (3)].sym)->data = (yyvsp[(3) - (3)].datalist);;}
    break;

  case 111:

/* Line 1464 of yacc.c  */
#line 772 "ncgen.y"
    {(yyval.datalist) = (yyvsp[(1) - (1)].datalist);;}
    break;

  case 112:

/* Line 1464 of yacc.c  */
#line 773 "ncgen.y"
    {(yyval.datalist) = (yyvsp[(1) - (1)].datalist);;}
    break;

  case 113:

/* Line 1464 of yacc.c  */
#line 777 "ncgen.y"
    {(yyval.datalist) = builddatalist(0);;}
    break;

  case 114:

/* Line 1464 of yacc.c  */
#line 781 "ncgen.y"
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[(1) - (1)].constant)));;}
    break;

  case 115:

/* Line 1464 of yacc.c  */
#line 783 "ncgen.y"
    {datalistextend((yyvsp[(1) - (3)].datalist),&((yyvsp[(3) - (3)].constant))); (yyval.datalist)=(yyvsp[(1) - (3)].datalist);;}
    break;

  case 116:

/* Line 1464 of yacc.c  */
#line 787 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);;}
    break;

  case 117:

/* Line 1464 of yacc.c  */
#line 788 "ncgen.y"
    {(yyval.constant)=builddatasublist((yyvsp[(2) - (3)].datalist));;}
    break;

  case 118:

/* Line 1464 of yacc.c  */
#line 792 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_CHAR);;}
    break;

  case 119:

/* Line 1464 of yacc.c  */
#line 793 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_BYTE);;}
    break;

  case 120:

/* Line 1464 of yacc.c  */
#line 794 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_SHORT);;}
    break;

  case 121:

/* Line 1464 of yacc.c  */
#line 795 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT);;}
    break;

  case 122:

/* Line 1464 of yacc.c  */
#line 796 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT64);;}
    break;

  case 123:

/* Line 1464 of yacc.c  */
#line 797 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UBYTE);;}
    break;

  case 124:

/* Line 1464 of yacc.c  */
#line 798 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_USHORT);;}
    break;

  case 125:

/* Line 1464 of yacc.c  */
#line 799 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT);;}
    break;

  case 126:

/* Line 1464 of yacc.c  */
#line 800 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT64);;}
    break;

  case 127:

/* Line 1464 of yacc.c  */
#line 801 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_FLOAT);;}
    break;

  case 128:

/* Line 1464 of yacc.c  */
#line 802 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_DOUBLE);;}
    break;

  case 129:

/* Line 1464 of yacc.c  */
#line 803 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_STRING);;}
    break;

  case 130:

/* Line 1464 of yacc.c  */
#line 804 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_OPAQUE);;}
    break;

  case 131:

/* Line 1464 of yacc.c  */
#line 805 "ncgen.y"
    {(yyval.constant)=makeenumconst((yyvsp[(1) - (1)].sym));;}
    break;

  case 132:

/* Line 1464 of yacc.c  */
#line 806 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_FILLVALUE);;}
    break;

  case 133:

/* Line 1464 of yacc.c  */
#line 810 "ncgen.y"
    {(yyval.datalist) = builddatalist(0); datalistextend((yyval.datalist),&((yyvsp[(1) - (1)].constant)));;}
    break;

  case 134:

/* Line 1464 of yacc.c  */
#line 811 "ncgen.y"
    {(yyval.datalist)=(yyvsp[(1) - (3)].datalist); datalistextend((yyvsp[(1) - (3)].datalist),&((yyvsp[(3) - (3)].constant)));;}
    break;

  case 135:

/* Line 1464 of yacc.c  */
#line 816 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT);;}
    break;

  case 136:

/* Line 1464 of yacc.c  */
#line 818 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT);;}
    break;

  case 137:

/* Line 1464 of yacc.c  */
#line 820 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_INT64);;}
    break;

  case 138:

/* Line 1464 of yacc.c  */
#line 822 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_UINT64);;}
    break;

  case 139:

/* Line 1464 of yacc.c  */
#line 826 "ncgen.y"
    {(yyval.constant)=makeconstdata(NC_STRING);;}
    break;

  case 140:

/* Line 1464 of yacc.c  */
#line 830 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);;}
    break;

  case 141:

/* Line 1464 of yacc.c  */
#line 831 "ncgen.y"
    {(yyval.constant)=(yyvsp[(1) - (1)].constant);;}
    break;



/* Line 1464 of yacc.c  */
#line 2867 "ncgen.tab.c"
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
#line 835 "ncgen.y"


#ifndef NO_STDARG
static void
yyerror(const char *fmt, ...)
#else
static void
yyerror(fmt,va_alist) const char* fmt; va_dcl
#endif
{
    va_list argv;
    vastart(argv,fmt);
    (void)fprintf(stderr,"%s: %s line %d: ", progname, cdlname, lineno);
    vderror(fmt,argv);
}

/* undefine yywrap macro, in case we are using bison instead of yacc */
#ifdef yywrap
#undef yywrap
#endif

static int
ncgwrap(void)                    /* returns 1 on EOF if no more input */
{
    return  1;
}

/* get lexical input routine generated by lex  */
#include "ncgenyy.c"

/* Really should init our data within this file */
void
parse_init(void)
{
    int i;
    derror_count=0;
    opaqueid = 0;
    arrayuid = 0;
    symlist = NULL;
    stack = listnew();
    groupstack = listnew();
    consttype = NC_NAT;
    grpdefs = listnew();
    dimdefs = listnew();
    attdefs = listnew();
    gattdefs = listnew();
    xattdefs = listnew();
    typdefs = listnew();
    vardefs = listnew();
    condefs = listnew();
    tmp = listnew();
    createrootgroup();
    /* Create the primitive types */
    for(i=NC_NAT+1;i<=NC_STRING;i++) {
        primsymbols[i] = makeprimitivetype(i);
    }
    lex_init();
}

static Symbol*
makeprimitivetype(nc_type nctype)
{
    Symbol* sym = install(primtypenames[nctype]);
    sym->objectclass=NC_TYPE;
    sym->subclass=NC_PRIM;
    sym->ncid = nctype;
    sym->typ.basetype = NULL;
    sym->typ.typecode = nctype;
    sym->typ.size = ncsize(nctype);
    sym->typ.nelems = 1;
    sym->typ.alignment = nctypealignment(nctype);
    sym->prefix = listnew();
    return sym;
}

/* Symbol table operations for ncgen tool */
/* install sname in symbol table even if it is already there */
Symbol*
install(const char *sname)
{
    Symbol* sp;
    sp = (Symbol*) emalloc (sizeof (struct Symbol));
    memset((void*)sp,0,sizeof(struct Symbol));    
    sp->name = nulldup(sname);
    sp->next = symlist;
    sp->lineno = lineno;
    sp->location = currentgroup();
    sp->container = currentgroup();
    symlist = sp;
    return sp;
}


static void
setunlimiteddim(Symbol* udim)
{
    rootgroup->grp.unlimiteddim = udim;
}

static Symbol*
getunlimiteddim(void)
{
    return rootgroup->grp.unlimiteddim;
}

static Symbol*
currentgroup(void)
{
    if(listlength(groupstack) == 0) return rootgroup;
    return (Symbol*)listtop(groupstack);
}

static Symbol*
createrootgroup(void)
{
    Symbol* gsym = install(ROOTGROUPNAME);
    gsym->objectclass = NC_GRP;
    gsym->container = NULL;
    gsym->subnodes = listnew();
    gsym->grp.is_root = 1;
    gsym->grp.unlimiteddim = NULL;
    gsym->prefix = listnew();
    listpush(grpdefs,(elem_t)gsym);
    rootgroup = gsym;
    return gsym;
}

static Symbol*
creategroup(Symbol * gsym)
{
    /* See if this group already exists in currentgroup */
    gsym->objectclass = NC_GRP;
    if(dupobjectcheck(NC_GRP,gsym)) {
        derror("Duplicate group name in same scope: %s",gsym->name);
	return NULL;
    }
    addtogroup(gsym);
    gsym->subnodes = listnew();
    listpush(groupstack,(elem_t)gsym);
    listpush(grpdefs,(elem_t)gsym);
    return gsym;
}

static Constant
makeconstdata(nc_type nctype)
{
    Constant con = nullconstant;
    consttype = nctype;
    con.nctype = nctype;
    con.lineno = lineno;
    switch (nctype) {
	case NC_CHAR: con.value.charv = char_val; break;
        case NC_BYTE: con.value.int8v = byte_val; break;
        case NC_SHORT: con.value.int16v = int16_val; break;
        case NC_INT: con.value.int32v = int32_val; break;
        case NC_FLOAT:
	    con.value.floatv = float_val;
	    break;
        case NC_DOUBLE:
	    con.value.doublev = double_val;
	    break;
        case NC_STRING: { /* convert to a set of chars*/
	    int len;
	    len = strlen(lextext);
	    con.value.stringv.len = len;
	    con.value.stringv.stringv = nulldup(lextext);
	    }
	    break;

	/* Allow these constants even in netcdf-3 */
        case NC_UBYTE: con.value.uint8v = ubyte_val; break;
        case NC_USHORT: con.value.uint16v = uint16_val; break;
        case NC_UINT: con.value.uint32v = uint32_val; break;
        case NC_INT64: con.value.int64v = int64_val; break;
        case NC_UINT64: con.value.uint64v = uint64_val; break;

#ifdef USE_NETCDF4
	case NC_OPAQUE: {
	    char* s;
	    int len,padlen;
	    len = strlen(lextext);
	    padlen = len;
	    if(padlen < 16) padlen = 16;
	    if((padlen % 2) == 1) padlen++;
	    s = (char*)emalloc(padlen+1);
	    memset((void*)s,'0',padlen);
	    s[padlen]='\0';
	    strncpy(s,lextext,len);
	    con.value.opaquev.stringv = s;
	    con.value.opaquev.len = padlen;
	    } break;
#endif

	case NC_FILLVALUE:
	    break; /* no associated value*/
	    
	default:
	    yyerror("Data constant: unexpected NC type: %s",
		    nctypename(nctype));
	    con.value.stringv.stringv = NULL;    
	    con.value.stringv.len = 0;
    }
    return con;
}

static Constant
makeenumconst(Symbol* econst)
{
    Constant con;
    if(usingclassic) {
        markcdf4("Enum type");
    } 
    consttype = NC_ENUM;
    con.nctype = NC_ECONST;
    con.lineno = lineno;
    /* fix up econst to be a ref to an econst*/
    econst->objectclass = NC_TYPE;
    econst->subclass = NC_ECONST;
    {
	Symbol* defsym;
	defsym = locate(econst);
	if(defsym == NULL)
	    derror("Undefined or forward referenced enum constant: %s",econst->name);
	econst = defsym;
    }
    con.value.enumv = econst;
    return con;
}

static void
addtogroup(Symbol* sym)
{
    Symbol* grp = currentgroup();
    sym->container = grp;
    listpush(grp->subnodes,(elem_t)sym);
    setpathcurrent(sym);
}

/* Check for duplicate name of given type within current group*/
static int
dupobjectcheck(nc_class objectclass, Symbol* pattern)
{
    int i;
    Symbol* grp;
    if(pattern == NULL) return 0;
    grp = pattern->container;
    if(grp == NULL || grp->subnodes == NULL) return 0;
    for(i=0;i<listlength(grp->subnodes);i++) {
	Symbol* sym = (Symbol*)listget(grp->subnodes,i);
	if(!sym->is_ref && sym->objectclass == objectclass
	   && strcmp(sym->name,pattern->name)==0) return 1;
    }
    return 0;
}

static void
setpathcurrent(Symbol* sym)
{
    sym->is_prefixed = 0;
    sym->prefix = prefixdup(groupstack);
}

/* Convert an nc_type code to the corresponding Symbol*/
Symbol*
basetypefor(nc_type nctype)
{
    return primsymbols[nctype];
}

char*
specialname(int flag)
{
    switch (flag) {
    case _FILLVALUE_FLAG: return "_FillValue";
    case _FORMAT_FLAG: return "_Format";
    case _STORAGE_FLAG: return "_Storage";
    case _CHUNKSIZES_FLAG: return "_ChunkSizes";
    case _FLETCHER32_FLAG: return "_Fletcher32";
    case _DEFLATE_FLAG: return "_DeflateLevel";
    case _SHUFFLE_FLAG: return "_Shuffle";
    case _ENDIAN_FLAG: return "_Endianness";
    case _NOFILL_FLAG: return "_NoFill";
    default: break;
    }
    return "<unknown>";
}

static int
truefalse(Constant* con, int tag)
{
    if(con->nctype == NC_STRING) {
	char* sdata = con->value.stringv.stringv;
	if(strncmp(sdata,"false",NC_MAX_NAME) == 0
           || strncmp(sdata,"0",NC_MAX_NAME) == 0)
	    return 0;
	else if(strncmp(sdata,"true",NC_MAX_NAME) == 0
           || strncmp(sdata,"1",NC_MAX_NAME) == 0)
	    return 1;
	else goto fail;
    } else if(con->value.int32v < 0 || con->value.int32v > 1)
	goto fail;
    return con->value.int32v;

fail:
    derror("%s: illegal value",specialname(tag));
    return 0;
}

/* Since this may be affected by the _Format attribute, which
   may come last, capture all the special info and sort it out
   in semantics.
*/
static Symbol*
makespecial(int tag, Symbol* vsym, Symbol* tsym, void* data, int isconst)
{
    Symbol* attr = NULL;
    Datalist* list;
    Constant* con;
    Specialdata* special;
    Constant iconst;
    int tf = 0;
    char* sdata = NULL;
    int idata =  -1;

    if(isconst) {
	con = (Constant*)data;
	list = builddatalist(1);
        dlappend(list,(Constant*)data);
    } else {
        list = (Datalist*)data;
        con = (Constant*)list->data;
    }

    if(tag == _FORMAT && vsym != NULL) {
	derror("_Format: must be global attribute");
	vsym = NULL;
    }

    switch (tag) {
    case _FLETCHER32_FLAG:
    case _SHUFFLE_FLAG:
    case _NOFILL_FLAG:
	iconst.nctype = (con->nctype == NC_STRING?NC_STRING:NC_INT);
	convert1(con,&iconst);
	tf = truefalse(&iconst,tag);
	break;
    case _FORMAT_FLAG:
    case _STORAGE_FLAG:
    case _ENDIAN_FLAG:
	iconst.nctype = NC_STRING;
	convert1(con,&iconst);
	if(iconst.nctype == NC_STRING)
	    sdata = iconst.value.stringv.stringv;
	else
	    derror("%s: illegal value",specialname(tag));
	break;
    case _DEFLATE_FLAG:
	iconst.nctype = NC_INT;
	convert1(con,&iconst);
	if(iconst.nctype == NC_INT)
	    idata = iconst.value.int32v;
	else
	    derror("%s: illegal value",specialname(tag));
	break;
    case _CHUNKSIZES_FLAG:
    case _FILLVALUE_FLAG:
	/* Handle below */
	break;
    default: PANIC1("unexpected special tag: %d",tag);
    }
    
    if(vsym != NULL) special = &vsym->var.special;
    if(tag == _FORMAT_FLAG) {
	struct Kvalues* kvalue;
	int found;
        int modifier;
	found = 0;
        modifier = 0;
	if(kflag_flag == 0) goto done;
	/* Only use this tag if kflag is not set */
	/* Use the table in main.c */
        for(kvalue=legalkinds;kvalue->name;kvalue++) {
	    if(strcmp(sdata,kvalue->name) == 0) {
		modifier = kvalue->mode;
		found = 1;
	        break;
	    }
	}
	if(!found)
	    derror("_Format: illegal value: %s",sdata);
	else {
            /* Recompute mode flag */
	    cmode_modifier &= ~(NC_NETCDF4 | NC_CLASSIC_MODEL | NC_64BIT_OFFSET);
  	    cmode_modifier |= modifier;
            if((cmode_modifier & NC_NETCDF4)) {
	        allowspecial = 1;
                if((cmode_modifier & NC_CLASSIC_MODEL)) {
		    usingclassic = 1;
	        } else {
	            usingclassic = 0;
	        }
	    } else
	        usingclassic = 1;
	}
    } else if(tag == _FILLVALUE_FLAG) {
	special->_Fillvalue = list;
	/* fillvalue must be a single value*/
	if(list->length != 1)
	    derror("_FillValue: must be a single (possibly compound) value",
			vsym->name);
        /* check that the attribute value contains no fill values*/
        if(containsfills(list)) {
	    derror("Attribute data may not contain fill values (i.e. _ )");
        }
	/* _FillValue is also a real attribute*/
	if(vsym->objectclass != NC_VAR) {
	    derror("_FillValue attribute not associated with variable: %s",vsym->name);
	}
	if(tsym  == NULL) tsym = vsym->typ.basetype;
	else if(vsym->typ.basetype != tsym) {
	    derror("_FillValue attribute type does not match variable type: %s",vsym->name);
	}
	attr=makeattribute(install("_FillValue"),vsym,tsym,list,ATTRVAR);
    } else switch (tag) {
        case _STORAGE_FLAG:
            if(strcmp(sdata,"contiguous") == 0)
                special->_Storage = NC_CONTIGUOUS;
            else if(strcmp(sdata,"chunked") == 0)
                special->_Storage = NC_CHUNKED;
            else
                derror("_Storage: illegal value: %s",sdata);
            special->flags |= _STORAGE_FLAG;
            break;
        case _FLETCHER32_FLAG:
            special->_Fletcher32 = tf;
            special->flags |= _FLETCHER32_FLAG;
            break;
        case _DEFLATE_FLAG:
            special->_DeflateLevel = idata;
            special->flags |= _DEFLATE_FLAG;
            break;
        case _SHUFFLE_FLAG:
	    special->_Shuffle = tf;
            special->flags |= _SHUFFLE_FLAG;
            break;
        case _ENDIAN_FLAG:
            if(strcmp(sdata,"little") == 0)
                special->_Endianness = 1;
            else if(strcmp(sdata,"big") == 0)
                special->_Endianness = 2;
            else
                derror("_Endianness: illegal value: %s",sdata);
            special->flags |= _ENDIAN_FLAG;
            break;
        case _NOFILL_FLAG:
            special->_Fill = (1 - tf); /* negate */
            special->flags |= _NOFILL_FLAG;
            break;
        case _CHUNKSIZES_FLAG: {
	    int i;
            special->nchunks = list->length;
            special->_ChunkSizes = (size_t*)emalloc(sizeof(size_t)*special->nchunks);
            for(i=0;i<special->nchunks;i++) {
		iconst.nctype = NC_INT;
		convert1(&list->data[i],&iconst);
	        if(iconst.nctype == NC_INT) {
		    special->_ChunkSizes[i] = (size_t)iconst.value.int32v;
	        } else
		    derror("%s: illegal value",specialname(tag));
            }
            special->flags |= _CHUNKSIZES_FLAG;
	    /* Chunksizes => storage == chunked */
            special->flags |= _STORAGE_FLAG;
            special->_Storage = NC_CHUNKED;
            } break;
        default: PANIC1("makespecial: illegal token: %d",tag);
     }
done:
    return attr;
}

static Symbol*
makeattribute(Symbol* asym,
		Symbol* vsym,
		Symbol* tsym,
		Datalist* data,
		Attrkind kind) /* global var or unknown*/
{
    asym->objectclass = NC_ATT;
    asym->data = data;
    addtogroup(asym);
    switch (kind) {
    case ATTRVAR:
        asym->att.var = vsym;
        asym->typ.basetype = tsym;
        listpush(attdefs,(elem_t)asym);
	break;
    case ATTRGLOBAL:
        asym->att.var = NULL; /* NULL => NC_GLOBAL*/
        asym->typ.basetype = tsym;
        listpush(gattdefs,(elem_t)asym);
	break;
    default: PANIC1("unexpected attribute type: %d",kind);
    }
    /* finally; check that the attribute value contains no fill values*/
    if(containsfills(data)) {
	derror("Attribute data may not contain fill values (i.e. _ ): %s",asym->name);
    }
    return asym;
}

static int
containsfills(Datalist* list)
{
    int i;
    Constant* con = list->data;
    for(i=0;i<list->length;i++,con++) {
	if(con->nctype == NC_COMPOUND) {
	    if(containsfills(con->value.compoundv)) return 1;	
	} else if(con->nctype == NC_FILLVALUE) return 1;	
    }
    return 0;
}

static void
datalistextend(Datalist* dl, Constant* con)
{
    dlappend(dl,con);
}

static void
vercheck(int ncid)
{
    char* tmsg = NULL;
    switch (ncid) {
    case NC_UBYTE: tmsg = "netCDF4 type: UBYTE"; break;
    case NC_USHORT: tmsg = "netCDF4 type: USHORT"; break;
    case NC_UINT: tmsg = "netCDF4 type: UINT"; break;
    case NC_INT64: tmsg = "netCDF4 type: INT64"; break;
    case NC_UINT64: tmsg = "netCDF4 type: UINT64"; break;
    case NC_STRING: tmsg = "netCDF4 type: STRING"; break;
    case NC_VLEN: tmsg = "netCDF4 type: VLEN"; break;
    case NC_OPAQUE: tmsg = "netCDF4 type: OPAQUE"; break;
    case NC_ENUM: tmsg = "netCDF4 type: ENUM"; break;
    case NC_COMPOUND: tmsg = "netCDF4 type: COMPOUND"; break;
    default: break;
    }
    if(tmsg != NULL) markcdf4(tmsg);
}

