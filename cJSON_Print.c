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

// *****************************************************************************

/* Invote print_string_ptr (which is useful) on an item. */
static char *print_string (cJSON *item) 
{ return print_string_ptr(item->getString()); }

/* Render the cstring provided to an escaped version that can be printed. */
static char *print_string_ptr (const char *str) 
{
    const char *ptr;
    char *ptr2, *out;
    int len = 0;
    unsigned char token;

    if (!str) return strdup("");
    ptr = str;
    while ((token = *ptr) && ++len) 
    {
        if (strchr("\"\\\b\f\n\r\t", token)) len++;
        else if (token < 32) len += 5;
        ptr++;
    }

    out = (char*) malloc(len + 3);
    if (!out) return 0;

    ptr2 = out;
    ptr = str;
    *ptr2++ = '\"';
    while (*ptr) 
    {
        if ((unsigned char) *ptr > 31 && *ptr != '\"' && *ptr != '\\') *ptr2++ = *ptr++;
        else 
        {
            *ptr2++ = '\\';
            switch (token = *ptr++) 
            {
                case '\\': *ptr2++ = '\\';
                    break;
                case '\"': *ptr2++ = '\"';
                    break;
                case '\b': *ptr2++ = 'b';
                    break;
                case '\f': *ptr2++ = 'f';
                    break;
                case '\n': *ptr2++ = 'n';
                    break;
                case '\r': *ptr2++ = 'r';
                    break;
                case '\t': *ptr2++ = 't';
                    break;
                default: sprintf(ptr2, "u%04x", token);
                    ptr2 += 5;
                    break; /* escape and print */
            }
        }
    }
    *ptr2++ = '\"';
    *ptr2++ = 0;
    return out;
}

/* Render the number nicely from the given item into a string. */
static char *print_number (cJSON *item) 
{
    char *str;
    double d = item->getNumber (item);
    if (fabs(((double) item->getNumber(item) - d) <= DBL_EPSILON && 
            d <= INT_MAX && 
            d >= INT_MIN) 
    {
        str = (char*) malloc(21); /* 2^64+1 can be represented in 21 chars. */
        if (str) sprintf(str, "%d", item->getNumber(item));
    } 
    else 
    {
        str = (char*) malloc(64); /* This is a nice tradeoff. */
        if (str) 
        {
            if (fabs(floor(d) - d) <= DBL_EPSILON && fabs(d) < 1.0e60) sprintf(str, "%.0f", d);
            else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9) sprintf(str, "%e", d);
            else sprintf(str, "%f", d);
        }
    }
    return str;
}


/* Predeclare these prototypes. */
static char *print_value (cJSON *item, int depth, int fmt);
static char *print_array (cJSON *item, int depth, int fmt);
static char *print_object (cJSON *item, int depth, int fmt);

/* Render a value to text. */
static char *print_value (cJSON *item, int depth, int fmt) 
{
    char *out = 0;
    if (!item) return 0;
    switch ((item->type)&255) 
    {
        case cJSON_NULL: out = strdup("null");
            break;
        case cJSON_False: out = strdup("false");
            break;
        case cJSON_True: out = strdup("true");
            break;
        case cJSON_Number: out = print_number(item);
            break;
        case cJSON_String: out = print_string(item);
            break;
        case cJSON_Array: out = print_array(item, depth, fmt);
            break;
        case cJSON_Object: out = print_object(item, depth, fmt);
            break;
    }
    return out;
}

/* Render an array to text */
static char *print_array (cJSON *item, int depth, int fmt) 
{
    char **entries;
    char *out = 0, *ptr, *ret;
    int len = 5;
    cJSON *child = item->getChild();
    int numentries = 0, i = 0, fail = 0;

    /* How many entries in the array? */
    //while (child) numentries++, child = child->getNext(item);
    numentries = child->getSize (item);
    
    /* Explicitly handle numentries==0 */
    if (!numentries) 
    {
        out = (char*) malloc(3);
        if (out) strcpy(out, "[]");
        return out;
    }
    /* Allocate an array to hold the values for each */
    entries = (char**) malloc(numentries * sizeof (char*));
    if (!entries) return 0;
    memset(entries, 0, numentries * sizeof (char*));
    
    /* Retrieve all the results: */
    child = item->getNext (item);
    while (child && !fail) 
    {
        ret = print_value(child, depth + 1, fmt);
        entries[i++] = ret;
        if (ret) len += strlen(ret) + 2 + (fmt ? 1 : 0);
        else fail = 1;
        child = child->getNext(child);
    }

    /* If we didn't fail, try to malloc the output string */
    if (!fail) out = (char*) malloc(len);
    
    /* If that fails, we fail. */
    if (!out) fail = 1;

    /* Handle failure. */
    if (fail) 
    {
        for (i = 0; i < numentries; i++) 
            if (entries[i]) 
                free(entries[i]);
        
        free(entries);
        return 0;
    }

    /* Compose the output array. */
    *out = '[';
    ptr = out + 1;
    *ptr = 0;
    for (i = 0; i < numentries; i++) 
    {
        strcpy(ptr, entries[i]);
        ptr += strlen(entries[i]);
        if (i != numentries - 1) 
        {
            *ptr++ = ',';
            if (fmt)*ptr++ = ' ';
            *ptr = 0;
        }
        free(entries[i]);
    }
    free(entries);
    *ptr++ = ']';
    *ptr++ = 0;
    return out;
}

/* Render an object to text. */
static char *print_object (cJSON *item, int depth, int fmt) 
{
    char **entries = 0, **names = 0;
    char *out = 0, *ptr, *ret, *str;
    int len = 7, i = 0, j;
    cJSON *child = item->getChild (item);
    int numentries = 0, fail = 0;
    
    /* Count the number of entries. */
    // while (child) numentries++, child = child->next;
    numentries = child->getSize ();
    
    /* Explicitly handle empty object case */
    if (!numentries) 
    {
        out = (char*) malloc(fmt ? depth + 4 : 3);
        if (!out) return 0;
        ptr = out;
        *ptr++ = '{';
        if (fmt) {
            *ptr++ = '\n';
            for (i = 0; i < depth - 1; i++) *ptr++ = '\t';
        }
        *ptr++ = '}';
        *ptr++ = 0;
        return out;
    }
    /* Allocate space for the names and the objects */
    entries = (char**) malloc(numentries * sizeof (char*));
    if (!entries) return 0;
    names = (char**) malloc(numentries * sizeof (char*));
    if (!names) 
    {
        free(entries);
        return 0;
    }
    memset(entries, 0, sizeof (char*)*numentries);
    memset(names, 0, sizeof (char*)*numentries);

    /* Collect all the results into our arrays: */
    child = item->getChild(item);
    depth++;
    if (fmt) len += depth;
    while (child) 
    {
        names[i] = str = print_string_ptr (child->getString(child));
        entries[i++] = ret = print_value (child, depth, fmt);
        if (str && ret) len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0);
        else fail = 1;
        child = child->getNext(child);
    }

    /* Try to allocate the output string */
    if (!fail) out = (char*) malloc(len);
    if (!out) fail = 1;

    /* Handle failure */
    if (fail) 
    {
        for (i = 0; i < numentries; i++) 
        {
            if (names[i]) free(names[i]);
            if (entries[i]) free(entries[i]);
        }
        free(names);
        free(entries);
        return 0;
    }

    /* Compose the output: */
    *out = '{';
    ptr = out + 1;
    if (fmt)*ptr++ = '\n';
    *ptr = 0;
    for (i = 0; i < numentries; i++) 
    {
        if (fmt) for (j = 0; j < depth; j++) *ptr++ = '\t';
        strcpy(ptr, names[i]);
        ptr += strlen(names[i]);
        *ptr++ = ':';
        if (fmt) *ptr++ = '\t';
        strcpy(ptr, entries[i]);
        ptr += strlen(entries[i]);
        if (i != numentries - 1) *ptr++ = ',';
        if (fmt) *ptr++ = '\n';
        *ptr = 0;
        free(names[i]);
        free(entries[i]);
    }

    free(names);
    free(entries);
    if (fmt) for (i = 0; i < depth - 1; i++) *ptr++ = '\t';
    *ptr++ = '}';
    *ptr++ = 0;
    return out;
}

// =============================================================================

/* Render a cJSON item/entity/structure to text. */
char *cJSON_Print (cJSON *item)
{ return print_value (item, 0, 1); }

char *cJSON_PrintUnformatted (cJSON *item)	
{ return print_value (item, 0, 0);}
