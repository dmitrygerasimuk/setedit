/**[txh]********************************************************************

  Copyright (c) 2002 by Iv�n Baldo.
  This program is covered by the GPL license.

  Description:
  Parses a PHP script looking for function and class definitions.
  It can be compiled as an standalone program by defining STANDALONE.
  That's just an heuristic, not a real parser and I'm quite sure it can be
fooled, but is fast and small.
  Based on code from other parsers in SETEdit.


  Temporary notes (last upd. 2002/11/11):
    - normalize some names, maybe translate go* to seek* or search* and eat*
to skip*.
***************************************************************************/

//#define STANDALONE

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifndef STANDALONE
    #include <bufun.h>
#endif

#ifdef STANDALONE
//The following is as defined in bufun.h:
typedef void (*tAddFunc)(char *name, int len, int lineStart, int lineEnd);
const int MaxLenWith0=256;
char bfNomFun[MaxLenWith0];


int SearchPHPFuncs(char *srcBuffer, unsigned len, int mode /*FIXME: WTF?*/,
    tAddFunc addFunc);

static void printFunc(char *name, int len, int lineStart, int lineEnd)
{
    printf("%s [%d,%d]\n", name, lineStart, lineEnd);
}

int main(int argc, char *argv[])
{
    puts("PHP Functions Parser, Copyright 2002 by Iv�n Baldo.");
    if(argc != 2) {
        puts("Use: pphp file");
        return 1;
    }
    FILE *f = fopen(argv[1], "rt");
    if(!f) {
        printf("Can't open '%s'.\n", argv[1]);
        return 2;
    }
    fseek(f,0,SEEK_END);
    long len = ftell(f);
    fseek(f,0,SEEK_SET);
    char *buffer = new char[len+1];
    size_t bread = fread(buffer, len, 1, f);
    if(bread != 1) {
        printf("Error reading file '%s'.\n", argv[1]);
        perror("Error");
        fclose(f);
        return 3;
    }
    fclose(f);
    buffer[len] = 0;
    SearchPHPFuncs(buffer, len, 0, printFunc);
    delete [] buffer;
    return 0;
}
#endif // STANDALONE

/*** DECLARATIONS ***/

//Source code buffer pointers:
static const char *srcBufStart, *srcBufEnd; //Note: past from end!
static const char *curBufPtr; //Pointer to current position on buffer.
static unsigned curLine; //Current line number.
static unsigned curFuncNum; //Current function number.
static tAddFunc funcAddPtr;

enum eKeyWord { KEYWORD_NONE, KEYWORD_FUNCTION, KEYWORD_CLASS };

static eKeyWord goClassOrFunc();
static void goEndOfIdentifier();
static void processFunc(const char *className=NULL,
    const unsigned classNameSize=0);
static void processClass();
static void goMatching(const char startChar, const char endChar);
static void registerFunc(const char *funcName, const unsigned funcNameSize,
    const char *param, const unsigned paramSize, const unsigned funcStartLine,
    const char *className=NULL, unsigned classNameSize=0);


/*** INLINE DEFINITIONS ***/

inline static bool atEndOfBuf()
{
    return curBufPtr >= srcBufEnd;
}

inline static bool atStartOfString()
{
    if(*curBufPtr=='\'' || *curBufPtr=='"' || *curBufPtr=='`')
        return true;
    return false;
}

inline static bool atStartOfComment()
{
    if(atEndOfBuf()) return false;
    if(*curBufPtr=='#')
        return true;
    else if(*curBufPtr=='/')
        if(*(curBufPtr+1)=='/' || *(curBufPtr+1)=='*')
            return true;
    return false;
}

//Skip blanks leaving curBufPtr after the blanks.
//Returns true if it has skipped something.
inline static bool eatBlanks()
{
    const char * const p = curBufPtr;
    for(; !atEndOfBuf() && isspace(*curBufPtr); curBufPtr++)
        if(*curBufPtr=='\n') ++curLine;
    return p!=curBufPtr;
}

//Skip the rest of a line leaving curBufPtr at the beginning of the new line.
inline static void eatLine()
{
    for(; !atEndOfBuf(); curBufPtr++)
        if(*curBufPtr=='\n') {
            ++curLine;
            ++curBufPtr;
            return;
        }
}

//Skip a comment, leaving curBufPtr after it.
//Returns true if it has skipped something.
inline static bool eatComment()
{
    if(atEndOfBuf()) return false;
    if(*curBufPtr=='#') {
        eatLine();
        return true;
    } else if(*curBufPtr=='/' && *(curBufPtr+1)=='/') {
        eatLine();
        return true;
    } else if(*curBufPtr=='/' && *(curBufPtr+1)=='*') {
        for(curBufPtr+=2; !atEndOfBuf(); curBufPtr++) {
            if(*curBufPtr=='*' && *(curBufPtr+1)=='/') {
                curBufPtr+=2;
                return true;
            } else if(*curBufPtr=='\n')
                curLine++;
        }
        return true;
    }
    return false;
}

/*
    Position curBufPtr after the end of a string (after the ending
character). You should be positioned at the starting character of the string.
    Takes into account new lines and escape characters.
    Note: doesn't support "Here Documents", a different mechanism is needed
for that.
    Returns true if it has found the string and skipped it.
*/
inline static bool eatString()
{
    if(!atStartOfString()) return false;
    const char endChar=*curBufPtr;
    for(curBufPtr++; !atEndOfBuf(); curBufPtr++) {
        if(*curBufPtr=='\n')
            ++curLine;
        else if(*curBufPtr=='\\') {
            ++curBufPtr; //Skip next char...
            if(*curBufPtr=='\n') ++curLine; //But take new lines into account!
        } else if(*curBufPtr==endChar) {
            ++curBufPtr; //Skip ending char!
            return true;
        }
    }
    return true;
}

//Skip blanks or comments leaving curBufPtr after them.
//Returns true if it has skipped something.
inline static bool eatBlanksOrComments()
{
    const char * const p = curBufPtr;
    const char *lastBufPtr=0;
    while(lastBufPtr!=curBufPtr) {
        lastBufPtr=curBufPtr;
        eatBlanks();
        eatComment();
    }
    return p!=curBufPtr;
}

//Skip blanks, comments or strings, leaving curBufPtr after them.
//Returns true if it has skipped something.
inline static bool eatBlanksOrCommentsOrStrings()
{
    const char * const p = curBufPtr;
    const char *lastBufPtr=0;
    while(lastBufPtr!=curBufPtr) {
        lastBufPtr=curBufPtr;
        eatBlanks();
        eatComment();
        eatString();
    }
    return p!=curBufPtr;
}

/*
    Returns true if at curBufPtr says "function" (or "class" respectively)
case insensitively.
    This functions avoid calling the strncasecmp libc function and speed up
things, while at the same time not crossing the end of the buffer.
*/
inline static bool saysFunction()
{
    if(*curBufPtr!='f' && *curBufPtr!='F') return false;
    if(*(curBufPtr+1)!='u' && *(curBufPtr+1)!='U') return false;
    if(curBufPtr+sizeof("function")-1 >= srcBufEnd) return false;
    if(*(curBufPtr+2)!='n' && *(curBufPtr+2)!='N') return false;
    if(*(curBufPtr+3)!='c' && *(curBufPtr+3)!='C') return false;
    if(*(curBufPtr+4)!='t' && *(curBufPtr+4)!='T') return false;
    if(*(curBufPtr+5)!='i' && *(curBufPtr+5)!='I') return false;
    if(*(curBufPtr+6)!='o' && *(curBufPtr+6)!='O') return false;
    if(*(curBufPtr+7)!='n' && *(curBufPtr+7)!='N') return false;
    return true;
}
inline static bool saysClass()
{
    if(*curBufPtr!='c' && *curBufPtr!='C') return false;
    if(*(curBufPtr+1)!='l' && *(curBufPtr+1)!='L') return false;
    if(curBufPtr+sizeof("class")-1 >= srcBufEnd) return false;
    if(*(curBufPtr+2)!='a' && *(curBufPtr+2)!='A') return false;
    if(*(curBufPtr+3)!='s' && *(curBufPtr+3)!='S') return false;
    if(*(curBufPtr+4)!='s' && *(curBufPtr+4)!='S') return false;
    return true;
}

inline static int min(int value1, int value2)
{
    return value1 < value2 ? value1 : value2;
}


/*** DEFINITIONS ***/

//The entry point:
int SearchPHPFuncs(char *srcBuffer, unsigned len, int mode /*FIXME: WTF?*/,
    tAddFunc addFunc)
{
    srcBufStart = srcBuffer;
    srcBufEnd = srcBufStart + len + 1; //Note: 1 past the end!
    curBufPtr = srcBufStart;
    curLine = 1;
    curFuncNum = 0;
    funcAddPtr = addFunc;
    
    while( !atEndOfBuf() ) {
        const eKeyWord kwrdt = goClassOrFunc();
        if( kwrdt == KEYWORD_FUNCTION )
            processFunc();
        else if( kwrdt == KEYWORD_CLASS )
            processClass();
        else
            ++curBufPtr; //a } could be found or other strange character.
    }
    return curFuncNum; //we return the number of functions found!
}

/*
    Position curBufPtr at the first letter of a class or function
declaration (at the beginning of its name).
    If a } is found, we return KEYWORD_NONE.
    If we reach the end of the buffer, it returns KEYWORD_NONE.
    Note: doesn't detect functions or classes at the beginning of the buffer
or at the starting of a PHP block, only when they are surrounded of blanks
and comments, shouldn't be a big problem right?
*/
static eKeyWord goClassOrFunc()
{
    while( !atEndOfBuf() ) {
        if( eatBlanksOrComments() ) { //Possible beginning of keyword!
            if( saysFunction() ) {
                curBufPtr += sizeof("function")-1;
                if( eatBlanksOrComments() ) {
                    return KEYWORD_FUNCTION;
                }
            } else {
                if( saysClass() ) {
                    curBufPtr += sizeof("class")-1;
                    if( eatBlanksOrComments() ) {
                        return KEYWORD_CLASS;
                    }
                }
            }
        } else { //Not a blank or comment...
            if(!eatString()) {
                if(*curBufPtr=='}')
                    return KEYWORD_NONE; //Needed so we can process it
                else                     //somewhere else
                    ++curBufPtr; //Unknown character, skip it.
            }
        }
    }
    return KEYWORD_NONE; //No more keywords found!
}

/*
    Process a function. Should be called with curBufPtr pointing exactly at
the first letter of the identifier (after eating the blanks following the
"function" keyword).
    It returns after processing the function body, just after the enclosing
'}'.
    If we are inside a class definition, then pass a pointer to the beginning
of the name of the class and its length in characters, without counting the
null character or other characters (real length of the word).
*/
static void processFunc(const char *className=NULL,
    const unsigned classNameSize=0)
{
    const char *funcNameStartPtr = curBufPtr;
    const unsigned funcStartLine = curLine;
    goEndOfIdentifier();
    const unsigned funcNameSize = curBufPtr-funcNameStartPtr;
    eatBlanksOrComments();
    if(*curBufPtr != '(') return; //Must be the parameters declaration!
    const char *paramStartPtr = curBufPtr;
    goMatching('(',')');
    const unsigned paramSize = curBufPtr-paramStartPtr;
    eatBlanksOrComments();
    if(*curBufPtr != '{') return; //Must be the function body!
    goMatching('{','}');
    
    registerFunc(funcNameStartPtr, funcNameSize, paramStartPtr, paramSize,
        funcStartLine, className, classNameSize);
}


/*
    Process a class. Should be called with curBufPtr pointing exactly at
the first letter of the identifier (after eating the blanks following the
"class" keyword).
    It returns after processing the class body, just after the enclosing
'}'.
*/
static void processClass()
{
    const char *classNameStartPtr = curBufPtr;
    goEndOfIdentifier();
    const unsigned classNameSize = curBufPtr-classNameStartPtr;
    
    while(!atEndOfBuf()) {
        const eKeyWord kwrdt = goClassOrFunc();
        if( kwrdt == KEYWORD_FUNCTION )
            processFunc(classNameStartPtr, classNameSize);
        else { //No more functions, find the end of the class:
            while( !atEndOfBuf() ) {
                if(*curBufPtr=='}') {
                    ++curBufPtr;
                    return;
                }
                if(*curBufPtr=='{')
                    goMatching('{','}');
                else
                    if( !eatBlanksOrCommentsOrStrings() )
                        ++curBufPtr; //Unknown char.
            }
        }
    }
}

/*
    Position curBufPtr after the matching endChar, balancing the startChar's
and endChar's between.
    You should be positioned just over the startChar.
*/
static void goMatching(const char startChar, const char endChar)
{
    unsigned int level=1;
    ++curBufPtr;
    while(!atEndOfBuf()) {
        eatBlanksOrCommentsOrStrings();
        if(*curBufPtr==startChar)
            ++level;
        else if(*curBufPtr==endChar) {
            --level;
            if(level==0) {
                ++curBufPtr;
                return;
            }
        }
        ++curBufPtr;
    }
}

/*
    Position curBufPtr after the end of an identifier (name of function,
etc.). When calling, should be positioned on the first letter of an
identifier.
*/
static void goEndOfIdentifier()
{
    //First character must be an alphabetic letter:
    if(!isalpha(*curBufPtr)) return;
    ++curBufPtr;
    for(; !atEndOfBuf() && (isalnum(*curBufPtr)||*curBufPtr=='_');
        curBufPtr++);
}

/*
    It calls funcAddPtr with the apropriate parameters and registers the
function found.
*/
static void registerFunc(const char *funcName, const unsigned funcNameSize,
    const char *param, const unsigned paramSize, const unsigned funcStartLine,
    const char *className=NULL, unsigned classNameSize=0)
{
    ++curFuncNum;
    int tsz=funcNameSize+paramSize+1+1+classNameSize+1;
    if(tsz > MaxLenWith0-1) return; //Doesn't fit on the buffer!
    char *p=bfNomFun;
    strncpy(p, funcName, funcNameSize);
    p+=funcNameSize;
    strncpy(p, param, paramSize);
    p+=paramSize;
    if(className && classNameSize) {
        *p++=' ';
        *p++='(';
        strncpy(p, className, classNameSize);
        p+=classNameSize;
        *p++=')';
    }
    *p++=0;

    funcAddPtr(bfNomFun, p-bfNomFun-1, funcStartLine, curLine);
}

