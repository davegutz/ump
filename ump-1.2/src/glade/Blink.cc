// 	$Id: Blink.cc,v 1.2 2007/01/27 04:38:26 davegutz Exp $	
/* -*-coding: utf-8;-*- */
// Ball tracking driver using libglade.
// The one unique thing about this program that you should be reminded of 
// is the way parameters are initialized.  When the program starts up it
// checks the pwd and xwd directories (working and calibration) for existence
// of the needed files. If the .cam, or .dat files are missing but .cal exists
// in the working directory, it will read the initial values for .cam and .dat
// form the .cal file where they are saved anytime 'extCal' is run.
// The video0.tune and video1.tune files are installed in the working directory
// only.  User can put his own .tune files in the calibration directory if
// desired.
// To use the program, follow the 'guide' until all the boxes are checked.
// Then you can use the stuff on the 'main.'

using namespace std;
#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>                 // Argument list handling
#endif
#include <iostream>                  // cerr etc
#include <Tune.h>                    // MyCamData etc
#include <callbacks.h>               // local callbacks
#include <State.h>                   // StateMachine
#include <zap.h>

// Exported globals 
const gchar    *configFileName = { "cwdPath"  }; // config file with pwd name
const gchar    *xonfigFileName = { "calPath"  }; // config file with xwd name
const gchar    *calLRawName    = { "cal0.raw" }; // Device 0, diff balls.raw
const gchar    *calRRawName    = { "cal1.raw" }; // Device 1, diff balls.raw
const gchar    *ballLRawName[NUM_BALLS] =        // Device 0, diff balls.raw
  {
    "ballLA.raw",  "ballLB.raw",  "ballLC.raw" 
  };
const gchar    *ballRRawName[NUM_BALLS] =        // Device 1, diff blanks.raw
  {
    "ballRA.raw",  "ballRB.raw",  "ballRC.raw" 
  };
const gchar    *blankLRawName[NUM_BALLS] =       // Device 0, diff blanks.raw
  {
    "blankLA.raw",  "blankLB.raw",  "blankLC.raw" 
  };
const gchar    *blankRRawName[NUM_BALLS] =       // Device 1, diff blanks.raw
  {
    "blankRA.raw",  "blankRB.raw",  "blankRC.raw" 
  };
const gchar    *revName[NUM_REVERT_BUTTONS] =    // Revert buttons 
  {
    "revertZone",   "revertWorld",
    "ab0Revert",    "cam0Revert",
    "maskD0Revert", "maskD0ARevert", "maskD0BRevert", "maskD0CRevert",
    "ab1Revert",    "cam1Revert",
    "maskD1Revert", "maskD1ARevert", "maskD1BRevert", "maskD1CRevert",
    "T0Revert",
    "T1Revert"
  };
const gchar    *mRawName[NUM_MRAW_BUTTONS] =     // MeasureRaw buttons
  {
    "mRawA0Button", "mRawB0Button", "mRawC0Button", 
    "mRawA1Button", "mRawB1Button", "mRawC1Button"
  };
const gchar    *tempFileName[NUM_MRAW_BUTTONS] =  // MeasureRaw temp records
  {
    "tempA0", "tempB0", "tempC0",
    "tempA1", "tempB1", "tempC1"
  };
const gchar    *guideName[NUM_GUIDE_BUTTONS] =    // Guide buttons
  {
    "choseWorkingDirectory", "choseCalDirectory",
    "world_datSaved",        "guideTuneSaved",     "extCalRun",
    "guideD0Saved",
    "leftMASaved",           "leftMBSaved",        "leftMCSaved",
    "guideD1Saved",     
    "rightMASaved",          "rightMBSaved",       "rightMCSaved",
    "leftAberrationSaved",   "rightAberrationSaved",
    "leftPixelDataSaved",    "rightPixelDataSaved",
    "snapsDone",         
    "slicerawRun",
    "endEndReady",
    "leftTuneSaved", "rightTuneSaved",
    "whiteA",    "whiteB",    "whiteC",
    "blackA",    "blackB",    "blackC",
    "stillLeft", "stillRight",
  };
const gchar    *msImFileName[NUM_M_IMAGES] =       // Mask image file names
  {
    "video0mask.ppm",
    "video0maskA.ppm", "video0maskB.ppm", "video0maskC.ppm",
    "video1mask.ppm",
    "video1maskA.ppm", "video1maskB.ppm", "video1maskC.ppm"
  };
const gchar    *msFileName[NUM_M_IMAGES] =         // Mask file names
  {
    "video0.mask",
    "video0.maskA",    "video0.maskB",     "video0.maskC",
    "video1.mask",
    "video1.maskA",    "video1.maskB",     "video1.maskC"
  };
const gchar      *stIntName[NUM_INT_STILL_BUTTONS] =
  {
    "leftStillButton", "rightStillButton"
  };
const gchar      *stImName[NUM_INT_STILL_BUTTONS] =
  {
    "leftIntStill.ppm", "rightIntStill.ppm"
  };
const gchar      *stName[NUM_STILL_BUTTONS] =
  {
    "stillWhiteA", "stillWhiteB", "stillWhiteC",
    "stillBlackA", "stillBlackB", "stillBlackC"
  };
const gchar    *msSnFileName[NUM_M_IMAGES] =      // Mask image snap names
  {
    "snapL.ppm",
    "snapLA.ppm", "snapLB.ppm", "snapLC.ppm",
    "snapR.ppm",
    "snapRA.ppm", "snapRB.ppm", "snapRC.ppm"
  };
const gchar    *stSnFileName[NUM_BALLS] =         // Ball labels
  {
    "A", "B", "C"
  };
const gchar    *jumpName[NUM_JUMP_BUTTONS] =      // Guide button names
  {
    "jumpToMain",     "mainGoGuide",
    "jumpToCal",      "calGoGuide",
    "jumpToWorld",    "worldGoGuide",
    "guideGoTune",    "tuneGoGuide",
    "guideGoMaskD0",
    "jumpToLeftMA",   "leftMAGoGuide",
    "jumpToLeftMB",   "leftMBGoGuide",
    "jumpToLeftMC",   "leftMCGoGuide",
    "guideGoMaskD1",
    "jumpToRightMA",  "rightMAGoGuide",
    "jumpToRightMB",  "rightMBGoGuide",
    "jumpToRightMC",  "rightMCGoGuide",
    "jumpToLeftAberration",   "leftAberrationGoGuide",
    "jumpToRightAberration",  "rightAberrationGoGuide",
    "jumpToLeftPixelData",
    "jumpToRightPixelData",
    "jumpToSnaps",    "snapsGoGuide",
    "jumpToLeftTune", "jumpToRightTune",
  };
const gchar      *qsvFileName[]  =                  // Query popup verbiage
  {
    "world file (world.dat)",
    "left aberration file (video0.cam)", "right aberration file (video1.cam)",
    "left pixel data file (video0.dat)", "right pixel data file (video1.dat)",
    "zone file (zone.tune)",
    "left tune file (video0.tune)",      "right tune file (video1.tune)",
    "left mask file (video0.mask)",      "right mask file (video1.mask)",
  };
const gchar      *qowFileName[]  =                 // Query popup verbiage
  {
    "world file (world.dat)",
    "left aberration file (video0.cam)", "right aberration file (video1.cam)",
    "left pixel data file (video0.dat)", "right pixel data file (video1.dat)",
    "zone file (zone.tune)",
    "left tune file (video0.tune)",      "right tune file (video1.tune)",
    "left mask file (video0.mask)",      "right mask file (video1.mask)",
  };
GtkNotebook    *notebook1;                   // Main notebook
GtkNotebook    *notebookL;                   // Left Camera notebook
GtkNotebook    *notebookR;                   // Right Camera notebook
GtkWindow      *maskWindowL;                 // Mask display windows
GtkWindow      *maskWindowR;                 // Mask display windows
GtkWindow      *qsvWindow[NUM_QSV_FILES];    // Dialog save on quit
GtkDialog      *qsv[NUM_QSV_FILES];          // Dialog save on quit
gulong          qsvHand[NUM_QSV_FILES];      // Handler save on quit
GtkHBox        *qsvHbox[NUM_QSV_FILES];      // Dialog Hbox
GtkImage       *qsvIcon[NUM_QSV_FILES];      // Dialog icon
GtkLabel       *qsvText[NUM_QSV_FILES];      // Dialog text
char           *qsvLabel[NUM_QSV_FILES];     // Dialog text
int             qsvAsked[NUM_QSV_FILES];     // Indicator if already asked
GtkWindow      *qowWindow[NUM_QOW_FILES];    // Dialog save on quit
GtkDialog      *qow[NUM_QOW_FILES];          // Dialog save on quit
gulong          qowHand[NUM_QOW_FILES];      // Handler save on quit
GtkHBox        *qowHbox[NUM_QOW_FILES];      // Dialog Hbox
GtkImage       *qowIcon[NUM_QOW_FILES];      // Dialog icon
GtkLabel       *qowText[NUM_QOW_FILES];      // Dialog text
char           *qowLabel[NUM_QOW_FILES];     // Dialog text
int             qowAsked[NUM_QOW_FILES];     // Indicator if already asked
GtkButton      *mRawButton[NUM_MRAW_BUTTONS];// Measure Raw buttons
gulong          mRawHand[NUM_MRAW_BUTTONS];  // Handlers
GtkButton      *revButton[NUM_REVERT_BUTTONS];// Revert buttons
gulong          revHand[NUM_REVERT_BUTTONS]; // Handlers
GtkButton      *jumpButton[NUM_JUMP_BUTTONS];  // Navigation buttons
gulong          jumpHand[NUM_JUMP_BUTTONS];    // Handlers
GtkImage       *maskImage[NUM_M_IMAGES];     // Mask image
gulong          msImHand[NUM_M_BUTTONS];     // Handlers
gulong          msShowMHand[NUM_M_BUTTONS];  // Handlers
gulong          msSaveMHand[NUM_M_BUTTONS];  // Handlers
gulong          recalibrateHand;             // Handlers
gulong          endEndHand;                  // Handlers
int             msImDev[NUM_M_IMAGES]    = {0,0,0,0,1,1,1,1}; // Device by mask
char           *mImgFile[NUM_M_IMAGES];      // Storage for mask image
char           *stillFile[2];                // Snapshot sources
char           *snapFile[NUM_M_IMAGES];      // Snapshot sources
char           *calLFile = new char[MAX_CANON]; // cal0.raw file
char           *calRFile = new char[MAX_CANON]; // cal1.raw file
int             mImgFileExists[NUM_M_IMAGES];   // Image file exists
int             mImgFileExistsPWD[NUM_M_IMAGES];// Image file exists pwd
int             snapFileExists[NUM_M_IMAGES];   // Snap file exists
int             snapFileExistsPWD[NUM_M_IMAGES];// Snap file exists pwd
char           *msFile[NUM_M_IMAGES];           // Mask file path
char           *worldFile    = new char[MAX_CANON]; // world.dat file
char           *tuneFile     = new char[MAX_CANON]; // zone.tune file
GtkButton      *jump[NUM_JUMP_BUTTONS];         // Guide buttons
GtkButton      *rev[NUM_REVERT_BUTTONS];        // Revert buttons
GtkButton      *mRaw[NUM_MRAW_BUTTONS];         // measureRaw buttons
GtkButton      *autoButton;                     // auto button
GtkButton      *slicerawButton;                 // sliceraw button
GtkButton      *endEndButton;                   // endEnd button
GtkButton      *functionShowM[NUM_M_IMAGES];    // Show masks
GtkButton      *saveMButton[NUM_M_IMAGES];      // Save masks
GtkButton      *calcButton[NUM_CALC_BUTTONS];   // Calculator buttons
gulong          calcHand[NUM_CALC_BUTTONS];     // Calculator handler
GtkCheckButton *guideButton[NUM_GUIDE_BUTTONS]; // Guide button
gint           *guideChecked[NUM_GUIDE_BUTTONS];// Guide check button status
gulong          guideHand[NUM_GUIDE_BUTTONS];   // Handlers
GtkButton      *stillButton[NUM_STILL_BUTTONS]; // Stills button
gulong          stHand[NUM_STILL_BUTTONS];  // Handlers
GtkButton      *intStillButton[NUM_INT_STILL_BUTTONS]; // Stills button
gulong          stIntHand[NUM_INT_STILL_BUTTONS];  // Handlers
gint            blackWhiteDone[NUM_STILL_BUTTONS]; // Snap indicators
GtkButton      *recalibrateButton;          // Calibrate
GtkButton      *recalibrateButtonL;         // Calibrate in left
GtkButton      *recalibrateButtonR;         // Calibrate in right
GtkButton      *refRunPar0Button;           // Save and calibrate
GtkButton      *refRunPar1Button;           // Save and calibrate
GtkButton      *saveT0Button;               // Save tune
GtkButton      *saveT1Button;               // Save tune
GtkButton      *killump;                    // run killUmp
gchar          *pwd;                        // Present Working Directory 
gchar          *home;                       // Home directory
gchar          *xwd;                        // Calibration Directory 
GtkCheckButton *stills;                     // Stills button
gint            still        = 0;           // Stills option
GtkCheckButton *calLink;                    // Link button, xwd/*.cal to pwd
gint            calLinked    = 0;           // Link status, xwd/*.cal to pwd
gulong          calLinkHand;                // Link button handler
GtkProgressBar *progressbar;                // progress bar TODO: not used?
guint           permissions;                // File open permissions
GnomeVFSURI    *configDirUri;               // configuration file dir uri
GnomeVFSURI    *configFileUri;              // configuration file uri
GnomeVFSURI    *xonfigFileUri;              // cal configuration file uri
GnomeApp       *app;                        // Main app
int             numDev       = 2;           // Number cameras used
char            device_digit[2];            // Camera number
int             verbose      = 0;           // Verbosity of messages
int             autoWait     = 25;          // Auto mode wait time
int             calibrate    = 0;           // Controls error message
int             worldSpinChanged = 0;       // keep track of world spin
int             tuneSpinChanged  = 0;       // keep track of tune spin
int             maskSpinChanged[NUM_M_IMAGES]  = {0, 0 ,0, 0,
						  0, 0, 0, 0};
                                            // keep track of mask spin
int             abSpinChanged[2]  = {0, 0}; // keep track of aberration
int             datSpinChanged[2] = {0, 0}; // keep track of aberration
int             tunSpinChanged[2] = {0, 0}; // keep track of tune
Zone           *ZON;                        // tune information
Calibration    *CAL[2];                     // Calibration properties
MyCamData      *CAM[2];                     // Camera properties
Tune           *TUN[2];                     // Camera tune properties (pwd only)
char           *rawFile[2]    = {new char[MAX_CANON], new char[MAX_CANON]};
char           *camFile[2]    = {new char[MAX_CANON], new char[MAX_CANON]};
char           *datFile[2]    = {new char[MAX_CANON], new char[MAX_CANON]};
char           *tunFile[2]    = {new char[MAX_CANON], new char[MAX_CANON]};
char           *calPath[2]    = {new char[MAX_CANON], new char[MAX_CANON]};
char           *calFile[2]    = {new char[MAX_CANON], new char[MAX_CANON]};
char           *calFilePWD[2] = {new char[MAX_CANON], new char[MAX_CANON]};
char           *device[2]     = {new char[MAX_CANON], new char[MAX_CANON]};
GtkTextBuffer  *mainDirBuffer;
GtkTextBuffer  *calDirBuffer;
GtkTextBuffer  *runPar0Buffer;
GtkTextBuffer  *runPar1Buffer;
GtkTextBuffer  *linkedDirBuffer[2];
gchar          *runPar0;
gchar          *runPar1;
gchar          *defMaskImage;
int             blockQueryRemove = 0;       // Block during init

// Spin buttons
GtkSpinButton  *spM[NUM_M_BUTTONS];
gulong          spMHand[NUM_M_BUTTONS];
gdouble         spMVa[NUM_M_BUTTONS];
GtkSpinButton  *spCa[NUM_CA_BUTTONS];
gulong          spCaHand[NUM_CA_BUTTONS];
gdouble         spCaVa[NUM_CA_BUTTONS];
GtkSpinButton  *spAb[NUM_AB_BUTTONS];
gulong          spAbHand[NUM_AB_BUTTONS];
gdouble         spAbVa[NUM_AB_BUTTONS];
GtkSpinButton  *spDat[NUM_DAT_BUTTONS];
gulong          spDatHand[NUM_DAT_BUTTONS];
gdouble         spDatVa[NUM_DAT_BUTTONS];
GtkSpinButton  *spWorld[NUM_WORLD_BUTTONS];
gulong          spWorldHand[NUM_WORLD_BUTTONS];
gdouble         spWorldVa[NUM_WORLD_BUTTONS];
GtkSpinButton  *spTune[NUM_TUNE_BUTTONS];
gulong          spTuneHand[NUM_TUNE_BUTTONS];
gdouble         spTuneVa[NUM_TUNE_BUTTONS];

// State machine
StateMachine SM, BLACKWHITE;

// Program usage
void usage(char *prog)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    
    fprintf(stdout,
	    "\'%s\' Executive GUI for \'ump\' program.\n"
	    "Tracks ball position and predicts zone impact."
	    "Reads world.dat, video?.dat, video?.cam."
	    "Writes video?.cal."
	    "Runs \'ump\'."
	    "usage:\n %s [ options ]\n"
	    "\n"
	    "options:\n"
	    "  -h, --help         this screen                     \n"
	    "  -v, --verbose      print lots of information             [%d]\n"
	    "                                                               \n"
	    "examples:                                                      \n"
	    "\'%s\'\n"
	    "     normal mode\n"
	    "--                                                             \n"
	    "(C) 2000-2006 Dave Gutz <davegutz@alum.mit.edu>\n",
	    prog, prog, verbose, prog);
}
/* ---------------------------------------------------------------------- */

///////////////////////////////// main /////////////////////////////////
int main(int argc, char **argv)
{
  GladeXML         *ui;
  GnomeProgram     *program;
  GtkWindow        *calculator;
  GtkImageMenuItem *about1;
  GtkImageMenuItem *info;
  GnomeAbout       *aboutBlink;
  GtkDialog        *mainDirChanger;
  GtkDialog        *calDirChanger;
  GnomeFileEntry   *mainDirEntry;
  GnomeFileEntry   *calDirEntry;
  GtkImageMenuItem *open1;
  GtkImageMenuItem *quit1;
  GtkButton        *mainDirButton;
  GtkButton        *continuous;
  GtkButton        *pause;
  GtkButton        *single;
  GtkButton        *replay; 
  GtkButton        *leftShowStill; 
  GtkButton        *rightShowStill; 
  GtkButton        *plot;
  GtkButton        *dialog;
  GtkButton        *store;
  GtkButton        *mainDirCancel;
  GtkButton        *mainDirApply; 
  GtkButton        *mainDirOk;
  GtkButton        *showM0Button;
  GtkButton        *showM1Button;
  GtkButton        *saveTheWorld;  
  GtkButton        *saveZoneTuneButton;
  GtkButton        *calDirButton;
  GtkButton        *calDirOk;
  GtkButton        *calDirCancel;
  GtkButton        *calDirApply;
  const gchar       configDirName[]  = ".Blink";
  GnomeVFSHandle   *configFile;
  GnomeVFSDirectoryHandle   *configDir;
  GnomeVFSHandle   *xonfigFile;
  GnomeVFSFileSize  bytesRead;
  GnomeVFSResult    result;
  gchar             owd[MAX_CANON];   // Old working directory
  gchar             oxwd[MAX_CANON];  // Old calibration working directory
  gchar            *gladefile;
  GnomeVFSFileSize  bytesWritten;
  gchar            *dirPath;
  GtkTextView      *mainDirView;
  GtkTextView      *calDirView;
  GtkTextView      *runPar0View;
  GtkTextView      *runPar1View;
  GtkTextView      *linkedDirView0;
  GtkTextView      *linkedDirView1;
  const gchar      *calcName[NUM_CALC_BUTTONS] =
    {
      "calc0Button", "calc0AButton", "calc0BButton", "calc0CButton",
      "calc1Button", "calc1AButton", "calc1BButton", "calc1CButton"
    };
  const gchar      *saveMName[NUM_M_IMAGES] =
  {
    "saveM0Button", "saveM0AButton", "saveM0BButton", "saveM0CButton",
    "saveM1Button", "saveM1AButton", "saveM1BButton", "saveM1CButton",
  };
  const gchar      *msImName[] =
    {
      "maskImage0", "maskImage0A", "maskImage0B", "maskImage0C",
      "maskImage1", "maskImage1A", "maskImage1B", "maskImage1C"
    };
  const gchar      *spMName[]  =
    {
      "SxD0-spin",   "SyD0-spin",   "DxD0-spin",  "DyD0-spin",  "DDxD0-spin",
      "DDyD0-spin",  "valD0-spin",
      "Sx10-spin",   "Sy10-spin",   "Dx10-spin",  "Dy10-spin",  "DDx10-spin",
      "DDy10-spin",  "val10-spin",
      "Sx20-spin",   "Sy20-spin",   "Dx20-spin",  "Dy20-spin",  "DDx20-spin",
      "DDy20-spin",  "val20-spin",
      "Sx30-spin",   "Sy30-spin",   "Dx30-spin",  "Dy30-spin",  "DDx30-spin",
      "DDy30-spin",  "val30-spin",
      "Sx40-spin",   "Sy40-spin",   "Dx40-spin",  "Dy40-spin",  "DDx40-spin",
      "DDy40-spin",  "val40-spin",
      "Sx50-spin",   "Sy50-spin",   "Dx50-spin",  "Dy50-spin",  "DDx50-spin",
      "DDy50-spin",  "val50-spin",
      "SxD0A-spin",  "SyD0A-spin",  "DxD0A-spin", "DyD0A-spin", "DDxD0A-spin",
      "DDyD0A-spin", "valD0A-spin",
      "Sx10A-spin",  "Sy10A-spin",  "Dx10A-spin", "Dy10A-spin", "DDx10A-spin",
      "DDy10A-spin", "val10A-spin",
      "Sx20A-spin",  "Sy20A-spin",  "Dx20A-spin", "Dy20A-spin", "DDx20A-spin",
      "DDy20A-spin", "val20A-spin",
      "Sx30A-spin",  "Sy30A-spin",  "Dx30A-spin", "Dy30A-spin", "DDx30A-spin",
      "DDy30A-spin", "val30A-spin",
      "Sx40A-spin",  "Sy40A-spin",  "Dx40A-spin", "Dy40A-spin", "DDx40A-spin",
      "DDy40A-spin", "val40A-spin",
      "Sx50A-spin",  "Sy50A-spin",  "Dx50A-spin", "Dy50A-spin", "DDx50A-spin",
      "DDy50A-spin", "val50A-spin",
      "SxD0B-spin",  "SyD0B-spin",  "DxD0B-spin", "DyD0B-spin", "DDxD0B-spin",
      "DDyD0B-spin", "valD0B-spin",
      "Sx10B-spin",  "Sy10B-spin",  "Dx10B-spin", "Dy10B-spin", "DDx10B-spin",
      "DDy10B-spin", "val10B-spin",
      "Sx20B-spin",  "Sy20B-spin",  "Dx20B-spin", "Dy20B-spin", "DDx20B-spin",
      "DDy20B-spin", "val20B-spin",
      "Sx30B-spin",  "Sy30B-spin",  "Dx30B-spin", "Dy30B-spin", "DDx30B-spin",
      "DDy30B-spin", "val30B-spin",
      "Sx40B-spin",  "Sy40B-spin",  "Dx40B-spin", "Dy40B-spin", "DDx40B-spin",
      "DDy40B-spin", "val40B-spin",
      "Sx50B-spin",  "Sy50B-spin",  "Dx50B-spin", "Dy50B-spin", "DDx50B-spin",
      "DDy50B-spin", "val50B-spin",
      "SxD0C-spin",  "SyD0C-spin",  "DxD0C-spin", "DyD0C-spin", "DDxD0C-spin",
      "DDyD0C-spin", "valD0C-spin",
      "Sx10C-spin",  "Sy10C-spin",  "Dx10C-spin", "Dy10C-spin", "DDx10C-spin",
      "DDy10C-spin", "val10C-spin",
      "Sx20C-spin",  "Sy20C-spin",  "Dx20C-spin", "Dy20C-spin", "DDx20C-spin",
      "DDy20C-spin", "val20C-spin",
      "Sx30C-spin",  "Sy30C-spin",  "Dx30C-spin", "Dy30C-spin", "DDx30C-spin",
      "DDy30C-spin", "val30C-spin",
      "Sx40C-spin",  "Sy40C-spin",  "Dx40C-spin", "Dy40C-spin", "DDx40C-spin",
      "DDy40C-spin", "val40C-spin",
      "Sx50C-spin",  "Sy50C-spin",  "Dx50C-spin", "Dy50C-spin", "DDx50C-spin",
      "DDy50C-spin", "val50C-spin",
      "SxD1-spin",   "SyD1-spin",   "DxD1-spin",  "DyD1-spin",  "DDxD1-spin",
      "DDyD1-spin",  "valD1-spin",
      "Sx11-spin",   "Sy11-spin",   "Dx11-spin",  "Dy11-spin",  "DDx11-spin",
      "DDy11-spin",  "val11-spin",
      "Sx21-spin",   "Sy21-spin",   "Dx21-spin",  "Dy21-spin",  "DDx21-spin",
      "DDy21-spin",  "val21-spin",
      "Sx31-spin",   "Sy31-spin",   "Dx31-spin",  "Dy31-spin",  "DDx31-spin",
      "DDy31-spin",  "val31-spin",
      "Sx41-spin",   "Sy41-spin",   "Dx41-spin",  "Dy41-spin",  "DDx41-spin",
      "DDy41-spin",  "val41-spin",
      "Sx51-spin",   "Sy51-spin",   "Dx51-spin",  "Dy51-spin",  "DDx51-spin",
      "DDy51-spin",  "val51-spin",
      "SxD1A-spin",  "SyD1A-spin",  "DxD1A-spin", "DyD1A-spin", "DDxD1A-spin",
      "DDyD1A-spin", "valD1A-spin",
      "Sx11A-spin",  "Sy11A-spin",  "Dx11A-spin", "Dy11A-spin", "DDx11A-spin",
      "DDy11A-spin", "val11A-spin",
      "Sx21A-spin",  "Sy21A-spin",  "Dx21A-spin", "Dy21A-spin", "DDx21A-spin",
      "DDy21A-spin", "val21A-spin",
      "Sx31A-spin",  "Sy31A-spin",  "Dx31A-spin", "Dy31A-spin", "DDx31A-spin",
      "DDy31A-spin", "val31A-spin",
      "Sx41A-spin",  "Sy41A-spin",  "Dx41A-spin", "Dy41A-spin", "DDx41A-spin",
      "DDy41A-spin", "val41A-spin",
      "Sx51A-spin",  "Sy51A-spin",  "Dx51A-spin", "Dy51A-spin", "DDx51A-spin",
      "DDy51A-spin", "val51A-spin",
      "SxD1B-spin",  "SyD1B-spin",  "DxD1B-spin", "DyD1B-spin", "DDxD1B-spin",
      "DDyD1B-spin", "valD1B-spin",
      "Sx11B-spin",  "Sy11B-spin",  "Dx11B-spin", "Dy11B-spin", "DDx11B-spin",
      "DDy11B-spin", "val11B-spin",
      "Sx21B-spin",  "Sy21B-spin",  "Dx21B-spin", "Dy21B-spin", "DDx21B-spin",
      "DDy21B-spin", "val21B-spin",
      "Sx31B-spin",  "Sy31B-spin",  "Dx31B-spin", "Dy31B-spin", "DDx31B-spin",
      "DDy31B-spin", "val31B-spin",
      "Sx41B-spin",  "Sy41B-spin",  "Dx41B-spin", "Dy41B-spin", "DDx41B-spin",
      "DDy41B-spin", "val41B-spin",
      "Sx51B-spin",  "Sy51B-spin",  "Dx51B-spin", "Dy51B-spin", "DDx51B-spin",
      "DDy51B-spin", "val51B-spin",
      "SxD1C-spin",  "SyD1C-spin",  "DxD1C-spin", "DyD1C-spin", "DDxD1C-spin",
      "DDyD1C-spin", "valD1C-spin",
      "Sx11C-spin",  "Sy11C-spin",  "Dx11C-spin", "Dy11C-spin", "DDx11C-spin",
      "DDy11C-spin", "val11C-spin",
      "Sx21C-spin",  "Sy21C-spin",  "Dx21C-spin", "Dy21C-spin", "DDx21C-spin",
      "DDy21C-spin", "val21C-spin",
      "Sx31C-spin",  "Sy31C-spin",  "Dx31C-spin", "Dy31C-spin", "DDx31C-spin",
      "DDy31C-spin", "val31C-spin",
      "Sx41C-spin",  "Sy41C-spin",  "Dx41C-spin", "Dy41C-spin", "DDx41C-spin",
      "DDy41C-spin", "val41C-spin",
      "Sx51C-spin",  "Sy51C-spin",  "Dx51C-spin", "Dy51C-spin", "DDx51C-spin",
      "DDy51C-spin", "val51C-spin"
    };
  const gchar      *spCaName[]  =
    {
      "n-spin",  "m-spin",  "Sx-spin", "Sy-spin", "Dx-spin", "Dy-spin",
      "x0-spin", "y0-spin", "x1-spin", "y1-spin", "dx-spin", "dy-spin"
    };
  const gchar      *spAbName[]  =
    {
      "imWidth0-spin", "imHeight0-spin", "focal0-spin",
      "AbC0-spin",     "Abr0-spin",      "Abi0-spin",  "Abj0-spin", "ARC0-spin",
      "SY0-spin",
      "imWidth1-spin", "imHeight1-spin", "focal1-spin",
      "AbC1-spin",     "Abr1-spin",      "Abi1-spin",  "Abj1-spin", "ARC1-spin",
      "SY1-spin"
    };
  const gchar      *spDatName[]  =
    {
      "OAP00-spin", "OAP10-spin",
      "OBP00-spin", "OBP10-spin",
      "OCP00-spin", "OCP10-spin",
      "OAP01-spin", "OAP11-spin",
      "OBP01-spin", "OBP11-spin",
      "OCP01-spin", "OCP11-spin"
    };
  const gchar      *spWorldName[]  =
    {
      "PlX-spin", "PlY-spin", "PlZ-spin",
      "PrX-spin", "PrY-spin", "PrZ-spin",
      "OAX-spin", "OAY-spin", "OAZ-spin",
      "OBX-spin", "OBY-spin", "OBZ-spin",
      "OCX-spin", "OCY-spin", "OCZ-spin"
    };
  const gchar      *spTuneName[]  =
    {
      "ballDia-spin", "triangulationScalar-spin", "dxZone-spin", "dzZone-spin",
      "dXdMphZone-spin", "dZdMphZone-spin",
      "velThresh-spin", "velMax-spin", "gravity-spin", "left-spin", "top-spin",
      "right-spin", "bottom-spin", "speedDisplayScalar-spin", "verbose-spin",
      "auto-spin",
      "agaussScalar0-spin",        "bgaussScalar0-spin",
      "minBallDensityScalar0-spin",
      "minBallAreaScalar0-spin",   "maxBallAreaScalar0-spin",
      "minBallRatioScalar0-spin",  "maxBallRatioScalar0-spin",
      "minProxScalar0-spin",
      "agaussScalar1-spin",        "bgaussScalar1-spin",
      "minBallDensityScalar1-spin",
      "minBallAreaScalar1-spin",   "maxBallAreaScalar1-spin",
      "minBallRatioScalar1-spin",  "maxBallRatioScalar1-spin",
      "minProxScalar1-spin",
    };


  char c;

  // parse options
  int num_args=0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"help",        0, 0, 'h'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "hVv:",
		      long_options, &option_index);
#else
      c = getopt(argc, argv, "hVv:");
#endif // HAVE_GETOPT_LONG
      if ( '?' == c ) 
	{
	  usage(argv[0]);
	  exit(1);
	}
      if ( -1  == c )
	{
	  break;
	}
      num_args++;
      switch (c) 
	{
	case 'V':
	  cout << "$Id: Blink.cc,v 1.2 2007/01/27 04:38:26 davegutz Exp $"
	       << "written by Dave Gutz Copyright (C) 2006"   << endl;
	  exit(0);
	  break;
	case 'v':
	  verbose = atoi(optarg);
	  if ( 0 > verbose )
	    {
	      cerr << "ERROR(Blink):  " << __FILE__ << " : " << __LINE__
		   << " : " << "bad verbose specified\n";
	      exit(1);
	    }
#ifndef VERBOSE
	  if ( verbose )
	    cerr << "WARNING(Blink):  recompile \'-DVERBOSE\' to use all "
		 << "the verbose messages.\n" << flush;
#endif

	  break;
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  break;
	default:
	  usage(argv[0]);
	  if( 2 > num_args )exit(1);
	  break;
	}
    }

  // Initialize video card names
  strcpy(device[0], "/dev/video0");
  strcpy(device[1], "/dev/video1");

  // get home
  home = gnome_util_prepend_user_home("");

  // initialize GNOME 
#ifdef VERBOSE
  if ( 4 < verbose ) cerr << "MESSAGE(Blink):  initializing program\n" << flush;
#endif
  program = gnome_program_init("Blink", "0.13-7",
                               LIBGNOMEUI_MODULE,
                               argc, argv,
                               GNOME_PROGRAM_STANDARD_PROPERTIES,
                               GNOME_PARAM_HUMAN_READABLE_NAME, "Umpire",
                               NULL);
  // Open .Blink directory
  dirPath       = gnome_util_prepend_user_home(configDirName);
  configDirUri  = gnome_vfs_uri_new(dirPath);
  result = gnome_vfs_directory_open_from_uri(&configDir, configDirUri,
					    GNOME_VFS_FILE_INFO_DEFAULT);
  if ( GNOME_VFS_OK != result )
    {
      // create and open path file URI
      cout << "MESSAGE(Blink) : creating " << dirPath << endl << flush;
      permissions = GNOME_VFS_PERM_USER_READ | GNOME_VFS_PERM_USER_WRITE |
	GNOME_VFS_PERM_USER_EXEC | GNOME_VFS_PERM_GROUP_READ |
	GNOME_VFS_PERM_GROUP_WRITE | GNOME_VFS_PERM_GROUP_EXEC |
	GNOME_VFS_PERM_OTHER_READ |
	GNOME_VFS_PERM_OTHER_WRITE | GNOME_VFS_PERM_OTHER_EXEC;
      gnome_vfs_make_directory_for_uri(configDirUri, permissions);
      if ( 0 > chdir(home) )
	{
	  cerr << "WARNING(blink) : couldn't open directory "
	       << home << endl << flush;
	}
    }
  else
    {
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "MESSAGE(Blink) : found " << dirPath
			      << endl << flush;
#endif
    }

  configFileUri = gnome_vfs_uri_append_file_name(configDirUri, configFileName);
  xonfigFileUri = gnome_vfs_uri_append_file_name(configDirUri, xonfigFileName);

  // permissions: user has read-write access, group and other read-only
  permissions = GNOME_VFS_PERM_USER_READ | GNOME_VFS_PERM_USER_WRITE |
    GNOME_VFS_PERM_GROUP_READ | GNOME_VFS_PERM_OTHER_READ;

  // Open path file URI
  result = gnome_vfs_open_uri(&configFile, configFileUri, GNOME_VFS_OPEN_READ);
  mainDirBuffer           = gtk_text_buffer_new(NULL);
  calDirBuffer            = gtk_text_buffer_new(NULL);
  linkedDirBuffer[0]      = gtk_text_buffer_new(NULL);
  linkedDirBuffer[1]      = gtk_text_buffer_new(NULL);
  if ( GNOME_VFS_OK != result )
    {
      // create and open path file URI
      result = gnome_vfs_create_uri(&configFile, configFileUri,
				GNOME_VFS_OPEN_WRITE, FALSE, permissions); 
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
      // write pwd into path file descriptor
      if ( !(pwd = getcwd(pwd, MAX_CANON)) )
	{
	  perror("Couldn't open directory:");
	  exit(1);
	}
      result = gnome_vfs_write(configFile, pwd, strlen(pwd)+1, &bytesWritten);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
    }
  else
    {      
      // read pwd from path file descriptor
      result = gnome_vfs_read(configFile, owd, MAX_CANON, &bytesRead);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
      
      // close path file descriptor
      result = gnome_vfs_close(configFile);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
      if ( owd )
	{
	  if ( 0 > chdir(owd) )
	    {
	      perror("Couldn't open ow directory:");
	    }
	}
    }

  // Open path file URI
  result = gnome_vfs_open_uri(&xonfigFile, xonfigFileUri, GNOME_VFS_OPEN_READ);
  if ( GNOME_VFS_OK != result )
    {
      // create and open path file URI
      result = gnome_vfs_create_uri(&xonfigFile, xonfigFileUri,
				GNOME_VFS_OPEN_WRITE, FALSE, permissions); 
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}
      // write pwd into path file descriptor
      if ( !(pwd = getcwd(pwd, MAX_CANON)) )
	{
	  perror("Couldn't open directory:");
	  exit(1);
	}
      xwd = strdup(pwd);
      result = gnome_vfs_write(xonfigFile, xwd, strlen(xwd)+1, &bytesWritten);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}
    }
  else
    {      
      // read xwd from path file descriptor
      result = gnome_vfs_read(xonfigFile, oxwd, MAX_CANON, &bytesRead);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}
      
      // close path file descriptor
      result = gnome_vfs_close(xonfigFile);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}
      xwd = strdup(oxwd);
    }

  // get the pwd 
  if ( !(pwd = getcwd(pwd, MAX_CANON)) )
    {
      perror("Couldn't open cw directory:");
      exit(1);
    }
  g_print("Working directory %s\n", pwd);
  g_print("Calibrate directory %s\n", xwd);
  gtk_text_buffer_set_text(mainDirBuffer, pwd, -1);
  gtk_text_buffer_set_text(calDirBuffer,  xwd, -1);
  
  // Find default mask image file
  defMaskImage = gnome_program_locate_file(program,
					   GNOME_FILE_DOMAIN_APP_DATADIR,
					   "Blink.png", TRUE, NULL);

  // read the definitions with libglade
  gladefile = gnome_program_locate_file(program, GNOME_FILE_DOMAIN_APP_DATADIR,
  					"blink.glade", TRUE, NULL);
  if ( !gladefile )
    {
      perror("ERROR(Blink):  blink.glade not found");
      exit(1);
    }

  ui = glade_xml_new(gladefile, NULL, NULL);
#ifdef VERBOSE
  if ( 4 < verbose )
    cerr << "MESSAGE(Blink):  blink.glade loaded\n" << flush;
#endif

  // initialize GnomeVFS
  if (!gnome_vfs_init())
    {
      g_error("could not initialize GnomeVFS\n");
      exit(1);
    }
#ifdef VERBOSE
  else if ( 4 < verbose )
    cerr << "MESSAGE(Blink):  initialized GnomeVFS\n" << flush;
#endif

  // Check file paths and existence
  for (int i = 0; i < numDev; i++ )
    device_digit[i] = device[i][strlen(device[i])-1];
  for (int i=LM; i < NUM_M_IMAGES; i++)
    {
      mImgFile[i] = new char[MAX_CANON];
      stillFile[i] = new char[MAX_CANON];
      snapFile[i] = new char[MAX_CANON];
      msFile[i]    = new char[MAX_CANON];
    }
  checkSetFilePaths(pwd);
  checkSetFilePaths(xwd);
  checkSetPathMainMask(pwd);
#ifdef VERBOSE
  if ( 4 < verbose )
    cerr << "MESSAGE(Blink):  initialized paths\n" << flush;
#endif

  // Instantiate

  for (int i=QSVWORLD; i<NUM_QSV_FILES; i++)
    {
      qsvAsked[i]  = 0;
      qsvWindow[i] = GTK_WINDOW(g_object_new(GTK_TYPE_WINDOW,
					     "default_height", 200,
					     "default_width", 200,
					     "border-width", 12,
					     "title", "Query",
					     NULL));
      qsv[i] =
	GTK_DIALOG(gtk_dialog_new_with_buttons("Query",       // title
					       qsvWindow[i],  // parent
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       "_Save",   GTK_RESPONSE_OK,
					       "_Skip",   GTK_RESPONSE_NO,
					       NULL));
      qsvLabel[i] = new char[MAX_CANON];
    }
  for (int i=QOWWORLD; i<NUM_QOW_FILES; i++)
    {
      qowAsked[i]  = 0;
      qowWindow[i] = GTK_WINDOW(g_object_new(GTK_TYPE_WINDOW,
					     "default_height", 200,
					     "default_width", 200,
					     "border-width", 12,
					     "title", "Query",
					     NULL));
      qow[i] =
	GTK_DIALOG(gtk_dialog_new_with_buttons("Query",       // title
					       qowWindow[i],  // parent
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       "_Overwrite", GTK_RESPONSE_OK,
					       "_Skip",      GTK_RESPONSE_NO,
					       NULL));
      qowLabel[i] = new char[MAX_CANON];
    }
  for (int i=JUMP_TO_MAIN; i < NUM_JUMP_BUTTONS; i++)
    jumpButton[i] = GTK_BUTTON(glade_xml_get_widget(ui,           jumpName[i]));
  for (int i=REVERT_ZONE; i < NUM_REVERT_BUTTONS; i++)
    revButton[i] = GTK_BUTTON(glade_xml_get_widget(ui,             revName[i]));
  for (int i=MRAWA0; i < NUM_MRAW_BUTTONS; i++)
    mRawButton[i] = GTK_BUTTON(glade_xml_get_widget(ui,           mRawName[i]));
  for (int i=CHOSE_WORKING_DIRECTORY; i < NUM_GUIDE_BUTTONS; i++)
    guideButton[i] = GTK_CHECK_BUTTON(glade_xml_get_widget(ui,   guideName[i]));
  for (int i=CALC0; i < NUM_CALC_BUTTONS; i++)
    calcButton[i] =  GTK_BUTTON(glade_xml_get_widget(ui,          calcName[i]));
  for (int i=LM; i < NUM_M_IMAGES; i++)
    functionShowM[i] = (GtkButton*)g_object_new(GTK_TYPE_BUTTON, NULL);
  for (int i=LM; i < NUM_M_IMAGES; i++)
    saveMButton[i] = GTK_BUTTON(glade_xml_get_widget(ui,         saveMName[i]));
  for (int i=SXD0SP; i < NUM_M_BUTTONS; i++)
    spM[i] = GTK_SPIN_BUTTON(glade_xml_get_widget(ui,              spMName[i]));
  for (int i=NSP; i < NUM_CA_BUTTONS; i++)
    spCa[i] = GTK_SPIN_BUTTON(glade_xml_get_widget(ui,            spCaName[i]));
  for (int i=IMW0SP; i < NUM_AB_BUTTONS; i++)
    spAb[i] = GTK_SPIN_BUTTON(glade_xml_get_widget(ui,            spAbName[i]));
  for (int i=OAP00SP; i < NUM_DAT_BUTTONS; i++)
    spDat[i] = GTK_SPIN_BUTTON(glade_xml_get_widget(ui,          spDatName[i]));
  for (int i=PLXSP; i < NUM_WORLD_BUTTONS; i++)
    spWorld[i] = GTK_SPIN_BUTTON(glade_xml_get_widget(ui,      spWorldName[i]));
  for (int i=BALLDIASP; i < NUM_TUNE_BUTTONS; i++)
    spTune[i] = GTK_SPIN_BUTTON(glade_xml_get_widget(ui,        spTuneName[i]));
  for (int i=STILL_WHITEA; i < NUM_STILL_BUTTONS; i++)
    stillButton[i] = GTK_BUTTON(glade_xml_get_widget(ui,            stName[i]));
  for (int i=LEFT_STILL_BUTTON; i < NUM_INT_STILL_BUTTONS; i++)
    intStillButton[i] = GTK_BUTTON(glade_xml_get_widget(ui,      stIntName[i]));
  notebook1      = GTK_NOTEBOOK(glade_xml_get_widget(ui,          "notebook1"));
  notebookL      = GTK_NOTEBOOK(glade_xml_get_widget(ui,          "notebookL"));
  notebookR      = GTK_NOTEBOOK(glade_xml_get_widget(ui,          "notebookR"));
  progressbar    = GTK_PROGRESS_BAR(glade_xml_get_widget(ui,    "progressbar"));
  quit1          = GTK_IMAGE_MENU_ITEM(glade_xml_get_widget(ui,       "quit1"));
  open1          = GTK_IMAGE_MENU_ITEM(glade_xml_get_widget(ui,       "open1"));
  about1         = GTK_IMAGE_MENU_ITEM(glade_xml_get_widget(ui,      "about1"));
  aboutBlink     = GNOME_ABOUT(glade_xml_get_widget(ui,          "aboutBlink"));
  info           = GTK_IMAGE_MENU_ITEM(glade_xml_get_widget(ui,        "info"));
  autoButton     = GTK_BUTTON(glade_xml_get_widget(ui,           "autoButton"));
  slicerawButton = GTK_BUTTON(glade_xml_get_widget(ui,       "slicerawButton"));
  endEndButton   = GTK_BUTTON(glade_xml_get_widget(ui,         "endEndButton"));
  mainDirButton  = GTK_BUTTON(glade_xml_get_widget(ui,        "mainDirButton"));
  calDirButton   = GTK_BUTTON(glade_xml_get_widget(ui,         "calDirButton"));
  recalibrateButton=GTK_BUTTON(glade_xml_get_widget(ui,   "recalibrateButton"));
  recalibrateButtonL=GTK_BUTTON(glade_xml_get_widget(ui, "recalibrateButtonL"));
  recalibrateButtonR=GTK_BUTTON(glade_xml_get_widget(ui, "recalibrateButtonR"));
  refRunPar0Button= GTK_BUTTON(glade_xml_get_widget(ui,    "refRunPar0Button"));
  refRunPar1Button= GTK_BUTTON(glade_xml_get_widget(ui,    "refRunPar1Button"));
  saveT0Button   = GTK_BUTTON(glade_xml_get_widget(ui,         "saveT0Button"));
  saveT1Button   = GTK_BUTTON(glade_xml_get_widget(ui,         "saveT1Button"));
  app            = GNOME_APP(glade_xml_get_widget(ui,                   "app"));
  calculator     = GTK_WINDOW(glade_xml_get_widget(ui,           "calculator"));
  mainDirChanger = GTK_DIALOG(glade_xml_get_widget(ui,       "mainDirChanger"));
  calDirChanger  = GTK_DIALOG(glade_xml_get_widget(ui,        "calDirChanger"));
  mainDirEntry   = GNOME_FILE_ENTRY(glade_xml_get_widget(ui,   "mainDirEntry"));
  calDirEntry    = GNOME_FILE_ENTRY(glade_xml_get_widget(ui,    "calDirEntry"));
  mainDirCancel  = GTK_BUTTON(glade_xml_get_widget(ui,        "mainDirCancel"));
  calDirCancel   = GTK_BUTTON(glade_xml_get_widget(ui,         "calDirCancel"));
  mainDirApply   = GTK_BUTTON(glade_xml_get_widget(ui,         "mainDirApply"));
  calDirApply    = GTK_BUTTON(glade_xml_get_widget(ui,          "calDirApply"));
  mainDirOk      = GTK_BUTTON(glade_xml_get_widget(ui,            "mainDirOk"));
  calDirOk       = GTK_BUTTON(glade_xml_get_widget(ui,             "calDirOk"));
  killump        = GTK_BUTTON(glade_xml_get_widget(ui,           "killButton"));
  continuous     = GTK_BUTTON(glade_xml_get_widget(ui,           "continuous"));
  saveTheWorld   = GTK_BUTTON(glade_xml_get_widget(ui,   "saveTheWorldButton"));
  showM0Button   = GTK_BUTTON(glade_xml_get_widget(ui,         "showM0Button"));
  showM1Button   = GTK_BUTTON(glade_xml_get_widget(ui,         "showM1Button"));
  saveZoneTuneButton= GTK_BUTTON(glade_xml_get_widget(ui,"saveZoneTuneButton"));
  pause          = GTK_BUTTON(glade_xml_get_widget(ui,                "pause"));
  single         = GTK_BUTTON(glade_xml_get_widget(ui,               "single"));
  replay         = GTK_BUTTON(glade_xml_get_widget(ui,               "replay"));
  leftShowStill  = GTK_BUTTON(glade_xml_get_widget(ui,  "leftShowStillButton"));
  rightShowStill = GTK_BUTTON(glade_xml_get_widget(ui, "rightShowStillButton"));
  plot           = GTK_BUTTON(glade_xml_get_widget(ui,                 "plot"));
  dialog         = GTK_BUTTON(glade_xml_get_widget(ui,               "dialog"));
  store          = GTK_BUTTON(glade_xml_get_widget(ui,                "store"));
  stills         = GTK_CHECK_BUTTON(glade_xml_get_widget(ui,         "stills"));
  calLink        = GTK_CHECK_BUTTON(glade_xml_get_widget(ui,     "linkButton"));
  mainDirView    = GTK_TEXT_VIEW(glade_xml_get_widget(ui,       "mainDirView"));
  gtk_text_view_set_buffer(mainDirView, mainDirBuffer);
  calDirView     = GTK_TEXT_VIEW(glade_xml_get_widget(ui,        "calDirView"));
  gtk_text_view_set_buffer(calDirView, calDirBuffer);
  linkedDirView0 = GTK_TEXT_VIEW(glade_xml_get_widget(ui,    "linkedDirView0"));
  gtk_text_view_set_buffer(linkedDirView0, linkedDirBuffer[0]);
  linkedDirView1 = GTK_TEXT_VIEW(glade_xml_get_widget(ui,    "linkedDirView1"));
  gtk_text_view_set_buffer(linkedDirView1, linkedDirBuffer[1]);
  runPar0Buffer  = gtk_text_buffer_new(NULL);
  runPar0View    = GTK_TEXT_VIEW(glade_xml_get_widget(ui,       "runPar0View"));
  gtk_text_view_set_buffer(runPar0View, runPar0Buffer);
  runPar1Buffer  = gtk_text_buffer_new(NULL);
  runPar1View    = GTK_TEXT_VIEW(glade_xml_get_widget(ui,       "runPar1View"));
  gtk_text_view_set_buffer(runPar1View, runPar1Buffer);
  maskWindowL     = GTK_WINDOW(glade_xml_get_widget(ui,         "maskWindowL"));
  maskWindowR     = GTK_WINDOW(glade_xml_get_widget(ui,         "maskWindowR"));

  // get handle on each of mask images
  GdkPixbuf *orig_pixbuf;
  orig_pixbuf = gdk_pixbuf_new_from_file(defMaskImage, NULL);
  for (int i=LM; i < NUM_M_IMAGES; i++)
    {
      maskImage[i] = GTK_IMAGE(glade_xml_get_widget(ui, msImName[i]));
      g_object_set(maskImage[i], "pixbuf", orig_pixbuf, NULL);
      if ( mImgFileExists[i] )
	{
	  GdkPixbuf *pixbuf;
	  pixbuf = gdk_pixbuf_new_from_file(mImgFile[i], NULL);
	  g_object_set(maskImage[i], "pixbuf", pixbuf, NULL);
	  g_object_unref(pixbuf);
	}
    }
  g_object_unref(orig_pixbuf);

  // attach handlers 
  for (int i=CHOSE_WORKING_DIRECTORY; i<NUM_GUIDE_BUTTONS; i++)
    guideHand[i] = g_signal_connect(GTK_CHECK_BUTTON(guideButton[i]),
				    "clicked",
				    G_CALLBACK(on_guide_clicked),
				    GUINT_TO_POINTER(i));

  for (int i=LM; i<NUM_M_IMAGES; i++)
    msImHand[i] = g_signal_connect(maskImage[i],
				   "realize",
				   G_CALLBACK(on_maskImage_realize),
				   GUINT_TO_POINTER(i));


  for (int i=QSVWORLD; i<NUM_QSV_FILES; i++)
    {
      gtk_dialog_set_default_response(qsv[i], GTK_RESPONSE_NO);
      qsvHand[i] = g_signal_connect(qsv[i],
				    "response",
				    G_CALLBACK(qsv_action),
				    GUINT_TO_POINTER(i));
      // Hide dialog box when it returns a response
      g_signal_connect_swapped(qsv[i], "response",
			       G_CALLBACK(gtk_widget_hide), qsv[i]);
      // Fill dialog window:  create HBox packed with icon and text
      qsvHbox[i] = GTK_HBOX(g_object_new(GTK_TYPE_HBOX,
					 "border-width", 8, NULL));
      qsvIcon[i] = GTK_IMAGE(g_object_new(GTK_TYPE_IMAGE,
					  "stock", GTK_STOCK_DIALOG_WARNING,
					  "icon-size", GTK_ICON_SIZE_DIALOG,
					  "xalign", 0.5,
					  "yalign", 1.0,
					  NULL));
      gtk_box_pack_start(GTK_BOX(qsvHbox[i]), GTK_WIDGET(qsvIcon[i]),
			 FALSE, FALSE, 0);
      sprintf(qsvLabel[i], "<big><b>Do you want to save %s.?</b></big>",
	      qsvFileName[i]);
      qsvText[i] = GTK_LABEL(g_object_new(GTK_TYPE_LABEL,
					  "wrap", TRUE,
					  "use-markup", TRUE,
					  "label", qsvLabel[i],
					  NULL));
      gtk_box_pack_start(GTK_BOX(qsvHbox[i]), GTK_WIDGET(qsvText[i]),
			 TRUE, TRUE, 0);
      gtk_box_pack_start(GTK_BOX(qsv[i]->vbox), GTK_WIDGET(qsvHbox[i]),
			 FALSE, FALSE, 0);
    }

  for (int i=QOWWORLD; i<NUM_QOW_FILES; i++)
    {
      gtk_dialog_set_default_response(qow[i], GTK_RESPONSE_NO);
      qowHand[i] = g_signal_connect(qow[i],
				    "response",
				    G_CALLBACK(qow_action),
				    GUINT_TO_POINTER(i));
      // Hide dialog box when it returns a response
      g_signal_connect_swapped(qow[i], "response",
			       G_CALLBACK(gtk_widget_hide), qow[i]);
      // Fill dialog window:  create HBox packed with icon and text
      qowHbox[i] = GTK_HBOX(g_object_new(GTK_TYPE_HBOX,
					 "border-width", 8, NULL));
      qowIcon[i] = GTK_IMAGE(g_object_new(GTK_TYPE_IMAGE,
					  "stock", GTK_STOCK_DIALOG_WARNING,
					  "icon-size", GTK_ICON_SIZE_DIALOG,
					  "xalign", 0.5,
					  "yalign", 1.0,
					  NULL));
      gtk_box_pack_start(GTK_BOX(qowHbox[i]), GTK_WIDGET(qowIcon[i]),
			 FALSE, FALSE, 0);
      sprintf(qowLabel[i], "<big><b>Do you want to overwrite %s.?</b></big>",
	      qowFileName[i]);
      qowText[i] = GTK_LABEL(g_object_new(GTK_TYPE_LABEL,
					  "wrap", TRUE,
					  "use-markup", TRUE,
					  "label", qowLabel[i],
					  NULL));
      gtk_box_pack_start(GTK_BOX(qowHbox[i]), GTK_WIDGET(qowText[i]),
			 TRUE, TRUE, 0);
      gtk_box_pack_start(GTK_BOX(qow[i]->vbox), GTK_WIDGET(qowHbox[i]),
			 FALSE, FALSE, 0);
    }

  for (int i=JUMP_TO_MAIN; i<NUM_JUMP_BUTTONS; i++)
    jumpHand[i] = g_signal_connect(jumpButton[i],
				  "clicked",
				  G_CALLBACK(on_jump_clicked),
				  GUINT_TO_POINTER(i));

  for (int i=REVERT_ZONE; i<NUM_REVERT_BUTTONS; i++)
    revHand[i] = g_signal_connect(revButton[i],
				  "clicked",
				  G_CALLBACK(on_revert_clicked),
				  GUINT_TO_POINTER(i));

  for (int i=MRAWA0; i<NUM_MRAW_BUTTONS; i++)
    mRawHand[i] = g_signal_connect(mRawButton[i],
				   "clicked",
				   G_CALLBACK(on_mRaw_clicked),
				   GUINT_TO_POINTER(i));

  for (int i=STILL_WHITEA; i<NUM_STILL_BUTTONS; i++)
    stHand[i] = g_signal_connect(stillButton[i],
				 "clicked",
				 G_CALLBACK(on_still_clicked),
				 GUINT_TO_POINTER(i));

  for (int i=LEFT_STILL_BUTTON; i<NUM_INT_STILL_BUTTONS; i++)
    stIntHand[i] = g_signal_connect(intStillButton[i],
				    "clicked",
				    G_CALLBACK(on_intStill_clicked),
				    GUINT_TO_POINTER(i));

  for (int i=LM; i<NUM_M_IMAGES; i++)
    msShowMHand[i] = g_signal_connect(functionShowM[i],
				   "clicked",
				   G_CALLBACK(on_showM_clicked),
				   GUINT_TO_POINTER(i));

  for (int i=LM; i<NUM_M_IMAGES; i++)
    msSaveMHand[i] = g_signal_connect(saveMButton[i],
				      "clicked",
				      G_CALLBACK(on_saveM_clicked),
				      GUINT_TO_POINTER(i));

  for (int i=CALC0; i<NUM_CALC_BUTTONS; i++)
    calcHand[i] = g_signal_connect(calcButton[i],
				   "clicked",
				   G_CALLBACK(on_calcButton_clicked),
				   GUINT_TO_POINTER(calculator));

  for (int i=SXD0SP; i<NUM_M_BUTTONS; i++)
    spMHand[i] = g_signal_connect(spM[i],
				  "value-changed",
				  G_CALLBACK(spM_changed),
				  GUINT_TO_POINTER(i));

  for (int i=NSP; i<NUM_CA_BUTTONS; i++)
    spCaHand[i] = g_signal_connect(spCa[i],
				   "value-changed",
				   G_CALLBACK(spCa_changed),
				   GUINT_TO_POINTER(i));

  for (int i=IMW0SP; i<NUM_AB_BUTTONS; i++)
    spAbHand[i] = g_signal_connect(spAb[i],
				   "value-changed",
				   G_CALLBACK(spAb_changed),
				   GUINT_TO_POINTER(i));

  for (int i=OAP00SP; i<NUM_DAT_BUTTONS; i++)
    spDatHand[i] = g_signal_connect(spDat[i],
				    "value-changed",
				    G_CALLBACK(spDat_changed),
				    GUINT_TO_POINTER(i));
  
  for (int i=PLXSP; i<NUM_WORLD_BUTTONS; i++)
    spWorldHand[i] = g_signal_connect(spWorld[i],
				    "value-changed",
				    G_CALLBACK(spWorld_changed),
				    GUINT_TO_POINTER(i));
  
  for (int i=BALLDIASP; i<NUM_TUNE_BUTTONS; i++)
    spTuneHand[i] = g_signal_connect(spTune[i],
				    "value-changed",
				    G_CALLBACK(spTune_changed),
				    GUINT_TO_POINTER(i));

  g_signal_connect(app,    "delete_event",
		   G_CALLBACK(delete_event), NULL);

  g_signal_connect(app,    "destroy",
		   G_CALLBACK(end_program),  NULL);

  g_signal_connect(aboutBlink, "close",
		   G_CALLBACK(on_aboutBlink_hide),
		   NULL);

  g_signal_connect(aboutBlink, "destroy",
		   G_CALLBACK(on_aboutBlink_hide),
		   NULL);

  g_signal_connect(aboutBlink, "delete-event",
		   G_CALLBACK(on_aboutBlink_hide),
		   NULL);

  g_signal_connect(about1, "delete-event",
		   G_CALLBACK(on_aboutBlink_hide),
		   GUINT_TO_POINTER(aboutBlink));

  g_signal_connect(about1, "activate",
		   G_CALLBACK(on_about1_activate),
		   GUINT_TO_POINTER(aboutBlink));

  g_signal_connect(info, "activate",
		   G_CALLBACK(on_info_activate),
		   NULL);

  g_signal_connect(quit1,  "activate",
		   G_CALLBACK(end_program),
		   NULL);

  g_signal_connect(open1,  "activate",
		   G_CALLBACK(on_mainDir_clicked),
		   GUINT_TO_POINTER(mainDirEntry));

  g_signal_connect(mainDirButton, "clicked",
		   G_CALLBACK(on_mainDir_clicked),
		   GUINT_TO_POINTER(mainDirEntry));

  g_signal_connect(calDirButton, "clicked",
		   G_CALLBACK(on_calDir_clicked),
		   GUINT_TO_POINTER(calDirEntry));

  g_signal_connect(refRunPar0Button, "clicked",
		   G_CALLBACK(on_refRunPar0_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(refRunPar1Button, "clicked",
		   G_CALLBACK(on_refRunPar1_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(saveT0Button, "clicked",
		   G_CALLBACK(on_saveT0_clicked),
		   NULL);

  g_signal_connect(saveT1Button, "clicked",
		   G_CALLBACK(on_saveT1_clicked),
		   NULL);

  g_signal_connect(mainDirEntry,"activate",
		   G_CALLBACK(on_mainDirEntry_activate),
		   GUINT_TO_POINTER(app));

  g_signal_connect(calDirEntry,"activate",
		   G_CALLBACK(on_calDirEntry_activate),
		   GUINT_TO_POINTER(app));

  g_signal_connect(mainDirEntry, "hide",
		   G_CALLBACK(on_mainDirEntry_hide),
		   GUINT_TO_POINTER(mainDirChanger));

  g_signal_connect(calDirEntry, "hide",
		   G_CALLBACK(on_calDirEntry_hide),
		   GUINT_TO_POINTER(calDirChanger));

  g_signal_connect(calculator, "destroy",
		   G_CALLBACK(on_calculator_hide),
		   NULL);

  g_signal_connect(calculator, "delete_event",
		   G_CALLBACK(on_calculator_hide),
		   NULL);

  g_signal_connect(calculator, "show",
		   G_CALLBACK(on_calculator_show),
		   NULL);

  g_signal_connect(maskWindowL, "destroy",
		   G_CALLBACK(on_maskWindowL_hide),
		   NULL);

  g_signal_connect(maskWindowL, "delete_event",
		   G_CALLBACK(on_maskWindowL_hide),
		   NULL);

  g_signal_connect(maskWindowL, "show",
		   G_CALLBACK(on_maskWindowL_show),
		   NULL);

  g_signal_connect(maskWindowR, "destroy",
		   G_CALLBACK(on_maskWindowR_hide),
		   NULL);

  g_signal_connect(maskWindowR, "delete_event",
		   G_CALLBACK(on_maskWindowR_hide),
		   NULL);

  g_signal_connect(maskWindowR, "show",
		   G_CALLBACK(on_maskWindowR_show),
		   NULL);

  g_signal_connect(mainDirEntry, "show",
		   G_CALLBACK(on_mainDirEntry_show),
		   GUINT_TO_POINTER(mainDirChanger));

  g_signal_connect(calDirEntry, "show",
		   G_CALLBACK(on_calDirEntry_show),
		   GUINT_TO_POINTER(calDirChanger));

  g_signal_connect(mainDirOk, "clicked",
		   G_CALLBACK(on_mainDirOk_clicked),
		   GUINT_TO_POINTER(mainDirEntry));

  g_signal_connect(calDirOk, "clicked",
		   G_CALLBACK(on_calDirOk_clicked),
		   GUINT_TO_POINTER(calDirEntry));

  g_signal_connect(mainDirCancel, "clicked", 
		   G_CALLBACK(on_mainDirCancel_clicked),
		   GUINT_TO_POINTER(mainDirEntry));

  g_signal_connect(calDirCancel, "clicked",
		   G_CALLBACK(on_calDirCancel_clicked),
		   GUINT_TO_POINTER(calDirEntry));

  g_signal_connect(mainDirApply, "clicked",
		   G_CALLBACK(on_mainDirApply_clicked),
		   GUINT_TO_POINTER(mainDirEntry));

  g_signal_connect(saveTheWorld, "clicked",
		   G_CALLBACK(on_saveTheWorld_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(continuous, "clicked",
		   G_CALLBACK(on_continuous_clicked),
		   NULL);

  g_signal_connect(showM0Button, "clicked",
		   G_CALLBACK(on_showM0_clicked),
		   GUINT_TO_POINTER(maskWindowL));

  g_signal_connect(showM1Button, "clicked",
		   G_CALLBACK(on_showM1_clicked),
		   GUINT_TO_POINTER(maskWindowR));

  g_signal_connect(saveZoneTuneButton, "clicked",
		   G_CALLBACK(on_saveZoneTune_clicked),
		   NULL);

  g_signal_connect(calDirApply, "clicked",
		   G_CALLBACK(on_calDirApply_clicked),
		   GUINT_TO_POINTER(calDirEntry));

  g_signal_connect(dialog, "clicked",
		   G_CALLBACK(on_dialog_clicked),
		   NULL);

  g_signal_connect(killump, "clicked",
		   G_CALLBACK(on_kill_clicked),
		   NULL);

  g_signal_connect(plot, "clicked",
		   G_CALLBACK(on_plot_clicked),
		   NULL);

  g_signal_connect(store, "clicked",
		   G_CALLBACK(on_store_clicked),
		   NULL);

  g_signal_connect(recalibrateButton,
		   "clicked",
		   G_CALLBACK(on_recalibrate_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(recalibrateButtonL,
		   "clicked",
		   G_CALLBACK(on_recalibrate_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(recalibrateButtonR,
		   "clicked",
		   G_CALLBACK(on_recalibrate_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(slicerawButton,
		   "clicked",
		   G_CALLBACK(on_sliceraw_clicked),
		   GUINT_TO_POINTER(app));

  g_signal_connect(autoButton,
		   "clicked",
		   G_CALLBACK(on_auto_clicked),
		   NULL);

  endEndHand = g_signal_connect(endEndButton,
				"clicked",
				G_CALLBACK(on_endend_clicked),
				GUINT_TO_POINTER(app));

  g_signal_connect(replay, "clicked",
		   G_CALLBACK(on_replay_clicked),
		   NULL);

  g_signal_connect(leftShowStill, "clicked",
		   G_CALLBACK(on_leftShowStill_clicked),
		   NULL);

  g_signal_connect(rightShowStill, "clicked",
		   G_CALLBACK(on_rightShowStill_clicked),
		   NULL);

  g_signal_connect(single, "clicked",
		   G_CALLBACK(on_single_clicked),
		   NULL);

  g_signal_connect(app, "realize",
		   G_CALLBACK(on_app_realize),
		   NULL);

  calLinkHand = g_signal_connect(calLink, "clicked",
				 G_CALLBACK(on_calLink_clicked), NULL);


 // Setup the state machine
  int ist;

  ist = SM.addState("chose working directory");
  SM.addDepend(ist, "chose working directory not home",
	       (function_t)checkPwdNotHome);
  SM.addCallback(ist, (GtkWidget *)guideButton[CHOSE_WORKING_DIRECTORY],
		 "clicked", guideHand[CHOSE_WORKING_DIRECTORY],
		 "CHOSE_WORKING_DIRECTORY");

  ist = SM.addState("chose cal directory");
  SM.addDepend(ist, "chose cal directory not home",
	       (function_t)checkXwdNotHome);
  SM.addCallback(ist, (GtkWidget *)guideButton[CHOSE_CAL_DIRECTORY],
		 "clicked", guideHand[CHOSE_CAL_DIRECTORY],
		 "guideButton[CHOSE_CAL_DIRECTORY]");

  ist = SM.addState("world.dat saved");
  SM.addDepend(ist, "make world.dat file", (function_t)checkWorldFileExists);
  SM.addDependNot(ist, "save world.dat file",  &worldSpinChanged);
  SM.addCallback(ist, (GtkWidget *)guideButton[WORLD_DAT_SAVED],
		 "clicked", guideHand[WORLD_DAT_SAVED],
		 "guideButton[WORLD_DAT_SAVED]");
  
  ist = SM.addState("zone.tune saved");
  SM.addDepend(ist, "make zone.tune file", (function_t)checkTuneFileExists);
  SM.addDependNot(ist, "save world.dat file",  &tuneSpinChanged);
  SM.addCallback(ist, (GtkWidget *)guideButton[GUIDE_TUNE_SAVED],
		 "clicked", guideHand[GUIDE_TUNE_SAVED],
		 "guideButton[GUIDE_TUNE_SAVED]");
  
  ist = SM.addState("left tune saved");
  SM.addDepend(ist, "make video0.tune file",
	       (function_t)checkLeftTuneFileExists);
  SM.addDependNot(ist, "save video0.tune file",  &tunSpinChanged[0]);
  SM.addCallback(ist, (GtkWidget *)guideButton[LEFT_TUNE_SAVED],
		 "clicked", guideHand[LEFT_TUNE_SAVED],
		 "guideButton[LEFT_TUNE_SAVED]");
  
  ist = SM.addState("right tune saved");
  SM.addDepend(ist, "make video1.tune file",
	       (function_t)checkRightTuneFileExists);
  SM.addDependNot(ist, "save video1.tune file",  &tunSpinChanged[1]);
  SM.addCallback(ist, (GtkWidget *)guideButton[RIGHT_TUNE_SAVED],
		 "clicked", guideHand[RIGHT_TUNE_SAVED],
		 "guideButton[RIGHT_TUNE_SAVED]");
  
  ist = SM.addState("left aberration saved");
  SM.addDepend(ist, "make video0.cam file", (function_t)checkLeftCamFileExists);
  SM.addDependNot(ist, "save video0.cam file", &abSpinChanged[0]);
  SM.addCallback(ist, (GtkWidget *)guideButton[LEFT_ABERRATION_SAVED],
		 "clicked", guideHand[LEFT_ABERRATION_SAVED],
		 "guideButton[LEFT_ABERRATION_SAVED]");

  ist = SM.addState("right aberration saved");
  SM.addDepend(ist, "make video1.cam file",(function_t)checkRightCamFileExists);
  SM.addDependNot(ist, "save video1.cam file", &abSpinChanged[1]);
  SM.addCallback(ist, (GtkWidget *)guideButton[RIGHT_ABERRATION_SAVED],
		 "clicked", guideHand[RIGHT_ABERRATION_SAVED],
		 "guideButton[RIGHT_ABERRATION_SAVED]");
  
  function_t checkOldDatFile[2] = {checkOldDatFile0, checkOldDatFile1};
  ist = SM.addState("left pixel data saved");
  SM.addDepend(ist, "make left pixel data file",
	       (function_t)checkLeftDatFileExists);
  SM.addDependNot(ist, "save left pixel data file", &datSpinChanged[0]);
  SM.addDependNot(ist, "old left pixel data file",
		  (function_t)checkOldDatFile[0]);
  SM.addCallback(ist, (GtkWidget *)guideButton[LEFT_PIXEL_DATA_SAVED],
		 "clicked", guideHand[LEFT_PIXEL_DATA_SAVED],
		 "guideButton[LEFT_PIXEL_DATA_SAVED]");

  ist = SM.addState("right pixel data saved");
  SM.addDepend(ist, "make right pixel data file",
	       (function_t)checkRightDatFileExists);
  SM.addDependNot(ist, "save right pixel data file", &datSpinChanged[1]);
  SM.addDependNot(ist, "old right pixel data file",
		  (function_t)checkOldDatFile[1]);
  SM.addCallback(ist, (GtkWidget *)guideButton[RIGHT_PIXEL_DATA_SAVED],
		 "clicked", guideHand[RIGHT_PIXEL_DATA_SAVED],
		 "guideButton[RIGHT_PIXEL_DATA_SAVED]");
  
  ist = SM.addState("cal0.raw");
  SM.addDepend(ist, "make cal0.raw file", checkRawFileExists0);
  
  ist = SM.addState("cal1.raw");
  SM.addDepend(ist, "make cal1.raw file", checkRawFileExists1);

  ist = SM.addState("calibration files exist");
  SM.addDepend(ist, "complete calL.*", checkCalLExists);
  SM.addDepend(ist, "complete calR.*", checkCalRExists);
  
  ist = SM.addState("external calibration run");
  SM.addDepend(ist, "new calibration files", (function_t)checkNewCals);
  SM.addDepend(ist, "complete calL.*", checkCalLExists);
  SM.addDepend(ist, "complete calR.*", checkCalRExists);
  SM.addCallback(ist, (GtkWidget *)guideButton[EXT_CAL_RUN], "clicked",
		 guideHand[EXT_CAL_RUN], "guideButton[EXT_CAL_RUN]");
  
  ist = SM.addState("extCal ready to run");
  SM.addDepend(ist, "complete world.dat",  SM.status("world.dat saved"));
  SM.addDepend(ist, "complete left aberration saved",
	       SM.status("left aberration saved"));
  SM.addDepend(ist, "complete right aberration saved",
	       SM.status("right aberration saved"));
  SM.addDepend(ist, "complete left pixel data",
	       SM.status("left pixel data saved"));
  SM.addDepend(ist, "complete right pixel data",
	       SM.status("right pixel data saved"));

  function_t checkBallLRawExists[3] =
    {checkBallLRawExistsA, checkBallLRawExistsB, checkBallLRawExistsC};
  function_t checkBallRRawExists[3] =
    {checkBallRRawExistsA, checkBallRRawExistsB, checkBallRRawExistsC};
  function_t checkBlankLRawExists[3] =
    {checkBlankLRawExistsA, checkBlankLRawExistsB, checkBlankLRawExistsC};
  function_t checkBlankRRawExists[3] =
    {checkBlankRRawExistsA, checkBlankRRawExistsB, checkBlankRRawExistsC};
  function_t checkOldSnapFile[NUM_M_IMAGES] =
    {checkOldSnapFile0,
     checkOldSnapFile0A, checkOldSnapFile0B, checkOldSnapFile0C, 
     checkOldSnapFile1,
     checkOldSnapFile1A, checkOldSnapFile1B, checkOldSnapFile1C};

  ist = SM.addState("snaps done");
  for ( int i=BALLA; i<NUM_BALLS; i++ )
    {
      char desc[MAX_CANON];
      sprintf(desc, "complete ballL%s", stSnFileName[i]);
      SM.addDepend(ist, desc, checkBallLRawExists[i]);
      sprintf(desc, "complete ballR%s", stSnFileName[i]);
      SM.addDepend(ist, desc, checkBallRRawExists[i]);
      sprintf(desc, "complete blankL%s", stSnFileName[i]);
      SM.addDepend(ist, desc, checkBlankLRawExists[i]);
      sprintf(desc, "complete blankR%s", stSnFileName[i]);
      SM.addDepend(ist, desc, checkBlankRRawExists[i]);
    }
  SM.addCallback(ist, (GtkWidget *)guideButton[SNAPS_DONE], "clicked",
		 guideHand[SNAPS_DONE], "guideButton[SNAPS_DONE]");

  ist = SM.addState("sliceraw run");
  for( int i=LM; i<NUM_M_IMAGES; i++ )
    {
      char desc[MAX_CANON];
      sprintf(desc, "old %s", msSnFileName[i]);
      SM.addDepend(ist, desc, checkOldSnapFile[i]);
    }
  SM.addDepend(ist, "complete calL.*", checkCalLExists);
  SM.addDepend(ist, "complete calR.*", checkCalRExists);
  SM.addCallback(ist, (GtkWidget *)guideButton[SLICERAW_RUN], "clicked",
		 guideHand[SLICERAW_RUN], "guideButton[SLICERAW_RUN]");
  
  ist = SM.addState("video0.cal");
  SM.addDepend(ist, "make video0.cal file",(function_t)checkLeftCalFileExists);
  
  ist = SM.addState("video1.cal");
  SM.addDepend(ist, "make video1.cal file",(function_t)checkRightCalFileExists);
  
  function_t checkMFileExists[NUM_M_IMAGES] =
    {checkMFileExistsLM,
     checkMFileExistsLMA, checkMFileExistsLMB, checkMFileExistsLMC, 
     checkMFileExistsRM,
     checkMFileExistsRMA, checkMFileExistsRMB, checkMFileExistsRMC};

  for ( int i=LM; i<NUM_M_IMAGES; i++ )
    {
      char desc[MAX_CANON];
      sprintf(desc, "%s saved", msFileName[i]);
      ist = SM.addState(desc);
      sprintf(desc, "%s exists", msFileName[i]);
      SM.addDepend(ist, desc, checkMFileExists[i]);
      sprintf(desc, "NOTmaskSpinChanged[%d]", i);
      SM.addDependNot(ist, desc, &maskSpinChanged[i]);
      sprintf(desc, "guideButton[GUIDE_D0_SAVED+%d]", i);
      SM.addCallback(ist, (GtkWidget *)guideButton[GUIDE_D0_SAVED+i],
		 "clicked", guideHand[GUIDE_D0_SAVED+i], desc);
    }


  ist = SM.addState("masks");
  char desc[MAX_CANON];
  sprintf(desc, "%s saved", msFileName[LMA]);
  SM.addDepend(ist, desc, SM.status(desc) );
  sprintf(desc, "%s saved", msFileName[LMB]);
  SM.addDepend(ist, desc, SM.status(desc) );
  sprintf(desc, "%s saved", msFileName[LMC]);
  SM.addDepend(ist, desc, SM.status(desc) );
  sprintf(desc, "%s saved", msFileName[RMA]);
  SM.addDepend(ist, desc, SM.status(desc) );
  sprintf(desc, "%s saved", msFileName[RMB]);
  SM.addDepend(ist, desc, SM.status(desc) );
  sprintf(desc, "%s saved", msFileName[RMC]);
  SM.addDepend(ist, desc, SM.status(desc) );

  ist = SM.addState("end end ready");
  SM.addDepend(ist, "complete external calib",
	       SM.status("external calibration run"));
  SM.addDepend(ist, "complete cal0.raw",   SM.status("cal0.raw"));
  SM.addDepend(ist, "complete cal1.raw",   SM.status("cal1.raw"));
  SM.addDepend(ist, "complete sliceraw run",SM.status("sliceraw run"));
  SM.addDepend(ist, "complete video0.cal", SM.status("video0.cal"));
  SM.addDepend(ist, "complete video1.cal", SM.status("video1.cal"));
  SM.addDepend(ist, "complete masks",      SM.status("masks"));
  SM.addCallback(ist, (GtkWidget *)guideButton[END_END_READY], "clicked",
		 guideHand[END_END_READY], "guideButton[END_END_READY]");

  // Internal still state
  function_t checkIntStillFileExists[NUM_INT_STILL_BUTTONS] =
    {checkIntStillFileExistsL, checkIntStillFileExistsR};
  for ( int i=0; i<NUM_INT_STILL_BUTTONS; i++ )
    {
      char desc[MAX_CANON];
      sprintf(desc, "%s exists", stIntName[i]);
      ist = SM.addState(desc);
      SM.addDepend(ist, desc, checkIntStillFileExists[i]);
      sprintf(desc, "guideButton[STILLLEFT+%d]", i);
      SM.addCallback(ist, (GtkWidget *)guideButton[STILLLEFT+i],
		     "clicked", guideHand[STILLLEFT+i], desc);
    }

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "\nSM=" << SM << flush;
  if ( verbose ) cout << "MESSAGE(Blink) : SM.update()\n"
		      << flush;
#endif

  SM.update();

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "\nAfter initial update SM=" << SM << flush;
#endif

  // Black and White snap state memory
  for ( int i=0; i<NUM_STILL_BUTTONS; i++ )
    {
      blackWhiteDone[i] = 0;
      ist = BLACKWHITE.addState(stName[i]);
      BLACKWHITE.addDepend(ist, stName[i], &blackWhiteDone[i]);
      BLACKWHITE.addCallback(ist, (GtkWidget *)guideButton[WHITEA+i], "clicked",
			     guideHand[WHITEA+i], "guideButton[WHITEA+]");
    }
  ist = BLACKWHITE.addState("All stills done");
  for ( int i=0; i<NUM_STILL_BUTTONS; i++ )
    {
      BLACKWHITE.addDepend(ist, stName[i], &blackWhiteDone[i]);
    }

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "\nBLACKWHITE=" << BLACKWHITE << flush;
  if ( verbose ) cout << "MESSAGE(Blink) : BLACKWHITE.update()\n"
		      << flush;
#endif

  BLACKWHITE.update();

#ifdef VERBOSE
  if ( 1 < verbose ) cout << "\nAfter initial update BLACKWHITE="
			  << BLACKWHITE << flush;
#endif


  // set the initial values
  SM.blockUpdate();
  g_object_set(spCa[NSP],  "value", 320., NULL);
  g_signal_emit_by_name(spCa[NSP], "value-changed");
  g_object_set(spCa[DXSP], "value",   1., NULL);
  g_signal_emit_by_name(spCa[DXSP], "value-changed");
  g_object_set(spCa[DYSP], "value",   1., NULL);
  g_signal_emit_by_name(spCa[DYSP], "value-changed");
  g_object_set(spTune[VERBOSESP], "value", (float)verbose, NULL);
  g_signal_emit_by_name(spTune[VERBOSESP], "value-changed");
  g_object_set(spTune[AUTOSP], "value", (float)autoWait, NULL);
  g_signal_emit_by_name(spTune[AUTOSP], "value-changed");

  TUN[0] = new Tune(0);
  g_object_set(spTune[AGS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[AGS0SP], "value-changed");
  g_object_set(spTune[BGS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[BGS0SP], "value-changed");
  g_object_set(spTune[MBDS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MBDS0SP], "value-changed");
  g_object_set(spTune[MBAS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MBAS0SP], "value-changed");
  g_object_set(spTune[XBAS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[XBAS0SP], "value-changed");
  g_object_set(spTune[MBRS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MBRS0SP], "value-changed");
  g_object_set(spTune[XBRS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[XBRS0SP], "value-changed");
  g_object_set(spTune[MPS0SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MPS0SP], "value-changed");
  tunSpinChanged[0] = 0;

  TUN[1] = new Tune(1);
  g_object_set(spTune[AGS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[AGS1SP], "value-changed");
  g_object_set(spTune[BGS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[BGS1SP], "value-changed");
  g_object_set(spTune[MBDS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MBDS1SP], "value-changed");
  g_object_set(spTune[MBAS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MBAS1SP], "value-changed");
  g_object_set(spTune[XBAS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[XBAS1SP], "value-changed");
  g_object_set(spTune[MBRS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MBRS1SP], "value-changed");
  g_object_set(spTune[XBRS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[XBRS1SP], "value-changed");
  g_object_set(spTune[MPS1SP], "value", 1., NULL);
  g_signal_emit_by_name(spTune[MPS1SP], "value-changed");
  tunSpinChanged[1] = 0;

  // Initialize
  blockQueryRemove = 1;
  g_signal_emit_by_name(app, "realize");
  blockQueryRemove = 0;
  SM.unBlockUpdate();
  SM.update();

  // start GTK+ main event loop 
  gtk_main();

  // zap memory
  for (int i=LM; i < NUM_M_IMAGES; i++)
    {
      if ( mImgFile[i]  ) zaparr(mImgFile[i]);
      if ( stillFile[i] ) zaparr(stillFile[i]);
      if ( snapFile[i]  ) zaparr(snapFile[i]);
      if ( msFile[i]    ) zaparr(msFile[i]);
    }
  if ( calLFile  ) zaparr(calLFile);
  if ( calRFile  ) zaparr(calRFile);
  if ( worldFile ) zaparr(worldFile);
  if ( tuneFile  ) zaparr(tuneFile);

  for (int i=QSVWORLD; i<NUM_QSV_FILES; i++)
    if ( qsvLabel[i] ) zaparr(qsvLabel[i]);
  for (int i=QOWWORLD; i<NUM_QOW_FILES; i++)
    if ( qowLabel[i] ) zaparr(qowLabel[i]);
  for (int i=0; i<2; i++)
    {
      if ( rawFile[i]    ) zaparr(rawFile[i]);
      if ( camFile[i]    ) zaparr(camFile[i]);
      if ( datFile[i]    ) zaparr(datFile[i]);
      if ( tunFile[i]    ) zaparr(tunFile[i]);
      if ( calPath[i]    ) zaparr(calPath[i]);
      if ( calFile[i]    ) zaparr(calFile[i]);
      if ( calFilePWD[i] ) zaparr(calFilePWD[i]);
      if ( device[i]     ) zaparr(device[i]);
      if ( TUN[i]        ) zap(TUN[i]);
      if ( CAL[i]        ) zap(CAL[i]);
      if ( CAM[i]        ) zap(CAM[i]);
      if ( ZON           ) zap(ZON);
    }
  return 0;
}








