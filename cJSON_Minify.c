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

void JSON_Minify(char *json) 
{
    char *into = json;
    while (*json) 
    {
        if (*json == ' ') json++;
        else if (*json == '\t') json++; // Whitespace characters.
        else if (*json == '\r') json++;
        else if (*json == '\n') json++;
        else if (*json == '/' && json[1] == '/') while (*json && *json != '\n') json++; // double-slash comments, to end of line.
        else if (*json == '/' && json[1] == '*') {
            while (*json && !(*json == '*' && json[1] == '/')) json++;
            json += 2;
        }// multiline comments.
        else if (*json == '\"') 
        {
            *into++ = *json++;
            while (*json && *json != '\"') 
            {
                if (*json == '\\') *into++ = *json++;
                *into++ = *json++;
            }
            *into++ = *json++;
        }// string literals, which are \" sensitive.
        else *into++ = *json++; // All other characters.
    }
    *into = 0; // and null-terminate.
}
