//___________________________________________________________________________________________________________nFO
// "endianfix.cpp" - required for big endian support.
//
// Written by:	Axel 'awe' Wefers		[mailto:awe@fruitz-of-dojo.de].
//		©2002 Fruitz Of Dojo 		[http://www.fruitz-of-dojo.de].
//
// Version History:
// v1.0.0: Initial release.
//______________________________________________________________________________________________________iNCLUDES

#pragma mark =Includes=

#include <cstdio>

#include "swchpal.h"
#include "endianfix.h"

#pragma mark -

//___________________________________________________________________________________________fUNCTION_pROTOTYPES

#pragma mark =Function Prototypes=

void	Endian_Swap (pakentry_t *theEntry, size_t theCount);
void	Endian_Swap (pakheader_t *theHeader, size_t theCount);
void	Endian_Swap (dentry_t *theEntry);
void	Endian_Swap (dentry_t *theEntry, size_t theCount);

#pragma mark -

//_____________________________________________________________________________________Endian_Swap(pakentry_t *)

void	Endian_Swap (pakentry_t *theEntry, size_t theCount)
{
    for (size_t i = 0; i < theCount; i++)
    {
        Endian32_Swap (theEntry[i].offset);
        Endian32_Swap (theEntry[i].size);
    }
}

//____________________________________________________________________________________Endian_Swap(pakheader_t *)

void	Endian_Swap (pakheader_t *theHeader, size_t theCount)
{
    for (size_t i = 0; i < theCount; i++)
    {
        Endian32_Swap (theHeader[i].diroffset);
        Endian32_Swap (theHeader[i].dirsize);
    }
}

//_______________________________________________________________________________________Endian_Swap(dentry_t *)

void	Endian_Swap (dentry_t *theEntry)
{
    Endian32_Swap (theEntry->offset);
    Endian32_Swap (theEntry->size);
}

//_______________________________________________________________________________Endian_Swap(dentry_t *, size_t)

void	Endian_Swap (dentry_t *theEntry, size_t theCount)
{
    for (size_t i = 0; i < theCount; i++)
    {
        Endian_Swap (&theEntry[i]);
    }
}

//______________________________________________________________________________________Endian_Swap(dheader_t *)

void	Endian_Swap (dheader_t *theHeader, size_t theCount)
{
    for (size_t i = 0; i < theCount; i++)
    {
        Endian32_Swap (theHeader[i].version);
    
        Endian_Swap (&theHeader[i].entities);
        Endian_Swap (&theHeader[i].planes);
        Endian_Swap (&theHeader[i].miptex);
        Endian_Swap (&theHeader[i].vertices);
        Endian_Swap (&theHeader[i].visilist);
        Endian_Swap (&theHeader[i].nodes);
        Endian_Swap (&theHeader[i].texinfo);
        Endian_Swap (&theHeader[i].faces);
        Endian_Swap (&theHeader[i].lightmaps);
        Endian_Swap (&theHeader[i].clipnodes);
        Endian_Swap (&theHeader[i].leaves);
        Endian_Swap (&theHeader[i].lface);
        Endian_Swap (&theHeader[i].edges);
        Endian_Swap (&theHeader[i].ledges);
        Endian_Swap (&theHeader[i].models);
    }
}

//__________________________________________________________________________________________fread(pakheader_t *)

size_t	 fread (pakheader_t *theHeader, size_t theSize, size_t theCount, FILE *theFile)
{
    size_t	myBytes;
    
    myBytes = fread ((void*) theHeader, theSize, theCount, theFile);

    Endian_Swap (theHeader, theCount);
    return (myBytes);
}

//_________________________________________________________________________________________fwrite(pakheader_t *)

size_t	 fwrite (pakheader_t *theHeader, size_t theSize, size_t theCount, FILE *theFile)
{
    size_t	myBytes;

    Endian_Swap (theHeader, theCount);    
    myBytes = fwrite ((void*) theHeader, theSize, theCount, theFile);
    Endian_Swap (theHeader, theCount);
    return (myBytes);
}

//___________________________________________________________________________________________fread(pakentry_t *)

size_t	 fread (pakentry_t *theEntry, size_t theSize, size_t theCount, FILE *theFile)
{
    size_t	myBytes;
    
    myBytes = fread ((void*) theEntry, theSize, theCount, theFile);
    Endian_Swap (theEntry, theCount);
    return (myBytes);
}

//__________________________________________________________________________________________fwrite(pakentry_t *)

size_t	 fwrite (pakentry_t *theEntry, size_t theSize, size_t theCount, FILE *theFile)
{
    size_t	myBytes;
    
    Endian_Swap (theEntry, theCount);
    myBytes = fwrite ((void*) theEntry, theSize, theCount, theFile);
    Endian_Swap (theEntry, theCount);
    return (myBytes);
}

//____________________________________________________________________________________________fread(dheader_t *)

size_t	 fread (dheader_t *theHeader, size_t theSize, size_t theCount, FILE *theFile)
{
    size_t	myBytes;
    
    myBytes = fread ((void*) theHeader, theSize, theCount, theFile);
    Endian_Swap (theHeader, theCount);
    return (myBytes);
}

//___________________________________________________________________________________________fwrite(dheader_t *)

size_t	 fwrite (dheader_t *theHeader, size_t theSize, size_t theCount, FILE *theFile)
{
    size_t	myBytes;
    
    Endian_Swap (theHeader, theCount);
    myBytes = fwrite ((void*) theHeader, theSize, theCount, theFile);
    Endian_Swap (theHeader, theCount);

    return (myBytes);
}

//___________________________________________________________________________________________________________eOF
