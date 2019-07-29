
%{
/* The parser */

#include "jos.h"
#include "as6502.h"
#include "id.h"
#include "code.h"
#include "assemble.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

PUBLIC int lineno=0;

extern word lc;
extern FILE *listfile;
extern int pass;
extern char listline[];
extern int yylex();
extern void yyerror(const char *s);

PRIVATE int flagset;

PRIVATE void exprelerr(flag)
	int flag;
	{
		if (flag) error("Invalid expression with relocatable or external components");
	}

%}

%union
	{
		long num;
		void *id;
		char str[STRINGLEN];
		char mnem[4];
		struct	{
				boolean external;
				boolean absolute;
				void *id;
				long num;
			} extval;
		struct	{
				boolean external;
				boolean absolute;
				void *id;
				adrmode adrmode;
				word num;
			} adrmode;
		struct
			{
				long num;
				boolean absolute;
			} expval;
	}

	
%token		eofSYM
%token 		newlineSYM
%token 		orgSYM
%token 		equSYM
%token		externSYM
%token		publicSYM
%token		endSYM
%token 		regaSYM
%token 		regxSYM
%token 		regySYM
%token		dbSYM
%token		dwSYM
%token		dsSYM
%token		ddSYM
%token <id>	idSYM
%token <id>	extidSYM
%token <num>	numSYM
%token 		plusSYM
%token 		starSYM
%token 		minusSYM
%token 		divideSYM
%token 		andSYM
%token 		orSYM
%token 		notSYM
%token		commentSYM
%token		hashSYM
%token		dummySYM
%token		commaSYM
%token		lparenSYM
%token		rparenSYM
%token <str>	stringSYM
%token <id>	labelSYM
%token <mnem>	mnemonicSYM
 
%type <expval>	expression
%type <extval>	extexpression
%type <num>	absexpression

%type <id>	label
%type <id>	id
%type <adrmode> adrmode

%left andSYM orSYM
%left plusSYM minusSYM
%left starSYM divideSYM
%left UMINUS
 
%start start

%%
start		:	assource eofSYM
			{
				YYACCEPT;
			}
		;
		
assource	:	oneline
		|	oneline assource
		;

oneline		:	oneline2
			{
				if (listfile) fputs(listline,listfile);
				dequeue_errs();
			}
					
oneline2	:	statement optcomment newlineSYM
		|	error newlineSYM
			{
				error("Invalid syntax in statement");
			}
       		;
				
optcomment	:	commentSYM
		|	/* epsilon */
		;
		
statement	:	orgstat
		|	equstat
		|	asstat
		|	publicstat
		|	externstat
		|	endstat
			{
				YYACCEPT;
			}
		|	dbstat
		|	dwstat
		|	ddstat
		|	dsstat
		|	/* epsilon */
		;
		
orgstat		:	orgSYM expression
			{
				set_lc((word)$2.num);
			}
		;
		
equstat		:	label equSYM extexpression
			{
				setvalue($1,$3.num);
				
				if ($3.absolute) setflags($1,ABSOLUTE_EXP);
				if ($3.external) setflags($1,EXTERNALSYM);
			}
		;
		
asstat		:	optlabel mnemonicSYM adrmode
			{
				assemble($2,$3.adrmode,$3.external,$3.id,$3.num,$3.absolute);
			}
		;

publicstat	:	publicSYM 
			{
				flagset=PUBLICSYM;
			}
			idlist
		;

externstat	:	externSYM 
			{
				flagset=EXTERNALSYM;
			}
			idlist
		;

idlist		:	idlist commaSYM id
			{
				setflags($3,flagset);
			}
		|	id
			{
				setflags($1,flagset);
			}
		;

endstat		:	endSYM 
		|	endSYM idSYM		
			{
				setflags($2,STARTSYM);
			}
		;
		
optlabel	:	/* epsilon */
		|	label
		;

extexpression	:	extidSYM
			{
				$$.external=TRUE;
				$$.id=$1;
				$$.num=0L;
				$$.absolute=TRUE;
			}
		|	expression
			{
				$$.external=FALSE;
				$$.num=$1.num;
				$$.absolute=$1.absolute;
			}
		;

absexpression	:	extexpression
			{
				exprelerr(!$1.absolute||$1.external);
				$$=$1.num;
			}
		;
				
expression	:	numSYM
			{
				$$.num=$1;
				$$.absolute=TRUE;
			}
		|	idSYM
			{
				$$.num=getvalue($1);
				$$.absolute=getflags($1)&ABSOLUTE_EXP;
			}
		|	starSYM
			{
				$$.num=(long)lc;
				$$.absolute=FALSE;
			}
		|	expression plusSYM expression
			{
				exprelerr(!($1.absolute||$3.absolute));
				$$.num=$1.num+$3.num;
				$$.absolute=$1.absolute&&$3.absolute;
			}
		|	expression minusSYM expression
			{
				/* some smarty optimization here */
				
				if (($1.absolute+$3.absolute)&1) exprelerr(1);

				$$.absolute=TRUE;
				$$.num=$1.num-$3.num;
			}
		|	absexpression starSYM absexpression
			{
				$$.num=$1*$3;
				$$.absolute=TRUE;
			}
		|	absexpression divideSYM absexpression
			{
				$$.num=$1/$3;
				$$.absolute=TRUE;
			}
		|	absexpression andSYM absexpression
			{
				$$.num=$1 & $3;
				$$.absolute=TRUE;
			}
		|	absexpression orSYM absexpression
			{
				$$.num=$1 | $3;
				$$.absolute=1;
			}
		|	minusSYM absexpression %prec UMINUS
			{
				$$.num= -($2);
				$$.absolute=1;
			}
		;
			
adrmode		:	/* epsilon */
			{
				$$.adrmode=IMPLIED;
        $$.id=NULL;
        $$.external=0;
			}
		|	regaSYM
			{
				$$.adrmode=ACCUMULATOR;
        $$.id=NULL;
        $$.external=0;
			}
		|	extexpression
			{
				$$.adrmode=ABSOLUTE;
				$$.external=$1.external;
				$$.num=(word)$1.num;
				$$.id=$1.id;
			}
		|	hashSYM absexpression
			{
				$$.adrmode=IMMEDIATE;
				$$.num=$2%256;
        $$.id=NULL;
        $$.external=0;
			}
		|	extexpression commaSYM regxSYM
			{
				$$.adrmode=ABSX;
				$$.external=$1.external;
				$$.absolute=$1.absolute;
				$$.num=(word)$1.num;
				$$.id=$1.id;
			}			
		|	extexpression commaSYM regySYM
			{
				$$.adrmode=ABSY;
				$$.external=$1.external;
				$$.absolute=$1.absolute;
				$$.num=(word)$1.num;
				$$.id=$1.id;
			}			
		|	lparenSYM extexpression commaSYM regxSYM rparenSYM
			{
				$$.adrmode=INDX;
				$$.external=$2.external;
				$$.absolute=$2.absolute;
				$$.num=(word)$2.num;
				$$.id=$2.id;
			}			
		|	lparenSYM extexpression rparenSYM commaSYM regySYM
			{
				$$.adrmode=INDY;
				$$.external=$2.external;
				$$.absolute=$2.absolute;
				$$.num=(word)$2.num;
				$$.id=$2.id;
			}			
		|	lparenSYM extexpression rparenSYM
			{
				$$.adrmode=INDIRECT;
				$$.absolute=$2.absolute;
				$$.external=$2.external;
				$$.num=(word)$2.num;
				$$.id=$2.id;
			}			
		;

label		:	labelSYM
			{
				setvalue($1,(long)lc);
				$$ = $1;
			}
		;
		
dbstat		:	optlabel dbSYM dblist
		;
		
dblist		:	dbitem
		|	dblist commaSYM dbitem
		;
		
dbitem		:	absexpression
			{
				emit((byte)$1);
			}
		|	stringSYM
			{
				int i;
				
				for (i=0; $1[i]; i++)
					emit((byte)$1[i]);
			}
		;

dwstat		:	optlabel dwSYM dwlist
		;
		
dwlist		:	dwitem
		|	dwlist commaSYM dwitem
		;
		
dwitem		:	extexpression
			{
				if ($1.external) external_req($1.id,lc);
				if (!$1.absolute) reloc_req(lc);
				emit2((word)$1.num);
			}
		;
		
ddstat		:	optlabel ddSYM ddlist
		;
		
ddlist		:	dditem
		|	ddlist commaSYM dditem
		;
		
dditem		:	extexpression
			{
				if ($1.external) external_req($1.id,lc);
				if (!$1.absolute) reloc_req(lc);
				
				emit4((doubleword)$1.num);
			}
		;
		
dsstat		:	optlabel dsSYM dslist
		;
		
dslist		:	dsitem
		|	dslist commaSYM dsitem
		;
		
dsitem		:	absexpression
			{
				set_lc(lc+$1);
			}
		;

id		:	idSYM
		|	extidSYM
		;
				
%%
