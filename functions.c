
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include <uuid/uuid.h>

const char* strDupe (const char* str)
{
      int len;
      char* copy;

      len = strlen (str) + 1;
      copy = (char*) malloc (len);
      
      if (!copy) 
      {
          printf ("error allocating space for duplicate string\r\n");
          return 0;
      }
      
      memset (copy, 0, len);
      memcpy (copy, str, len);
      
      return (copy);
}


char * strLeft (char *szStr, int nLen)
{  
    char *rtn = malloc (nLen+1);
    strncpy (rtn, szStr, nLen);
    return (rtn); 
}

char * strTrim (char *szStr)
{  
    int i;
    int begin = 0;
    int end = strlen(szStr) - 1;

    while (isspace(szStr[begin]))
        begin++;

    while ((end >= begin) && isspace(szStr[end]))
        end--;

    // Shift all characters back to the start of the string array.
    for (i = begin; i <= end; i++)
        szStr[i - begin] = szStr[i];

    szStr[i - begin] = '\0'; // Null terminate string.
    
    return (szStr);
}

/*
char * date_now ()
{
    time_t curtime = time (NULL);

    struct tm *loctime = localtime (&curtime);
    
    char *template = "2015-01-01 12:45:78";
    char *timestamp; 
    
    timestamp = malloc (sizeof (template));
    
    sprintf (timestamp, "%i-%i-%i %i:%i:%i", 
            loctime->tm_year+1900,
            loctime->tm_mon+1,
            loctime->tm_mday,
            
            loctime->tm_hour,
            loctime->tm_min,
            loctime->tm_sec
            );

    return (timestamp);
}
*/
/*

char * getGUID ()
{
    uuid_t out;
    char *guid = malloc (40);
    uuid_generate_random (out);
    uuid_unparse (out, guid);
    return (guid);
}

*/
