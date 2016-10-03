/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   stubs.h
 * Author: mfuhrman
 *
 * Created on September 25, 2016, 10:40 PM
 */

#ifndef STUBS_H
#define STUBS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "cJSON.h"

extern cJSON * parse (const char * buffer);

/* Render a private item/entity/structure to text. */
extern const char * printUnformatted (cJSON * this);
extern const char * print (cJSON * this);



#ifdef __cplusplus
}
#endif

#endif /* STUBS_H */

