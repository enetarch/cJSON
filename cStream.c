/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "cStream.h"

#define cSTREAM_blocksize 65536 // 64k
#define null 0
#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

typedef struct Private Private;
typedef struct Block Block;

struct Block
{
    Block * next;
    int nPos;
    char chunk [cSTREAM_blocksize];
    
};

struct Private
{
    int length;
    int position;
    
    Block *first;
};

cStream * Stream ()
{
    Private * private = (private*) malloc(sizeof (Private));
    if (private) memset (private, 0, sizeof (Private));
    
    // =========================================================================
    // Constructor

    private->first = newBlock();
    
    Block * newBlock ()
    {
        Block *b = malloc(sizeof (Block));
        if (b) memset (b, 0, sizeof (Block));
        return (b);
    }
    
    // =========================================================================
    // Private Functions 
    
    // =========================================================================
    // Public Functions 
    
    void destroy (cStream * this)
    {
        if (!this) return;
        private = this->private;
        
        Block bn, b = private->first;
        
        while (!b)
        {
            bn = b->next;
            free (b->chuck);
            free (b);
            b = bn;
        }
        
        free (private);
        free (this);
    }
    
    int length (cStream * this)
    {
        if (!this) return;
        private = this->private;
        return (private->length);
    }

    char get (cStream * this)
    {
        if (!this) return;
        private = this->private;
        
        if (private->position >= private->length)
            return ("");
        
        int nPos = private->position;

        Block *b = private->first;
        
        while ((nPos >= cSTREAM_blocksize)  && b)
        {
            nPos =- cSTREAM_blocksize;
            b = b->next;
        }
        
        if (!b)
            return ("");
        
        private->position++;
        
        return (b->chunk [nPos]);        
    }

    int put (cStream * this, char value)
    {
        if (!this) return;
        private = this->private;

        Block *b = private->first;
        Block *bprev = null;
        
        while ((nPos >= cSTREAM_blocksize)  && b)
        {
            nPos =- cSTREAM_blocksize;
            bprev = b;
            b = b->next;
        }
        
        if (!b)
        {
            bprev->next = newBlock();
            b = bprev->next;
        }
        
        b->chunk [nPos] = value;
        private->length++;
        
        return (1);        
    }
    
    char * read (cStream * this, int nLength)
    {
        if (!this) return;
        private = this->private;
        
        if (private->position >= private->length)
            return ("");
        
        int nPos = private->position;

        Block *b = private->first;
        
        while ((nPos >= cSTREAM_blocksize)  && b)
        {
            nPos =- cSTREAM_blocksize;
            b = b->next;
        }
        
        if (!b)
            return ("");
        
        // allocate the memory needed for the complete string
        // read the blocks in sequence
        
        int nLen = 0;
        int nTotal = 0;
        
        char * rtn = malloc (nLength+1);
        memset (rtn, nLength+1, 0);
        
        while ((nLength > 0) && (b != null))
        {
            nLen = min (nLength, cSTREAM_blocksize - nPos);
            memcpy (rtn[nTotal], b->chunk[nPos], nLen);
            nTotal =+ nLen;
            nLength =- nLen;
            private->position =+ nLen;
            nPos = 0;
            b = b->next;
        }
        
        return (rtn);
    }

    int write (cStream * this, char * value, int nLength)
    {
        if (!this) return;
        private = this->private;
        
        Block *b = private->first;
        Block *bprev = null;
        
        while ((nPos >= cSTREAM_blocksize)  && b)
        {
            nPos =- cSTREAM_blocksize;
            bprev = b;
            b = b->next;
        }
        
        // allocate the chunk needed to write the string
        // write the blocks in sequence

        int nLen = 0;
        int nTotal = 0;

        while ((nLength > 0) && (b != null))
        {
            nLen = min (nLength, cSTREAM_blocksize - nPos);
            memcpy (b->chunk[nPos], value[nTotal], nLen);
            nTotal =+ nLen;
            nLength =- nLen;
            private->length =+ nLen;
            nPos = 0;
            b->next = newBlock();
            b = b->next;
        }
        
        return (nTotal);
    }
    
    void clear_blocks (cStream * this)
    {
        if (!this) return;
        private = this->private;

        Block *b = private->first;
        while (b)
        {
            memset (b, cSTREAM_blocksize, 0);
            b = b->next;
        }
    }
    
    void reset (cStream * this)
    {
        if (!this) return;
        private = this->private;
        
        private->length = 0;
        private->position = 0;
        clear_blocks (this);
    }

    void resetRead (cStream * this)
    {
        if (!this) return;
        private = this->private;
        
        private->position = 0;
        
    }

    void resetWrite (cStream * this)
    {
        if (!this) return;
        private = this->private;
        
        private->length = 0;
        clear_blocks (this);
    }
    
    // =========================================================================
    /* Internal constructor. */

    cStream * this = malloc (sizeof (cStream));
    
    this->private = private;
    
    this->destroy = destroy;
    
    this->get = get;
    this->put = put;
    this->read = read;
    this->write = write;
    
    this->length = length;
    
    this->reset = reset;
    this->resetRead = resetRead;
    this->resetWrite = resetWrite;
    
    return (this);
    
}