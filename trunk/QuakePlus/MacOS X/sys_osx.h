//_________________________________________________________________________________________________________________________nFO
// "sys_osx.m" - MacOS X system functions.
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
// Modified by: Martin Linklater            [mailto:mslinklater@mac.com]
//                                          [http://www.95percentchimp.co.uk]
//
// Quakeª is copyrighted by id software		[http://www.idsoftware.com].
//
//______________________________________________________________________________________________________________________mACROS

#pragma mark =Macros=

template< class T >
inline void Sys_CheckMalloc( T* pMem )
{
	if( pMem == 0 ) Sys_Error( (char*)"Out of memory!" );
}

#pragma mark -

//___________________________________________________________________________________________________________________vARIABLES

#pragma mark =Variables=

extern SInt32		gSysArgCount;
extern char **		gSysArgValues;

#pragma mark -

//_________________________________________________________________________________________________________fUNCTION_pROTOTYPES

#pragma mark =Function Prototypes=

void *		Sys_GetProcAddress (const char *theName, Boolean theSafeMode);
char *		Sys_GetClipboardData (void);
void	 	Sys_DoEvents (NSEvent *, NSEventType);
void		Sys_CheckForIDDirectory (void);
void		Sys_SwitchCase (char *, UInt16);
void		Sys_FixFileNameCase (char *);
double		Sys_FloatTime (void);
char *		Sys_sprintf (char *, ...);
double		Sys_FloatTime (void);

//_________________________________________________________________________________________________________________________eOF
