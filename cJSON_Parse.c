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
    size_t len;
    char* copy;

    len = strlen(str) + 1;
    if (!(copy = (char*) malloc(len))) return 0;
    memcpy(copy, str, len);
    return copy;
}

// *****************************************************************************

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

// *****************************************************************************

/* Utility to jump whitespace and cr/lf */
static const char *skip (const char *in) 
{
    while (in && *in && (unsigned char) *in <= 32) in++;
    return in;
}

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

// *****************************************************************************

static const char *parse_string (cJSON *item, const char *str) 
{
    const char *ptr = str + 1;
    char *ptr2;
    char *out;
    int len = 0;
    unsigned uc, uc2;
    if (*str != '\"') 
    {
        ep = str;
        return 0;
    } /* not a string! */

    while (*ptr != '\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++; /* Skip escaped quotes. */

    out = (char*) malloc(len + 1); /* This is how long we need for the string, roughly. */
    if (!out) return 0;

    ptr = str + 1;
    ptr2 = out;
    while (*ptr != '\"' && *ptr) 
    {
        if (*ptr != '\\') 
        { *ptr2++ = *ptr++; }
        else 
        {
            ptr++;
            switch (*ptr) 
            {
                case 'b': *ptr2++ = '\b';
                    break;
                case 'f': *ptr2++ = '\f';
                    break;
                case 'n': *ptr2++ = '\n';
                    break;
                case 'r': *ptr2++ = '\r';
                    break;
                case 't': *ptr2++ = '\t';
                    break;
                case 'u': /* transcode utf16 to utf8. */
                    uc = parse_hex4(ptr + 1);
                    ptr += 4; /* get the unicode char. */

                    if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break; /* check for invalid.	*/

                    if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs.	*/ 
                    {
                        if (ptr[1] != '\\' || ptr[2] != 'u') break; /* missing second-half of surrogate.	*/
                        uc2 = parse_hex4(ptr + 3);
                        ptr += 6;
                        if (uc2 < 0xDC00 || uc2 > 0xDFFF) break; /* invalid second-half of surrogate.	*/
                        uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
                    }

                    len = 4;
                    if (uc < 0x80) len = 1;
                    else if (uc < 0x800) len = 2;
                    else if (uc < 0x10000) len = 3;
                    ptr2 += len;

                    switch (len) 
                    {
                        case 4: *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 3: *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 2: *--ptr2 = ((uc | 0x80) & 0xBF);
                            uc >>= 6;
                        case 1: *--ptr2 = (uc | firstByteMark[len]);
                    }
                    ptr2 += len;
                    break;
                default: *ptr2++ = *ptr;
                    break;
            }
            ptr++;
        }
    }
    *ptr2 = 0;
    if (*ptr == '\"') ptr++;
    item->valuestring = out;
    item->type = cJSON_String;
    return ptr;
}

/* Predeclare these prototypes. */
static const char *parse_value (cJSON *item, const char *value);
static const char *parse_array (cJSON *item, const char *value);
static const char *parse_object (cJSON *item, const char *value);


/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number (cJSON *item, const char *num) 
{
    double n = 0, sign = 1, scale = 0;
    int subscale = 0, signsubscale = 1;

    if (*num == '-') sign = -1, num++; /* Has sign? */
    if (*num == '0') num++; /* is zero */
    if (*num >= '1' && *num <= '9') 
        do 
            n = (n * 10.0)+(*num++ -'0'); 
        while (*num >= '0' && *num <= '9'); /* Number? */

    if (*num == '.' && num[1] >= '0' && num[1] <= '9') 
    {
        num++;
        do 
            n = (n * 10.0)+(*num++ -'0'), scale--; 
        while (*num >= '0' && *num <= '9');

    } /* Fractional part? */
    if (*num == 'e' || *num == 'E') /* Exponent? */ 
    {
        num++;
        if (*num == '+') num++;
        else if (*num == '-') signsubscale = -1, num++; /* With sign? */
        while (*num >= '0' && *num <= '9') 
            subscale = (subscale * 10)+(*num++ -'0'); /* Number? */
    }

    n = sign * n * pow(10.0, (scale + subscale * signsubscale)); /* number = +/- number.fraction * 10^+/- exponent */

    item->setNumber (item, n);
    return num;
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value (cJSON *item, const char *value) 
{
    if (!value) return 0; /* Fail on null. */

    if (!strncmp(value, "null", 4))
    {
        item->type = cJSON_NULL;
        return value + 4;
    }

    if (!strncmp(value, "false", 5)) 
    {
        item->type = cJSON_False;
        item->valueint = 0;
        return value + 5;
    }

    if (!strncmp(value, "true", 4)) 
    {
        item->type = cJSON_True;
        item->valueint = 1;
        return value + 4;
    }

    if (*value == '\"') 
    { return parse_string(item, value); }

    if (*value == '-' || (*value >= '0' && *value <= '9')) 
    { return parse_number(item, value); }

    if (*value == '[') 
    { return parse_array(item, value); }

    if (*value == '{') 
    { return parse_object(item, value); }

    ep = value;
    return 0; /* failure. */
}

/* Build an array from input text. */
static const char *parse_array (cJSON *item, const char *value) 
{
    private *child;
    if (*value != '[') 
    {
        ep = value;
        return 0;
    } /* not an array! */

    item->type = cJSON_Array;
    value = skip(value + 1);
    if (*value == ']') return value + 1; /* empty array. */

    item->child = child = New_Item();
    if (!item->child) return 0; /* memory fail */
    value = skip(parse_value(child, skip(value))); /* skip any spacing, get the value. */
    if (!value) return 0;

    while (*value == ',') 
    {
        private *new_item;
        if (!(new_item = New_Item())) return 0; /* memory fail */
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_value(child, skip(value + 1)));
        if (!value) return 0; /* memory fail */
    }

    if (*value == ']') return value + 1; /* end of array */
    ep = value;
    return 0; /* malformed. */
}


/* Build an object from the text. */
static const char *parse_object (cJSON *item, const char *value) 
{
    private *child;
    if (*value != '{') 
    {
        ep = value;
        return 0;
    } /* not an object! */

    item->type = cJSON_Object;
    value = skip(value + 1);
    if (*value == '}') 
        return value + 1; /* empty array. */
    
    char *name = "";
    child = item->addObject(item, name);
    if (!item->child) 
        return 0;
    
    value = skip (parse_string (child, skip(value)));
    if (!value) return 0;
    child->string = child->valuestring;
    child->valuestring = 0;
    if (*value != ':') 
    {
        ep = value;
        return 0;
    } /* fail! */
    value = skip(parse_value (child, skip(value + 1))); /* skip any spacing, get the value. */
    if (!value) return 0;

    while (*value == ',') 
    {
        private *new_item;
        if (!(new_item = New_Item())) return 0; /* memory fail */
        child->next = new_item;
        new_item->prev = child;
        child = new_item;
        value = skip(parse_string(child, skip(value + 1)));
        if (!value) return 0;
        child->string = child->valuestring;
        child->valuestring = 0;
        if (*value != ':') 
        {
            ep = value;
            return 0;
        } /* fail! */
        value = skip(parse_value(child, skip(value + 1))); /* skip any spacing, get the value. */
        if (!value) return 0;
    }

    if (*value == '}') return value + 1; /* end of array */
    ep = value;
    return 0; /* malformed. */
}

/* Parse an object - create a new root, and populate. */
cJSON *ParseWithOpts (const char *value, const char **return_parse_end, bool require_null_terminated) 
{
    ep = 0;

    const char *end = 0;
    
    cJSON *c = JSON();
    if (!c) 
        return (null); /* memory fail */

    end = parse_value (c, skip (value));
    
    if (!end) 
    {
        c->delete(c);
        return 0;
    } /* parse failure. ep is set. */

    /* if we require null-terminated JSON without appended garbage, skip and 
     * then check for a null terminator */
    if (require_null_terminated) 
    {
        end = skip (end);
        if (*end) 
        {
            c->delete(c);
            ep = end;
            return (null);
        }
    }
    if (return_parse_end) 
        *return_parse_end = end;
    
    return (c);
}


// =============================================================================

/* Default options for cJSON_Parse */
cJSON *cJSON_Parse (const char *value) 
{ return ParseWithOpts (value, 0, 0); }

