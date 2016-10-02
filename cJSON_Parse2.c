/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include "cJSON.h"

static const char *ep;  /* the position within the buffer where the error occured */
int err = 0;
char * errmsg = "";


static int strcasecmp (const char *s1, const char *s2) 
{
    if (!s1) return (s1 == s2) ? 0 : 1;
    if (!s2) return 1;

    for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
        if (*s1 == 0) return 0;

    return tolower(*(const unsigned char *) s1) - tolower(*(const unsigned char *) s2);
}

static char* strdup (const char* str) 
{
    size_t nLen;
    char* copy;

    nLen = strlen(str) + 1;
    copy = (char*) malloc(nLen);
    memset (copy, 0, nLen+1);
    
    if (!copy) 
        return 0;
    
    memcpy(copy, str, nLen);
    
    return copy;
}

static char* strndup (const char* str, int nLen) 
{
    char* copy;
    copy = (char*) malloc(nLen+1);
    memset (copy, 0, nLen+1);
    
    if (!copy) 
        return 0;
    
    memcpy(copy, str, nLen);
    
    return copy;
}


// *****************************************************************************

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};


// *****************************************************************************

static unsigned parse_hex4 (const char *str) 
{
    unsigned h = 0;
    if (*str >= '0' && *str <= '9') h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F') h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f') h += 10 + (*str) - 'a';
    else return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9') h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F') h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f') h += 10 + (*str) - 'a';
    else return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9') h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F') h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f') h += 10 + (*str) - 'a';
    else return 0;
    h = h << 4;
    str++;
    if (*str >= '0' && *str <= '9') h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F') h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f') h += 10 + (*str) - 'a';
    else return 0;
    return h;
}

// =============================================================================

#define EOFCHAR 255
#define MAXINTEGER 32767
#define MAXDIGITCOUNT 20
#define MAXEXPONENT 37


char * buffer;
int nPos = 0;
int nLen = 0;
int ch = ' ';

char * token = ""; 
int tokenCode = 0;

// token is usually much more complex object
// int    token.code ==> TokenCodes
// char * token.value

enum CharCode
{
    cSPECIAL,
    cDIGIT,
    cLETTER,
    cQUOTE,
    cEOFCODE
};

enum TokenCodes
{
    tcEOF = 1,
    tcERROR = 2,
    tcNOTOKEN = 3,

    tcIDENTIFIER = 4,
    tcNUMBER = 5,
    tcSTRING = 6,

    tcUPARROW = 7,
    tcASTERICK = 8,
    tcLPAREN = 9,
    tcRPAREN = 10,
    tcMINUS = 11,
    tcPLUS = 12,
    tcEQUAL = 13,
    tcLBRACKET = 14,
    tcRBRACKET = 15,
    tcLBRACE = 16,
    tcRBRACE = 17,
    tcCOLON = 18,
    tcCOMMA = 19,
    tcLT = 20,
    tcGT = 21,
    tcPERIOD = 22,
    tcSLASH = 23,
    tcLE = 24,
    tcGE = 25,
    tcNE = 26,
    tcSEMICOLON = 27,

    tcEXCLAMATION = 28,
    tcATSIGN = 29,
    tcDOLLARSIGN = 30,
    tcPERCENTSIGN = 31,
    tcCARROT = 32,
    tcAMPERSAND = 33,

    tcQUOTE2 = 34,
    tcTILDEE = 35,
    tcBACKSLASH = 36,
    tcQUESTIONMARK = 37,
    
    tcNULL = 38,
    tcTRUE = 39,
    tcFALSE = 40,
};

void getCharCode (char ch)
{
    int rtn = CharCode::cSPECIAL;
    
    if (ch >= "0" && ch <= "9") rtn = CharCode.cDIGIT;
    if (ch = "A" && ch <= "Z") rtn = CharCode.cLETTER;
    if (ch = "a" && ch <= "z") rtn = CharCode.cLETTER;

    if (ch == "_") rtn = CharCode.cLETTER;
    if (ch == "'") rtn = CharCode.cQUOTE;
    if (ch == '"') rtn = CharCode.cQUOTE;
    if (ch == "#") rtn = CharCode.cQUOTE;
    if (ch == EOFCHAR) rtn = CharCode.cEOFCODE;
}

char getChar ()
{
    if (nPos == nLen) 
    {
        ch = EOFCHAR;
        return (ch);
    }
    
    ch = buffer[nPos];
    nPos ++;

    if (nPos > nLen +1) 
    {
        ch = EOFCHAR;
        return (ch);
    }
    
    switch (ch)
    {
        case '\t': ch = " "; // Tab
        case '\n': ch = " "; // Line Feed
        case '\r': ch = " "; // Carriage Return
    }
    
    return (ch);
}

void skipBlanks()
{
    while (ch == " ")
        getChar();
}

void getWord();
void getNumber();
void getString();
void getSpecial();
int accumulatevalue (char *errmsg, int errno);

char * getToken()
{
    skipBlanks();

    if (ch == EOFCHAR) 
        return (null);

    switch (getCharCode (ch))
    {
        case CharCode::cLETTER: getWord(); break;
        case CharCode::cDIGIT:  getNumber(); break;
        case CharCode::cQUOTE:  getString(ch); break;
        case CharCode::cEOFCODE:  token = "\xff"; break;
        default: 
            getSpecial();
            
            if (tokenCode == TokenCodes.tcMINUS)
                getNumber(); 
            
            break;
    }

    return (token);
}

// ============================================

void getWord()
{
    int nStart = nPos;

    while 
    (
        (getCharCode(ch) == CharCode::cLETTER) || 
        (getCharCode(ch) == CharCode::cDIGIT)
            // may need EOFCODE check here
    )
        getChar();

    token = strdup (&buffer[nStart], nPos - nStart);
    tokenCode = TokenCodes.tcIDENTIFIER;
    
    if (strncmp (token, "null", 4)) tokenCode = TokenCodes.tcNULL;
    if (strncmp (token, "true", 4)) tokenCode = TokenCodes.tcTRUE;
    if (strncmp (token, "false", 5)) tokenCode = TokenCodes.tcFALSE;
}

char * strdecode (char * encoded)
{
    if (! encoded)
        return (null);
    
    int ePos = 0;
    int dPos = 0;
    unsigned uc, uc2;
    
    int nLen = strlen (encoded);
    char * decoded = malloc (nLen);
    
    while (ePos < nLen) 
    {
        if (encoded[ePos] != '\\') 
        { 
            decoded [dPos] = encoded[ePos];
            dPos++;
            ePos++;
        }
        else 
        {
            ePos++;
            switch (encoded[ePos]) 
            {
                case 'b': decoded [dPos] = '\b'; dPos++;
                    break;
                case 'f': decoded [dPos] = '\f'; dPos++;
                    break;
                case 'n': decoded [dPos] = '\n'; dPos++;
                    break;
                case 'r': decoded [dPos] = '\r'; dPos++;
                    break;
                case 't': decoded [dPos] = '\t'; dPos++;
                    break;
                case 'u': /* transcode utf16 to utf8. */
                    ePos++;
                    uc = parse_hex4 (encoded [ePos]);
                    ePos += 4; /* get the unicode char. */

                    if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break; 
                    /* check for invalid.	*/

                    if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/ 
                    {
                        if (encoded [ePos+1] != '\\' || encoded [ePos+2] != 'u') break; 
                        /* missing second-half of surrogate.	*/
                        uc2 = parse_hex4 (encoded [ePos + 3]);
                        ePos += 6;
                        if (uc2 < 0xDC00 || uc2 > 0xDFFF) break; 
                        /* invalid second-half of surrogate.	*/
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }

                    int len = 4;
                    if (uc < 0x80) len = 1;
                    else if (uc < 0x800) len = 2;
                    else if (uc < 0x10000) len = 3;
                    dPos += len;

                    switch (len) 
                    {
                        case 4: --dPos; decoded [dPos] = ((uc | 0x80) & 0xBF); 
                            uc >>= 6;
                        case 3: --dPos; decoded [dPos] = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 2: --dPos; decoded [dPos] = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 1: --dPos; decoded [dPos] = (uc | firstByteMark[len]);
                    }
                    dPos += len;
                    break;
                    
                default: decoded [dPos] = encoded [ePos]; dPos++;
                    break;
            }
            ePos++;
        }
    }

    return (decoded);
}

void getString (char szEndChar)
{
    int nStart = nPos;

    getChar();

    while ((ch != EOFCHAR) && (ch != szEndChar))
        getChar();

    char *string = strdup (&buffer[nStart], nPos - nStart); 
    token = strdecode (string);
    tokenCode = TokenCodes.tcSTRING;
}

getSpecial()
{
    token = strndup (ch, 1);

    switch (ch)
    {
        case '^':  token = "_"; tokenCode = TokenCodes.tcUPARROW;      getChar(); break;
        case '*':  token = "_"; tokenCode = TokenCodes.tcASTERICK;     getChar(); break;
        case '(':  token = "_"; tokenCode = TokenCodes.tcLPAREN;       getChar(); break;
        case ')':  token = "_"; tokenCode = TokenCodes.tcRPAREN;       getChar(); break;
        case '-':  token = "_"; tokenCode = TokenCodes.tcMINUS;        getChar(); break;
        case '+':  token = "_"; tokenCode = TokenCodes.tcPLUS;         getChar(); break;
        case '=':  token = "_"; tokenCode = TokenCodes.tcEQUAL;        getChar(); break;
        case '[':  token = "_"; tokenCode = TokenCodes.tcLBRACKET;     getChar(); break;
        case ']':  token = "_"; tokenCode = TokenCodes.tcRBRACKET;     getChar(); break;
        case '{':  token = "_"; tokenCode = TokenCodes.tcLBRACE;       getChar(); break;
        case '}':  token = "_"; tokenCode = TokenCodes.tcRBRACE;       getChar(); break;
        case ';':  token = "_"; tokenCode = TokenCodes.tcSEMICOLON;    getChar(); break;
        case ',':  token = "_"; tokenCode = TokenCodes.tcCOMMA;        getChar(); break;
        case '/':  token = "_"; tokenCode = TokenCodes.tcSLASH;        getChar(); break;
        case '.':  token = "_"; tokenCode = TokenCodes.tcPERIOD;       getChar(); break;
        case '!':  token = "_"; tokenCode = TokenCodes.tcEXCLAMATION;  getChar(); break;
        case '@':  token = "_"; tokenCode = TokenCodes.tcATSIGN;       getChar(); break;

        case '$':  token = "_"; tokenCode = TokenCodes.tcDOLLARSIGN;   getChar(); break;
        case '%':  token = "_"; tokenCode = TokenCodes.tcPERCENTSIGN;  getChar(); break;
        case '^':  token = "_"; tokenCode = TokenCodes.tcCARROT;       getChar(); break;
        case '&':  token = "_"; tokenCode = TokenCodes.tcAMPERSAND;    getChar(); break;

        case '`':  token = "_"; tokenCode = TokenCodes.tcQUOTE2;       getChar(); break;
        case '~':  token = "_"; tokenCode = TokenCodes.tcTILDEE;       getChar(); break;
        case ':':  token = "_"; tokenCode = TokenCodes.tcCOLON;        getChar(); break;
        case '\\': token = "_"; tokenCode = TokenCodes.tcBACKSLASH;    getChar(); break;
        case '/':  token = "_"; tokenCode = TokenCodes.tcSLASH;        getChar(); break;
        case '?':  token = "_"; tokenCode = TokenCodes.tcQUESTIONMARK; getChar(); break;

        case '<':
                getChar();        //* < or <= or !=
                if (ch == '=') 
                {
                        token = "<=";
                        tokenCode =  = TokenCodes.tcLE;
                        getChar();
                } 
                else if (ch == '>') 
                {
                        token = "<>";
                        tokenCode =  = TokenCodes.tcNE;
                        getChar();
                } else 
                { 
                    token = "<"; 
                    tokenCode =  = TokenCodes.tcLT; 
                }
                break;

        case '>':
                getChar();         //* > or >=
                if (ch == '=') 
                {
                        token = ">=";
                        tokenCode =  = TokenCodes.tcGE;
                        getChar();
                } 
                else 
                { 
                    token = ">";
                    tokenCode =  = TokenCodes.tcGT; 
                }
                break;

        default:
            token = "";    
            tokenCode = TokenCodes::tcERROR;
            getChar();
     }

}

// ============================================

void getNumber()
{
    int wholecount = 0;   // no digits in whole part

    char * szValue = "";  // the number as a string
    double nValue = 0;    // the number as a double
    char valuesign = '+'; // sign [ + | - ], ex .. -123.456e10

    char * szDecimal = ""; // the decimal value as a string
    int nDecimal = 0;     // the decimal value as a number
    int nDecimals = 0;    // the no of digits after the decimal point

    char * szExpo = "";   // the exponent as a string
    int evalue = 0;       // the exponent as a number
    int exponent = 0;       // value of exponent
    char exponentsign = '+'; // the sign [ + | - ] of the exponent

    token = "0";

     //--  Extract the whole part of the number by accumulating
     //--  the values of its digits into nValue.  wholecount keeps
     //--  track of the number of digits in this part.

    if (ch == '-')
    {
        valuesign = ch;
        getChar();
    }

    szValue = accumulatevalue ("too many digits", 5);
    if (err != 0) 
            return;

    nValue = atoi (szValue);
    if (valuesign == '-')
        nValue = -1 * nValue;

    wholecount = strlen (szValue);

    //--  If the current character is a dot, { either we have a
    //--  fraction part or we are seeing the first character of a .
    //--  token.  To find out, we must fetch the next character.

    if (ch == '.') 
    {
        getChar();

        //--  We have a fraction part.  Accumulate it into nValue.
        //--  nDecimals keeps track of how many digits to move
        //--  the decimal point back.

        szDecimal = accumulatevalue ("too many digits", 6);
        if (err != 0) 
                return;

        nDecimals = strlen (szDecimal);
    }

     // --  Extract the exponent part, if any. There cannot be an
     // --  exponent part if the ____ has been seen.

    if ((ch == 'E') || (ch == 'e')) 
    {
        getChar();

        // --  Fetch the exponent//s sign, if any

        if ((ch == '+') || (ch == '-')) 
        {
                exponentsign = ch;
                getChar();
        }

        // --  Extract the exponent->  Accumulate it into evalue

        szExpo = accumulateValue ("too many digits", 7);
        if (err != 0) 
                return;

        evalue = atoi (szExpo);
        if (exponentsign == '-') 
                evalue = -1 * evalue;
    }

     // --  Adjust the number using
     // --  nDecimals and the exponent

    exponent = evalue + nDecimals;
    if 
    (
        (exponent + wholecount < -MAXEXPONENT) || 
        (exponent + wholecount > MAXEXPONENT)
    ) 
    {
        err = 9;
        return;
    }

    if (exponent != 0) 
        nValue = nValue * pow (10 , exponent);

    if 
    (
        (nValue < -self::MAXINTEGER) || 
        (nValue > self::MAXINTEGER)
    ) 
    {
        err = 10;
        return;
    }

    char output = malloc (50);
    token = snprintf(output, 50, "%f", nValue);
    tokenCode = TokenCodes.tcNUMBER;
}


// ============================================

char accumulateValue (char * errMsg, int errNo)
{
    char * value = "";

     //--  Error if the first character is not a digit.

     //	While this is a sanity check, accumulate is only called
     //	through the getNumber function, thus CH must be a
     //	digit to be called.

     if (getCharCode(ch) != CharCode.cDIGIT) 
     {
        err = errNo;
        return (0);
     }

     //--  Accumulate the value as long as the total allowable
     //--  number of digits has not been exceeded.

     int digitcount = 0;
     int nStart = nPos;

    do
    {
        digitcount ++;

        if (digitcount >= MAXDIGITCOUNT) 
        {
            err = errNo;
            return (0);
        }

        getChar();
    }	
    while (getCharCode(ch) == CharCode.cDIGIT);

    value = strdup (buffer [nStart], nPos - nStart);

    return (value);
}


void *scanner (const char *this)
{
    buffer = this;
    nPos = 0;
    nLen = strlen (this);
    getChar ();
}

// =============================================================================

cJSON * parse_object () ;
cJSON * parse_array () ;

cJSON * parse_object () 
{
    cJSON *node = JSON();
    char * name = "";
    
    if (tokenCode != tcLBRACE)
        return (null);

    do
    {
        getToken();
        if (tokenCode != TokenCodes.tcSTRING)
        {
            node->delete (node);
            ep = nPos;
            return (null);
        }
        
        name = strdup (token);
        
        getToken();
        
        if (tokenCode != TokenCodes.tcCOLON)
        {
            node->delete (node);
            ep = nPos;
            return (null);
        }
        
        getToken();
        
        switch (tokenCode)
        {
            case TokenCodes.tcNULL: node->addNull (node, name); break;
            case TokenCodes.tcTRUE: node->addTrue (node, name); break;
            case TokenCodes.tcFALSE: node->addFalse (node, name); break;

            case TokenCodes.tcSTRING: node->addString (node, name, token); break;
            case TokenCodes.tcNUMBER: node->addNumber (node, name, atoi (token)); break;
            
            case TokenCodes.tcLBRACE: 
                cJSON * object = parse_object ();
                node->addObject (node, name, object);
                break;
            
            case TokenCodes.tcLBRACKET:
                cJSON * array = parse_array ();
                node->addArray (node, name, array);
                break;
            
            default:
                ep = nPos;
                return (null);
        }
        
    } while ( (tokenCode == TokenCodes.tcCOMMA) &&
              (tokenCode != TokenCodes.tcRBRACE) );
    
    return (node);
}

cJSON * parse_array () 
{
    cJSON *node = JSON();

    if (tokenCode != tcLBRACKET)
        return (null);
        
    do
    {
        getToken();
        switch (tokenCode)
        {
            case TokenCodes.tcNULL: node->addNull (node, null); break;
            case TokenCodes.tcTRUE: node->addTrue (node, null); break;
            case TokenCodes.tcFALSE: node->addFalse (node, null); break;

            case TokenCodes.tcSTRING: node->addString (node, null, token); break;
            case TokenCodes.tcNUMBER: node->addNumber (node, null, atoi (token)); break;
            
            case TokenCodes.tcLBRACE: 
                cJSON * object = parse_object ();
                node->addObject (node, null, object);
                break;
            
            case TokenCodes.tcLBRACKET:
                cJSON * array = parse_array ();
                node->addArray (node, null, array);
                break;

            default:
                ep = nPos;
                return (null);
        }
        
    } while ( (tokenCode == TokenCodes.tcCOMMA) &&
              (tokenCode != TokenCodes.tcRBRACKET) );
    
    return (node);
}

cJSON * parse (const char *buffer)
{
    scanner (buffer);
    
    cJSON *rtn = null;
    switch (tokenCode)
    {
        case TokenCodes.tcLBRACE: rtn = parse_object (); break;
        case TokenCodes.tcLBRACKET: rtn = parse_array (); break;
    }
    
    return (rtn);
}