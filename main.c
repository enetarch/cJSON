/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: mfuhrman
 *
 * Created on September 24, 2016, 3:12 PM
 */

#include <stdlib.h>
#include <stdio.h>

#include "cJSON.h"
#include "functions.h"


/*
 * 
 */
int main(int argc, char** argv) 
{
    const char * str1 = "hello world";
    printf ("str1 = %s\r\n" , str1);
    const char * str2 = strDupe (str1);
    printf ("str2 = %s\r\n" , str2);

    cJSON * json = JSON(null);
    cJSON * json2 = JSON(null);
    cJSON json3;
    
    printf (" json = %x\r\n", json);
    printf ("json2 = %x\r\n", json2);

    printf (" json->addNumber = %x\r\n", json->addNumber);
    printf ("json2->addNumber = %x\r\n", json2->addNumber);

    
    printf ("enetering ... json->getSize (json)\r\n");
    int n = json->getSize (json);
    printf ("json->getSize (json) = ");
    printf ("%d\r\n" , n);
    
    printf ("enetering ... json->getName (json)\r\n");
    const char * name = json->getName (json);
    printf ("json->getName (json) = ");
    printf ("%s\r\n" , name);
    
    printf ("enetering ... json->getName (json, [Root])\r\n");
    const char * name2 = json->setName (json, "Root");
    printf ("json->getName (json, [Root]) = ");
    printf ("%s\r\n" , name2);
    
    printf ("enetering ... json->getName (json)\r\n");
    const char * name3 = json->getName (json);
    printf ("json->getName (json) = ");
    printf ("%s\r\n" , name3);
    
    printf ("enetering ... json->getType (json)\r\n");
    int type = json->getType (json);
    printf ("json->getType (json) = ");
    printf ("%d\r\n" , type);
    
    printf (" json->addNumber = %x\r\n", json->addNumber);

    printf ("compare .. json->addNumber == json2->addNumber = %s\r\n", 
            (json->addNumber == json2->addNumber)?"t":"f");
    
    printf ("enetering ... json->addNumber (json, [number], 123)\r\n");
    cJSON * num = json->addNumber (json, "number", 123);
    
    printf ("enetering ... json->getSize (json)\r\n");
    int n2 = json->getSize (json);
    printf ("json->getSize (json) = ");
    printf ("%d\r\n" , n2);

    printf ("enetering ... json->addString (json, [string], [abc123])\r\n");
    cJSON * str = json->addString (json, "string", "abc123");

    printf ("enetering ... json->addObject (json, [object])\r\n");
    printf ("json2->addNumber = %x\r\n", *json2->addNumber);
    cJSON * obj = json->addObject (json, "object");
    printf ("enetering ... json->getType (json)\r\n");
    int type2 = obj->getType (obj);
    printf ("json->getType (json) = ");
    printf ("%d\r\n" , type2);

    printf ("enetering ... json->addArray (json, [array])\r\n");
    cJSON * ary = json->addArray (json, "array");
    printf ("enetering ... json->getType (json) = ");
    int type3 = ary->getType (ary);
    printf ("json->getType (json) = ");
    printf ("%d\r\n" , type3);

    printf ("enetering ... json->addNull (json, [null])\r\n");
    json->addNull (json, "null" );

    printf ("enetering ... json->addTrue (json, [true])\r\n");
    json->addTrue (json, "true" );

    printf ("enetering ... json->addFalse (json, [false])\r\n");
    json->addFalse (json, "false" );

    printf ("enetering ... json->addBool (json, [booltrue])\r\n");
    json->addBool (json, "booltrue", true);

    printf ("enetering ... json->addBool (json, [boolfalse])\r\n");
    json->addBool (json, "boolfalse", false);
    
    printf ("enetering ... json->getType (json)\r\n");
    int n3 = json->getSize (json);
    printf ("json->getType (json) = ");
    printf ("%d\r\n" , type2);
    
    printf ("enetering ... json->getByPos (json)\r\n");
    cJSON * obj2 = json->getByPos (json, 2);
    printf ("json->getByPos (json) = ");
    printf ("%d", (obj2)?0:1);

    printf ("enetering ... json->getByName (json)\r\n");
    cJSON * obj3 = json->getByName (json, "object");
    printf ("json->getByName (json) = ");
    printf ("%d", (obj3)?0:1);
    
    printf ("enetering ... json->getChild (json)\r\n");
    cJSON * obj4 = obj3->getChild (obj3);
    printf ("json->getChild (obj3) = ");
    printf ("%d", (obj4)?0:1);
    return 0;
}

