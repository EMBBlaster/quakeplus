//___________________________________________________________________________________________________________nFO
// "endianfix.h" - required for big endian support.
//
// Written by:	Axel 'awe' Wefers		[mailto:awe@fruitz-of-dojo.de].
//		©2002 Fruitz Of Dojo 		[http://www.fruitz-of-dojo.de].
//
// Version History:
// v1.0.0: Initial release.
//________________________________________________________________________________________________cOMPILER_mAGIC

#ifndef ENDIANFIX_H

#ifndef __cplusplus
#error The header file "endianfix.h" requires C++.
#endif /* __cplusplus__ */

#define ENDIANFIX_H

//________________________________________________________________________________________________________mACROS

#pragma mark =Macros=

#ifdef __BIG_ENDIAN__

#define Endian16_Swap(value)	(value = (((((unsigned short) value) << 8) & 0xFF00)  | \
                                          ((((unsigned short) value) >> 8) & 0x00FF)))

#define Endian32_Swap(value)    (value = (((((unsigned long) value) << 24) & 0xFF000000)  | \
                                          ((((unsigned long) value) <<  8) & 0x00FF0000)  | \
                                          ((((unsigned long) value) >>  8) & 0x0000FF00)  | \
                                          ((((unsigned long) value) >> 24) & 0x000000FF)))

#define Endian64_Swap(value)	(value = (((((unsigned long long) value) << 56) & 0xFF00000000000000ULL)  | \
                                          ((((unsigned long long) value) << 40) & 0x00FF000000000000ULL)  | \
                                          ((((unsigned long long) value) << 24) & 0x0000FF0000000000ULL)  | \
                                          ((((unsigned long long) value) <<  8) & 0x000000FF00000000ULL)  | \
                                          ((((unsigned long long) value) >>  8) & 0x00000000FF000000ULL)  | \
                                          ((((unsigned long long) value) >> 24) & 0x0000000000FF0000ULL)  | \
                                          ((((unsigned long long) value) >> 40) & 0x000000000000FF00ULL)  | \
                                          ((((unsigned long long) value) >> 56) & 0x00000000000000FFULL)))

#else

#define	Endian16_Swap(value)
#define	Endian32_Swap(value)
#define	Endian64_Swap(value)

#endif /* __BIG_ENDIAN__ */

#pragma mark -

//___________________________________________________________________________________________fUNCTION_pROTOTYPES

#pragma mark =Function Prototypes=

#ifdef __BIG_ENDIAN__

size_t	 fread (pakheader_t *theHeader, size_t theSize, size_t theCount, FILE *theFile);
size_t	 fwrite (pakheader_t *theHeader, size_t theSize, size_t theCount, FILE *theFile);

size_t	 fread (pakentry_t *theEntry, size_t theSize, size_t theCount, FILE *theFile);
size_t	 fwrite (pakentry_t *theEntry, size_t theSize, size_t theCount, FILE *theFile);

size_t	 fread (dheader_t *theEntry, size_t theSize, size_t theCount, FILE *theFile);
size_t	 fwrite (dheader_t *theEntry, size_t theSize, size_t theCount, FILE *theFile);

#endif /* __BIG_ENDIAN__ */

#endif /* ENDIANFIX_H */

//___________________________________________________________________________________________________________eOF
