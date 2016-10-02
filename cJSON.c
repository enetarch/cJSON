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

/* cJSON */
/* JSON parser in C. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "cJSON.h"

#define true 1
#define false 0

/* The cJSON structure: */
typedef struct Private Private;
struct Private 
{
	Private *next,*prev;	/* link list of children in current level. */
	cJSON * child;		/* child pointer to next level, if type == [ Object, Array ] */

        cJSON * public;         /* pointer to public functions */
        
	int type;		/* The type of the item == [ TRUE, FALSE, NULL, NUMBER, STRING, ARRAY, OBJECT ] */

	char *name;		/* The item's name string, if Parent's type OR this type == cJSON_Object */

	const char *valuestring;	/* The item's string, if type == String */
	int valueint;		/* The item's number, if type == Number */
	double valuedouble;	/* The item's number, if type == Number */
        
        int errorno;            /* the error number of the last command */
};

cJSON * JSON (const char *szJSON)
{
    // =========================================================================
    // Private Functions 
    
    // int getType (cJSON * this);
    /* return the type of object that this is. */
    int getType (cJSON * this)
    {
        printf ("get type\r\n");
        
        if (!this) return (null);
        
        Private * private = this->private;
        
        return (private->type);
    }

    /* Utility for array list handling. */
    void insert_object (Private *prev, Private *item) 
    {
        printf ("insert object into list\r\n");

        prev->next = item;
        item->prev = prev;
    }
    
    void addItemToArray (cJSON *this, cJSON *item) 
    {
        printf ("add Item To Array\r\n");

        if (!this) return;
        if (!item) return;

        Private * private = this->private;
        
        cJSON *child = private->child;
        
        if (!child) 
        {
            private->child = item;
        } else 
        {
            Private *c = child->private;
            
            while (c && c->next) c = c->next;
            insert_object (c, item->private);
        }
    }

    void addItemToObject (cJSON *this, const char *name, cJSON *item) 
    {
        printf ("add Item To Object [ %s ]\r\n", name);

        if (!this) return;
        if (!item) return;

        Private * private = item->private;
        
        if (getType (this) == cJSON_Object)
        {
            if (private->name) 
                free (private->name);

            private->name = strDupe (name);
        }
        else
        { private->name = null; }

        addItemToArray (this, item);
    }

    // =========================================================================
    // Public Functions 
    
    int getError (cJSON * this) 
    { 
        printf ("get error \r\n");

        if (!this) return (0);
        
        Private * private = this->private;
        
        return (private->errorno); 
    }

    /* Delete a private structure. */
    bool destruct (cJSON * this) 
    {
        printf ("object destruct \r\n");

        if (!this) return (false);
        
        Private * private = this->private;
        
        Private * next;
        
        while (private) 
        {
            next = private->next;
            if (!(private->type) && private->child) 
                destruct (private->child);
            
            if (!(private->type) && private->valuestring) 
                free (private->valuestring);
            
            if (private->name) 
                free (private->name);
            
            free (private);
            private = next;
        }
        
        free (this);
        
        return (true);
    }

    /* Render a private item/entity/structure to text. */
    char *print (cJSON *this) ;
//    { return print_value (item, 0, 1); }

    char *printUnformatted (cJSON *this) ;
//    { return print_value (item, 0, 0); }

    // =========================================================================

    /* return the number of items in this object / array. */
    int getSize (cJSON *this) 
    {
        printf ("get size \r\n");

        if (!this) return (null);
        
        Private * private = this->private;
        
        int i = 0;
        cJSON *child = private->child;
        if (!child)
            return (0);
        
        Private *c = child->private; 
        while (c)
        {
            i++;
            c = c->next;
        }
        
        return (i);
    }
    
    /* return the item at position N. */
    cJSON *getByPos (cJSON * this, int nPos) 
    {
        printf ("get by pos = [ %d ] \r\n", nPos);

        if (!this) return (null);
        
        Private * private = this->private;
        
        cJSON *child = private->child;
        Private *c = child->private; 
        while (c && nPos > 0) 
        {
            nPos--; 
            c = c->next;
        }
        
        return (c->public);
    }

    /* return the item at matching the name. */
    cJSON *getByName (cJSON *this, const char *name) 
    {
        printf ("get by name = [ %s ] \r\n", name);

        if (!this) return (null);
        if (getType (this) != cJSON_Object) return (null);
        
        Private * private = this->private;
        
        cJSON *child = private->child;
        Private *c = child->private; 
        bool bFound = false;
        
        if (c->name)
            bFound = strcasecmp (c->name, name);

        while ((c != null) && (bFound != 0)) 
        {
            c = c->next;
            if (c != null)
            if (c->name)
                bFound = strcasecmp (c->name, name);
        }
        
        if (bFound == 0)
        {
            if (c != null)
            { return (c->public); }
            else
            { return (null); }
        }
        else
        { return (null); }
    }

    /* return the child of this item, if there is one. */
    cJSON *getChild (cJSON *this) 
    {
        printf ("get children \r\n");
        
        if (!this) return (null);
        if 
        (
           (getType (this) != cJSON_Array) ||
           (getType (this) != cJSON_Object) 
        )
        return (null);
        
        Private * private = this->private;
        
        return (private->child);
    }

    // =========================================================================

    cJSON * getNext (cJSON * this)
    {
        printf ("get next \r\n");

        if (!this) return (null);
        
        Private * private = this->private;
        
        return (private->next->public);
    }
    
    double getNumber (cJSON * this)
    {
        printf ("get number \r\n");

        if (!this) return (null);
        if (getType (this) != cJSON_Number) return (null);
        
        Private * private = this->private;
        
        return (private->valuedouble);
    }
    
    const char * getString (cJSON * this)
    {
        printf ("get string \r\n");

        if (!this) return (null);
        if (getType (this) != cJSON_String) return (null);
        
        Private * private = this->private;
        
        return (private->valuestring);
    }

    const char * getName (cJSON * this)
    {
        printf ("get name \r\n");

        if (!this) return (null);
        
        Private * private = this->private;
        
        return (private->name);
    }

    const char * setName (cJSON * this, const char * name)
    {
        printf ("set name = [ %s ] \r\n", name);
        
        if (!this) return (null);
        if (getType (this) != cJSON_Object)
            return (null);
        
        Private * private = this->private;
        
        if (private->name) 
            free (private->name);

        private->name = strDupe (name);

        return (private->name);
    }

    // =========================================================================

    double setNumber (cJSON * this, double value)
    {
        printf ("set number value = [ %d ] \r\n", value);
        
        if (!this) return (null);
        if (getType (this) != cJSON_Number) return (null);
        
        Private * private = this->private;
        
        return (private->valuedouble = value);
    }
    
    const char * setString (cJSON * this, const char * value)
    {
        printf ("set string value = [ %s ] \r\n", value);
        
        if (!this) return (null);
        if (getType (this) != cJSON_String) return (null);
        
        Private * private = this->private;
        
        return (private->valuestring = value);
    }

    // =========================================================================

    cJSON * addNull (cJSON * this, const char *name)
    {
        printf ("add null [ %s ] \r\n", name);
        
        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_NULL;
        
        addItemToObject (this, name, item);
        return (item);
    }
    
    cJSON * addTrue (cJSON *this, const char *name)		
    {
        printf ("add true [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_True;
        
        addItemToObject (this, name, item);
        return (item);
    }
    
    cJSON * addFalse (cJSON *this, const char *name)		
    {
        printf ("add false [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_False;
        
        addItemToObject (this, name, item);
        return (item);
    }
    
    cJSON * addBool (cJSON *this, const char *name, bool b)	
    {
        printf ("add bool [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = b ? cJSON_True : cJSON_False;
        
        addItemToObject (this, name, item);
        return (item);
    }
    
    cJSON * addNumber (cJSON *this, const char *name, double number)	
    {
        printf ("add number [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_Number;
        private->valuedouble = number;
        private->valueint = (int) number;
        
        addItemToObject (this, name, item);
        return (item);
    }
    
    cJSON * addString (cJSON *this, const char *name, const char *string)	
    {
        printf ("add string [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_String;
        private->valuestring = strdup(string);
        
        addItemToObject (this, name, item);
        return (item);
    }

    cJSON *addArray (cJSON *this, const char * name) 
    {
        printf ("add array [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_Array;

        addItemToObject (this, name, item);
        
        return (item);
    }

    cJSON *addObject (cJSON *this, const char * name) 
    {
        printf ("add object [ %s ] \r\n", name);

        if (!this) return (null);
        int nType = getType (this);
        
        if ( (nType != cJSON_Array) &&
         (nType != cJSON_Object) )
            return (null);
            
        if ((nType == cJSON_Object) && (!name))
            return (null);
        
        cJSON *item = JSON("");
        if (!item)
            return (null);
        
        Private * private = item->private;
        private->type = cJSON_Object;
        
        addItemToObject (this, name, item);
        
        return (item);
    }

    /* Create Arrays: */
    cJSON * addNumbers (cJSON * this, int count, double *numbers)
    {
        printf ("add numbers [ %d ] \r\n", count);
        
        if (!this) return (false);
        if (getType (this) != cJSON_Array) 
            return (false);
        
        int i;
        for (i = 0; i < count; i++) 
            addNumber (this, "", numbers[i]);

        return (this);
    }

    cJSON * addStrings (cJSON * this, int count, const char **strings)
    {
        printf ("add strings [ %d ] \r\n", count);
        
        if (!this) return (false);
        if (getType (this) != cJSON_Array) 
            return (false);
        
        int i;
        for (i = 0; i < count; i++) 
            addString (this, "", strings[i]);

        return (this);
    }

    // =========================================================================

    /* Replace array/object items with new ones. */
    bool setArray (cJSON *this, int nPos, cJSON *newitem) 
    {
        printf ("set object by position [ %d ] \r\n", nPos);
        
        if (!this) return (false);
        if (getType (this) != cJSON_Array) 
            return (false);
        
        if (!newitem) return (false);
        
        Private * private = this->private;
        
        cJSON *child = private->child;
        Private *c = child->private;
        while (c && nPos > 0) 
        {
            c = c->next;
            nPos--;
        }
        
        if (!c) return (false);
        
        Private * newprivate = newitem->private;
        newprivate->next = c->next;
        newprivate->prev = c->prev;
        
        if (newprivate->next) newprivate->next->prev = newprivate;
        if (private->child == c->public) 
        { private->child = newitem; }
        else 
        { newprivate->prev->next = newprivate; }
        
        c->next = c->prev = 0;
        destruct (c->public);
    }

    bool setObject (cJSON *this, const char *name, cJSON *newitem) 
    {
        printf ("set object by name [ %s ] \r\n", name);
        
        if (!this) return (false);
        if (getType (this) != cJSON_Object) 
            return (null);
        
        if (!newitem) return (false);
        
        Private * private = this->private;
        
        cJSON *child = private->child;
        Private *c = child->private;
        while (c && strcasecmp(c->name, name))
        { c = c->next; }
        
        if (!c) return (false);
        
        Private * newprivate = newitem->private;
        newprivate->name = strdup(name);
        newprivate->next = c->next;
        newprivate->prev = c->prev;
        
        if (newprivate->next) newprivate->next->prev = newprivate;
        if (private->child == c->public) 
        { private->child = newitem; }
        else 
        { newprivate->prev->next = newprivate; }

        c->next = c->prev = null;
        destruct (c->public);

        return (true);
    }

    // =========================================================================

    bool unsetItem (cJSON *this, int nPos) 
    {
        printf ("unset object by position [ %d ] \r\n", nPos);
        
        if (!this) return (false);
        if (getType (this) != cJSON_Array) 
            return (false);
        
        Private * private = this->private;
        cJSON *child = private->child;
        Private *c = child->private;
        
        while (c && nPos > 0) 
        {
            c = c->next;
            nPos--;
        }
        
        if (!c) return (false);
        
        if (c->prev) c->prev->next = c->next;
        if (c->next) c->next->prev = c->prev;
        
        if (private->child == c->public) 
            private->child = c->next->public;
        
        c->prev = c->next = 0;
        
        return (destruct (c->public));
    }

    bool unsetObject (cJSON *this, const char *name) 
    {
        printf ("unset object by name [ %s ] \r\n", name);
        
        if (!this) return (false);
        if (getType (this) != cJSON_Object) 
            return (null);
        
        Private * private = this->private;
        cJSON *child = private->child;
        Private *c = child->private;
        
        while (c && strcasecmp (c->name, name))
            c = c->next;
        
        if (!c) return (false);
        
        if (c->prev) c->prev->next = c->next;
        if (c->next) c->next->prev = c->prev;
        
        if (private->child == c->public) 
            private->child = c->next->public;
        
        c->prev = c->next = null;

        return (destruct (c->public));
    }

    // =========================================================================
    // =========================================================================
    /* Duplicate will create a new, identical cJSON item to the one you pass, in 
     * new memory that will need to be released. With recurse!=0, it will duplicate 
     * any children connected to the item. The item->next and ->prev pointers are 
     * always zero on return from Duplicate. */
    
    /* Duplication */
    cJSON *duplicate(cJSON *this, int recurse) 
    {
        printf ("duplicate object\r\n");
        
        cJSON *dupethis, *dupechild, *child;
        Private *dupeprivate, *childprivate, *next = 0;
        
        /* Bail on bad ptr */
        if (!this) 
            return (null);
        
        Private * private = this->private;
        
        /* Create new item */
        dupethis = JSON("");
        if (!dupethis) return 0;
        
        dupeprivate = dupethis->private;
        
        /* Copy over all vars */
        dupeprivate->type = private->type;
        dupeprivate->valueint = private->valueint;
        dupeprivate->valuedouble = private->valuedouble;
        
        if (private->valuestring) 
        {
            dupeprivate->valuestring = strdup (private->valuestring);
            if (!dupeprivate->valuestring) 
            {
                destruct(dupethis);
                return 0;
            }
        }
        
        if (private->name) 
        {
            dupeprivate->name = strdup (private->name);
            if (!dupeprivate->name) 
            {
                destruct (dupethis);
                return 0;
            }
        }
        
        /* If non-recursive, then we're done! */
        if (!recurse) return dupethis;
        
        /* Walk the ->next chain for the child. */
        child = private->child;
        while (child) 
        {
            /* Duplicate (with recurse) each item in the ->next chain */
            dupechild = duplicate (child, 1); 
            if (!dupechild) 
            {
                destruct(dupechild);
                return 0;
            }
            childprivate = dupechild->private;
            
            if (next)
            {
                next->next = childprivate, childprivate->prev = next;
                next = childprivate;
            }/* If newitem->child already set, then crosswire ->prev and ->next 
              * and move on */
            else 
            {
                dupeprivate->child = childprivate->public;
                next = childprivate;
            } /* Set newitem->child and move to it */
            
            child = childprivate->next->public;
        }
        return dupethis;
    }

    // =========================================================================
    /* Internal constructor. */

    printf ("construct object\r\n");
    printf ("sizeof (Private) = %d \r\n", sizeof (Private));
    printf ("sizeof (cJSON) = %d \r\n", sizeof (cJSON));
    
    Private * private = (Private *) malloc(sizeof (Private));
    if (!private)
        printf ("error allocating space for private\r\n");
    
    if (private) memset (private, 0, sizeof (Private));
    
    private->type = cJSON_Object;
    
    // =========================================================================
    // Constructor

    cJSON * this = (cJSON *) malloc (sizeof (cJSON));
    if (!this)
        printf ("error allocating space for this\r\n");
    
    if (this) memset (this, 0, sizeof (cJSON));
    
    this->private = private;
    private->public = this;
    
    this->addNull = addNull;
    this->addTrue = addTrue;
    this->addFalse = addFalse;
    this->addBool = addBool;
    this->addNumber = addNumber;
    this->addString = addString;
    this->addNumbers = addNumbers;
    this->addStrings = addStrings;
    this->addArray = addArray;
    this->addObject = addObject;
    
    this->getNext = getNext;
    this->setArray = setArray;
    this->setObject = setObject;
    this->getError = getError;

    this->printUnformatted = printUnformatted;
    this->print = print;
    this->duplicate = duplicate;

    this->getByPos = getByPos;
    this->getByName = getByName;
    this->getChild = getChild;

    this->getSize = getSize;
    this->getType = getType;
    this->getName = getName;
    this->setName = setName;

    this->getNumber = getNumber;
    this->getString = getString;
    this->setNumber = setNumber;
    this->setString = setString;
    
    this->destruct = destruct;
    
    // =========================================================================
    // Constructor

    if (szJSON)
        parse (szJSON);
    
    // =========================================================================

    return (this);
};



