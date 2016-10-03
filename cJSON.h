/*
  Copyright (c) 2009 Dave Gamble
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif
    
#include <stdlib.h>

#define bool int
#define true 1
#define false 0
#define null 0
    
    
/* cJSON Types: */
#define cJSON_False 0
#define cJSON_True 1
#define cJSON_NULL 2
#define cJSON_Number 3
#define cJSON_String 4
#define cJSON_Array 5
#define cJSON_Object 6
	

typedef struct cJSON cJSON;
struct cJSON
{
    void * private;

    cJSON * (* addNull) (cJSON * this, const char *name);
    cJSON * (* addTrue) (cJSON * this, const char *name);
    cJSON * (* addFalse) (cJSON * this, const char *name);
    cJSON * (* addBool) (cJSON * this, const char *name, bool b);
    cJSON * (* addNumber) (cJSON * this, const char *name, double number);
    cJSON * (* addString) (cJSON * this, const char *name, const char *string);
    cJSON * (* addNumbers) (cJSON * this, int count, double *numbers);
    cJSON * (* addStrings) (cJSON * this, int count, const char **strings);
    cJSON * (* addArray) (cJSON * this, const char *name);
    cJSON * (* addObject) (cJSON * this, const const char *name);

    /* Delete a cJSON entity and all children. */
    bool  (* destruct) (cJSON * this);

    /* Returns the number of items in an array (or object). */
    int   (* getSize) (cJSON * this);
    int   (* getType) (cJSON * this);
    const char * (* getName) (cJSON * this);
    const char * (* setName) (cJSON * this, const char * name);

    cJSON * (* getNext) (cJSON * this);
    
    double (* getNumber) (cJSON * this);
    const char * (* getString) (cJSON * this);
    double (* setNumber) (cJSON * this, double value);
    const char * (* setString) (cJSON * this, const char * value);
    
    /* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
    cJSON * (* getByPos) (cJSON *this, int nPos);
    
    /* Get item "string" from object. Case insensitive. */
    cJSON * (* getByName) (cJSON *this, const char *string);
    
    /* return the child of this item, if there is one. */
    cJSON * (* getChild) (cJSON *this);

    /* Duplicate a cJSON item */
    cJSON * (* duplicate) (cJSON *this, int recurse);
    
    /* Render a cJSON entity to text for transfer/storage. Free the char* when finished. */
    const char * (* print) (cJSON * this);

    /* Render a cJSON entity to text for transfer/storage without any formatting. Free the char* when finished. */
    const char * (* printUnformatted) (cJSON * this);

    /* For analysing failed parses. This returns a pointer to the parse error. 
     * You'll probably need to look a few chars back to make sense of it. Defined 
     * when Parse() returns 0. 0 when Parse() succeeds. */
    int (* getError) (cJSON * this);

    /* Update array items. */
    bool  (* setArray) (cJSON * this, int nPos, cJSON *newitem);
    bool  (* setObject) (cJSON * this, const char *string, cJSON *newitem);

    void  (* unsetArray) (cJSON * this, int nPos, cJSON *newitem);
    void  (* unsetObject) (cJSON * this, const char *string, cJSON *newitem);
};
    
/* Supply a block of JSON, and this returns a cJSON object you can interrogate. Call Delete when finished. */
extern cJSON * JSON (const char * szJSON);



#ifdef __cplusplus
}
#endif

#endif
