//_________________________________________________________________________________________________________________________nFO
// "Quake.h" - the controller.
//
// Written by:	Axel 'awe' Wefers			[mailto:awe@fruitz-of-dojo.de].
//				©2001-2006 Fruitz Of Dojo 	[http://www.fruitz-of-dojo.de].
//
// Quakeª is copyrighted by id software		[http://www.idsoftware.com].
//
//____________________________________________________________________________________________________________________iNCLUDES

#pragma mark =Includes=

#import "FDLinkView.h"

#import "WebKit/WebKit.h"

#pragma mark -

//_____________________________________________________________________________________________________________________dEFINES

#pragma mark =Defines=

#define	DEFAULT_BASE_PATH			@"QuakePlus ID1 Path"
#define	DEFAULT_OPTION_KEY			@"QuakePlus Dialog Requires Option Key"
#define DEFAULT_USE_MP3				@"QuakePlus Use MP3"
#define DEFAULT_MP3_PATH			@"QuakePlus MP3 Path"
#define DEFAULT_USE_PARAMETERS		@"QuakePlus Use Command-Line Parameters"
#define DEFAULT_PARAMETERS			@"QuakePlus Command-Line Parameters"
#define DEFAULT_FADE_ALL			@"QuakePlus Fade All Displays"
#define DEFAULT_DISPLAY				@"QuakePlus Display"
#define DEFAULT_WINDOW_WIDTH		@"QuakePlus Window Width"
#define DEFAULT_CUR_WINDOW_WIDTH	@"QuakePlus Current Window Width"
#define DEFAULT_GL_DISPLAY			@"QuakePlus Display"
#define	DEFAULT_GL_DISPLAY_MODE		@"QuakePlus Display Mode"
#define DEFAULT_GL_COLORS			@"QuakePlus Display Depth"
#define	DEFAULT_GL_SAMPLES			@"QuakePlus Samples"
#define DEFAULT_GL_FADE_ALL			@"QuakePlus Fade All Displays"
#define DEFAULT_GL_FULLSCREEN		@"QuakePlus Fullscreen"
#define	DEFAULT_GL_OPTION_KEY		@"QuakePlus Dialog Requires Option Key"

#define INITIAL_BASE_PATH			@"id1"
#define	INITIAL_OPTION_KEY			@"NO"
#define INITIAL_USE_MP3				@"NO"
#define INITIAL_MP3_PATH			@""
#define INITIAL_USE_PARAMETERS		@"NO"
#define INITIAL_PARAMETERS			@""
#define INITIAL_DISPLAY				@"0"
#define INITIAL_FADE_ALL			@"YES"
#define INITIAL_WINDOW_WIDTH		@"0"
#define INITIAL_CUR_WINDOW_WIDTH	@"0"
#define INITIAL_GL_DISPLAY			@"0"
#define	INITIAL_GL_DISPLAY_MODE		@"640x480 60Hz"
#define INITIAL_GL_COLORS			@"0"
#define	INITIAL_GL_SAMPLES			@"0"
#define INITIAL_GL_FADE_ALL			@"YES"
#define	INITIAL_GL_FULLSCREEN		@"YES"
#define	INITIAL_GL_OPTION_KEY		@"NO"

#define	SYS_ABOUT_TOOLBARITEM		@"QuakePlus About Toolbaritem"
#define SYS_VIDEO_TOOLBARITEM		@"QuakePlus Displays Toolbaritem"
#define	SYS_AUDIO_TOOLBARITEM		@"QuakePlus Sound Toolbaritem"
#define	SYS_PARAM_TOOLBARITEM		@"QuakePlus Parameters Toolbaritem"
#define	SYS_START_TOOLBARITEM		@"QuakePlus Start Toolbaritem"

#define	OPTION_KEY_DEFAULT			DEFAULT_GL_OPTION_KEY
#define	OPTION_KEY_INITIAL			INITIAL_GL_OPTION_KEY
#define FADE_ALL_DEFAULT			DEFAULT_GL_FADE_ALL
#define FADE_ALL_INITIAL			DEFAULT_GL_FADE_ALL
#define DISPLAY_DEFAULT				DEFAULT_GL_DISPLAY
#define DISPLAY_INITIAL				INITIAL_GL_DISPLAY

#pragma mark -

//_____________________________________________________________________________________________________________iNTERFACE_Quake

#pragma mark =Interface=

@interface Quake : NSObject
{
//    IBOutlet NSWindow				*mediascanWindow;
//    IBOutlet NSTextField			*mediascanTextField;
//    IBOutlet NSProgressIndicator	*mediascanProgressIndicator;

//    IBOutlet NSView					*mp3HelpView;

//    IBOutlet NSView					*aboutView;
//    IBOutlet NSView					*audioView;
//    IBOutlet NSView					*parameterView;

//    IBOutlet NSView					*videoView;
        
    IBOutlet NSPopUpButton			*displayPopUp;
    IBOutlet NSButton				*fadeAllCheckBox;
    
    IBOutlet NSPopUpButton			*modePopUp;
    IBOutlet NSPopUpButton			*colorsPopUp;
    IBOutlet NSPopUpButton			*samplesPopUp;
    IBOutlet NSButton				*fullscreenCheckBox;

    IBOutlet NSButton				*mp3CheckBox;
    IBOutlet NSButton				*mp3Button;
    IBOutlet NSTextField			*mp3TextField;

    IBOutlet NSButton				*optionCheckBox;
    IBOutlet NSButton				*parameterCheckBox;
    IBOutlet NSTextField			*parameterTextField;
    IBOutlet NSMenuItem				*pasteMenuItem;
    IBOutlet NSWindow				*settingsWindow;
    IBOutlet FDLinkView				*linkView;

	// new stuff
	
	IBOutlet NSWindow*				launcherWindow;
	IBOutlet WebView*				newInfo;
	
	IBOutlet NSButton*				wireframeCheckbox;
	
	IBOutlet NSTextField*			consoleTextField;
	IBOutlet NSTextView*			consoleTextView;
	
	// end new stuff
	
    NSMutableArray					*mModeList;
	
    NSMutableDictionary				*mToolbarItems;
    NSView							*mEmptyView;
    NSTimer							*mFrameTimer;
    NSMutableArray					*mRequestedCommands;
    NSString						*mMP3Folder,
									*mModFolder;
    NSDate							*mDistantPast;
    double							mOldFrameTime;
    BOOL							mOptionPressed,
									mDenyDrag,
									mHostInitialized,
									mAllowAppleScriptRun,
									mMediaScanCanceled;
}

+ (void) initialize;
- (void) dealloc;

- (void) stringToParameters: (NSString *) theString;
- (void) requestCommand: (NSString *) theCommand;
- (BOOL) wasDragged;
- (BOOL) hostInitialized;
- (void) setHostInitialized: (BOOL) theState;
- (BOOL) allowAppleScriptRun;
- (void) enableAppleScriptRun: (BOOL) theState;
- (NSDate *) distantPast;
- (NSString *) modFolder;
- (NSString *) mediaFolder;
- (BOOL) abortMediaScan;

- (void)setupNewDialog;

- (IBAction)wireframeCheckboxAction:(id)sender;

@end

//_________________________________________________________________________________________________________________________eOF
