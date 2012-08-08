/* copyright 2009, ucar/unidata and opendap, inc.
   see the copyright file for more information.
*/
/*$id$*/

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "nclist.h"

#include "netcdf.h"
#include "crce.h"

static int crcdebug = 0;

#define allocincr 16
/* mnemonic */
#define optional 1

#define indentchunk "  "

static char hexchars[17] = "01234567890abcdef";

static char delims[] = "{}[]:,;";

/* token types */
#define LPAREN '('
#define RPAREN ')'
#define COMMA ','
#define COLON ':'
#define PERIOD '.'
#define EQUAL '='
#define AMPERSAND '&'

#define _NUMBER	-1
#define _WORD	-2
#define _STRING	-3
#define _NULL	-4

#define ENTER(proc) {if(crcdebug) trace(proc,0,0);}
#define LEAVE(proc,tf) {if(crcdebug) trace(proc,1,tf);}

#define FAIL(lexer,msg) do {setlexerr((lexer),(msg)); goto fail;} while(0)

/**************************************************/
/* Forwards */
typedef struct CRClexer CRClexer;
struct crctext;
struct crclist;

static int crcprojection(CRClexer* lexer, CRCprojection** projp);
static int crcsegment(CRClexer* lexer, CRCsegment** segp);
static int crcslice(CRClexer* lexer, CRCslice** slicep);

static CRClexer* createLexer(char*);
static void reclaimLexer(CRClexer*);
static void fillError(CRClexer* lexer, CRCerror* err);
static char* tokentext(CRClexer* lexer);
static int nexttoken(CRClexer* lexer);
static void backup(CRClexer* lexer);
static unsigned int tohex(int c);
static void dumptoken(CRClexer* lexer, int);
static int removeescapes(CRClexer* lexer);
static void setlexerr(CRClexer*,char*);

static void textclear(struct crctext* lexer);
static int textadd(struct crctext* text, int c);
static int textterminate(struct crctext* text);

static void pushback(CRClexer* lexer, int c);
static int unpush(CRClexer* lexer);
static int readc(CRClexer* lexer);

#ifdef IGNORE
static int listadd(struct crclist* list, crcnode* node);
static void listclear(struct crclist* list);
#endif

enum nonterms {
_projection,
_segment,
_slice
};

static char* nontermnames[] = {"projection","segment","slice",NULL};

static void trace(enum nonterms nt, int leave, int ok);

/**************************************************/

/*
Equivalent yacc grammar:

projectionlist:
	  projection
	| projectionlist ',' projection
	;

projection:
	  segment
	| segmentlist '.' segment
	;

segment:
	  WORD
	| WORD '(' rangelist ')'
	;

rangelist: 
	  range
        | rangelist ',' range
	;

range:
	  INTEGER
	| INTEGER ':' INTEGER
	| INTEGER ':' INTEGER ':' INTEGER
	;

*/

/**************************************************/

/* Invariant: at end of a parse unit,
   currenttoken() == first unconsumed token
*/

int
crce(char* src, NClist* projections, CRCerror* err)
{
    int ncstat = NC_NOERR;
    CRClexer* lexer;
    CRCprojection* proj = NULL;
    int token;

    lexer = createLexer(src);
    if(lexer == NULL) {ncstat = NC_ENOMEM; goto fail;}

    token = nexttoken(lexer); /* prime the pump */

    for(;;) {
	if((ncstat=crcprojection(lexer,&proj))) goto fail;
	nclistpush(projections,(ncelem)proj);	
	token = nexttoken(lexer);
	if(token == EOF) goto done;
	if(token != COMMA) FAIL(lexer,"expected comma");
    }

done:
    reclaimLexer(lexer);
    return ncstat;

fail:
    if(err) {
	fillError(lexer,err);
    }
    reclaimLexer(lexer);
    return ncstat;
}

static int
crcprojection(CRClexer* lexer, CRCprojection** projp)
{
    int ncstat = NC_NOERR;
    int token = 0;
    CRCprojection* proj = NULL;

    ENTER(_projection);

    proj = (CRCprojection*)malloc(sizeof(CRCprojection));
    if(proj == NULL) {ncstat = NC_ENOMEM; goto fail;}
    memset((void*)proj,0,sizeof(CRCprojection));
    if(*projp) *projp = proj;
    proj->segments = nclistnew();

    for(;;) {
	CRCsegment* seg = NULL;
	if((ncstat=crcsegment(lexer,&seg))) goto fail;
	nclistpush(proj->segments,(ncelem)seg);	
	token = nexttoken(lexer);
	/* Check for legal stop tokens */
	if(token == EOF) goto done;
	if(token == COMMA) {backup(lexer); goto done;}
	if(token != PERIOD) FAIL(lexer,"expected '.'");
    }

done:
    LEAVE(_projection,1);
    return ncstat;

fail:
    LEAVE(_projection,0);
    return ncstat;
}

static int
crcsegment(CRClexer* lexer, CRCsegment** segp)
{
    int ncstat = NC_NOERR;
    int token = 0;
    CRCsegment* seg = NULL;

    ENTER(_segment);

    seg = (CRCsegment*)malloc(sizeof(CRCsegment));
    if(seg == NULL) {ncstat = NC_ENOMEM; goto fail;}
    memset((void*)seg,0,sizeof(CRCsegment));
    if(*segp) *segp = seg;
    seg->slicerank = 0;

    /* There must be at least a word */
    token = nexttoken(lexer);
    if(token != _WORD) FAIL(lexer,"expected word");
    seg->name = strdup(tokentext(lexer));

    /* Check for legal stop tokens when no projection indices*/
    token = nexttoken(lexer);
    if(token == EOF) goto done;
    if(token == COMMA) {backup(lexer); goto done;}
    if(token == PERIOD) {backup(lexer); goto done;}

    /* Must be a parenthesized, comma separated set of ranges */
    token = nexttoken(lexer);
    if(token != LPAREN) FAIL(lexer,"expected left paren");

    for(;;) {
	CRCslice* slice = &seg->slices[seg->slicerank];
	if((ncstat=crcslice(lexer,&slice))) goto fail;
	seg->slicerank++;
	token = nexttoken(lexer);
	/* Check for legal stop tokens */
	if(token == RPAREN) {backup(lexer); break;}
	if(token != COMMA) FAIL(lexer,"expected ','");
    }

    token = nexttoken(lexer);
    if(token != RPAREN) FAIL(lexer,"expected right paren");

done:
    LEAVE(_segment,1);
    return ncstat;

fail:
    LEAVE(_segment,0);
    return ncstat;
}

static int
crcslice(CRClexer* lexer, CRCslice** slicep)
{
    int ncstat = NC_NOERR;
    CRCslice* slice = NULL;
    int token, count;

    ENTER(_slice);

    slice = (CRCslice*)malloc(sizeof(CRCslice));
    if(slice == NULL) {ncstat = NC_ENOMEM; goto fail;}
    memset((void*)slice,0,sizeof(CRCslice));
    if(slicep) *slicep = slice;
    slice->start = 0;
    slice->stop = -1;
    slice->stride = 1;

    token = nexttoken(lexer);
    if(token != _NUMBER) FAIL(lexer,"expected integer start value");
    count = sscanf(tokentext(lexer),"%d",&slice->start);    
    if(count != 1) FAIL(lexer,"illegal slice start value");
    if(slice->start < 0) FAIL(lexer,"illegal slice start value");
 
    token = nexttoken(lexer);
    if(token == RPAREN) {backup(lexer); goto done;}    
    if(token == COMMA) {backup(lexer); goto done;}    
    if(token != COLON) FAIL(lexer,"expected colon");

    token = nexttoken(lexer);
    if(token != _NUMBER) FAIL(lexer,"expected integer end value");
    count = sscanf(tokentext(lexer),"%d",&slice->stop);    
    if(count != 1) FAIL(lexer,"illegal slice end value");
    if(slice->stop <= 0) FAIL(lexer,"illegal slice end value");

    token = nexttoken(lexer);
    if(token == RPAREN) {backup(lexer); goto done;}    
    if(token == COMMA) {backup(lexer); goto done;}    
    if(token != COLON) FAIL(lexer,"expected colon");

    token = nexttoken(lexer);
    if(token != _NUMBER) FAIL(lexer,"expected integer stride value");
    count = sscanf(tokentext(lexer),"%d",&slice->stride);    
    if(count != 1) FAIL(lexer,"illegal slice stride value");
    if(slice->stride <= 0) FAIL(lexer,"illegal slice stride value");
 
    token = nexttoken(lexer);
    if(token == RPAREN) {backup(lexer); goto done;}    
    if(token == COMMA) {backup(lexer); goto done;}    
    FAIL(lexer,"expected right paren or comma");


done:
    LEAVE(_slice,1);
    return ncstat;

fail:
    if(slice != NULL) free(slice);
    LEAVE(_slice,0);
    return ncstat;
}

void
CRCsegmentfree(CRCsegment* seg)
{
    int i;
    if(seg == NULL) return;
    free(seg);    
}

void
CRCprojectionfree(CRCprojection* proj)
{
    int i;
    if(proj == NULL) return;
    for(i=0;i<nclistlength(proj->segments);i++) {
	CRCsegmentfree((CRCsegment*)nclistget(proj->segments,i));
    }
    nclistfree(proj->segments);
    free(proj);    
}

/****************************************/
/* LEXER */

#define ALLOCINCR 256

/* Lexer data structures */

struct crctext {
    char* text;
    size_t len; /* |text| */
    size_t alloc; /* |text| */
    int pushback[2]; /* max pushback needed */
};

struct CRClexer {
    char* input;
    size_t pos;
    int token;    
    struct crctext text;
    int pushedback; /* 1=>keep current token */
    int lineno;
    int charno;
    char* errmsg;
};

static CRClexer*
createLexer(char* src)
{
    CRClexer* lexer = (CRClexer*)calloc(1,sizeof(CRClexer));
    if(lexer != NULL) {
        lexer->input = src;
        lexer->lineno = 1;
        lexer->charno = 1;
        lexer->errmsg = NULL;;
    }
    return lexer;
}

static void
reclaimLexer(CRClexer* lexer)
{
    if(lexer == NULL) return;
    if(lexer->text.text) free(lexer->text.text);
    free(lexer);    
}

static void
fillError(CRClexer* lexer, CRCerror* err)
{
    err->lineno = lexer->lineno;
    err->charno = lexer->charno-1;
    err->errmsg = lexer->errmsg;
}

/* Caller must strdup */
static char*
tokentext(CRClexer* lexer)
{
    return lexer->text.text;
}

static void
setlexerr(CRClexer* lexer, char* msg)
{
    lexer->errmsg = msg;
}

static void
backup(CRClexer* lexer)
{
    lexer->pushedback = 1;
    if(crcdebug > 1)
	dumptoken(lexer,1);
}

static int
nexttoken(CRClexer* lexer)
{
    int token;
    int c;
    if(lexer->pushedback)
	{token = lexer->token; lexer->pushedback = 0; goto done;}
    token = 0;
    textclear(&lexer->text);
    while(token==0) {
	c=readc(lexer);
	lexer->charno++;
	if(c == EOF) {
	    token = EOF;
	    lexer->charno--;
	    break;
	} else if(c == '\n') {
	    lexer->lineno++;
	    lexer->charno = 1;
	} else if(c == '/') { 
	    c = readc(lexer);
	    if(c == '/') {/* single line comment */
	        while((c=readc(lexer)) != EOF) {if(c == '\n') break;}
	    } else {
		pushback(lexer,c); c = '/';
	    }
	}
	if(c <= ' ' || c == '\177') {
	    /* ignore */
	} else if(strchr(delims,c) != NULL) {
	    textadd(&lexer->text,c);
	    token = c;
	} else if(c == '"') {
	    int more = 1;
	    while(more) {
		c = readc(lexer);
		switch (c) {
		case EOF: goto fail;
		case '"': more=0; break;
		case '\\':
		    textadd(&lexer->text,c);
		    c=readc(lexer);
		    textadd(&lexer->text,c);		    
		    break;
		default: textadd(&lexer->text,c);
		}
	    }
	    if(!removeescapes(lexer)) goto fail;
	    token=_STRING;
	} else { /* WORD */
	    textadd(&lexer->text,c);
	    while((c=readc(lexer))) {
		if(c == '/' || c <= ' ' || c == '\177') {pushback(lexer,c); break;}
		else if(strchr(delims,c) != NULL) {pushback(lexer,c); break;}
		textadd(&lexer->text,c);
	    }
	    if(!removeescapes(lexer)) goto fail;
	    { /* See if this looks like a number */
		double d;
		if(sscanf(lexer->text.text,"%lg",&d) == 1)
		    token = _NUMBER;
		else
		    token = _WORD;
	    }
	}
    }
done:
    lexer->token = token;
    if(crcdebug > 1)
	dumptoken(lexer,0);
    return token;
fail:
    return EOF;
}

static unsigned int
tohex(int c)
{
    if(c >= 'a' && c <= 'f') return (c - 'a') + 0xa;
    if(c >= 'A' && c <= 'F') return (c - 'A') + 0xa;
    if(c >= '0' && c <= '9') return (c - '0');
    return 0;
}

static void
dumptoken(CRClexer* lexer, int pushed)
{
    fprintf(stderr,"%s : %d = |%s|\n",
	(pushed?"PUSHED":"TOKEN"),
	lexer->token,lexer->text.text);
}


static void
textclear(struct crctext* txt)
{
    if(txt->len > 0) memset(txt->text,0,txt->alloc);
    txt->len = 0;
}

static int
textterminate(struct crctext* text)
{
    return textadd(text,'\0');
}

static int
textadd(struct crctext* text, int c)
{
    if(text->len >= text->alloc) {
        if(text->alloc == 0) {
	    text->text = (char*)malloc(ALLOCINCR+1);
	    if(text->text == NULL) return 0;
	    text->alloc = ALLOCINCR;
	    text->len = 0;
	} else {
	    text->text = (char*)realloc((void*)text->text,text->alloc+ALLOCINCR+1);
	    if(text->text == NULL) return 0;
	    text->alloc += ALLOCINCR;
	}
	text->text[text->alloc] = '\0';
    }
    text->text[text->len++] = c;
    return 1;    
}

static void
pushback(CRClexer* lexer, int c)
{
    if(lexer->text.pushback[0] == 0) lexer->text.pushback[0] = c;
    else lexer->text.pushback[1] = c;
}

static int
unpush(CRClexer* lexer)
{
    int c = '\0';
    int i;
    for(i=1;i>=0;i--) {
        if(lexer->text.pushback[i] != 0) {
	    c = lexer->text.pushback[i];
	    lexer->text.pushback[i] = '\0';
	    break;
	}
    }
    return c;
}

static int
readc(CRClexer* lexer)
{
    int c = 0;
    c = unpush(lexer);
    if(c == 0) {
	c = lexer->input[lexer->pos++];
    }
    return c;
}

/* Convert the characters in lexer->text.text to
   remove escapes. Assumes that all escapes are smaller
   than the unescaped value.
*/
static int
removeescapes(CRClexer* lexer)
{
    char* p = lexer->text.text;
    char* q = p;
    int cp;
    while((cp=*p++)) {
        switch (cp) {
	case '\\':
	    cp=*p++;
	    switch (cp) {
	    case '\0': *q++ = cp; goto done;
            case 'r': *q++ = '\r'; break;
            case 'n': *q++ = '\n'; break;
            case 'f': *q++ = '\f'; break;
            case 't': *q++ = '\t'; break;
            case 'b': *q++ = '\b'; break;
            case '/': *q++ = '/'; break; /* CRC requires */
            case 'x': {
                unsigned int d[2];
                int i;
                for(i=0;i<2;i++) {
                    if((cp = *p++) == '\0') goto fail;
                    d[i] = tohex(cp);
                }
                /* Convert to a sequence of utf-8 characters */
                cp = (d[0]<<4)|d[1];
		*q++ = cp;
            } break;

            default: break;

            }
            break;
        }
    }
done:
    return 1;
fail:
    return 0;
}

#ifdef IGNORE
static int
listadd(crclist* list, CRCprojection* var)
{
    if(list->len >= list->alloc) {
        if(list->alloc == 0) {
	    list->contents = (CRCprojection**)malloc(sizeof(CRCprojection)*ALLOCINCR);
	    if(list->contents == NULL) return 0;
	    list->alloc = ALLOCINCR;
	    list->len = 0;
	} else {
	    list->contents = (CRCprojection**)realloc((void*)list->contents,sizeof(CRCprojection)*(list->alloc+ALLOCINCR));
	    if(list->contents == NULL) return 0;
	    list->alloc += ALLOCINCR;
	}
    }
    list->contents[list->len++] = var;
    return 1;    
}

static void
listclear(crclist* list)
{
    if(list->contents != NULL) free(list->contents);
}
#endif

static void
indent(FILE* f, int depth)
{
#ifdef IGNORE
    while(depth--) fputs(INDENTCHUNK,f);
#endif
}

static void
stringify(char* s, struct crctext* tmp)
{
    char* p = s;
    int c;
    textclear(tmp);
    while((c=*p++)) {
	if(c == '"' || c < ' ' || c >= '\177') {
	    textadd(tmp,'\\');
	    switch (c) {
	    case '"': textadd(tmp,'"'); break;
	    case '\r': textadd(tmp,'r'); break;
	    case '\n': textadd(tmp,'r'); break;
	    case '\t': textadd(tmp,'r'); break;
	    default:
	        textadd(tmp,'x');
	        textadd(tmp,hexchars[(c & 0xf0)>>4]);
	        textadd(tmp,hexchars[c & 0x0f]);
		break;
	    }
	} else 
	    textadd(tmp,c);
    }
}

static int
isword(char* s)
{
    char* p = s;
    int c;
    while((c=*p++)) {
	if(strchr(delims,c) != NULL
	   || c == '/' || c <= ' ' || c >= '\177') return 0;
    }
    return 1;
}


static void
trace(enum nonterms nt, int leave, int ok)
{
    if(!leave) {
	fprintf(stderr,"enter: %s\n",nontermnames[(int)nt]);
    } else {/* leave */
	fprintf(stderr,"leave: %s : %s\n",nontermnames[(int)nt],
		(ok?"succeed":"fail"));
    }
}

#ifdef IGNORE
static int
isoneline(CRCprojection* var)
{
    int i;
    for(i=0;i<var->list.nvalues;i++) {
	crcclass cl;
	CRCprojection* member;
	member = var->list.values[i];
	if(var->varclass == crc_map) {
    	    cl = member->pair.value->varclass;
	} else if(var->varclass == crc_array) {
    	    cl = member->varclass;
	} else return 0;
        if(cl == crc_array || cl == crc_map) return 0;
    }
    return 1;    
}

static void
crcdumpr(CRCprojection* var, FILE* f, struct crctext* tmp, int depth, int meta)
{
    int i;
    int oneline;
    int endpoint;
    int lparen, rparen;
    char* tag = NULL;

    switch (var->varclass) {
    case crc_map:
	{lparen = LBRACE; rparen = RBRACE; tag = "<map>";}
	/* fall thru */
    case crc_array:
	if(tag == NULL)
	    {lparen = LBRACK; rparen = RBRACK; tag = "<array>";}
	oneline = isoneline(var);
	indent(f,depth);
	if(meta) fputs(tag,f);
	if(meta || depth > 0) fputc(lparen,f);
	endpoint = var->list.nvalues - 1;
	for(i=0;i<=endpoint;i++) {
	    CRCprojection* member = var->list.values[i];
	    if(i>0) fputs(" ",f);
	    crcdumpr(member,f,tmp,depth+1,meta);
	    if(i<endpoint && !oneline) {fputs("\n",f); indent(f,depth);}
	}
	if(!oneline) {
	    if(i > 0) fputs("\n",f);
	    indent(f,depth);
	}
	if(meta || depth > 0) fputc(rparen,f);
	fputs("\n",f);
	break;

    case crc_pair:
        crcdumpr(var->pair.key,f,tmp,depth+1,meta);
	fputs(" : ",f);
        crcdumpr(var->pair.value,f,tmp,depth+1,meta);
	break;

    case crc_const:
        switch (var->constclass) {
        case crc_string:
	    if(meta) fputs("<string>",f);
	    stringify(var->constvalue,tmp);
	    textterminate(tmp);
	    if(isword(tmp->text))
	        fprintf(f,"%s",tmp->text);
	    else
		fprintf(f,"\"%s\"",tmp->text);
	    break;
        case crc_number:
	    if(meta) fputs("<number>",f);
	    fprintf(f,"%s",var->constvalue);
	    break;
        case crc_true:
	    if(meta) fputs("<true>",f);
	    fputs("true",f);
	    break;
        case crc_false:
	    if(meta) fputs("<false>",f);
	    fputs("false",f);
	    break;
        case crc_null:
	    if(meta) fputs("<null>",f);
	    fputs("null",f);
	    break;
        default: abort();
	}
	break;

    default: abort();
    }
}

void
crcdump(CRCprojection* var, FILE* f)
{
    struct crctext tmp = {NULL,0,0};
    textclear(&tmp);
    crcdumpr(var,f,&tmp,0,0);
}

void
crcdumpmeta(CRCprojection* var, FILE* f)
{
    struct crctext tmp = {NULL,0,0};
    textclear(&tmp);
    crcdumpr(var,f,&tmp,0,1);
}
/**************************************************/

CRCprojection*
crclookup(CRCprojection* var, char* key)
{
    int i;
    if(var->varclass != crc_map) return NULL;
    for(i=0;i<var->list.nvalues;i++) {
	CRCprojection* pair = var->list.values[i];
	if(strcmp(pair->pair.key->constvalue,key)==0)
	    return pair->pair.value;
    }
    return NULL;
}

CRCprojection*
crcget(CRCprojection* var, int index)
{
    int i;
    if(var->varclass == crc_map || var->varclass == crc_array) {
        if(index < 0 || index >= var->list.nvalues) return NULL;
	return var->list.values[i];	
    }
    return NULL;
}


/**************************************************/
/* Provide support for url matching */

static int
urlmatch(char* pattern, char* url)
{
    if(strcmp(pattern,"*") == 0) return 1;
    if(strncmp(url,pattern,strlen(pattern))==0) return 1;
    return 0;
}

static void
insert(CRCprojection** matches, int len, CRCprojection* pair)
{
    int i,j;
    /* handle initial case separately */
    if(len > 0) {
        /* sort lexically as determined by strcmp */
        for(i=0;i<len;i++) {
	    if(strcmp(matches[i]->pair.key->constvalue,
                      pair->pair.key->constvalue) > 0) {
	        for(j=(len-1);j>=i;j--) matches[j+1] = matches[j]; 
	        matches[i] = pair;
   	        return;
	    }
	}
    }
    matches[len] = pair; /* default: add at end */
}

static int
collectsortedmatches(char* url, CRCprojection* map, CRCprojection*** matchesp)
{
    int i,j;
    int nvalues = map->list.nvalues;
    CRCprojection* star;
    CRCprojection** matches = NULL;
    matches = (CRCprojection**)malloc(sizeof(CRCprojection*)*(nvalues+1));
    assert(map->varclass == crc_map);
    for(j=0,i=0;i<nvalues;i++) {
	CRCprojection* pair = map->list.values[i];
	if(urlmatch(pair->pair.key->constvalue,url)) {
	    insert(matches,j,pair);
	    j++;
	}
    }
    /* Add "*" key at end */
    star = crclookup(map,"*");
    if(star) matches[j++] = star;
    /* Add whole map at end */
    matches[j++] = map;
    if(matchesp) *matchesp = matches;
    return j;
}

#endif



