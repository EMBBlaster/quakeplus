//_________________________________________________________________________________________________________________________nFO
// "FDGLScreenshot.h" - Save screenshots (of the current OpenGL context) to various image formats.
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
//
//
//
//____________________________________________________________________________________________________________________iNCLUDES

#import "FDScreenshot.h"

//___________________________________________________________________________________________________________________iNTERFACE

@interface FDGLScreenshot : FDScreenshot
{
}

+ (BOOL) writeToFile: (NSString *) theFile ofType: (NSBitmapImageFileType) theType;
+ (BOOL) writeToBMP: (NSString *) theFile;
+ (BOOL) writeToGIF: (NSString *) theFile;
+ (BOOL) writeToJPEG: (NSString *) theFile;
+ (BOOL) writeToPNG: (NSString *) theFile;
+ (BOOL) writeToTIFF: (NSString *) theFile;

@end

//_________________________________________________________________________________________________________________________eOF
