
%{

#include <stdio.h>
#include <string.h>

#include "jos.h"
#include "compat.h"
#include "as6502.h"
#include "code.h"
#include "assemble.h"			/* for adrmode type def in yylval union */
#include "parser.h"
#include "id.h"
#include "utils.h"
#include "compat.h"

extern YYSTYPE yylval;
extern int lineno;
extern word lc;
extern FILE *listfile;
extern char listline[];

/* Forward declarations */
PRIVATE int mnem();
PRIVATE int as_input(char *buf, int maxsize);

#define RETURN(x) return x;

#undef YY_INPUT
#define YY_INPUT(buf,result,maxsize) { result = as_input(buf, maxsize); }

%}

%%
<<EOF>>			RETURN(eofSYM);
[ \t]*					;
\n					RETURN(newlineSYM);
\r\n					RETURN(newlineSYM);
[Oo][Rr][Gg]				RETURN(orgSYM);
[Ee][Qq][Uu]				RETURN(equSYM);
[Aa]					RETURN(regaSYM);
[Xx]					RETURN(regxSYM);
[Yy]					RETURN(regySYM);
[Dd][Bb]				RETURN(dbSYM);
[Dd][Ww]				RETURN(dwSYM);
[Dd][Ss]				RETURN(dsSYM);
[Dd][Dd]				RETURN(ddSYM);
[Pp][Uu][Bb][Ll][Ii][Cc]		RETURN(publicSYM);
[Ee][Xx][Tt][Ee][Rr][Nn]		RETURN(externSYM);
[Ee][Nn][Dd]				RETURN(endSYM);
[Aa]]Dd][Cc]				RETURN(mnem());
[Aa][Nn][Dd]				RETURN(mnem());
[Aa][Ss][Ll]				RETURN(mnem());
[Bb][Cc][Cc]				RETURN(mnem());
[Bb][Cc][Ss]				RETURN(mnem());
[Bb][Ee][Qq]				RETURN(mnem());
[Bb][Ii][Tt]				RETURN(mnem());
[Bb][Mm][Ii]				RETURN(mnem());
[Bb][Nn][Ee]				RETURN(mnem());
[Bb][Pp][Ll]				RETURN(mnem());
[Bb][Rr][Kk]				RETURN(mnem());
[Bb][Vv][Cc]				RETURN(mnem());
[Bb][Vv][Ss]				RETURN(mnem());
[Cc][Ll][Cc]				RETURN(mnem());
[Cc][Ll][Dd]				RETURN(mnem());
[Cc][Ll][Ii]				RETURN(mnem());
[Cc][Ll][Vv]				RETURN(mnem());
[Cc][Mm][Pp]				RETURN(mnem());
[Cc][Pp][Xx]				RETURN(mnem());
[Cc][Pp][Yy]				RETURN(mnem());
[Dd][Ee][Cc]				RETURN(mnem());
[Dd][Ee][Xx]				RETURN(mnem());
[Dd][Ee][Yy]				RETURN(mnem());
[Ee][Oo][Rr]				RETURN(mnem());
[Ii][Nn][Cc]				RETURN(mnem());
[Ii][Nn][Xx]				RETURN(mnem());
[Ii][Nn][Yy]				RETURN(mnem());
[Jj][Mm][Pp]				RETURN(mnem());
[Jj][Ss][Rr]				RETURN(mnem());
[Ll][Dd][Aa]				RETURN(mnem());
[Ll][Dd][Xx]				RETURN(mnem());
[Ll][Dd][Yy]				RETURN(mnem());
[Ll][Ss][Rr]				RETURN(mnem());
[Nn][Oo][Pp]				RETURN(mnem());
[Oo][Rr][Aa]				RETURN(mnem());
[Pp][Hh][Aa]				RETURN(mnem());
[Pp][Hh][Pp]				RETURN(mnem());
[Pp][Ll][Aa]				RETURN(mnem());
[Pp][Ll][Pp]				RETURN(mnem());
[Rr][Oo][Ll]				RETURN(mnem());
[Rr][Oo][Rr]				RETURN(mnem());
[Rr][Tt][Ii]				RETURN(mnem());
[Rr][Tt][Ss]				RETURN(mnem());
[Ss][Bb][Cc]				RETURN(mnem());
[Ss][Ee][Cc]				RETURN(mnem());
[Ss][Ee][Dd]				RETURN(mnem());
[Ss][Ee][Ii]				RETURN(mnem());
[Ss][Tt][Aa]				RETURN(mnem());
[Ss][Tt][Xx]				RETURN(mnem());
[Ss][Tt][Yy]				RETURN(mnem());
[Tt][Aa][Xx]				RETURN(mnem());
[Tt][Aa][Yy]				RETURN(mnem());
[Tt][Ss][Xx]				RETURN(mnem());
[Tt][Xx][Aa]				RETURN(mnem());
[Tt][Xx][Ss]				RETURN(mnem());
[Tt][Yy][Aa]				RETURN(mnem());
^[A-Za-z@_][A-Za-z@_$0-9]*		{
						yytext[IDLEN-1]='\0';
						yylval.id=getid(strupr(yytext));
						RETURN(labelSYM);
					}
[A-Za-z@_][A-Za-z@_$0-9]*		{
						int flags;
						
						yytext[IDLEN-1]='\0';
						yylval.id=getid(strupr(yytext));
						flags=getflags(yylval.id);
						
						if (flags&EXTERNALSYM)
							RETURN(extidSYM)
						else
							RETURN(idSYM);
					}
[0-9]*					{
						yylval.num=(long)atol(yytext);
						RETURN(numSYM);
					}
\$[0-9A-Fa-f]*				{
						strupr(yytext);
						yylval.num=calcnum(yytext,16);
						RETURN(numSYM);
					}
\%[01]*					{
						yylval.num=calcnum(yytext,2);
						RETURN(numSYM);
					}
\+					RETURN(plusSYM);
\*					RETURN(starSYM);
\-					RETURN(minusSYM);
\/					RETURN(divideSYM);
\&					RETURN(andSYM);
\|					RETURN(orSYM);
\~					RETURN(notSYM);
\;.*					RETURN(commentSYM);
\#					RETURN(hashSYM);
\,					RETURN(commaSYM);
\(					RETURN(lparenSYM);
\)					RETURN(rparenSYM);
\'[^\'\n]*\'				{
						yytext[yyleng-1]='\0';
						yytext[STRINGLEN-1]='\0';
						strcpy(yylval.str,yytext+1);
						RETURN(stringSYM);
					}
.					RETURN(dummySYM);
%%

#define UNPUTSTACKSIZE (30)
#define CURLINESIZE (150)
#define LISTLINESIZE (180)

PRIVATE char unputstack[UNPUTSTACKSIZE];
PRIVATE int unputsp=0;
PRIVATE char curline[CURLINESIZE]={'\0'};
PRIVATE int lineptr=0;

PUBLIC char listline[LISTLINESIZE];

PRIVATE int mnem()
	{
		strupr(yytext);
		strcpy(yylval.mnem,yytext);
		return mnemonicSYM;
	}

PRIVATE char as_unput(c)
	char c;
	{
		if (unputsp>=UNPUTSTACKSIZE) fatal("UnPut stack overflow");
		
		unputstack[unputsp++]=c;
		return c;
	}
	
PRIVATE char output(c)
	char c;
	{
		return c;
	}
	
PRIVATE int as_input(char *buf, int maxsize) {
		char c;

		*buf = 0;
		*(buf + 1) = 0;

		if (unputsp>0) {
			*buf = unputstack[--unputsp];
			return 1;
		}
		
		if (curline[lineptr]=='\0') {
			if (feof(yyin)) {
				return 0;
			}
			
			if (!fgets(curline,CURLINESIZE-1,yyin)) return 0;
				
			sprintf(listline,"%04X               %4d %s",lc,++lineno,curline);
			lineptr=0;
		}

		*buf =  curline[lineptr++];

		return 1;
	}
	
PUBLIC void reset_input()
	{
		rewind(yyin);
		lineptr=0;
		curline[0]='\0';
		unputsp=0;
		lineno=0;
	}
