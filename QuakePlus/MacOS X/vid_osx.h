//_________________________________________________________________________________________________________________________nFO
// "vid_osx.h" - MacOS X Video driver
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
// Modified by: Martin Linklater            [mailto:mslinklater@mac.com]
//                                          [http://www.95percentchimp.co.uk]
//
// Quakeª is copyrighted by id software	[http://www.idsoftware.com].
//
//_____________________________________________________________________________________________________________________dEFINES

#pragma mark =Constants=

static const int kVidMaxDisplays = 8;
static const float kVidFadeDuration = 1.0f;

static const int kVidGammaTableSize = 256;
static const int kVidFontWidth = 8;
static const int kVidFontHeight = 8;

#pragma mark -

//____________________________________________________________________________________________________________________tYPEDEFS

#pragma mark =TypeDefs=

typedef struct			{
                                CGDirectDisplayID	displayID;
                                CGGammaValue		component[9];
                        }	vid_gamma_t;

typedef struct			{
                                CGTableCount		count;
                                CGGammaValue		red[ kVidGammaTableSize ];
                                CGGammaValue		green[ kVidGammaTableSize ];
                                CGGammaValue		blue[ kVidGammaTableSize ];
                        }	vid_gammatable_t;

#pragma mark -

//___________________________________________________________________________________________________________________vARIABLES

#pragma mark =Variables=

extern  cvar_t				_windowed_mouse;

extern  NSWindow *			gVidWindow;
extern  BOOL				gVidIsMinimized,
							gVidDisplayFullscreen,
							gVidFadeAllDisplays;
extern  UInt32				gVidDisplay;
extern  CGDirectDisplayID  	gVidDisplayList[ kVidMaxDisplays ];
extern  CGDisplayCount		gVidDisplayCount;
extern	float				gVidWindowPosX,
							gVidWindowPosY;
extern	vid_gamma_t *		gVshOriginalGamma;

extern  NSDictionary *		gVidDisplayMode;
extern	SInt32				gGLMultiSamples;
extern	vid_gammatable_t *	gVshGammaTable;

extern bool gGraphicsWireframe;

//_________________________________________________________________________________________________________fUNCTION_pROTOTYPES

#pragma mark =Function Prototypes=

extern void	M_Menu_Options_f (void);
extern void	M_Print (int, int, char *);
extern void	M_PrintWhite (int, int, char *);
extern void	M_DrawCharacter (int, int, int);
extern void	M_DrawTransPic (int, int, qpic_t *);
extern void	M_DrawPic (int, int, qpic_t *);

BOOL	GL_CheckARBMultisampleExtension (CGDirectDisplayID theDisplay);
void	GL_SetMiniWindowBuffer (void);
    
void	VSH_DisableQuartzInterpolation (id theView);
BOOL	VSH_CaptureDisplays (BOOL theCaptureAllDisplays);
BOOL	VSH_ReleaseDisplays (BOOL theCaptureAllDisplays);
int		VSH_SortDisplayModesCbk(id pMode1, id pMode2, void* pContext);
void	VSH_FadeGammaOut (BOOL theFadeOnAllDisplays, float theDuration);
void	VSH_FadeGammaIn (BOOL theFadeOnAllDisplays, float theDuration);
void	VSH_FadeGammaRelease (void);

//_________________________________________________________________________________________________________________________eOF
