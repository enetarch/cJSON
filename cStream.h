/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cStream.h
 * Author: mfuhrman
 *
 * Created on September 20, 2016, 4:35 PM
 */

#ifndef CSTREAM_H
#define CSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct cStream cStream;
struct cStream
{
    void * private;
    
    void (* destroy) (cStream * this);
    
    char (* get) (cStream * this);
    int (* put) (cStream * this, char value);
    
    int (* length) (cStream * this);

    char * (* read) (cStream * this, int nLength);
    int (* write) (cStream * this, char * value, int nLength);
    
    void (* reset) (cStream * this);
    void (* resetRead) (cStream * this);
    void (* resetWrite) (cStream * this);
};

extern void Stream (const char * value);


#ifdef __cplusplus
}
#endif

#endif /* CSTREAM_H */

