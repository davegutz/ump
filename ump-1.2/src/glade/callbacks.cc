// 	$Id: callbacks.cc,v 1.1 2007/12/16 21:11:17 davegutz Exp davegutz $	
// Callbacks for Blink.cc

// Includes section
using namespace std;
#include <common.h>
#include <string>                   // string utilities
#include <iostream>                 // cerr etc
#include <iomanip>                  // setprecision
#include <fstream>                  // ofstream
#include <callbacks.h>              // local
#include <Tune.h>                   // local
//#include <String.h>                 // local
#include <State.h>                  // local
#include <MyMask.h>                 // local
#include <zap.h>                    // local

extern StateMachine    SM, BLACKWHITE;             // Guide & jump
extern GtkNotebook    *notebook1;                  // Main notebook
extern GtkNotebook    *notebookL;                  // Left Camera notebook
extern GtkNotebook    *notebookR;                  // Right Camera notebook
extern GtkWindow      *maskWindowL;                // Mask display windows
extern GtkWindow      *maskWindowR;                // Mask display windows
extern GtkImage      **maskImage[NUM_M_IMAGES];    // Mask image
extern gulong          msImHand[NUM_M_BUTTONS];    // Handlers
extern gulong          msShowMHand[NUM_M_BUTTONS]; // Handlers
extern gulong          msSaveMHand[NUM_M_BUTTONS]; // Handlers
extern gulong          stHand[NUM_STILL_BUTTONS];  // Handlers
extern gint            blackWhiteDone[NUM_STILL_BUTTONS]; // Snap indicators
extern GtkDialog      *qsv[NUM_QSV_FILES];         // Dialog save on quit
extern const gchar    *qsvFileName[];              // Query save file names
extern int             qsvAsked[NUM_QSV_FILES];    // Indicator if already asked
extern GtkDialog      *qow[NUM_QOW_FILES];         // Dialog overwrite
extern gulong          qowHand[NUM_QOW_FILES];     // Handler save on quit
extern const gchar    *qowFileName[];              // Query overwrite file names
extern int             qowAsked[NUM_QOW_FILES];    // Indicator if already asked
extern const gchar    *configFileName;             // config file with pwd name
extern const gchar    *xonfigFileName;             // config file with xwd name
extern const gchar    *calLRawName;                // Device 0, diff balls.raw
extern const gchar    *calRRawName;                // Device 1, diff balls.raw
extern const gchar    *ballLRawName[];             // Device 0, diff balls.raw
extern const gchar    *ballRRawName[];             // Device 1, diff balls.raw
extern const gchar    *blankLRawName[];            // Device 0, diff blanks.raw
extern const gchar    *blankRRawName[];            // Device 1, diff blanks.raw
extern const gchar    *msImFileName[];             // Mask image file
extern const gchar    *stName[];                   // Still file name
extern const gchar    *stIntName[];                // Still file name
extern const gchar    *stImName[];                 // Still image file name
extern const gchar    *msSnFileName[];             // Mask snap file
extern const gchar    *stSnFileName[];             // Still snap file
extern const gchar    *tempFileName[];             // Name of temp file
extern const gchar    *msFileName[];               // Mask file
extern int             msImDev[NUM_M_IMAGES];      // Associate device with mask
extern int             stDev[NUM_STILL_BUTTONS];   // Associate device by still
extern char           *mImgFile[NUM_M_IMAGES];     // Storage for mask image
extern char           *stillFile[2];               // Snapshot sources
extern char           *snapFile[NUM_M_IMAGES];     // Snapshot sources
extern char           *calLFile;                   // Calibration snap sources
extern char           *calRFile;                   // Calibration snap sources
extern int             mImgFileExists[NUM_M_IMAGES]; // Image file exists
extern int             snapFileExists[NUM_M_IMAGES]; // Snap file exists
extern int             mImgFileExistsPWD[NUM_M_IMAGES]; // Image file exists pwd
extern int             snapFileExistsPWD[NUM_M_IMAGES]; // Snap file exists pwd
extern char           *msFile[NUM_M_IMAGES];       // Mask file path
extern char           *worldFile;                  // world file path
extern char           *tuneFile;                   // zone.tune file path
extern GtkButton      *functionShowM[NUM_M_IMAGES];// Show masks
extern GtkButton      *saveMButton[NUM_M_IMAGES];  // Save masks
extern GtkButton      *refRunPar0Button;           // Save and calibrate
extern GtkButton      *refRunPar1Button;           // Save and calibrate
extern GtkButton      *killump;                    // Run kill jobs
extern int             verbose;                    // Global verbosity
extern int             autoWait;                   // Auto mode wait time
extern GtkSpinButton  *spM[NUM_M_BUTTONS];         // Mask spin buttons
extern gulong          spMHand[NUM_M_BUTTONS];     // Mask spin handlers
extern gdouble         spMVa[NUM_M_BUTTONS];       // Mask spin values
extern GtkSpinButton  *spCa[NUM_CA_BUTTONS];       // Calculator spin buttons
extern gulong          spCaHand[NUM_CA_BUTTONS];   // Calculator spin handlers
extern gdouble         spCaVa[NUM_CA_BUTTONS];     // Calculator spin values
extern GtkSpinButton  *spAb[NUM_AB_BUTTONS];       // Aberration spin buttons
extern gulong          spAbHand[NUM_AB_BUTTONS];   // Aberration spin handlers
extern gdouble         spAbVa[NUM_AB_BUTTONS];     // Aberration spin values
extern GtkSpinButton  *spDat[NUM_DAT_BUTTONS];     // Pixel data spin buttons
extern gulong          spDatHand[NUM_DAT_BUTTONS]; // Pixel data spin handlers
extern gdouble         spDatVa[NUM_DAT_BUTTONS];   // Pixel data spin values
extern GtkSpinButton  *spWorld[NUM_WORLD_BUTTONS]; // World spin buttons
extern gulong          spWorldHand[NUM_WORLD_BUTTONS];// World spin handlers
extern gdouble         spWorldVa[NUM_WORLD_BUTTONS];  // World spin values
extern GtkSpinButton  *spTune[NUM_TUNE_BUTTONS];   // Tune spin buttons
extern gulong          spTuneHand[NUM_TUNE_BUTTONS];// Tune spin handlers
extern gdouble         spTuneVa[NUM_TUNE_BUTTONS]; // Tune spin values
extern GtkCheckButton *stills;                     // Stills button
extern gint            still;                      // Stills option
extern GtkCheckButton *calLink;                    // Link button
extern gint            calLinked;                  // Link status
extern gulong          calLinkHand;                // Link callback handle
extern GtkCheckButton *guideButton[NUM_GUIDE_BUTTONS];// Guide button
extern gint           *guideChecked[NUM_GUIDE_BUTTONS];// Guide check button
extern gulong          guideHand[NUM_GUIDE_BUTTONS];// Handlers
extern int             numDev;                     // Nuber cameras used
extern char           *device[2];                  // Camera names
extern char           *camFile[2];                 // Aberration paths
extern char           *datFile[2];                 // Pixel data paths
extern char           *calPath[2];                 // Cal paths
extern char           *calFile[2];                 // Cal file paths
extern char           *calFilePWD[2];              // Cal file paths in pwd
extern char           *tunFile[2];                 // Tune file paths
extern gchar          *pwd;                        // Present Working Directory 
extern gchar          *home;                       // Home directory
extern gchar          *xwd;                        // Calibration Directory 
extern Zone           *ZON;                        // tune information
extern MyCamData      *CAM[2];                     // Calibration properties
extern Tune           *TUN[2];                     // Tuning properties
extern Calibration    *CAL[2];                     // Calibration properties
extern GnomeApp       *app;                        // Main app
extern GnomeVFSURI    *configFileUri;              // main dir uri
extern GnomeVFSURI    *xonfigFileUri;              // cal dir uri
extern guint           permissions;                // File permissions
extern GtkTextBuffer  *mainDirBuffer;              // Text buffers display
extern GtkTextBuffer  *calDirBuffer;               // Text buffers display
extern GtkTextBuffer  *runPar0Buffer;              // Text buffers display
extern GtkTextBuffer  *runPar1Buffer;              // Text buffers display
extern GtkTextBuffer  *linkedDirBuffer[];          // Text buffers display
extern gchar          *runPar0;
extern gchar          *runPar1;
extern char            device_digit[2];            // Camera digit 0=left 1=rt
extern int             worldSpinChanged;           // keep track of world spin
extern int             tuneSpinChanged;            // keep track of tune spin
extern int             maskSpinChanged[NUM_M_IMAGES]; // keep track of mask spin
extern int             abSpinChanged[2];           // keep track aberration spin
extern int             datSpinChanged[2];          // keep track pixel data spin
extern int             tunSpinChanged[2];          // keep track tune
extern gchar          *defMaskImage;
extern int             blockQueryRemove;           // Block during init

// Local globals
int                    snapOR = 0;                 // really crude hack
int                    linkIsWorking[2] = {0, 0};  // If working link
MyMask  *MASK[NUM_M_IMAGES];                       // Calibration properties
const int MAXMASK   = 6; // Maximum number of mask input lines allowed in window
const int NUMFRAMES =25; // Number of still frames each ball
const int FPS       =30; // Assumed frame rate that is used in ump.x.

// Dialog to request save on exit
void qsv_action(GtkDialog *dialog, gint response, gpointer id_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "qsv_action : " << response << endl << flush;
#endif
  QsvId qsvId = (QsvId)((int)id_ptr);
  g_assert(QSVWORLD <= qsvId && NUM_QSV_FILES > qsvId);

  switch ( response )
    {
    case GTK_RESPONSE_OK:
#ifdef VERBOSE
      if ( verbose ) cout << "qsv_action : GTK_RESPONSE_OK\n" << flush;
#endif
      switch ( qsvId )
	{
	case QSVWORLD: saveWorld(); qsvAsked[QSVWORLD] = 0; break;
	case QSVLCAM:  saveAb(0);   qsvAsked[QSVLCAM]  = 0; break;
	case QSVRCAM:  saveAb(1);   qsvAsked[QSVRCAM]  = 0; break;
	case QSVLDAT:  saveDat(0);  qsvAsked[QSVLDAT]  = 0; break;
	case QSVRDAT:  saveDat(1);  qsvAsked[QSVRDAT]  = 0; break;
	case QSVTUNE:  saveTune();  qsvAsked[QSVTUNE]  = 0; break;
	case QSVLTUNE: saveTn(0);   qsvAsked[QSVLTUNE] = 0; break;
	case QSVRTUNE: saveTn(1);   qsvAsked[QSVRTUNE] = 0; break;
	case QSVLDM:
	  saveM(LM);
	  g_signal_emit_by_name(maskImage[LM], "realize");
	  qsvAsked[QSVLDM] = 0;
	  break;
	case QSVRDM:
	  saveM(RM);
	  g_signal_emit_by_name(maskImage[RM], "realize");
	  qsvAsked[QSVRDM] = 0;
	  break;
	default:
	  cerr << "WARNING(qsv_action) : unknown target = " << qsvId 
	       << " to save.\n" << flush;
	  return;
	  break;
	}
      break;
    case GTK_RESPONSE_CANCEL:
    default:
#ifdef VERBOSE
      if ( verbose ) cout << "qsv_action : GTK_RESPONSE_CANCEL\n"
			  << flush;
#endif
      cerr << "WARNING(qsv_action) : did not overwrite "
	   << qsvFileName[qsvId] << endl << flush;
      break;
    }
  return;
}

// Dialog to overwrite
void qow_action(GtkDialog *dialog, gint response, gpointer id_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "qow_action : " << response << endl << flush;
#endif
  QowId qowId = (QowId)((int)id_ptr);
  g_assert(QOWWORLD <= qowId && NUM_QOW_FILES > qowId);

  switch ( response )
    {
    case GTK_RESPONSE_OK:
#ifdef VERBOSE
      if ( verbose ) cout << "qow_action : GTK_RESPONSE_OK\n" << flush;
#endif
      switch ( qowId )
	{
	case QOWWORLD: saveWorld(); qowAsked[QOWWORLD] = 0; break;
	case QOWLCAM:  saveAb(0);   qowAsked[QOWLCAM]  = 0; break;
	case QOWRCAM:  saveAb(1);   qowAsked[QOWRCAM]  = 0; break;
	case QOWLDAT:  saveDat(0);  qowAsked[QOWLDAT]  = 0; break;
	case QOWRDAT:  saveDat(1);  qowAsked[QOWRDAT]  = 0; break;
	case QOWTUNE:  saveTune();  qowAsked[QOWTUNE]  = 0; break;
	case QOWLTUNE: saveTn(0);   qowAsked[QOWLTUNE] = 0; break;
	case QOWRTUNE: saveTn(1);   qowAsked[QOWRTUNE] = 0; break;
	case QOWLDM:
	  saveM(LM);
	  g_signal_emit_by_name(maskImage[LM], "realize");
	  qowAsked[QOWLDM] = 0;
	  break;
	case QOWRDM:
	  saveM(RM);
	  g_signal_emit_by_name(maskImage[RM], "realize");
	  qowAsked[QOWRDM] = 0;
	  break;
	default:
	  cerr << "WARNING(qow_action) : unknown target = " << qowId 
	       << " to save.\n" << flush;
	  return;
	  break;
	}
      break;
    case GTK_RESPONSE_CANCEL:
    default:
#ifdef VERBOSE
      if ( verbose ) cout << "qow_action : GTK_RESPONSE_CANCEL\n"
			  << flush;
#endif
      cerr << "WARNING(qow_action) : did not overwrite "
	   << qowFileName[qowId] << endl << flush;
      break;
    }
  return;
}

// Check ball and blanks
int  checkIntStillFileExistsL()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, stImName[0]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkIntStillFileExistsL : " << result << endl
			  << flush;
#endif
  return result;
}
int  checkIntStillFileExistsR()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, stImName[1]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkIntStillFileExistsR : " << result << endl
			  << flush;
#endif
  return result;
}
int checkBallLRawExists(int ballId)
{
  int result = 0;
  switch(ballId)
    {
    case 0: result = checkBallLRawExistsA(); break;
    case 1: result = checkBallLRawExistsB(); break;
    case 2: result = checkBallLRawExistsC(); break;
    default: break;
    }
  return result;
}
int checkBallRRawExists(int ballId)
{
  int result = 0;
  switch(ballId)
    {
    case 0: result = checkBallRRawExistsA(); break;
    case 1: result = checkBallRRawExistsB(); break;
    case 2: result = checkBallRRawExistsC(); break;
    default: break;
    }
  return result;
}
int checkBlankLRawExists(int ballId)
{
  int result = 0;
  switch(ballId)
    {
    case 0: result = checkBlankLRawExistsA(); break;
    case 1: result = checkBlankLRawExistsB(); break;
    case 2: result = checkBlankLRawExistsC(); break;
    default: break;
    }
  return result;
}
int checkBlankRRawExists(int ballId)
{
  int result = 0;
  switch(ballId)
    {
    case 0: result = checkBlankRRawExistsA(); break;
    case 1: result = checkBlankRRawExistsB(); break;
    case 2: result = checkBlankRRawExistsC(); break;
    default: break;
    }
  return result;
}

// Check masks exist
int  checkMFileExistsLM()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", pwd, msFileName[LM]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsLM : " << result << endl << flush;
#endif
  return result;
}
int  checkMFileExistsLMA()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, msFileName[LMA]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsLMA : " << result << endl
			  << flush;
#endif
  return result;
}
int  checkMFileExistsLMB()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, msFileName[LMB]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsLMB : " << result << endl
			  << flush;
#endif
  return result;
}
int  checkMFileExistsLMC()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, msFileName[LMC]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsLMC : " << result << endl
			  << flush;
#endif
  return result;
}
int  checkMFileExistsRM()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", pwd, msFileName[RM]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsRM : " << result << endl << flush;
#endif
  return result;
}
int  checkMFileExistsRMA()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, msFileName[RMA]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsRMA : " << result << endl
			  << flush;
#endif
  return result;
}
int  checkMFileExistsRMB()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, msFileName[RMB]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsRMB : " << result << endl
			  << flush;
#endif
  return result;
}
int  checkMFileExistsRMC()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, msFileName[RMC]);
  int result = checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkMFileExistsRMC : " << result << endl
			  << flush;
#endif
  return result;
}

// Check old cam file
int checkOldCamFile0()
{
  int result = (compareFileMtime(calLFile, camFile[0]) &&
		compareFileMtime(calRFile, camFile[0])   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldCamFile0 : " << result << endl << flush;
#endif
  return result;
}
int checkOldCamFile1()
{
  int result = (compareFileMtime(calLFile, camFile[1]) &&
		compareFileMtime(calRFile, camFile[1])   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldCamFile1 : " << result << endl << flush;
#endif
  return result;
}

// Check old dat file
int checkOldDatFile0()
{
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldDatFile0 : " << flush;
#endif
  int result = (compareFileMtime(calLFile, datFile[0]) &&
		compareFileMtime(calRFile, datFile[0])   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << result << endl << flush;
#endif
  return result;
}
int checkOldDatFile1()
{
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldDatFile1 : " << flush;
#endif
  int result = (compareFileMtime(calLFile, datFile[1]) &&
		compareFileMtime(calRFile, datFile[1])   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << result << endl << flush;
#endif
  return result;
}

// Check old world file
int checkOldWorldFile()
{
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldWorldFile : " << flush;
#endif
  int result = (compareFileMtime(calLFile, worldFile) &&
		compareFileMtime(calRFile, worldFile)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << result << endl << flush;
#endif
  return result;
}

// Check old snap file
int checkOldSnapFile0()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[LM]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile0 : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile0A()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[LMA]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile0A : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile0B()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[LMB]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile0B : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile0C()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[LMC]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile0C : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile1()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[RM]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile1 : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile1A()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[RMA]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile1A : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile1B()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[RMB]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile1B : " << result << endl << flush;
#endif
  return result;
}
int checkOldSnapFile1C()
{
  char file[MAX_CANON];
  sprintf(file, "%s/%s", xwd, msSnFileName[RMC]);
  int result = (compareFileMtime(calLFile, file) &&
		compareFileMtime(calRFile, file)   );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkOldSnapFile1C : " << result << endl << flush;
#endif
  return result;
}

// Check ball .raw
int checkRawFileExists0()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/cal0.raw", xwd);
  int result = checkFileExists(file); 
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRawFileExists0 : " << result << endl << flush;
#endif
  return result;
}
int checkRawFileExists1()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/cal1.raw", xwd);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRawFileExists1 : " << result << endl << flush;
#endif
  return result;
}

// Check ball .cal
int checkCalLExists()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, calLRawName);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkCalLExists : " << result << endl << flush;
#endif
  return result;
}
int checkCalRExists()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, calRRawName);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkCalRExists : " << result << endl << flush;
#endif
  return result;
}

// Check ball left raw
int checkBallLRawExistsA()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, ballLRawName[0]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBallLRawExistsA : " << result << endl << flush;
#endif
  return result;
}
int checkBallLRawExistsB()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, ballLRawName[1]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBallLRawExistsB : " << result << endl << flush;
#endif
  return result;
}
int checkBallLRawExistsC()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, ballLRawName[2]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBallLRawExistsC : " << result << endl << flush;
#endif
  return result;
}

// Check ball and blank raw
int checkBallRRawExistsA()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, ballRRawName[0]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBallRRawExistsA : " << result << endl << flush;
#endif
  return result;
}
int checkBallRRawExistsB()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, ballRRawName[1]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBallRRawExistsB : " << result << endl << flush;
#endif
  return result;
}
int checkBallRRawExistsC()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, ballRRawName[2]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBallRRawExistsC : " << result << endl << flush;
#endif
  return result;
}
int checkBlankLRawExistsA()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, blankLRawName[0]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBlankLRawExistsA : " << result << endl << flush;
#endif
  return result;
}
int checkBlankLRawExistsB()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, blankLRawName[1]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBlankLRawExistsB : " << result << endl << flush;
#endif
  return result;
}
int checkBlankLRawExistsC()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, blankLRawName[2]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBlankLRawExistsC : " << result << endl << flush;
#endif
  return result;
}
int checkBlankRRawExistsA()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, blankRRawName[0]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBlankRRawExistsA : " << result << endl << flush;
#endif
  return result;
}
int checkBlankRRawExistsB()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, blankRRawName[1]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBlankRRawExistsB : " << result << endl << flush;
#endif
  return result;
}
int checkBlankRRawExistsC()
{
  char file[MAX_CANON];
  sprintf(file,  "%s/%s", xwd, blankRRawName[2]);
  int result =  checkFileExists(file);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkBlankRRawExistsC : " << result << endl << flush;
#endif
  return result;
}

// print a VFS error code, its description, and the problem URI
int print_error(GnomeVFSResult code, const gchar *uri)
{
  const gchar *errorDesc;

  errorDesc = gnome_vfs_result_to_string(code);
  cerr << "ERROR(Blink) : print_error : error " << code
       << " when accessing " << uri << " : " << errorDesc << endl << flush;

  return (code);
}

// check for unsaved files.  Return if all were saved so can quit.
int filesSaved()
{
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "quitting\n" << flush;
#endif
  int quitting = 1;

  // world.dat
  if ( !SM.complete("world.dat saved") && !qsvAsked[QSVWORLD] &&
    worldSpinChanged )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : world.dat not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVWORLD] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVWORLD]));
    }

  // left aberrataion
  if ( !SM.complete("left aberration saved") && !qsvAsked[QSVLCAM] &&
       abSpinChanged[0] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video0.cam not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVLCAM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLCAM]));
    }

  // right aberration
  if ( !SM.complete("right aberration saved") && !qsvAsked[QSVRCAM] &&
       abSpinChanged[1] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video1.cam not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVRCAM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRCAM]));
    }

  // left pixel data
  if ( !SM.complete("left pixel data saved") && !qsvAsked[QSVLDAT] &&
       datSpinChanged[0])
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video0.dat not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVLDAT] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLDAT]));
    }

  // right pixel data
  if ( !SM.complete("right pixel data saved") && !qsvAsked[QSVRDAT] &&
       datSpinChanged[1])
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video1.dat not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVRDAT] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRDAT]));
    }

  // zone.tune
  if ( !SM.complete("zone.tune saved") && !qsvAsked[QSVTUNE] && tuneSpinChanged)
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : zone.tune not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVTUNE] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVTUNE]));
    }

  // left tune
  if ( !SM.complete("left tune saved") && !qsvAsked[QSVLTUNE] &&
       tunSpinChanged[0] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video0.tune not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVLTUNE] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLTUNE]));
    }

  // right tune
  if ( !SM.complete("right tune saved") && !qsvAsked[QSVRTUNE] &&
       tunSpinChanged[1] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video1.tune not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVRTUNE] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRTUNE]));
    }

  // left default mask 
  if ( !SM.complete("video0.mask saved") && !qsvAsked[QSVLDM] &&
       maskSpinChanged[LM] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video0.mask not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVLDM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLDM]));
    }

  // right default mask
  if ( !SM.complete("video1.mask saved") && !qsvAsked[QSVRDM] &&
       maskSpinChanged[RM] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "filesSaved : video1.mask not saved\n" << flush;
#endif
      quitting = 0;
      qsvAsked[QSVRDM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRDM]));
    }


#ifdef VERBOSE
  if ( verbose ) cout << "quitting = " << quitting << endl << flush;
#endif
  return quitting;
}

// standard event handler
gint delete_event(GtkWidget *widget, GdkEvent event, gpointer data)
{
#ifdef VERBOSE
  if ( verbose ) cout << "delete_event\n" << flush;
#endif
  if ( filesSaved() ) return FALSE;
  else return TRUE;
}

// standard event handler
void end_program(GtkWidget *widget, gpointer data)
{
#ifdef VERBOSE
  if ( verbose ) cout << "end_program\n" << flush;
#endif

  // Don't quit if files to be saved
  if ( !filesSaved() ) return;

  // kill rogues
  g_signal_emit_by_name(killump, "clicked");

  // shut down GnomeVFS
  gnome_vfs_shutdown();

  // cleanup memory local globals
  cerr << "cleaning MASK" << endl;
  for(int i=0; i<NUM_M_IMAGES; i++) if ( MASK[i] ) zap(MASK[i]);

  gtk_main_quit();
}

// Replay ump program files in pwd
void on_replay_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_replay_clicked\n" << flush;
#endif
  gchar  *cmd;

  // Check for calibration done
  if ( !SM.complete("external calibration run") )
  {
    showErr("You must run external calibration \'calibrate\' before run");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
    return;
  }

  g_object_get(stills, "active", &still, NULL);
  if ( still )
    cmd = g_strdup_printf("ump.x -S -R");
  else
    cmd = g_strdup_printf(">temp ump -ixA.raw -ixB.raw -v%d", verbose);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_replay_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;
}

// Replay ump program files in pwd
void on_leftShowStill_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_leftShowStill_clicked\n" << flush;
#endif
  gchar  *cmd;

  // Check for calibration done
  char desc[MAX_CANON];
  sprintf(desc, "%s exists", stIntName[0]);
  if ( !SM.complete(desc) )
  {
    showErr("You must run \'Left Still\' before \'Show\'");
    return;
  }
  cmd = g_strdup_printf("gimp %s", stillFile[0]);
  if ( 0 != Execute(cmd, 1) )
    cerr << "ERROR(Blink) :  on_leftShowStill_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;
}

void on_rightShowStill_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_rightShowStill_clicked\n" << flush;
#endif
  gchar  *cmd;

  // Check for calibration done
  char desc[MAX_CANON];
  sprintf(desc, "%s exists", stIntName[1]);
  if ( !SM.complete(desc) )
  {
    showErr("You must run \'Right Still\' before \'Show\'");
    return;
  }
  cmd = g_strdup_printf("gimp %s", stillFile[1]);
  if ( 0 != Execute(cmd, 1) )
    cerr << "ERROR(Blink) :  on_rightShowStill_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;
}

// Record ump files in pwd
void on_single_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_single_clicked\n" << flush;
#endif
  gchar  *cmd;
  int nCal = CAM[0]->imWidthCalEx();
  int mCal = CAM[0]->imHeightCalEx();
  int sub  = 1;

  // Check for calibration done
  if ( !SM.complete("external calibration run") )
  {
    showErr("You must run external calibration \'calibrate\' before run");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
    return;
  }

  // Take picture
  g_object_get(stills, "active", &still, NULL);
  if ( still )
    cmd = g_strdup_printf("Ump -- -S -s%dx%d -u%d", nCal, mCal, sub);
  else
    cmd = g_strdup_printf("Ump --    -s%dx%d -u%d", nCal, mCal, sub);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_single_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;
}

// Display the plots from previous ump file run
void on_plot_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_plot_clicked\n" << flush;
#endif
  gchar *cmd;
  cmd = g_strdup_printf("showppms");
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_plot_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;

}

// Kill errant processes
void on_kill_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_kill_clicked\n" << flush;
#endif
  gchar *cmd;
  cmd = g_strdup_printf("killUmp");
  if ( 0 > Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_kill_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;

}

// Save the plots from previous ump file run in a unique directory
void on_store_clicked(GtkButton *button, gpointer entry_ptr)
{
  gchar *cmd;
#ifdef VERBOSE
  if ( verbose ) cout << "on_store_clicked\n" << flush;;
#endif
  cmd = g_strdup_printf("saveppms");
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_store_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;

}

// Run ump in a loop, saving at each run
void on_auto_clicked(GtkButton *button, gpointer entry_ptr)
{
  gchar *cmd;
  int nCal = CAM[0]->imWidthCalEx();
  int mCal = CAM[0]->imHeightCalEx();
#ifdef VERBOSE
  if ( verbose ) cout << "on_auto_clicked\n" << flush;;
#endif
  cmd = g_strdup_printf("Ump -- -A %d -s %dx%d -- -v%d", autoWait, nCal, mCal,
			verbose);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_auto_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;

}

// Display the temp file text output of ump file run
void on_dialog_clicked(GtkButton *button, gpointer entry_ptr)
{
  gchar *cmd;
#ifdef VERBOSE
  if ( verbose ) cout << "on_dialog_clicked\n" << flush;
#endif
  cmd = g_strdup_printf("gedit temp");
  if ( 0 != Execute(cmd, 1) )
    cerr << "ERROR(Blink) :  on_dialog_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;
}

// print a VFS error code, its description, and the problem URI
int printError(GnomeVFSResult code, const gchar *uri)
{
  const gchar *errorDesc;
  errorDesc = gnome_vfs_result_to_string(code);
  cerr << "ERROR(Blink) : printError : error " << code
       << " when accessing " << uri << " : " << errorDesc << endl << flush;
  return (code);
}

// Mask spin button calculations
void spM_changed(GtkSpinButton *spin_button, gpointer id_ptr)
{
  SpMId spMId = (SpMId)((int)id_ptr);
  g_assert(SXD0SP <= spMId && NUM_M_BUTTONS > spMId);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "spM_changed[" << spMId << "]\n" <<flush;
#endif

  // update the check box
  int maskNum = spMId*NUM_M_IMAGES/NUM_M_BUTTONS;
  maskSpinChanged[maskNum] = 1;
  char desc[MAX_CANON];
  sprintf(desc, "%s saved", msFileName[maskNum]);
  SM.update(desc);

  // read the input from the spin button 
  g_object_get(spM[spMId], "value", &spMVa[spMId], NULL);

  // determine the inter-relationships
  switch (spMId)
    {
    case DDXD0SP:  if ( 0.0 != spMVa[DDXD0SP] )  spMVa[DDYD0SP]  = 0.0; break;
    case DDYD0SP:  if ( 0.0 != spMVa[DDYD0SP] )  spMVa[DDXD0SP]  = 0.0; break;
    case DDX10SP:  if ( 0.0 != spMVa[DDX10SP] )  spMVa[DDY10SP]  = 0.0; break;
    case DDY10SP:  if ( 0.0 != spMVa[DDY10SP] )  spMVa[DDX10SP]  = 0.0; break;
    case DDX20SP:  if ( 0.0 != spMVa[DDX20SP] )  spMVa[DDY20SP]  = 0.0; break;
    case DDY20SP:  if ( 0.0 != spMVa[DDY20SP] )  spMVa[DDX20SP]  = 0.0; break;
    case DDX30SP:  if ( 0.0 != spMVa[DDX30SP] )  spMVa[DDY30SP]  = 0.0; break;
    case DDY30SP:  if ( 0.0 != spMVa[DDY30SP] )  spMVa[DDX30SP]  = 0.0; break;
    case DDX40SP:  if ( 0.0 != spMVa[DDX40SP] )  spMVa[DDY40SP]  = 0.0; break;
    case DDY40SP:  if ( 0.0 != spMVa[DDY40SP] )  spMVa[DDX40SP]  = 0.0; break;
    case DDX50SP:  if ( 0.0 != spMVa[DDX50SP] )  spMVa[DDY50SP]  = 0.0; break;
    case DDY50SP:  if ( 0.0 != spMVa[DDY50SP] )  spMVa[DDX50SP]  = 0.0; break;
    case DDXD0ASP: if ( 0.0 != spMVa[DDXD0ASP] ) spMVa[DDYD0ASP] = 0.0; break;
    case DDYD0ASP: if ( 0.0 != spMVa[DDYD0ASP] ) spMVa[DDXD0ASP] = 0.0; break;
    case DDX10ASP: if ( 0.0 != spMVa[DDX10ASP] ) spMVa[DDY10ASP] = 0.0; break;
    case DDY10ASP: if ( 0.0 != spMVa[DDY10ASP] ) spMVa[DDX10ASP] = 0.0; break;
    case DDX20ASP: if ( 0.0 != spMVa[DDX20ASP] ) spMVa[DDY20ASP] = 0.0; break;
    case DDY20ASP: if ( 0.0 != spMVa[DDY20ASP] ) spMVa[DDX20ASP] = 0.0; break;
    case DDX30ASP: if ( 0.0 != spMVa[DDX30ASP] ) spMVa[DDY30ASP] = 0.0; break;
    case DDY30ASP: if ( 0.0 != spMVa[DDY30ASP] ) spMVa[DDX30ASP] = 0.0; break;
    case DDX40ASP: if ( 0.0 != spMVa[DDX40ASP] ) spMVa[DDY40ASP] = 0.0; break;
    case DDY40ASP: if ( 0.0 != spMVa[DDY40ASP] ) spMVa[DDX40ASP] = 0.0; break;
    case DDX50ASP: if ( 0.0 != spMVa[DDX50ASP] ) spMVa[DDY50ASP] = 0.0; break;
    case DDY50ASP: if ( 0.0 != spMVa[DDY50ASP] ) spMVa[DDX50ASP] = 0.0; break;
    case DDXD0BSP: if ( 0.0 != spMVa[DDXD0BSP] ) spMVa[DDYD0BSP] = 0.0; break;
    case DDYD0BSP: if ( 0.0 != spMVa[DDYD0BSP] ) spMVa[DDXD0BSP] = 0.0; break;
    case DDX10BSP: if ( 0.0 != spMVa[DDX10BSP] ) spMVa[DDY10BSP] = 0.0; break;
    case DDY10BSP: if ( 0.0 != spMVa[DDY10BSP] ) spMVa[DDX10BSP] = 0.0; break;
    case DDX20BSP: if ( 0.0 != spMVa[DDX20BSP] ) spMVa[DDY20BSP] = 0.0; break;
    case DDY20BSP: if ( 0.0 != spMVa[DDY20BSP] ) spMVa[DDX20BSP] = 0.0; break;
    case DDX30BSP: if ( 0.0 != spMVa[DDX30BSP] ) spMVa[DDY30BSP] = 0.0; break;
    case DDY30BSP: if ( 0.0 != spMVa[DDY30BSP] ) spMVa[DDX30BSP] = 0.0; break;
    case DDX40BSP: if ( 0.0 != spMVa[DDX40BSP] ) spMVa[DDY40BSP] = 0.0; break;
    case DDY40BSP: if ( 0.0 != spMVa[DDY40BSP] ) spMVa[DDX40BSP] = 0.0; break;
    case DDX50BSP: if ( 0.0 != spMVa[DDX50BSP] ) spMVa[DDY50BSP] = 0.0; break;
    case DDY50BSP: if ( 0.0 != spMVa[DDY50BSP] ) spMVa[DDX50BSP] = 0.0; break;
    case DDXD0CSP: if ( 0.0 != spMVa[DDXD0CSP] ) spMVa[DDYD0CSP] = 0.0; break;
    case DDYD0CSP: if ( 0.0 != spMVa[DDYD0CSP] ) spMVa[DDXD0CSP] = 0.0; break;
    case DDX10CSP: if ( 0.0 != spMVa[DDX10CSP] ) spMVa[DDY10CSP] = 0.0; break;
    case DDY10CSP: if ( 0.0 != spMVa[DDY10CSP] ) spMVa[DDX10CSP] = 0.0; break;
    case DDX20CSP: if ( 0.0 != spMVa[DDX20CSP] ) spMVa[DDY20CSP] = 0.0; break;
    case DDY20CSP: if ( 0.0 != spMVa[DDY20CSP] ) spMVa[DDX20CSP] = 0.0; break;
    case DDX30CSP: if ( 0.0 != spMVa[DDX30CSP] ) spMVa[DDY30CSP] = 0.0; break;
    case DDY30CSP: if ( 0.0 != spMVa[DDY30CSP] ) spMVa[DDX30CSP] = 0.0; break;
    case DDX40CSP: if ( 0.0 != spMVa[DDX40CSP] ) spMVa[DDY40CSP] = 0.0; break;
    case DDY40CSP: if ( 0.0 != spMVa[DDY40CSP] ) spMVa[DDX40CSP] = 0.0; break;
    case DDX50CSP: if ( 0.0 != spMVa[DDX50CSP] ) spMVa[DDY50CSP] = 0.0; break;
    case DDY50CSP: if ( 0.0 != spMVa[DDY50CSP] ) spMVa[DDX50CSP] = 0.0; break;
    case DDXD1SP:  if ( 0.0 != spMVa[DDXD1SP] )  spMVa[DDYD1SP]  = 0.0; break;
    case DDYD1SP:  if ( 0.0 != spMVa[DDYD1SP] )  spMVa[DDXD1SP]  = 0.0; break;
    case DDX11SP:  if ( 0.0 != spMVa[DDX11SP] )  spMVa[DDY11SP]  = 0.0; break;
    case DDY11SP:  if ( 0.0 != spMVa[DDY11SP] )  spMVa[DDX11SP]  = 0.0; break;
    case DDX21SP:  if ( 0.0 != spMVa[DDX21SP] )  spMVa[DDY21SP]  = 0.0; break;
    case DDY21SP:  if ( 0.0 != spMVa[DDY21SP] )  spMVa[DDX21SP]  = 0.0; break;
    case DDX31SP:  if ( 0.0 != spMVa[DDX31SP] )  spMVa[DDY31SP]  = 0.0; break;
    case DDY31SP:  if ( 0.0 != spMVa[DDY31SP] )  spMVa[DDX31SP]  = 0.0; break;
    case DDX41SP:  if ( 0.0 != spMVa[DDX41SP] )  spMVa[DDY41SP]  = 0.0; break;
    case DDY41SP:  if ( 0.0 != spMVa[DDY41SP] )  spMVa[DDX41SP]  = 0.0; break;
    case DDX51SP:  if ( 0.0 != spMVa[DDX51SP] )  spMVa[DDY51SP]  = 0.0; break;
    case DDY51SP:  if ( 0.0 != spMVa[DDY51SP] )  spMVa[DDX51SP]  = 0.0; break;
    case DDXD1ASP: if ( 0.0 != spMVa[DDXD1ASP] ) spMVa[DDYD1ASP] = 0.0; break;
    case DDYD1ASP: if ( 0.0 != spMVa[DDYD1ASP] ) spMVa[DDXD1ASP] = 0.0; break;
    case DDX11ASP: if ( 0.0 != spMVa[DDX11ASP] ) spMVa[DDY11ASP] = 0.0; break;
    case DDY11ASP: if ( 0.0 != spMVa[DDY11ASP] ) spMVa[DDX11ASP] = 0.0; break;
    case DDX21ASP: if ( 0.0 != spMVa[DDX21ASP] ) spMVa[DDY21ASP] = 0.0; break;
    case DDY21ASP: if ( 0.0 != spMVa[DDY21ASP] ) spMVa[DDX21ASP] = 0.0; break;
    case DDX31ASP: if ( 0.0 != spMVa[DDX31ASP] ) spMVa[DDY31ASP] = 0.0; break;
    case DDY31ASP: if ( 0.0 != spMVa[DDY31ASP] ) spMVa[DDX31ASP] = 0.0; break;
    case DDX41ASP: if ( 0.0 != spMVa[DDX41ASP] ) spMVa[DDY41ASP] = 0.0; break;
    case DDY41ASP: if ( 0.0 != spMVa[DDY41ASP] ) spMVa[DDX41ASP] = 0.0; break;
    case DDX51ASP: if ( 0.0 != spMVa[DDX51ASP] ) spMVa[DDY51ASP] = 0.0; break;
    case DDY51ASP: if ( 0.0 != spMVa[DDY51ASP] ) spMVa[DDX51ASP] = 0.0; break;
    case DDXD1BSP: if ( 0.0 != spMVa[DDXD1BSP] ) spMVa[DDYD1BSP] = 0.0; break;
    case DDYD1BSP: if ( 0.0 != spMVa[DDYD1BSP] ) spMVa[DDXD1BSP] = 0.0; break;
    case DDX11BSP: if ( 0.0 != spMVa[DDX11BSP] ) spMVa[DDY11BSP] = 0.0; break;
    case DDY11BSP: if ( 0.0 != spMVa[DDY11BSP] ) spMVa[DDX11BSP] = 0.0; break;
    case DDX21BSP: if ( 0.0 != spMVa[DDX21BSP] ) spMVa[DDY21BSP] = 0.0; break;
    case DDY21BSP: if ( 0.0 != spMVa[DDY21BSP] ) spMVa[DDX21BSP] = 0.0; break;
    case DDX31BSP: if ( 0.0 != spMVa[DDX31BSP] ) spMVa[DDY31BSP] = 0.0; break;
    case DDY31BSP: if ( 0.0 != spMVa[DDY31BSP] ) spMVa[DDX31BSP] = 0.0; break;
    case DDX41BSP: if ( 0.0 != spMVa[DDX41BSP] ) spMVa[DDY41BSP] = 0.0; break;
    case DDY41BSP: if ( 0.0 != spMVa[DDY41BSP] ) spMVa[DDX41BSP] = 0.0; break;
    case DDX51BSP: if ( 0.0 != spMVa[DDX51BSP] ) spMVa[DDY51BSP] = 0.0; break;
    case DDY51BSP: if ( 0.0 != spMVa[DDY51BSP] ) spMVa[DDX51BSP] = 0.0; break;
    case DDXD1CSP: if ( 0.0 != spMVa[DDXD1CSP] ) spMVa[DDYD1CSP] = 0.0; break;
    case DDYD1CSP: if ( 0.0 != spMVa[DDYD1CSP] ) spMVa[DDXD1CSP] = 0.0; break;
    case DDX11CSP: if ( 0.0 != spMVa[DDX11CSP] ) spMVa[DDY11CSP] = 0.0; break;
    case DDY11CSP: if ( 0.0 != spMVa[DDY11CSP] ) spMVa[DDX11CSP] = 0.0; break;
    case DDX21CSP: if ( 0.0 != spMVa[DDX21CSP] ) spMVa[DDY21CSP] = 0.0; break;
    case DDY21CSP: if ( 0.0 != spMVa[DDY21CSP] ) spMVa[DDX21CSP] = 0.0; break;
    case DDX31CSP: if ( 0.0 != spMVa[DDX31CSP] ) spMVa[DDY31CSP] = 0.0; break;
    case DDY31CSP: if ( 0.0 != spMVa[DDY31CSP] ) spMVa[DDX31CSP] = 0.0; break;
    case DDX41CSP: if ( 0.0 != spMVa[DDX41CSP] ) spMVa[DDY41CSP] = 0.0; break;
    case DDY41CSP: if ( 0.0 != spMVa[DDY41CSP] ) spMVa[DDX41CSP] = 0.0; break;
    case DDX51CSP: if ( 0.0 != spMVa[DDX51CSP] ) spMVa[DDY51CSP] = 0.0; break;
    case DDY51CSP: if ( 0.0 != spMVa[DDY51CSP] ) spMVa[DDX51CSP] = 0.0; break;
    default: break;
    }

  // write the new values back into the spin buttons;
  // disable signal handlers when doing this so that this
  // handler function doesn't get called again and again and.. 
  for( int i=SXD0SP; i<NUM_M_BUTTONS; i++)
    {
      if ( spMId != i )
	{
	  g_signal_handler_block(spM[i], spMHand[i]);
	  g_object_set(spM[i], "value", spMVa[i], NULL);
	  g_signal_handler_unblock(spM[i], spMHand[i]);
	}
    }
}

// Calculator spin button calculations
void spCa_changed(GtkSpinButton *spin_button, gpointer id_ptr)
{
  SpCaId spCaId = (SpCaId)((int)id_ptr);
  g_assert(NSP <= spCaId && NUM_CA_BUTTONS > spCaId);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "on_spCa_changed[" << spCaId << "]\n" <<flush;
#endif

  // read the input from the spin button 
  g_object_get(spCa[spCaId], "value", &spCaVa[spCaId], NULL);

  // determine the relationships
  switch (spCaId)
    {
    case NSP:
      spCaVa[MSP]  = spCaVa[NSP]  * 3./4.;
      spCaVa[X0SP] = spCaVa[NSP]  * spCaVa[SXSP];
      spCaVa[Y0SP] = spCaVa[MSP]  * spCaVa[SYSP];
      spCaVa[DNSP] = spCaVa[NSP]  * spCaVa[DXSP];
      spCaVa[DMSP] = spCaVa[MSP]  * spCaVa[DYSP];
      spCaVa[X1SP] = spCaVa[X0SP] + spCaVa[DNSP];
      spCaVa[Y1SP] = spCaVa[Y0SP] + spCaVa[DMSP];
      break;
    case MSP:
      spCaVa[NSP]  = spCaVa[MSP]  * 4./3.;
      spCaVa[X0SP] = spCaVa[NSP]  * spCaVa[SXSP];
      spCaVa[Y0SP] = spCaVa[MSP]  * spCaVa[SYSP];
      spCaVa[DNSP] = spCaVa[NSP]  * spCaVa[DXSP];
      spCaVa[DMSP] = spCaVa[MSP]  * spCaVa[DYSP];
      spCaVa[X1SP] = spCaVa[X0SP] + spCaVa[DNSP];
      spCaVa[Y1SP] = spCaVa[Y0SP] + spCaVa[DMSP];
      break;
    case SXSP:
      spCaVa[X0SP] = spCaVa[NSP]  * spCaVa[SXSP];
      spCaVa[X1SP] = MIN(spCaVa[X0SP] + spCaVa[DNSP], spCaVa[NSP]);
      spCaVa[DNSP] = spCaVa[X1SP] - spCaVa[X0SP];
      spCaVa[DXSP] = spCaVa[DNSP] / spCaVa[NSP];
      break;
    case SYSP:
      spCaVa[Y0SP] = spCaVa[MSP]  * spCaVa[SYSP];
      spCaVa[Y1SP] = MIN(spCaVa[Y0SP] + spCaVa[DMSP], spCaVa[MSP]);
      spCaVa[DMSP] = spCaVa[Y1SP] - spCaVa[Y0SP];
      spCaVa[DYSP] = spCaVa[DMSP] / spCaVa[MSP];
      break;
    case DXSP:
      spCaVa[DNSP] = spCaVa[NSP]  * spCaVa[DXSP];
      spCaVa[X1SP] = MIN(spCaVa[X0SP] + spCaVa[DNSP], spCaVa[NSP]);
      spCaVa[DNSP] = spCaVa[X1SP] - spCaVa[X0SP];
      spCaVa[DXSP] = spCaVa[DNSP] / spCaVa[NSP];
      break;  
    case DYSP:
      spCaVa[DMSP] = spCaVa[MSP]  * spCaVa[DYSP];
      spCaVa[Y1SP] = MIN(spCaVa[Y0SP] + spCaVa[DMSP], spCaVa[MSP]);
      spCaVa[DMSP] = spCaVa[Y1SP] - spCaVa[Y0SP];
      spCaVa[DYSP] = spCaVa[DMSP] / spCaVa[MSP];
      break;
    case X0SP:
      spCaVa[SXSP] = spCaVa[X0SP] / spCaVa[NSP];
      spCaVa[X1SP] = MIN(spCaVa[X0SP] + spCaVa[DNSP], spCaVa[NSP]);
      spCaVa[DNSP] = spCaVa[X1SP]-spCaVa[X0SP];
      spCaVa[DXSP] = spCaVa[DNSP] /spCaVa[NSP];
      break;
    case X1SP:
      spCaVa[DNSP] = spCaVa[X1SP] - spCaVa[X0SP];
      spCaVa[DXSP] = spCaVa[DNSP] / spCaVa[NSP];
      break;
    case Y0SP:
      spCaVa[SYSP] = spCaVa[Y0SP] / spCaVa[MSP];
      spCaVa[Y1SP] = MIN(spCaVa[Y0SP] + spCaVa[DMSP], spCaVa[MSP]);
      spCaVa[DMSP] = spCaVa[Y1SP] - spCaVa[Y0SP];
      spCaVa[DYSP] = spCaVa[DMSP] / spCaVa[MSP];
      break;
    case Y1SP:
      spCaVa[DMSP] = spCaVa[Y1SP] - spCaVa[Y0SP];
      spCaVa[DYSP] = spCaVa[DMSP] / spCaVa[MSP];
      break;
    case DNSP:
      spCaVa[X1SP] = MIN(spCaVa[X0SP] + spCaVa[DNSP], spCaVa[NSP]);
      spCaVa[DNSP] = spCaVa[X1SP] - spCaVa[X0SP];
      spCaVa[DXSP] = spCaVa[DNSP] / spCaVa[NSP];
      break;
    case DMSP:
      spCaVa[Y1SP] = MIN(spCaVa[Y0SP] + spCaVa[DMSP], spCaVa[MSP]);
      spCaVa[DMSP] = spCaVa[Y1SP] - spCaVa[Y0SP];
      spCaVa[DYSP] = spCaVa[DMSP] / spCaVa[MSP];
      break;
    default:
      break;
    }

  // write the new values back into the spin buttons;
  // disable signal handlers when doing this so that this
  // handler function doesn't get called again and again and.. 
  for( int i=NSP; i<NUM_CA_BUTTONS; i++)
    {
      if ( spCaId != i )
	{
	  g_signal_handler_block(spCa[i], spCaHand[i]);
	  g_object_set(spCa[i], "value", spCaVa[i], NULL);
	  g_signal_handler_unblock(spCa[i], spCaHand[i]);
	}
    }
}

// Aberration setting spin button calculations
void spAb_changed(GtkSpinButton *spin_button, gpointer id_ptr)
{
  SpAbId spAbId = (SpAbId)((int)id_ptr);
  g_assert(IMW0SP <= spAbId && NUM_AB_BUTTONS > spAbId);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "spAb_changed[" << spAbId << "]\n" <<flush;
#endif

  // update the check box
  int devNum = spAbId*2/NUM_AB_BUTTONS;
  abSpinChanged[devNum] = 1;
  SM.update();

  // read the input from the spin button 
  g_object_get(spAb[spAbId], "value", &spAbVa[spAbId], NULL);

  // determine the relationships
  switch (spAbId)
    {
    case IMW0SP:
      CAM[0]->imWidthCalEx((int)spAbVa[spAbId]);
      g_object_set(spCa[NSP],  "value", (float)CAM[0]->imWidthCalEx(), NULL);
      g_signal_emit_by_name(spCa[NSP], "value-changed");
      break;
    case IMH0SP: CAM[0]->imHeightCalEx((int)spAbVa[spAbId]); break;
    case FOC0SP: CAM[0]->focalCalEx(spAbVa[spAbId]);         break;
    case ABC0SP: CAM[0]->AbConstant(spAbVa[spAbId]);         break;
    case ABR0SP: CAM[0]->Abrqw2(spAbVa[spAbId]);             break;
    case ABI0SP: CAM[0]->AbipimR(spAbVa[spAbId]);            break;
    case ABJ0SP: CAM[0]->AbjpjmR(spAbVa[spAbId]);            break;
    case ARC0SP: CAM[0]->ARC(spAbVa[spAbId]);                break;
    case SY0SP:  CAM[0]->SY(spAbVa[spAbId]);                 break;
    case IMW1SP:
      CAM[1]->imWidthCalEx((int)spAbVa[spAbId]);
      g_object_set(spCa[NSP],  "value", (float)CAM[1]->imWidthCalEx(), NULL);
      g_signal_emit_by_name(spCa[NSP], "value-changed");
      break;
    case IMH1SP: CAM[1]->imHeightCalEx((int)spAbVa[spAbId]); break;
    case FOC1SP: CAM[1]->focalCalEx(spAbVa[spAbId]);         break;
    case ABC1SP: CAM[1]->AbConstant(spAbVa[spAbId]);         break;
    case ABR1SP: CAM[1]->Abrqw2(spAbVa[spAbId]);             break;
    case ABI1SP: CAM[1]->AbipimR(spAbVa[spAbId]);            break;
    case ABJ1SP: CAM[1]->AbjpjmR(spAbVa[spAbId]);            break;
    case ARC1SP: CAM[1]->ARC(spAbVa[spAbId]);                break;
    case SY1SP:  CAM[1]->SY(spAbVa[spAbId]);                 break;
    default: break;
    }

  // write the new values back into the spin buttons;
  // disable signal handlers when doing this so that this
  // handler function doesn't get called again and again and.. 
  for( int i=IMW0SP; i<NUM_AB_BUTTONS; i++)
    {
      if ( spAbId != i )
	{
	  g_signal_handler_block(spAb[i], spAbHand[i]);
	  g_object_set(spAb[i], "value", spAbVa[i], NULL);
	  g_signal_handler_unblock(spAb[i], spAbHand[i]);
	}
    }
}

// Calibration ball data setting spin button calculations
void spDat_changed(GtkSpinButton *spin_button, gpointer id_ptr)
{
  SpDatId spDatId = (SpDatId)((int)id_ptr);
  g_assert(OAP00SP <= spDatId && NUM_DAT_BUTTONS > spDatId);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "on_spDat_changed[" << spDatId << "]\n" <<flush;
#endif

  // update the check box
  int devNum = spDatId*2/NUM_DAT_BUTTONS;
  datSpinChanged[devNum] = 1;
  SM.update();

  // read the input from the spin button 
  g_object_get(spDat[spDatId], "value", &spDatVa[spDatId], NULL);

  // determine the relationships
  switch (spDatId)
    {
    case OAP00SP:
      CAM[0]->OAp(spDatVa[spDatId], CAM[0]->OAP()[1], CAM[0]->focalCalEx());
      break;
    case OAP10SP:
      CAM[0]->OAp(CAM[0]->OAP()[0], spDatVa[spDatId], CAM[0]->focalCalEx());
      break;
    case OBP00SP:
      CAM[0]->OBp(spDatVa[spDatId], CAM[0]->OBP()[1], CAM[0]->focalCalEx());
      break;
    case OBP10SP:
      CAM[0]->OBp(CAM[0]->OBP()[0], spDatVa[spDatId], CAM[0]->focalCalEx());
      break;
    case OCP00SP:
      CAM[0]->OCp(spDatVa[spDatId], CAM[0]->OCP()[1], CAM[0]->focalCalEx());
      break;
    case OCP10SP:
      CAM[0]->OCp(CAM[0]->OCP()[0], spDatVa[spDatId], CAM[0]->focalCalEx());
      break;
    case OAP01SP:
      CAM[1]->OAp(spDatVa[spDatId], CAM[1]->OAP()[1], CAM[1]->focalCalEx());
      break;
    case OAP11SP:
      CAM[1]->OAp(CAM[1]->OAP()[0], spDatVa[spDatId], CAM[1]->focalCalEx());
      break;
    case OBP01SP:
      CAM[1]->OBp(spDatVa[spDatId], CAM[1]->OBP()[1], CAM[1]->focalCalEx());
      break;
    case OBP11SP:
      CAM[1]->OBp(CAM[1]->OBP()[0], spDatVa[spDatId], CAM[1]->focalCalEx());
      break;
    case OCP01SP:
      CAM[1]->OCp(spDatVa[spDatId], CAM[1]->OCP()[1], CAM[1]->focalCalEx());
      break;
    case OCP11SP:
      CAM[1]->OCp(CAM[1]->OCP()[0], spDatVa[spDatId], CAM[1]->focalCalEx());
      break;
    default:
        break;
    }

  // write the new values back into the spin buttons;
  // disable signal handlers when doing this so that this
  // handler function doesn't get called again and again and.. 
  for( int i=OAP00SP; i<NUM_DAT_BUTTONS; i++)
    {
      if ( spDatId != i )
	{
	  g_signal_handler_block(spDat[i], spDatHand[i]);
	  g_object_set(spDat[i], "value", spDatVa[i], NULL);
	  g_signal_handler_unblock(spDat[i], spDatHand[i]);
	}
    }
}

// Calibration ball data setting spin button calculations
void spWorld_changed(GtkSpinButton *spin_button, gpointer id_ptr)
{
  SpWorldId spWorldId = (SpWorldId)((int)id_ptr);
  g_assert(PLXSP <= spWorldId && NUM_WORLD_BUTTONS > spWorldId);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "on_spWorld_changed["<< spWorldId << "]\n" <<flush;
#endif

  // keep track if spin button changes not saved
  worldSpinChanged = 1;
  SM.update();
  
  // read the input from the spin button 
  g_object_get(spWorld[spWorldId], "value", &spWorldVa[spWorldId], NULL);
}

// Zone tuning spin button calculations
void spTune_changed(GtkSpinButton *spin_button, gpointer id_ptr)
{
  SpTuneId spTuneId = (SpTuneId)((int)id_ptr);
  g_assert(BALLDIASP <= spTuneId && NUM_TUNE_BUTTONS > spTuneId);
  int verboseP  = verbose;
  int autoWaitP = autoWait;
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "spTune_changed[" << spTuneId << "]\n" <<flush;
#endif

  // keep track if spin button changes not saved
  if ( BALLDIASP <= spTuneId && spTuneId <= SPEEDDISPLAYSCALARSP )
    tuneSpinChanged   = 1;
  else if ( AGS0SP    <= spTuneId && spTuneId <= MPS0SP )
    tunSpinChanged[0] = 1;
  else if ( AGS1SP    <= spTuneId && spTuneId <= MPS1SP )
    tunSpinChanged[1] = 1;

  // read the input from the spin button 
  g_object_get(spTune[spTuneId], "value", &spTuneVa[spTuneId], NULL);

  // Assign new input to class instance
  switch (spTuneId)
    {
    case BALLDIASP:   ZON->ballDia(spTuneVa[spTuneId]);                 break;
    case BOTTOMSP:    ZON->bottom(spTuneVa[spTuneId]);                  break;
    case DXZONESP:    ZON->dxZone(spTuneVa[spTuneId]);                  break;
    case DZZONESP:    ZON->dzZone(spTuneVa[spTuneId]);                  break;
    case DXDMPHZONESP:ZON->dXdMphZone(spTuneVa[spTuneId]);              break;
    case DZDMPHZONESP:ZON->dZdMphZone(spTuneVa[spTuneId]);              break;
    case GRAVITYSP:   ZON->gravity(spTuneVa[spTuneId]);                 break;
    case LEFTSP:      ZON->left(spTuneVa[spTuneId]);                    break;
    case RIGHTSP:     ZON->right(spTuneVa[spTuneId]);                   break;
    case SPEEDDISPLAYSCALARSP:
                      ZON->speedDisplayScalar(spTuneVa[spTuneId]);      break;
    case VELTHRESHSP: ZON->velThresh(spTuneVa[spTuneId]);               break;
    case TOPSP:       ZON->top(spTuneVa[spTuneId]);                     break;
    case TRIANGULATIONSCALARSP:
                      ZON->triangulationScalar(spTuneVa[spTuneId]);     break;
    case VELMAXSP:    ZON->velMax(spTuneVa[spTuneId]);                  break;
    case VERBOSESP:
      verboseP = verbose;
      verbose  = (int)spTuneVa[spTuneId];
      if ( verboseP || verbose )
	cerr << "MESSAGE(spTune_changed) : VERBOSESP  : verbose changed to "
	     << verbose << endl << flush;
#ifndef VERBOSE
      if ( verbose )
	cerr << "WARNING(Blink) : spTune_changed : "
	     << "recompile \'-DVERBOSE\' to"
	     << " get full verbose messages.\n" << flush;
#endif
      break;
    case AUTOSP:
      autoWaitP = autoWait;
      autoWait  = (int)spTuneVa[spTuneId];
      if ( 0 < verbose && (autoWaitP || autoWait) )
	cerr << "MESSAGE(spTune_changed) : AUTOSP  : autowait time changed to "
	     << autoWait << endl << flush;
      break;
    case AGS0SP:     TUN[0]->agaussScalar(spTuneVa[spTuneId]);          break;
    case BGS0SP:     TUN[0]->bgaussScalar(spTuneVa[spTuneId]);          break;
    case MBDS0SP:    TUN[0]->minBallDensityScalar(spTuneVa[spTuneId]);  break;
    case MBAS0SP:    TUN[0]->minBallAreaScalar(spTuneVa[spTuneId]);     break;
    case XBAS0SP:    TUN[0]->maxBallAreaScalar(spTuneVa[spTuneId]);     break;
    case MBRS0SP:    TUN[0]->minBallRatioScalar(spTuneVa[spTuneId]);    break;
    case XBRS0SP:    TUN[0]->maxBallRatioScalar(spTuneVa[spTuneId]);    break;
    case MPS0SP:     TUN[0]->minProxScalar(spTuneVa[spTuneId]);         break;

    case AGS1SP:     TUN[1]->agaussScalar(spTuneVa[spTuneId]);          break;
    case BGS1SP:     TUN[1]->bgaussScalar(spTuneVa[spTuneId]);          break;
    case MBDS1SP:    TUN[1]->minBallDensityScalar(spTuneVa[spTuneId]);  break;
    case MBAS1SP:    TUN[1]->minBallAreaScalar(spTuneVa[spTuneId]);     break;
    case XBAS1SP:    TUN[1]->maxBallAreaScalar(spTuneVa[spTuneId]);     break;
    case MBRS1SP:    TUN[1]->minBallRatioScalar(spTuneVa[spTuneId]);    break;
    case XBRS1SP:    TUN[1]->maxBallRatioScalar(spTuneVa[spTuneId]);    break;
    case MPS1SP:     TUN[1]->minProxScalar(spTuneVa[spTuneId]);         break;

    default:
      cerr << "WARNING(Blink) : spTune_changed : " << spTuneId
	   << " unknown." << endl << flush;
      break;
    }

  if ( spTuneId  != VERBOSESP ) SM.update();
}

// Save .cam
int saveAb(int i)
{
#ifdef VERBOSE
  if ( verbose ) cout << "saveAb(" << i << ")\n" << flush;
#endif
  gchar  *cmd;
  cmd = g_strdup_printf("rm -f %s", camFile[i]);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  saveAb : exec of \'" << cmd
	 << "\' failed\n" << flush;
  ofstream outf;
  outf.open(camFile[i], ios::out);
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveAb : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't open file " << camFile[i] << endl;
      return(-1);
    }
  outf << CAM[i]->print();
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveAb : " << __FILE__
	   << " : " << __LINE__ << " : "  
	   << "couldn't write file " << camFile[i] << endl;
      return(-1);
    }
  outf.close();
  cout << "MESSAGE(Blink) : saveAb :  wrote " << camFile[i]<< endl;

  // refresh check button status
  abSpinChanged[i] = 0;
  SM.update();

  return(0);
}

// Save .dat
int saveDat(int i)
{
#ifdef VERBOSE
  if ( verbose ) cout << "saveDat(" << i << ")\n" << flush;
#endif
  gchar  *cmd;
  cmd = g_strdup_printf("rm -f %s", datFile[i]);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  saveDat : exec of \'" << cmd
	 << "\' failed\n" << flush;
  ofstream outf;
  outf.open(datFile[i], ios::out);
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveDat(" << i << ") : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't open file " << datFile[i] << endl;
      return(-1);
    }
  outf << "# video.dat:\n"
       << "# Enter coordinates as detected by ump or read off\n"
       << "# of a screen shot of same resolution.\n"
       << "# First  ball is OAp (mask A)\n"
       << "# Second ball is OBp (mask B)\n"
       << "# Third  ball is OCp (mask C)\n"
       << "OAp   =  [ " << CAM[i]->OAP()[0] << "  " << CAM[i]->OAP()[1]
       << " ];\n"
       << "OBp   =  [ " << CAM[i]->OBP()[0] << "  " << CAM[i]->OBP()[1]
       << " ];\n"
       << "OCp   =  [ " << CAM[i]->OCP()[0] << "  " << CAM[i]->OCP()[1]
       << " ];\n";
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveDat(" << i << ") : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't write file " << datFile[i] << endl;
      return(-1);
    }
  outf.close();
  cout << "MESSAGE(Blink) : saveDat : wrote " << datFile[i] << endl;

  // refresh check button status
  datSpinChanged[i] = 0;
  SM.update();

  return(0);
}

// Save .dat
int saveTn(int i)
{
#ifdef VERBOSE
  if ( verbose ) cout << "saveTn(" << i << ")\n" << flush;
#endif
  ofstream outf;
  if ( !TUN[i] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "WARNING(Blink) : saveTn(" << i
			  << ") : TUN object doesn't exist.  Returning.\n"
			  << flush;
#endif
      return(-1);
    }
  gchar  *cmd;
  cmd = g_strdup_printf("rm -f %s", tunFile[i]);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  saveTn : exec of \'" << cmd
	 << "\' failed\n" << flush;
  outf.open(tunFile[i], ios::out);
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveTn(" << i << ") : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't open file " << tunFile[i] << endl;
      return(-1);
    }
#ifdef VERBOSE
  if ( 2 < verbose ) cout << *TUN[i] << endl << flush;
#endif
  outf << *TUN[i];
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveTn(" << i << ") : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't write file " << tunFile[i] << endl;
      return(-1);
    }
  outf.close();
  cout << "MESSAGE(Blink) : saveTn :  wrote " << tunFile[i] << endl;

  // refresh check button status
  tunSpinChanged[i] = 0;
  SM.update();

  return(0);
}

// Save world.dat
int saveWorld()
{
#ifdef VERBOSE
  if ( verbose ) cout << "saveWorld" << flush;
#endif
  gchar  *cmd;
  cmd = g_strdup_printf("rm -f %s", worldFile);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  saveWorld : exec of \'" << cmd
	 << "\' failed\n" << flush;
  ofstream outf;
  outf.open(worldFile, ios::out);
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveWorld : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't open file " << worldFile << endl;
      return(-1);
    }
  outf << "# Coordinate origin is front center of plate, on ground.\n"
       << "# X axis goes toward umpire right\n"
       << "# Y axis goes toward pitcher.\n"
       << "# Z axis goes upward.\n"
       << "# Left camera is from umpires point of view.\n"
       << "  Pl  = ["
       << setprecision(6) << setw(11) << spWorldVa[PLXSP]
       << setprecision(6) << setw(11) << spWorldVa[PLYSP]
       << setprecision(6) << setw(11) << spWorldVa[PLZSP]
       << "] # 3D position left cam, inches\n"
       << "  Pr  = ["
       << setprecision(6) << setw(11) << spWorldVa[PRXSP]
       << setprecision(6) << setw(11) << spWorldVa[PRYSP]
       << setprecision(6) << setw(11) << spWorldVa[PRZSP]
       << "] # 3D position right cam, inches\n"
       << "# You should spread objects in a triangle.  A straight line will\n"
       << "# create a numerical error.\n"
       << "  OA  = ["
       << setprecision(6) << setw(11) << spWorldVa[OAXSP]
       << setprecision(6) << setw(11) << spWorldVa[OAYSP]
       << setprecision(6) << setw(11) << spWorldVa[OAZSP]
       << "] # 3D position first object A, inches\n"
       << "  OB  = ["
       << setprecision(6) << setw(11) << spWorldVa[OBXSP]
       << setprecision(6) << setw(11) << spWorldVa[OBYSP]
       << setprecision(6) << setw(11) << spWorldVa[OBZSP]
       << "] # 3D position first object B, inches\n"
       << "  OC  = ["
       << setprecision(6) << setw(11) << spWorldVa[OCXSP]
       << setprecision(6) << setw(11) << spWorldVa[OCYSP]
       << setprecision(6) << setw(11) << spWorldVa[OCZSP]
       << "] # 3D position first object C, inches\n"
       << "\n";
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveWorld : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't write file " << worldFile << endl << flush;
      return(-1);
    }
  outf.close();
  if ( checkWorldFileExists() )
    {
      cout << "MESSAGE(Blink) : saveWorld :  wrote " << worldFile << endl
	   << flush;
      worldSpinChanged   = 0;   // reset
    }
  else
    {
      cout << "WARNING(Blink) : saveWorld :  failed to write " << worldFile
	   << endl << flush;
    }
  SM.update();
  return(0);
}

// Check pwd is not the home
int checkPwdNotHome()
{
  int result = strcmp(pwd, home);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkPwdNotHome : pwd=" << pwd << " : " << result
			  << endl << flush;
#endif
  return result;
}

// Check left cal file exists
int checkLeftCalFileExists()
{
  int result = checkFileExists(calFile[0]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkLeftCalFileExists : "  << calFile[0]
			  << " : " << result << endl << flush;
#endif
  return result;
}

// Check right cal file exists
int checkRightCalFileExists()
{
  int result = checkFileExists(calFile[1]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRightCalFileExists : "  << calFile[1]
			  << " : " << result << endl << flush;
#endif
  return result;
}

// Check left cal file exists in PWD
int checkLeftCalFileExistsPWD()
{
  int result = checkFileExists(calFilePWD[0]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkLeftCalFileExistsPWD : "  << calFilePWD[0]
			  << " : " << result << endl << flush;
#endif
  return result;
}

// Check right cal file exists
int checkRightCalFileExistsPWD()
{
  int result = checkFileExists(calFilePWD[1]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRightCalFileExistsPWD : "  << calFilePWD[1]
			  << " : " << result << endl << flush;
#endif
  return result;
}

// Check left tune file exists
int checkLeftTuneFileExists()
{
  int result = checkFileExists(tunFile[0]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkLeftTuneFileExists : "  << tunFile[0]
			  << " : " << result << endl << flush;
#endif
  return result;
}

// Check left tune file exists
int checkRightTuneFileExists()
{
  int result = checkFileExists(tunFile[1]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRightTuneFileExists : " << tunFile[1]
			  << " : "<< result << endl << flush;
#endif
  return result;
}

// Check zone.tune file exists
int checkTuneFileExists()
{
  int result = checkFileExists(tuneFile);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkTuneFileExists : " << tuneFile << " : "
			  << result << endl << flush;
#endif
  return result;
}

// Check world.dat file exists
int checkWorldFileExists()
{
  int result = checkFileExists(worldFile);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkWorldFileExists : " << worldFile << " : "
			  << result << endl << flush;
#endif
  return result;
}

// Check left dat file exists
int checkLeftDatFileExists()
{
  int result =  checkFileExists(datFile[0]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkLeftDatFileExists : " << datFile[0] << " : "
			  << result << endl << flush;
#endif
  return result;
}

// Check right dat file exists
int checkRightDatFileExists()
{
  int result =  checkFileExists(datFile[1]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRightDatFileExists : " << datFile[1]
			  << " : " << result << endl << flush;
#endif
  return result;
}

// Check left cam file exists
int checkLeftCamFileExists()
{
  int result = checkFileExists(camFile[0]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkLeftCamFileExists : " << camFile[0] << " : "
			  << result << endl << flush;
#endif
  return result;
}

// Check right cam file exists
int checkRightCamFileExists()
{
  int result = checkFileExists(camFile[1]);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkRightCamFileExists : " << camFile[1] << " : "
			  << result << endl << flush;
#endif
  return result;
}

// Check xwd is not the home
int checkXwdNotHome()
{
  int result = strcmp(xwd, home);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkXwdNotHome : xwd=" << xwd << " : " << result
			  << endl << flush;
#endif
  return result;
}

// Check the times to confirm external calibration run
int checkNewCals()
{
  int result = 0;
  result = (compareFileMtime(calFile[0], worldFile) ||
	    compareFileMtime(calFile[1], worldFile)   );
  for( int i=0; i<numDev; i++ )
    {
      result = result && (compareFileMtime(calFile[0], camFile[i]) ||
			  compareFileMtime(calFile[1], camFile[i])   );
      result = result && (compareFileMtime(calFile[0], datFile[i]) ||
			  compareFileMtime(calFile[1], datFile[i])    );
    }
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "checkNewCals : " << result << endl << flush;
#endif
  return result;
}

// Save zone.tune
int saveTune()
{
  if ( verbose ) cout << "saveTune\n" << flush;
  gchar  *cmd;
  cmd = g_strdup_printf("rm -f %s", tuneFile);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  saveTune : exec of \'" << cmd
	 << "\' failed\n" << flush;
  ofstream outf;
  outf.open(tuneFile, ios::out);
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveTune : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't open file " << tuneFile << endl;
      return(-1);
    }
  outf << *ZON;
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveTune : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't write file " << tuneFile << endl;
      return(-1);
    }
  outf.close();
  if ( checkTuneFileExists() )
    {
      cout << "MESSAGE(Blink) : saveTune :  wrote " << tuneFile << endl;
      tuneSpinChanged   = 0;   // reset
    }
  else
    {
      cout << "WARNING(Blink) : saveTune :  failed to write " << tuneFile
	   << endl;
    }
  SM.update();
  return(0);
}

// Show .mask
int showM(int i)
{
  int returnVal;

  // Reload mask from file
  if ( MASK[i] ) zap(MASK[i]);

  int dev  = msImDev[i];
  int nCal = CAM[dev]->imWidthCalEx();
  int mCal = CAM[dev]->imHeightCalEx();
  MASK[i]  = new MyMask(msFile[i], nCal, mCal);

  // Apply mask to image and write to file
  if ( !snapFileExists[i] )
    {
#ifdef VERBOSE
      if ( verbose )
	cout << "MESSAGE(Blink) : showM : " << snapFile[i]
	     << " does not exist\n" << flush;
#endif
      gchar  *cmd;
      cmd = g_strdup_printf("rm -f %s", mImgFile[i]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) :  showM : exec of \'" << cmd
	     << "\' failed\n" << flush;
      returnVal = -1;
    }
  else
    {
       returnVal =  MASK[i]->writeMaskImage(snapFile[i], mImgFile[i]);
    }
  
  mImgFileExists[i] = checkFileExists(mImgFile[i]);

  return returnVal;
}  // int showM(int i)

// Save .mask
int saveM(int i)
{
  if ( verbose ) cout << "saveM(" << i << ")\n" << flush;
  gchar  *cmd;
  cmd = g_strdup_printf("rm -f %s", msFile[i]);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  saveM : exec of \'" << cmd
	 << "\' failed\n" << flush;
  ofstream outf;
  outf.open(msFile[i], ios::out);
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveM : " << __FILE__ 
	   << " : " << __LINE__ << " : "  
	   << "couldn't open file \'" << msFile[i] << "\'\n";
      return(-1);
    }
  
  // Write out
  int DMSP = SX11SP  - SXD1SP;              // Mask line interval
  int DM   = SXD1ASP - SXD1SP;              // Mask interval
  outf << setprecision(2) << "# Mask " << msFileName[i] << "\n";
  outf << "#  Sx    Sy    Dx    Dy   DDx   DDy     1   \n";
  for( int ii=0; ii<6; ii++ )
    {
      for( int jj=0; jj<DMSP; jj++ )
	{
	  outf << setw(5) << spMVa[SXD0SP+ii*DMSP+jj+i*DM]  << " ";
	}
      outf << endl;
    }
  /*
       << "#  Explanation follows:\n"
       << "#  First blank the screen\n"
       << "#   0    0    1    1     0    0    0\n"
       << "# Now replace the alpha characters with numbers.\n"
       << "# The top left corner of the visible area is (Sx*N, Sy*M), where\n"
       << "# S is units of fraction of total NxM image size.\n"
       << "# The visible area is Dx*N wide by Dy*M tall.\n"
       << "# The lower right corner is skewed by DDx*N and DDy*M.\n"
       << "#  Sx   Sy    Dx   Dy   DDx   DDy  1\n"
       << "# ********A row with all zeroes is ultimately ignored ***********.\n"
       << "# See the info document or ump.html for description.\n"
       << "# Look at exmask.ps (included in doc) for a graphical example\n"
       << "# Get calculator to determine the fractions.\n"
       << "# You will have chance to iterate.\n"
       << "# The left camera from umpire point of view is 0.  Right is 1.\n"
       << "# Convention for balls A, B, C:  start at ball near home, go clockwise.\n";
  */
  if ( outf.fail() )
    {
      cerr << "WARNING(Blink) : saveM : " << __FILE__
	   << " : " << __LINE__ << " : "  
	   << "couldn't write file \'" << msFile[i] << "\'\n";
      return(-1);
    }
  outf.close();
  cout << "MESSAGE(Blink) : saveM :  wrote \'" << msFile[i] << "\'\n";

  // Reload mask into class
  showM(i);

  // refresh check button status
  maskSpinChanged[i] = 0;
  char desc[MAX_CANON];
  sprintf(desc, "%s saved", msFileName[i]);
  SM.update(desc);

  return(0);
}

// Save masks
void on_saveM_clicked(GtkButton *button, gpointer id_ptr)
{
  MsImId msImId = (MsImId) ((int)id_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_saveM_clicked[" << msImId << "]\n" <<flush;
#endif

  // Save the data
  if ( LM == msImId && checkMFileExistsLM() && !snapOR )
    {
      gtk_widget_show_all(GTK_WIDGET(qow[QOWLDM]));
      return;
    }
  else if ( RM == msImId && checkMFileExistsRM() && !snapOR )
    {
      gtk_widget_show_all(GTK_WIDGET(qow[QOWRDM]));
      return;
    }
  else saveM(msImId);

  g_signal_emit_by_name(maskImage[msImId], "realize");
}

// Take stills for all the external calibration work
void on_still_clicked(GtkButton *button, gpointer id_ptr)
{
  StId stId = (StId) ((int)id_ptr);
  gchar *cmd;
  int cmdSuccess = 1;
  int checkCalExists[2] = {checkLeftCalFileExists(), checkLeftCalFileExists() };
  int checkCamExists[2] = {checkLeftCamFileExists(), checkLeftCamFileExists() };
#ifdef VERBOSE
  if ( verbose ) cout << "on_still_clicked[" << stId << "]\n" <<flush;
#endif

  // Change directory to cal (this is safety measure to ensure cal exists)
  // ump is called with full path arguments
  gint status = 0;
  if ( 0 > (status = chdir(xwd)) )
    {
      cerr << "WARNING(blink) : on_still_clicked : couldn't open directory "
	   << xwd << endl << flush;
      return;
    }

  // Make temporary .cal files if needed
  int tempCal[2] = {0, 0};
  for ( int i=0; i<numDev; i++ )
    {
      if ( !checkCalExists[0] && !checkCalExists[1] )
	{
	  if ( checkCamExists[0] || checkCamExists[1] )
	    {
	      cmd = g_strdup_printf("cp %s/video%d.cam %s/video%d.cal",
				    xwd, i, xwd, i);
	      if ( 0 != Execute(cmd, 0) )
		{
		  cerr << "WARNING(Blink) :  on_still_clicked : exec of \'"
		       << cmd << "\' failed\n" << flush;
		  return;
		}
#ifdef VERBOSE
	      if ( verbose )
		cout << "Blink : on_still_clicked : made temporary "
		     << calFile[i] << ".  To be removed.\n" << flush;
#endif
	      tempCal[i] = 1;
	    }
	  else
	    {
	      cerr << "WARNING(Blink) : on_still_clicked : "
		   << ".cal & .cam file missing.  Quitting.\n" << flush;
	      showErr("You must save aberration files before stills taken.");
	      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
	      return;
	    }
	}
    }

  // Snap videos and map images to where needed
  // map to white or black
  int ballColor = stId/NUM_BALLS;
  // map to ball A, B, or C
  int ballId    = stId - ballColor*NUM_BALLS;
  // map to snap images
  int msimid[2] = {stId+1+ballColor, stId+ballColor+NUM_BALLS+2};

  // remove the snap image, so can check on success later.  Leave .raw
  // so if ump doesn't run because no camera cards, still can complete
  // calibration.   Also delete the master masks.  Later restore them.
  for ( int i=0; i<numDev; i++ )
    {
      // move masks
      char maskFile[MAX_CANON];
      sprintf(maskFile, "%s/video%d.mask", xwd, i);
      if ( checkFileExists(maskFile) )
	{
	  cmd = g_strdup_printf("mv %s/video%d.mask %s/video%d.mask.sav",
				xwd, i, xwd, i);
	  if ( 0 != Execute(cmd, 0) )
	    {
#ifdef VERBOSE
	      if ( verbose )
		cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
		     << "\' failed\n" << flush;
#endif
	      cmdSuccess = 0;
	    }
	}

    }

  // If first snap of set, make sure state reset
  if ( cmdSuccess && BLACKWHITE.complete("All stills done") )
    {
      for ( int snap=0; snap<NUM_STILL_BUTTONS; snap++)
	blackWhiteDone[snap] = 0;
    }
  BLACKWHITE.update();

  // take the snapshots.  ump removes old ppms when it runs
  if ( 0 == ballColor && cmdSuccess ) // White balls
    {
      char fileL[MAX_CANON];
      sprintf(fileL,  "%s/%s", xwd, ballLRawName[ballId]);
      char fileR[MAX_CANON];
      sprintf(fileR,  "%s/%s", xwd, ballRRawName[ballId]);
      cmd  = g_strdup_printf(">%s/temp ump -C -s%dx%d -t%d -o%s -o%s 2>&1", xwd,
			     CAM[0]->imWidthCalEx(), CAM[0]->imHeightCalEx(),
			     int(float(NUMFRAMES)*2.0), fileL, fileR);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
	       << "\' failed\n" << flush;
	  cmdSuccess = 0;
	}

      // Check for tempRCx003.ppm
      for ( int i=0; i<numDev; i++ )
	{
	  cmd = g_strdup_printf("test -s %s/tempRC%d003.ppm", xwd, i);
	  if ( Execute(cmd, 0) )
	    {
	      cmdSuccess = 0;
	      cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
		   << "\' failed.  Still photos not made.\n" << flush;
	    }
	}
      
      // Check for success left ball
      if ( cmdSuccess )
	{
	  if ( checkBallLRawExists(ballId) )
	    {
	      cout << "MESSAGE(Blink) : on_still_clicked : left ball "
		   << stSnFileName[ballId] << " exists\n" << flush;
	      blackWhiteDone[stId] = 1;
	      BLACKWHITE.update();
	      // Check for success right ball
	      if ( checkBallRRawExists(ballId) )
		{
		  cout << "MESSAGE(Blink) : on_still_clicked : right ball "
		       << stSnFileName[ballId] << " exists\n" << flush;
		  blackWhiteDone[stId] = 1;
		  BLACKWHITE.update();
		}
	      else
		{
		  cout <<"MESSAGE(Blink) : on_still_clicked : "
		       <<"failed to write right ball " << stSnFileName[ballId]
		       << endl << flush;
		  blackWhiteDone[stId] = 0;
		  BLACKWHITE.update();
		  return;
		}
	    }
	  else
	    {
	      cout << "MESSAGE(Blink) : on_still_clicked : "
		   << "failed to write left ball " << stSnFileName[ballId]
		   << endl << flush;
	      blackWhiteDone[stId] = 0;
	      BLACKWHITE.update();
	      return;
	    }
	}
    }
  else if ( cmdSuccess ) // Black balls
    {
      char fileL[MAX_CANON];
      sprintf(fileL,  "%s/%s", xwd, blankLRawName[ballId]);
      char fileR[MAX_CANON];
      sprintf(fileR,  "%s/%s", xwd, blankRRawName[ballId]);
      cmd  = g_strdup_printf(">%s/temp ump -C -s%dx%d -t%d -o%s -o%s 2>&1", xwd,
			     CAM[0]->imWidthCalEx(), CAM[0]->imHeightCalEx(),
			     int(float(NUMFRAMES)*2.0), fileL, fileR);
      if ( 0 != Execute(cmd, 0) )
	{
	  cmdSuccess = 0;
	  cerr << "MESSAGE(Blink) :  on_still_clicked : exec of \'" << cmd
	       << "\' failed\n" << flush;
	}
      
      // Check for tempRCx003.ppm
      for ( int i=0; i<numDev; i++ )
	{
	  cmd = g_strdup_printf("test -s %s/tempRC%d003.ppm", xwd, i);
	  if ( Execute(cmd, 0) )
	    {
	      cmdSuccess = 0;
	      cerr << "MESSAGE(Blink) :  on_still_clicked : exec of \'" << cmd
		   << "\' failed.  Still photos not made.\n" << flush;
	    }
	}

      // Check for success left blank
      if ( cmdSuccess )
	{
	  if ( checkBlankLRawExists(ballId) )
	    {
	      cout << "MESSAGE(Blink) : on_still_clicked : wrote left blank "
		   << stSnFileName[ballId] << "\n" << flush;
	      blackWhiteDone[stId] = 1;
	      BLACKWHITE.update();
	    }
	  else
	    {
	      cmdSuccess = 0;
	      cerr<<"WARNING(Blink) : on_still_clicked : failed to write "
		  << "left blank " << stSnFileName[ballId] << endl << flush;
	      blackWhiteDone[stId] = 0;
	      BLACKWHITE.update();
	    }
	}
    
  
      // Check for success right blank
      if ( cmdSuccess )
	{
	  if ( checkBlankRRawExists(ballId) )
	    {
	      cout << "MESSAGE(Blink) : on_still_clicked : wrote right blank "
		   << stSnFileName[ballId] << "\n" << flush;
	      blackWhiteDone[stId] = 1;
	      BLACKWHITE.update();
	    }
	  else
	    {
	      cmdSuccess = 0;
	      cerr<<"WARNING(Blink) : on_still_clicked : failed to write "
		  << "right blank " << stSnFileName[ballId] << endl << flush;
	      blackWhiteDone[stId] = 0;
	      BLACKWHITE.update();
	    }
	}
    }
      
  // Copy to snapshot and check for success
  if ( 0 == ballColor && cmdSuccess )
    {
      for ( int i=0; i<numDev; i++ )
	{
	  cmd = g_strdup_printf("cp %s/tempRC%d003.ppm %s",
				xwd, i, msSnFileName[msimid[i]]);
	  if ( 0 != Execute(cmd, 0) )
	    cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
		 << "\' failed\n" << flush;

	  if ( (snapFileExists[msimid[i]] =checkFileExists(snapFile[msimid[i]])) )
	    {
	      cout << "MESSAGE(Blink) : on_still_clicked : wrote "
		   << snapFile[msimid[i]] << "\n" << flush;

	      SM.update("sliceraw run");
	      g_signal_emit_by_name(saveMButton[msimid[i]], "clicked");
  	    }
	  else
	    cout << "MESSAGE(Blink) : on_still_clicked : failed to write "
		 << snapFile[msimid[i]] << endl << flush;

	  // copy to the overall image
	  if ( 0 == ballId )
	    {
	      cmd = g_strdup_printf("cp %s/tempRC%d003.ppm %s",
				    xwd, i, msSnFileName[i*4]);
	      if ( 0 != Execute(cmd, 0) )
		cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
		     << "\' failed\n" << flush;
	      
	      if ( (snapFileExists[i*4] = checkFileExists(snapFile[i*4])) )
		{
		  cout << "MESSAGE(Blink) : on_still_clicked : wrote "
		       << snapFile[i*4] << "\n" << flush;
		  SM.update("sliceraw run");
		  snapOR = 1;
		  g_signal_emit_by_name(saveMButton[i*4], "clicked");
		  snapOR = 0;
		}
	      else
		cout << "MESSAGE(Blink) : on_still_clicked : failed to write "
		     << snapFile[i*4] << endl << flush;
	    }
	}
    }

  // Restore master masks
  for ( int i=0; i<numDev; i++ )
    {
      char maskFile[MAX_CANON];
      sprintf(maskFile, "%s/video%d.mask.sav", xwd, i);
      if ( checkFileExists(maskFile) )
	{
	  cmd = g_strdup_printf("mv %s/video%d.mask.sav %s/video%d.mask",
				xwd, i, xwd, i);
	  if ( 0 != Execute(cmd, 0) )
	    cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
		 << "\' failed\n" << flush;
	}
    }

  // Remove temporary .cal files if needed
  for ( int i=0; i<numDev; i++ )
    {
      if ( tempCal[i] )
	{
	  cmd = g_strdup_printf("rm -f %s/video%d.cal", xwd, i);
	  if ( 0 != Execute(cmd, 0) )
	    {
	      cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
		   << "\' failed\n" << flush;
	      return;
	    }
#ifdef VERBOSE
	  if ( verbose )
	    cout << "Blink : on_still_clicked : removed temporary "
		 << calFile[i] << ".\n" << flush;
#endif
	}
    }

  // Change back to pwd
  if ( 0 > (status = chdir(pwd)) )
    {
      cerr << "WARNING(blink) : on_still_clicked : couldn't open directory "
	   << pwd << endl << flush;
      return;
    }

  SM.update();
}

// Take stills for all the internal calibration work
void on_intStill_clicked(GtkButton *button, gpointer id_ptr)
{
  StIntId stIntId = (StIntId) ((int)id_ptr);
  gchar *cmd;
  int cmdSuccess = 1;
  int checkCalExists[2]          = {checkLeftCalFileExists(),
				    checkLeftCalFileExists() };
  int checkCamExists[2]          = {checkLeftCamFileExists(),
				    checkLeftCamFileExists() };
  int checkIntStillFileExists[2] = {checkIntStillFileExistsL(),
				    checkIntStillFileExistsL()};
#ifdef VERBOSE
  if ( verbose ) cout << "on_intStill_clicked[" << stIntId << "]\n" <<flush;
#endif

  // Change directory to cal (this is safety measure to ensure cal exists)
  // ump is called with full path arguments
  gint status = 0;
  if ( 0 > (status = chdir(xwd)) )
    {
      cerr << "WARNING(blink) : on_intStill_clicked : couldn't open directory "
	   << xwd << endl << flush;
      return;
    }

  // Make temporary .cal files if needed
  int tempCal[2] = {0, 0};
  for ( int i=0; i<numDev; i++ )
    {
      if ( !checkCalExists[0] && !checkCalExists[1] )
	{
	  if ( checkCamExists[0] || checkCamExists[1] )
	    {
	      cmd = g_strdup_printf("cp %s/video%d.cam %s/video%d.cal",
				    xwd, i, xwd, i);
	      if ( 0 != Execute(cmd, 0) )
		{
		  cerr << "WARNING(Blink) :  on_intStill_clicked : exec of \'"
		       << cmd << "\' failed\n" << flush;
		  return;
		}
#ifdef VERBOSE
	      if ( verbose )
		cout << "Blink : on_intStill_clicked : made temporary "
		     << calFile[i] << ".  To be removed.\n" << flush;
#endif
	      tempCal[i] = 1;
	    }
	  else
	    {
	      cerr << "WARNING(Blink) : on_intStill_clicked : "
		   << ".cal & .cam file missing.  Quitting.\n" << flush;
	      showErr("You must save aberration files before stills taken.");
	      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
	      return;
	    }
	}
    }

  // take the snapshots.  ump removes old ppms when it runs
  if ( cmdSuccess )
    {
      char fileRaw[MAX_CANON];
      sprintf(fileRaw,  "%s/ballStill.raw", xwd);
      cmd =
	g_strdup_printf(">%s/temp ump -C -s%dx%d -t%d -o%s -c/dev/video%d 2>&1",
			xwd, CAM[stIntId]->imWidthCalEx(),
			CAM[stIntId]->imHeightCalEx(),
			int(float(NUMFRAMES)*2.0), fileRaw, stIntId);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "WARNING(Blink) :  on_still_clicked : exec of \'" << cmd
	       << "\' failed\n" << flush;
	  cmdSuccess = 0;
	}

      // Check for tempRCx003.ppm
      cmd = g_strdup_printf("test -s %s/tempRC%d003.ppm", xwd, stIntId);
      if ( Execute(cmd, 0) )
	{
	  cmdSuccess = 0;
	  cerr << "WARNING(Blink) :  on_intStill_clicked : exec of \'" << cmd
	       << "\' failed.  Still photos not made.\n" << flush;
	}

      // Check for success
      char desc[MAX_CANON];
      sprintf(desc, "%s exists", stIntName[stIntId]);
      if ( cmdSuccess )
	{
	  if ( checkIntStillFileExists[stIntId] )
	    {
	      cout << "MESSAGE(Blink) : on_intStill_clicked : ball "
		   << stIntName[stIntId] << " exists\n" << flush;
	      SM.update(desc);
	    }
	}
      else
	{
	  cout << "MESSAGE(Blink) : on_intStill_clicked : "
	       << "failed to write ball " << stIntName[stIntId]
	       << endl << flush;
	  SM.update(desc);
	  return;
	}

      // Copy to saved file
      cmd = g_strdup_printf("cp %s/tempRC%d003.ppm %s",
			    xwd, stIntId, stillFile[stIntId]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "WARNING(Blink) :  on_intStill_clicked : exec of \'" << cmd
	     << "\' failed\n" << flush;

      if ( checkFileExists(stillFile[stIntId]) )
	{
	  cout << "MESSAGE(Blink) : on_intStill_clicked : wrote "
	       << stillFile[stIntId] << "\n" << flush;
	}
      else
	cout << "MESSAGE(Blink) : on_intStill_clicked : failed to write "
	     << stillFile[stIntId] << endl << flush;
    }
  
  // Remove temporary .cal files if needed
  for ( int i=0; i<numDev; i++ )
    {
      if ( tempCal[i] )
	{
	  cmd = g_strdup_printf("rm -f %s/video%d.cal", xwd, i);
	  if ( 0 != Execute(cmd, 0) )
	    {
	      cerr << "WARNING(Blink) :  on_intStill_clicked : exec of \'"
		   << cmd << "\' failed\n" << flush;
	      return;
	    }
#ifdef VERBOSE
	  if ( verbose )
	    cout << "Blink : on_intStill_clicked : removed temporary "
		 << calFile[i] << ".\n" << flush;
#endif
	}
    }
  
  // Change back to pwd
  if ( 0 > (status = chdir(pwd)) )
    {
      cerr << "WARNING(blink) : on_intStill_clicked : couldn't open directory "
	   << pwd << endl << flush;
      return;
    }
  
  SM.update();
}

// Save world data
void on_saveTheWorld_clicked(GtkButton *saveTheWorldButton, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_saveTheWorld_clicked\n" <<flush;
#endif

  // Save the data
  if ( checkWorldFileExists() )
    {
      gtk_widget_show_all(GTK_WIDGET(qow[QOWWORLD]));
    }
  else saveWorld();
}

// Save world data
void on_continuous_clicked(GtkButton *saveTheWorldButton, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_continuous_clicked\n" <<flush;
#endif

  gchar  *cmd;
  int nCal = CAM[0]->imWidthCalEx();
  int mCal = CAM[0]->imHeightCalEx();
  int sub  = 1;

  // Check for calibration done
  if ( !SM.complete("external calibration run") )
  {
    showErr("You must run external calibration \'calibrate\' before run");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
    return;
  }

  // Take picture
  cmd = g_strdup_printf("Ump -- -r -s%dx%d -u%d", nCal, mCal, sub);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  on_continuous_clicked : exec of \'" << cmd
	 << "\' failed\n" << flush;
}

// Save zone.tune data
void on_saveZoneTune_clicked(GtkButton *saveZoneTuneButton,
				   gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_saveZoneTune_clicked\n" <<flush;
#endif

  // Save the data
  if ( checkTuneFileExists() )
    {
      gtk_widget_show_all(GTK_WIDGET(qow[QOWTUNE]));
    }
  else saveTune();
}

// Show masks
void on_showM_clicked(GtkButton *button, gpointer id_ptr)
{
  MsImId msImId = (MsImId) ((int)id_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_showM_clicked[" << msImId << "]\n" <<flush;
#endif

  showM(msImId);
  g_signal_emit_by_name(maskImage[msImId], "realize");
}

// Handle check boxes
void checkMe(GdBuId gdBuId, GtkCheckButton *checkButton, int wantCheck,
	    const char *tag, const char *failMsg)
{
  g_object_get(checkButton, "active", &guideChecked[gdBuId], NULL);
  if ( !wantCheck )  // if box is not wanted to be checked
    {
      if ( guideChecked[gdBuId] ) // but is
	{
#ifdef VERBOSE
	  if ( 1 < verbose ) cout << "checkMe : " << tag << " : " 
				  << failMsg << endl << flush;
#endif
	  g_signal_handler_block(checkButton, guideHand[gdBuId]);
	  g_signal_emit_by_name(checkButton, "clicked"); // uncheck it
	  g_signal_handler_unblock(checkButton, guideHand[gdBuId]);
	}
    }
  else // if box is wanted to be checked
    {
      if ( !guideChecked[gdBuId] )  // and is not
	{
	  cerr << "MESSAGE(Blink) : checkMe : " << tag
	       << endl << flush;
	  g_signal_handler_block(checkButton, guideHand[gdBuId]);
	  g_signal_emit_by_name(checkButton, "clicked");   // then check it
	  g_signal_handler_unblock(checkButton, guideHand[gdBuId]);
	}
    }
}

// Handle manual override of guide check boxes
void on_guide_clicked(GtkCheckButton *checkButton, gpointer id_ptr)
{
  GdBuId gdBuId = (GdBuId) ( (int)id_ptr); 
  int dev, maskNum;
  g_assert(CHOSE_WORKING_DIRECTORY <= gdBuId && NUM_GUIDE_BUTTONS > gdBuId );
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "on_guide_clicked[" << gdBuId << "]\n" <<flush;
#endif
  int checkIntStillFileExists[2] =
    {checkIntStillFileExistsL(), checkIntStillFileExistsR()};

  switch ( gdBuId )
    {

    case CHOSE_WORKING_DIRECTORY:
#ifdef VERBOSE
      if ( verbose )
	cout << "MESSAGE(on_guide_clicked) : CHOSE_WORKING_DIRECTORY\n"
	     << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("chose working directory"),
	      "CHOSE_WORKING_DIRECTORY",
	      "Do not use home.\nCorrect this by \'chose working directory\'");
      break;

    case CHOSE_CAL_DIRECTORY:
#ifdef VERBOSE
      if ( verbose )
	cout << "MESSAGE(on_guide_clicked) : CHOSE_CAL_DIRECTORY\n" << flush;
#endif
      SM.updateWithBlocking("chose cal directory");
      checkMe(gdBuId, checkButton,
	      SM.complete("chose cal directory"),
	      "CHOSE_CAL_DIRECTORY",
	      "Do not use home.\nCorrect this by \'chose cal directory\'");
      break;

    case WORLD_DAT_SAVED:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : WORLD_DAT_SAVED\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("world.dat saved"), "WORLD_DAT_SAVED", "");
      break;

    case GUIDE_TUNE_SAVED:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : GUIDE_TUNE_SAVED\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("zone.tune saved"), "GUIDE_TUNE_SAVED", "");
      break;

    case LEFT_TUNE_SAVED:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : LEFT_TUNE_SAVED\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("left tune saved"), "LEFT_TUNE_SAVED", "");
      break;

    case RIGHT_TUNE_SAVED:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : RIGHT_TUNE_SAVED\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("right tune saved"), "RIGHT_TUNE_SAVED", "");
      break;

    case GUIDE_D0_SAVED:
    case LEFT_MA_SAVED:  case LEFT_MB_SAVED:  case LEFT_MC_SAVED:
    case GUIDE_D1_SAVED:
    case RIGHT_MA_SAVED: case RIGHT_MB_SAVED: case RIGHT_MC_SAVED:
      maskNum = gdBuId - GUIDE_D0_SAVED;
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "on_guide_clicked : GUIDE_D_SAVED, maskNum "
			      << maskNum 
			      << " \n" << flush;
#endif
      char msg[MAX_CANON];
      // sprintf(msg, "maskSpinChanged[%d]", maskNum);
      sprintf(msg, "%s saved", msFileName[maskNum]);
      checkMe(gdBuId, checkButton,
	      SM.complete(msg), "GUIDE_D_SAVED", msg);
      // !maskSpinChanged[maskNum], "GUIDE_D_SAVED", msg);
      break;

    case LEFT_ABERRATION_SAVED:    case RIGHT_ABERRATION_SAVED:
      dev = gdBuId-LEFT_ABERRATION_SAVED;
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : ABERRATION_SAVED, device " << dev << " \n"
			      << flush;
#endif
      checkMe(gdBuId, checkButton,
	      ( 0 == dev && SM.complete("left aberration saved") ||
		1 == dev && SM.complete("right aberration saved")   ),
	      "ABERRATION_SAVED", "");
      break;

    case LEFT_PIXEL_DATA_SAVED:    case RIGHT_PIXEL_DATA_SAVED:
      dev = gdBuId-LEFT_PIXEL_DATA_SAVED;
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : PIXEL_DATA_SAVED, device " << dev << " \n"
			      << flush;
#endif
      checkMe(gdBuId, checkButton,
	      ((0==dev && SM.complete("left pixel data saved"))  ||
	       (1==dev && SM.complete("right pixel data saved"))),
	      "PIXEL_DATA_SAVED", "");
      break;

    case EXT_CAL_RUN:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "on_guide_clicked : EXT_CAL_RUN\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("external calibration run"), "EXT_CAL_RUN",
	      "Checkbutton reflects old world.dat, video?.cam, or video?.dat");
      break;
      
    case SNAPS_DONE:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "on_guide_clicked : SNAPS_DONE\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("snaps done"), "SNAPS_DONE",
	      "Checkbutton reflects all raw files ready");
      break;

    case SLICERAW_RUN:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "on_guide_clicked : SLICERAW_RUN\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("sliceraw run"), "SLICERAW_RUN",
	      "Checkbutton reflects all raw files ready");
      break;

    case END_END_READY:
#ifdef VERBOSE
      if ( 1 < verbose )cout << "on_guide_clicked : END_END_READY\n" << flush;
#endif
      checkMe(gdBuId, checkButton,
	      SM.complete("end end ready"), "END_END_READY", "");
      break;

    case WHITEA: case WHITEB: case WHITEC:
    case BLACKA: case BLACKB: case BLACKC:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : " << stName[gdBuId-WHITEA] << endl<< flush;
#endif
      checkMe(gdBuId, checkButton,
	      blackWhiteDone[gdBuId-WHITEA], stName[gdBuId-WHITEA], "");
      break;

    case STILLLEFT: case STILLRIGHT:
#ifdef VERBOSE
      if ( 1 < verbose ) cout << " : " << stIntName[gdBuId-STILLLEFT]
			      << endl<< flush;
#endif
      checkMe(gdBuId, checkButton, checkIntStillFileExists[gdBuId-STILLLEFT],
	      stIntName[gdBuId-STILLLEFT], "");
      break;

    default:
      break;
    }
  return;
}

// Revert buttons
void on_revert_clicked(GtkButton *revert, gpointer id_ptr)
{
  RevertId revId = (RevertId) ( (int)id_ptr); 
  g_assert(REVERT_ZONE <= revId && NUM_REVERT_BUTTONS > revId );
#ifdef VERBOSE
  if ( verbose ) cout << "on_revert_clicked[" << revId << "]" <<flush;
#endif

  switch ( revId )
    {

    case REVERT_ZONE:
#ifdef VERBOSE
      if ( verbose ) cout << " : REVERT_ZONE\n" << flush;
#endif
      loadZone();
      break;

    case REVERT_WORLD:
#ifdef VERBOSE
      if ( verbose ) cout << " : REVERT_WORLD\n" << flush;
#endif
      loadWorld(0);
      break;

    case AB0_REVERT: case CAM0_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : AB0_REVERT or CAM0_REVERT\n" << flush;
#endif
      loadCam(0);
      loadWorld(0);
      break;

    case MASKD0_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD0_REVERT\n" << flush;
#endif
      loadMask(LM);
      break;

    case MASKD0A_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD0A_REVERT\n" << flush;
#endif
      loadMask(LMA);
      break;

    case MASKD0B_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD0B_REVERT\n" << flush;
#endif
      loadMask(LMB);
      break;

    case MASKD0C_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD0C_REVERT\n" << flush;
#endif
      loadMask(LMC);
      break;

    case AB1_REVERT: case CAM1_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : AB1_REVERT or CAM1_REVERT\n" << flush;
#endif
      loadCam(1);
      loadWorld(1);
      break;

    case MASKD1_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD1_REVERT\n" << flush;
#endif
      loadMask(RM);
      break;

    case MASKD1A_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD1A_REVERT\n" << flush;
#endif
      loadMask(RMA);
      break;

    case MASKD1B_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD1B_REVERT\n" << flush;
#endif
      loadMask(RMB);
      break;

    case MASKD1C_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : MASKD1C_REVERT\n" << flush;
#endif
      loadMask(RMC);
      break;

    case T0_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : T0_REVERT\n" << flush;
#endif
      loadTune(0);
      break;

    case T1_REVERT:
#ifdef VERBOSE
      if ( verbose ) cout << " : T1_REVERT\n" << flush;
#endif
      loadTune(1);
      break;

    default:
      cerr << "ERROR(on_revert_clicked) : unknown option\n" << flush;
      break;
    }
  return;
}

// Measure Raw buttons
void on_mRaw_clicked(GtkButton *mRaw, gpointer id_ptr)
{
  MRawId mRawId = (MRawId) ( (int)id_ptr); 
  g_assert(MRAWA0 <= mRawId && NUM_MRAW_BUTTONS > mRawId );
#ifdef VERBOSE
  if ( verbose ) cout << "on_mRaw_clicked[" << mRawId << "]" <<flush;
#endif

  switch ( mRawId )
    {
    case MRAWA0: case MRAWB0: case MRAWC0:
    case MRAWA1: case MRAWB1: case MRAWC1:
      measureRaw(mRawId);
      break;
    default:
      cerr << "ERROR(on_mRaw_clicked) : unknown option\n" << flush;
      break;
    }
  return;
}

// Navigation buttons
void on_jump_clicked(GtkButton *present, gpointer id_ptr)
{
  JumpId jumpId = (JumpId) ( (int)id_ptr); 
  g_assert(JUMP_TO_MAIN <= jumpId && NUM_JUMP_BUTTONS > jumpId );
#ifdef VERBOSE
  if ( verbose ) cout << "on_jump_clicked[" << jumpId << "]" <<flush;
#endif

  switch ( jumpId )
    {
    case JUMP_TO_MAIN:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 0);
      break;
    case JUMP_TO_CAL:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 0);
      break;
    case JUMP_TO_WORLD:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 3);
      break;
    case GUIDE_GO_TUNE:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 4);
      break;
    case GUIDE_GO_MASK_D0:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookL), 1);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case JUMP_TO_LEFT_MA:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookL), 2);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case JUMP_TO_LEFT_MB:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookL), 3);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case JUMP_TO_LEFT_MC:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookL), 4);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case GUIDE_GO_MASK_D1:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookR), 1);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    case JUMP_TO_RIGHT_MA:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookR), 2);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    case JUMP_TO_RIGHT_MB:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookR), 3);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    case JUMP_TO_RIGHT_MC:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookR), 4);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    case JUMP_TO_LEFT_ABERRATION:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case JUMP_TO_RIGHT_ABERRATION:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    case JUMP_TO_LEFT_PIXEL_DATA:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case JUMP_TO_RIGHT_PIXEL_DATA:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    case JUMP_TO_SNAPS:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 3);
      break;
    case JUMP_TO_LEFT_TUNE:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookL), 1);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 1);
      break;
    case JUMP_TO_RIGHT_TUNE:
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebookR), 1);
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 2);
      break;
    default:
#ifdef VERBOSE
      if ( verbose ) cout << " : generic_GO_GUIDE\n" << flush;
#endif
      gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
      break;
    }
  return;
}

// Link .cal files in the cal xwd directory to ump run in pwd
void on_calLink_clicked(GtkCheckButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_calLink_clicked : " << flush;
#endif
  char *linkFileDisp[2] = {new char[MAX_CANON], new char[MAX_CANON]};

  // Get status of check button
  g_object_get(button, "active", &calLinked, NULL);

  // Initialize
  linkIsWorking[0] = 1;  linkIsWorking[1] = 1;
  int inCal[2]    = {checkLeftCalFileExists(),   checkRightCalFileExists() };
  int inCalPWD[2] = {checkLeftCalFileExistsPWD(),checkRightCalFileExistsPWD() };

  // If clicked and no links/files exist in pwd, link to xwd
  if ( calLinked )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "calLinked : " << calLinked << endl << flush;
#endif

      // calLinked and all files available GOOD
      if ( inCal[0] && inCal[1] && !inCalPWD[0] && !inCalPWD[1] )
	{
#ifdef VERBOSE
	  if ( verbose ) cout << "on_calLink_clicked : all OK\n" << flush;
#endif
	  for( int i = 0; i < numDev; i++ )
	    {
	      strcpy(calPath[i], xwd);
	      strcpy(linkFileDisp[i], calPath[i]);
	      gchar *cmd = g_strdup_printf("ln %s .", calFile[i]);
	      if ( pwd != xwd && 0 != Execute(cmd, 0) )
		{
		  cerr << "ERROR(Blink) : on_calLink_clicked : exec of \'"
		       << cmd << "\' failed\n" << flush;
		  linkIsWorking[i]  = 0;
		}
	      else
		{
		  cerr << "MESSAGE(Blink) : on_calLink_clicked : "
		       << "linked " << calFile[i] << " to " << calFilePWD[i] 
		       << endl << flush;
		}
	    }
	}

      // calLinked but files available only in working directory OK but toggle
      else if ( inCalPWD[0] &&  inCalPWD[1] )
      {
#ifdef VERBOSE
	  if ( verbose ) cout << "on_calLink_clicked : "
			      << "only in pwd OK but toggle off.\n" << flush;
#endif
	  for( int i = 0; i < numDev; i++ )
	    {
	      strcpy(calPath[i], pwd);
	      strcpy(linkFileDisp[i], calPath[i]);
	      linkIsWorking[i] = 0;
	    }
	  // toggle calLink back to off
	  cerr << "WARNING(Blink) : on_calLink_clicked : cannot link "
	       << "over existing files.\n" << flush;
	  if ( !blockQueryRemove )
	    {
	      query_remove(calFilePWD[0]);
	      query_remove(calFilePWD[1]);
	    }
	  g_signal_handler_block(calLink, calLinkHand);
	  g_signal_emit_by_name(calLink, "clicked");
	  g_signal_handler_unblock(calLink, calLinkHand);
      }

      // calLinked and some files missing  BAD.  Toggle calLink.
      else
	{
#ifdef VERBOSE
	  if ( verbose ) cout << "on_calLink_clicked : "
			      << "some files missing. Bad & toggle.\n" << flush;
#endif
	  for( int i = 0; i < numDev; i++ )
	    {
	      if ( inCalPWD[i] && !inCal[i] )
		{
		  strcpy(calPath[i], pwd);
		  strcpy(linkFileDisp[i], calPath[i]);
		}
	      else
		{
		  strcpy(calPath[i], pwd);
		  if ( inCalPWD[i] )
		    {
		      strcpy(linkFileDisp[i], calPath[i]);
		    }
		  else
		    {
		      linkIsWorking[i] = 0;
		      sprintf(linkFileDisp[i],"WARNING: No video%c.cal in Main",
			      device_digit[i]);
		    }
		}
	    }
	  // toggle calLink
	  g_signal_handler_block(calLink, calLinkHand);
	  g_signal_emit_by_name(calLink, "clicked");
	  g_signal_handler_unblock(calLink, calLinkHand);
	}
    }
  else  // cal unlinked
    {
#ifdef VERBOSE
      if ( verbose ) cout << "calLinked : " << calLinked << endl << flush;
#endif
      for( int i = 0; i < numDev; i++ )
	{
	  strcpy(calPath[i], pwd);
	  if ( xwd != pwd && inCalPWD[i] )
	    {
	      strcpy(linkFileDisp[i], calPath[i]);
	      gchar *cmd = g_strdup_printf("rm %s", calFilePWD[i]);
	      if ( 0 != Execute(cmd, 0) )
		{
		  cerr << "ERROR(Blink) : on_calLink_clicked : exec of \'"
		       << cmd << "\' failed\n" << flush;
		  linkIsWorking[i] = 1;
		}
	      else
		{
		  cerr << "MESSAGE(Blink) : on_calLink_clicked : "
		       << "unlinked " << calFile[i] << endl << flush;
		  linkIsWorking[i]  = 0;
		  sprintf(linkFileDisp[i], "WARNING:  No video%c.cal in Main",
			  device_digit[i]);
		}
	    }
	  else if ( xwd != pwd )
	    {
	      linkIsWorking[i] = 0;
	      sprintf(linkFileDisp[i], "WARNING:  No video%c.cal in Main",
		      device_digit[i]);
	    }
	  else
	    {
	      linkIsWorking[i] = 0;
	      sprintf(linkFileDisp[i], "WARNING:  main is cal");
	    }
	}
    }

  // Show text result in main folder
  for( int i = 0; i < numDev; i++ )
    {
      gtk_text_buffer_set_text(linkedDirBuffer[i], linkFileDisp[i], -1);
    }

#ifdef VERBOSE
  if ( verbose )
    {
      cerr << "paths to cal : ";
      for( int i = 0; i < numDev; i++ )
	  cerr << calPath[i] << "/video" << i << ".cal : ";
      cerr << endl;
    }
#endif

  for( int i=0; i<2; i++ )
    if ( linkFileDisp[i] ) zaparr(linkFileDisp[i]);

}  //void on_calLink_clicked(GtkCheckButton *button, gpointer entry_ptr)

// Display mask images
void on_maskImage_realize(GtkImage *maskImage, gpointer id_ptr)
{
  MsImId msImId = (MsImId) ( (int)id_ptr); 
  g_assert(LM <= msImId && NUM_M_IMAGES > msImId );
#ifdef VERBOSE
  if ( verbose ) cout << "on_maskImage_realize : loading "
		      << mImgFile[msImId] << "\n" <<flush;
#endif

  // Load image file into image buffer
  GdkPixbuf *maskImagebuf;
  if ( mImgFileExists[msImId] )
    {
      maskImagebuf = gdk_pixbuf_new_from_file(mImgFile[msImId], NULL);
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "on_maskImage_realize : " << mImgFile[msImId]
			      << " loaded into pixel buffer\n" <<flush;
#endif
    }
  else
    {
      maskImagebuf = gdk_pixbuf_new_from_file(defMaskImage, NULL);
#ifdef VERBOSE
      if ( 1 < verbose ) cout << "on_maskImage_realize : " << defMaskImage
			      << " default image loaded into pixel buffer\n"
			      <<flush;
#endif
    }
  g_object_set(maskImage, "pixbuf", maskImagebuf, NULL);
  g_object_unref(maskImagebuf);
 
  // Display
  if ( !msImDev[msImId] )
    g_signal_emit_by_name(maskWindowL, "show");
  else
    g_signal_emit_by_name(maskWindowR, "show");
}

// Reload, load, refresh the Zone information from zone.tune
void loadZone()
{
  if ( ZON ) zap(ZON);
  ZON = new Zone(tuneFile);

  // Initialize the tune from zone.tune file
  g_object_set(spTune[BALLDIASP], "value",   (float)ZON->ballDia(), NULL);
  g_signal_emit_by_name(spTune[BALLDIASP],   "value-changed");
  g_object_set(spTune[TRIANGULATIONSCALARSP], "value", 
	       (float)ZON->triangulationScalar(), NULL);
  g_signal_emit_by_name(spTune[TRIANGULATIONSCALARSP],   "value-changed");
  g_object_set(spTune[DXZONESP], "value",   (float)ZON->dxZone(), NULL);
  g_signal_emit_by_name(spTune[DXZONESP],   "value-changed");
  g_object_set(spTune[DZZONESP], "value",   (float)ZON->dzZone(), NULL);
  g_signal_emit_by_name(spTune[DZZONESP],   "value-changed");
  g_object_set(spTune[DXDMPHZONESP], "value",   (float)ZON->dXdMphZone(), NULL);
  g_signal_emit_by_name(spTune[DXDMPHZONESP],   "value-changed");
  g_object_set(spTune[DZDMPHZONESP], "value",   (float)ZON->dZdMphZone(), NULL);
  g_signal_emit_by_name(spTune[DZDMPHZONESP],   "value-changed");
  g_object_set(spTune[VELTHRESHSP], "value",   (float)ZON->velThresh(), NULL);
  g_signal_emit_by_name(spTune[VELTHRESHSP],   "value-changed");
  g_object_set(spTune[VELMAXSP], "value",   (float)ZON->velMax(), NULL);
  g_signal_emit_by_name(spTune[VELMAXSP],   "value-changed");
  g_object_set(spTune[GRAVITYSP], "value",   (float)ZON->gravity(), NULL);
  g_signal_emit_by_name(spTune[GRAVITYSP],   "value-changed");
  g_object_set(spTune[LEFTSP], "value",   (float)ZON->left(), NULL);
  g_signal_emit_by_name(spTune[LEFTSP],   "value-changed");
  g_object_set(spTune[TOPSP], "value",   (float)ZON->top(), NULL);
  g_signal_emit_by_name(spTune[TOPSP],   "value-changed");
  g_object_set(spTune[RIGHTSP], "value",   (float)ZON->right(), NULL);
  g_signal_emit_by_name(spTune[RIGHTSP],   "value-changed");
  g_object_set(spTune[BOTTOMSP], "value",   (float)ZON->bottom(), NULL);
  g_signal_emit_by_name(spTune[BOTTOMSP],   "value-changed");
  g_object_set(spTune[SPEEDDISPLAYSCALARSP], "value",
	       (float)ZON->speedDisplayScalar(), NULL);
  g_signal_emit_by_name(spTune[SPEEDDISPLAYSCALARSP],   "value-changed");
  tuneSpinChanged = 0;
  SM.update();
}

// Reload, load, refresh the tune information from videoi.tune
void loadTune(int i)
{
#ifdef VERBOSE
  if ( verbose ) cout << "loadTune : " << i << endl << flush;
#endif
  int checkTuneExists[2]={checkLeftTuneFileExists(),checkRightTuneFileExists()};

  if ( !checkTuneExists[i] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "loadTune : file " << tunFile[i]
			  << " doesn't exist.  Returning without "
			  << "initializing.\n" << flush;
#endif
      return;
    }

  if ( TUN[i] ) zap(TUN[i]);
  TUN[i] = new Tune(tunFile[i]);
  
  // Initialize the tune from videoi.tune file
  int DI = i*(AGS1SP-AGS0SP);
  g_object_set(spTune[AGS0SP+DI], "value", (float)TUN[i]->agaussScalar(), NULL);
  g_signal_emit_by_name(spTune[AGS0SP+DI],   "value-changed");
  g_object_set(spTune[BGS0SP+DI], "value", (float)TUN[i]->bgaussScalar(), NULL);
  g_signal_emit_by_name(spTune[BGS0SP+DI],   "value-changed");
  g_object_set(spTune[MBDS0SP+DI], "value",
	       (float)TUN[i]->minBallDensityScalar(), NULL);
  g_signal_emit_by_name(spTune[MBDS0SP+DI],   "value-changed");
  g_object_set(spTune[MBAS0SP+DI], "value",
	       (float)TUN[i]->minBallAreaScalar(), NULL);
  g_signal_emit_by_name(spTune[MBAS0SP+DI],   "value-changed");
  g_object_set(spTune[XBAS0SP+DI], "value",
	       (float)TUN[i]->maxBallAreaScalar(), NULL);
  g_signal_emit_by_name(spTune[XBAS0SP+DI],   "value-changed");
  g_object_set(spTune[MBRS0SP+DI], "value",
	       (float)TUN[i]->minBallRatioScalar(), NULL);
  g_signal_emit_by_name(spTune[MBRS0SP+DI],   "value-changed");
  g_object_set(spTune[XBRS0SP+DI], "value",
	       (float)TUN[i]->maxBallRatioScalar(), NULL);
  g_signal_emit_by_name(spTune[XBRS0SP+DI],   "value-changed");
  g_object_set(spTune[MPS0SP+DI], "value",
	       (float)TUN[i]->minProxScalar(), NULL);
  g_signal_emit_by_name(spTune[MPS0SP+DI],   "value-changed");
  tunSpinChanged[i] = 0;
  SM.update();
}

// Load/refresh masks from files
void loadMask(int i)
{
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "loadMask(" << i << ")\n" << flush;
#endif

  if ( MASK[i] ) zap(MASK[i]);
  int dev  = msImDev[i];
  int nCal = CAM[dev]->imWidthCalEx();
  int mCal = CAM[dev]->imHeightCalEx();
  MASK[i]  = new MyMask(msFile[i], nCal, mCal);
  g_signal_emit_by_name(functionShowM[i], "clicked");
  
  // Initialize mask spin buttons from files loaded above
  int DMSP = SX11SP  - SXD1SP;              // Mask line interval
  int DM   = SXD1ASP - SXD1SP;              // Mask interval

  // Read from file
  float tok;
  for( int ii=0; ii<MIN(MAXMASK, MASK[i]->numLines()); ii++)
    {
      for( int jj=0; jj<DMSP; jj++)
	{
	  int index = SXD0SP+ii*DMSP+jj+i*DM;
	  tok =  atof(MASK[i]->token(ii, jj).data());
	  g_object_set(spM[index], "value", tok, NULL);
	  g_signal_emit_by_name(spM[index],   "value-changed");
	}
    }
  // Intialize the rest
  for( int ii=MIN(MAXMASK, MASK[i]->numLines()); ii<MAXMASK; ii++)
    {
      for( int jj=0; jj<DMSP; jj++)
	{
	  int index = SXD0SP+ii*DMSP+jj+i*DM;
	  switch ( index )
	    {
	    case DXD0SP:  case DYD0SP:  case VALD0SP:
	    case DXD0ASP: case DYD0ASP: case VALD0ASP:
	    case DXD0BSP: case DYD0BSP: case VALD0BSP:
	    case DXD0CSP: case DYD0CSP: case VALD0CSP:
	    case DXD1SP:  case DYD1SP:  case VALD1SP:
	    case DXD1ASP: case DYD1ASP: case VALD1ASP:
	    case DXD1BSP: case DYD1BSP: case VALD1BSP:
	    case DXD1CSP: case DYD1CSP: case VALD1CSP:
	      tok = 1.; break;
	    default:
	      tok = 0.; break;
	    }
	  g_object_set(spM[index], "value", tok, NULL);
	  g_signal_emit_by_name(spM[index],   "value-changed");
	}
    }
  maskSpinChanged[i] = 0;
  char desc[MAX_CANON];
  sprintf(desc, "%s saved", msFileName[i]);
  SM.update(desc);
}

// Load world from a camera
void loadWorld(int i)
{ 
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "loadWorld(" << i << ")\n" << flush;
#endif
  int checkCalExists[2] = {checkLeftCalFileExists(), checkLeftCalFileExists() };
  if ( checkWorldFileExists() || checkCalExists[i] )
    {
      // Initialize the world from world.dat file
      g_object_set(spWorld[PLXSP], "value",   (float)CAM[i]->Pl()[0], NULL);
      g_signal_emit_by_name(spWorld[PLXSP],   "value-changed");
      g_object_set(spWorld[PLYSP], "value",   (float)CAM[i]->Pl()[1], NULL);
      g_signal_emit_by_name(spWorld[PLYSP],   "value-changed");
      g_object_set(spWorld[PLZSP], "value",   (float)CAM[i]->Pl()[2], NULL);
      g_signal_emit_by_name(spWorld[PLZSP],   "value-changed");
      g_object_set(spWorld[PRXSP], "value",   (float)CAM[i]->Pr()[0], NULL);
      g_signal_emit_by_name(spWorld[PRXSP],   "value-changed");
      g_object_set(spWorld[PRYSP], "value",   (float)CAM[i]->Pr()[1], NULL);
      g_signal_emit_by_name(spWorld[PRYSP],   "value-changed");
      g_object_set(spWorld[PRZSP], "value",   (float)CAM[i]->Pr()[2], NULL);
      g_signal_emit_by_name(spWorld[PRZSP],   "value-changed");
      g_object_set(spWorld[OAXSP], "value",   (float)CAM[i]->OA()[0], NULL);
      g_signal_emit_by_name(spWorld[OAXSP],   "value-changed");
      g_object_set(spWorld[OAYSP], "value",   (float)CAM[i]->OA()[1], NULL);
      g_signal_emit_by_name(spWorld[OAYSP],   "value-changed");
      g_object_set(spWorld[OAZSP], "value",   (float)CAM[i]->OA()[2], NULL);
      g_signal_emit_by_name(spWorld[OAZSP],   "value-changed");
      g_object_set(spWorld[OBXSP], "value",   (float)CAM[i]->OB()[0], NULL);
      g_signal_emit_by_name(spWorld[OBXSP],   "value-changed");
      g_object_set(spWorld[OBYSP], "value",   (float)CAM[i]->OB()[1], NULL);
      g_signal_emit_by_name(spWorld[OBYSP],   "value-changed");
      g_object_set(spWorld[OBZSP], "value",   (float)CAM[i]->OB()[2], NULL);
      g_signal_emit_by_name(spWorld[OBZSP],   "value-changed");
      g_object_set(spWorld[OCXSP], "value",   (float)CAM[i]->OC()[0], NULL);
      g_signal_emit_by_name(spWorld[OCXSP],   "value-changed");
      g_object_set(spWorld[OCYSP], "value",   (float)CAM[i]->OC()[1], NULL);
      g_signal_emit_by_name(spWorld[OCYSP],   "value-changed");
      g_object_set(spWorld[OCZSP], "value",   (float)CAM[i]->OC()[2], NULL);
      g_signal_emit_by_name(spWorld[OCZSP],   "value-changed");
      worldSpinChanged = 0;
      SM.update();
    }
  else
    worldSpinChanged = 1;
}

// Load/refresh camera data from files
void loadCam(int i)
{ 
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "loadCam(" << i << ")\n" << flush;
#endif
  int checkCamExists[2] = {checkLeftCamFileExists(), checkLeftCamFileExists() };
  int checkDatExists[2] = {checkLeftDatFileExists(), checkLeftDatFileExists() };
  int checkCalExists[2] = {checkLeftCalFileExists(), checkLeftCalFileExists() };
  if ( CAL[i] ) zap(CAL[i]);
  if ( CAM[i] ) zap(CAM[i]);
  if ( ( checkCamExists[i] && checkDatExists[i] &&  checkWorldFileExists() ) ||
       !checkCalExists[i] )
    {
      cerr << "MESSAGE(Blink) : loadCam : " << i << " : loading " << camFile[i]
	   << endl << flush;
      cerr << "MESSAGE(Blink) : loadCam : " << i << " : loading " << datFile[i]
	   << endl << flush;
      cerr << "MESSAGE(Blink) : loadCam : " << i << " : loading " << worldFile
	   << endl << flush;
      CAM[i] = new MyCamData(camFile[i], datFile[i], worldFile);
    }
  else if ( checkCalExists[i] )
    {
      cerr << "MESSAGE(Blink) : loadCam : " << i << " : loading " << calFile[i]
	   << endl << flush;
      CAM[i] = new MyCamData(calFile[i], calFile[i], calFile[i]);
    }
  int nCal = CAM[i]->imWidthCalEx();
  int mCal = CAM[i]->imHeightCalEx();
  CAL[i]   = new Calibration(calFile[i], &nCal, &mCal);
  
  // Display result
  if ( i==0 )
    {
      ostringstream oss0;
      oss0 << *CAL[0] << endl;
      runPar0 = g_strdup(oss0.str().data());
      GtkTextIter begin0, end0;
      gtk_text_buffer_set_text(runPar0Buffer, runPar0, -1);
      gtk_text_buffer_get_selection_bounds(runPar0Buffer, &begin0, &end0);
      if ( !gtk_text_iter_equal(&begin0, &end0) )
	{
	  GtkTextTag *tag = gtk_text_buffer_create_tag(runPar0Buffer,
						       "mono",
						       "family",
						       "Courier", NULL);
	  gtk_text_buffer_apply_tag(runPar0Buffer, tag, &begin0, &end0);
	}
    }
  else
    {
      ostringstream oss1;
      oss1 << *CAL[1] << endl;
      runPar1 = g_strdup(oss1.str().data());
  
      GtkTextIter begin1, end1;
      gtk_text_buffer_set_text(runPar1Buffer, runPar1, -1);
      gtk_text_buffer_get_selection_bounds(runPar1Buffer, &begin1, &end1);
      if ( !gtk_text_iter_equal(&begin1, &end1) )
	{
	  GtkTextTag *tag = gtk_text_buffer_create_tag(runPar1Buffer,
						       "mono",
						       "family",
						       "Courier", NULL);
	  gtk_text_buffer_apply_tag(runPar1Buffer, tag, &begin1, &end1);
	}
    }
  
  // Initialize Aberration and image sizes from .cam file
  int DI = i*(IMW1SP - IMW0SP);
  g_object_set(spAb[IMW0SP+DI],"value",(float)CAM[i]->imWidthCalEx(),NULL);
  g_signal_emit_by_name(spAb[IMW0SP+DI], "value-changed");
  g_object_set(spAb[IMH0SP+DI],"value",(float)CAM[i]->imHeightCalEx(),NULL);
  g_signal_emit_by_name(spAb[IMH0SP+DI], "value-changed");
  g_object_set(spAb[FOC0SP+DI], "value", CAM[i]->focalCalEx(), NULL);
  g_signal_emit_by_name(spAb[FOC0SP+DI], "value-changed");
  g_object_set(spAb[ABC0SP+DI], "value", CAM[i]->AbConstant(), NULL);
  g_signal_emit_by_name(spAb[ABC0SP+DI], "value-changed");
  g_object_set(spAb[ABR0SP+DI], "value", CAM[i]->Abrqw2(), NULL);
  g_signal_emit_by_name(spAb[ABR0SP+DI], "value-changed");
  g_object_set(spAb[ABI0SP+DI], "value", CAM[i]->AbipimR(), NULL);
  g_signal_emit_by_name(spAb[ABI0SP+DI], "value-changed");
  g_object_set(spAb[ABJ0SP+DI], "value", CAM[i]->AbjpjmR(), NULL);
  g_signal_emit_by_name(spAb[ABJ0SP+DI], "value-changed");
  g_object_set(spAb[ARC0SP+DI], "value", CAM[i]->ARC(), NULL);
  g_signal_emit_by_name(spAb[ARC0SP+DI], "value-changed");
  g_object_set(spAb[SY0SP+DI], "value",  CAM[i]->Sy(), NULL);
  g_signal_emit_by_name(spAb[SY0SP+DI], "value-changed");
  if ( ( checkCamExists[i] && checkDatExists[i] &&  checkWorldFileExists() ) ||
       checkCalExists[i] )
    {
      abSpinChanged[i] = 0;
      SM.update();
  
      // Initialize calibration ball pixel locations from .dat file
      int DO = i*(OAP01SP - OAP00SP);
      g_object_set(spDat[OAP00SP+DO], "value", CAM[i]->OAP()[0], NULL);
      g_signal_emit_by_name(spDat[OAP00SP+DO], "value-changed");
      g_object_set(spDat[OAP10SP+DO], "value", CAM[i]->OAP()[1], NULL);
      g_signal_emit_by_name(spDat[OAP10SP+DO], "value-changed");
      g_object_set(spDat[OBP00SP+DO], "value", CAM[i]->OBP()[0], NULL);
      g_signal_emit_by_name(spDat[OBP00SP+DO], "value-changed");
      g_object_set(spDat[OBP10SP+DO], "value", CAM[i]->OBP()[1], NULL);
      g_signal_emit_by_name(spDat[OBP10SP+DO], "value-changed");
      g_object_set(spDat[OCP00SP+DO], "value", CAM[i]->OCP()[0], NULL);
      g_signal_emit_by_name(spDat[OCP00SP+DO], "value-changed");
      g_object_set(spDat[OCP10SP+DO], "value", CAM[i]->OCP()[1], NULL);
      g_signal_emit_by_name(spDat[OCP10SP+DO], "value-changed");
      datSpinChanged[i] = 0;
      SM.update();
    }
  else
    {
      abSpinChanged[i]  = 1;
      datSpinChanged[i] = 1;
    }
}

// Initialize/refresh the main application
void on_app_realize(GnomeApp *app, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_app_realize\n" << flush;
#endif

  // Check and set file paths
  checkSetFilePaths(pwd);
  checkSetFilePaths(xwd);
  checkSetPathMainMask(pwd);

  // Load files and initialize spin buttons
#ifdef VERBOSE
  if ( 3 < verbose ) cout << "on_app_realize : " << __LINE__ << endl << flush;
#endif
  for( int i=0;   i<numDev;       i++ ) loadCam(i);
  for( int i=0;   i<numDev;       i++ ) loadTune(i);
  loadWorld(0);
  for( int i=LM; i<NUM_M_IMAGES; i++ ) loadMask(i);
  loadZone();

  // Try to link calibration files to PWD
  g_object_get(calLink, "active", &calLinked, NULL);
  if ( !calLinked ) g_signal_emit_by_name(calLink, "clicked");

  gtk_text_buffer_set_text(mainDirBuffer, pwd, -1);
  gtk_text_buffer_set_text(calDirBuffer,  xwd, -1);

  // Display setup menu if incomplete
  if ( !SM.complete() )
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
    
}

// Change the main directory
void on_mainDirEntry_activate(GnomeFileEntry *file_widget, gpointer app_ptr)
{
  gchar           *ans;
  gint             status = 0;
  GnomeApp        *app    = GNOME_APP(app_ptr);
  GnomeVFSHandle  *fd;
  GnomeVFSFileSize bytesWritten;
  GnomeVFSResult   result;
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDirEntry_activate\n" << flush;
#endif

  ans = gnome_file_entry_get_full_path(file_widget, FALSE);
  if ( ans )
    {
      pwd = g_strdup( ans );
      if ( 0 > ( status = chdir(pwd)) )
	{
	  cerr << "ERROR(blink) : on_mainDirEntry_activate : " 
	       << "couldn't open directory " << pwd 
	       << endl << flush;
	}
      g_object_get(calLink, "active", &calLinked, NULL);
      if ( calLinked ) g_signal_emit_by_name(calLink, "clicked");

      // get the pwd 
      if ( !(pwd = getcwd(pwd, MAX_CANON)) )
	{
	  cerr << "ERROR(blink) : on_mainDirEntry_activate : "
	       << "on_mainDirEntry_activate :  couldn't open directory "
	       << pwd << endl << flush;
	  exit(1);
	}
      cerr << "MESSAGE(Blink) : Working directory " << pwd << endl << flush;
      g_signal_emit_by_name(app, "realize");

      // create and open path file URI
      result = gnome_vfs_create_uri(&fd, configFileUri,
				GNOME_VFS_OPEN_WRITE, FALSE, permissions); 
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
      
      // write pwd into path file descriptor
      result = gnome_vfs_write(fd, pwd, strlen(pwd)+1, &bytesWritten);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
      
      // close path file descriptor
      result = gnome_vfs_close(fd);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, configFileName);
	}
    }
  else
    {
      cerr << "ERROR(Blink) : on_mainDirEntry_activate : " << __FILE__ 
	   << __LINE__ << "Dir does not exist\n" << flush;
    }

  // Check not  home and update
#ifdef VERBOSE
  if ( verbose ) cout << "MESSAGE(on_mainDirEntry_activate) : SM.update()\n"
		      << flush;
#endif
  SM.update();
  g_free(ans);
}

// Change the calibration directory
void on_calDirEntry_activate(GnomeFileEntry *file_widget, gpointer app_ptr)
{
  gchar           *ans;
  GnomeApp        *app    = GNOME_APP(app_ptr);
  GnomeVFSHandle  *fd;
  GnomeVFSFileSize bytesWritten;
  GnomeVFSResult   result;
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDirEntry_activate\n" << flush;
#endif

  ans = gnome_file_entry_get_full_path(file_widget, FALSE);
  if ( ans )
    {
      xwd = g_strdup( ans );
      cerr << "Cal directory " <<  xwd << endl << flush;
#ifdef VERBOSE
      if ( verbose ) cout << "MESSAGE(on_calDirEntry_activate) : SM.update()\n"
			  << flush;
#endif
      g_signal_emit_by_name(app, "realize");
      SM.update();

      // create and open path file URI
      result = gnome_vfs_create_uri(&fd, xonfigFileUri,
				GNOME_VFS_OPEN_WRITE, FALSE, permissions); 
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}
      
      // write xwd into path file descriptor
      result = gnome_vfs_write(fd, xwd, strlen(xwd)+1, &bytesWritten);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}

      // Break link
      g_object_get(calLink, "active", &calLinked, NULL);
      if ( calLinked ) g_signal_emit_by_name(calLink, "clicked");
      
      // Reset Black and White buttons
      for ( int snap=0; snap<NUM_STILL_BUTTONS; snap++)
	blackWhiteDone[snap] = 0;
      BLACKWHITE.update();

      // close path file descriptor
      result = gnome_vfs_close(fd);
      if ( GNOME_VFS_OK != result )
	{
	  printError(result, xonfigFileName);
	}
    }
  else
    {
      cerr << "ERROR(on_calDirEntry_activate) :  Dir does not exist\n" << flush;
    }

  g_free(ans);
}

// Initiate dialog for changing main directory
void on_mainDir_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDir_clicked\n" << flush;
#endif

  // zone.tune
  if ( !SM.complete("zone.tune saved") && !qsvAsked[QSVTUNE] && tuneSpinChanged)
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_mainDir_clicked : zone.tune not saved\n" << flush;
#endif
      qsvAsked[QSVTUNE] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVTUNE]));
      return;
    }

  // left tune
  if ( !SM.complete("left tune saved") && !qsvAsked[QSVLTUNE] &&
       tunSpinChanged[0] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_mainDir_clicked : video0.tune not saved\n" << flush;
#endif
      qsvAsked[QSVLTUNE] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLTUNE]));
      return;
    }

  // right tune
  if ( !SM.complete("right tune saved") && !qsvAsked[QSVRTUNE] &&
       tunSpinChanged[1] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_mainDir_clicked : video1.tune not saved\n" << flush;
#endif
      qsvAsked[QSVRTUNE] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRTUNE]));
      return;
    }

  // left default mask 
  if ( !SM.complete("video0.mask saved") && !qsvAsked[QSVLDM] &&
       maskSpinChanged[LM] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_mainDir_clicked : video0.mask not saved\n" << flush;
#endif
      qsvAsked[QSVLDM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLDM]));
      return;
    }

  // right default mask
  if ( !SM.complete("video1.mask saved") && !qsvAsked[QSVRDM] &&
       maskSpinChanged[RM] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_mainDir_clicked : video1.mask not saved\n" << flush;
#endif
      qsvAsked[QSVRDM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRDM]));
      return;
    }

  g_signal_emit_by_name(entry, "show");
}

// Initiate dialog for changing calibration directory
void on_calDir_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDir_clicked\n" << flush;
#endif

  // Ask if want to save .dat and .cam before change directory
  if ( !SM.complete("world.dat saved") && !qsvAsked[QSVWORLD] &&
    worldSpinChanged )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_calDir_clicked : world.dat not saved\n"
			  << flush;
#endif
      qsvAsked[QSVWORLD] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVWORLD]));
      return;
    }

  // left aberrataion
  if ( !SM.complete("left aberration saved") && !qsvAsked[QSVLCAM] &&
       abSpinChanged[0] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_calDir_clicked : video0.cam not saved\n"
			  << flush;
#endif
      qsvAsked[QSVLCAM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLCAM]));
      return;
    }

  // right aberration
  if ( !SM.complete("right aberration saved") && !qsvAsked[QSVRCAM] &&
       abSpinChanged[1] )
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_calDir_clicked : video1.cam not saved\n"
			  << flush;
#endif
      qsvAsked[QSVRCAM] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRCAM]));
      return;
    }

  // left pixel data
  if ( !SM.complete("left pixel data saved") && !qsvAsked[QSVLDAT] &&
       datSpinChanged[0])
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_calDir_clicked : video0.dat not saved\n"
			  << flush;
#endif
      qsvAsked[QSVLDAT] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVLDAT]));
      return;
    }

  // right pixel data
  if ( !SM.complete("right pixel data saved") && !qsvAsked[QSVRDAT] &&
       datSpinChanged[1])
    {
#ifdef VERBOSE
      if ( verbose ) cout << "on_calDir_clicked : video1.dat not saved\n"
			  << flush;
#endif
      qsvAsked[QSVRDAT] = 1;
      gtk_widget_show_all(GTK_WIDGET(qsv[QSVRDAT]));
      return;
    }

  g_signal_emit_by_name(entry, "show");
}

// Show calculator
void on_calcButton_clicked(GtkButton *button, gpointer calculator_ptr)
{
  GtkWindow *calculator = GTK_WINDOW(calculator_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calcButton_clicked\n" << flush;
#endif
  g_signal_emit_by_name(calculator, "show");
}

// Apply changes to inputs resulting in video?.tune files
void on_saveT0_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_saveT0_clicked\n" << flush;
#endif

  // Save the data
  if ( checkLeftTuneFileExists() )
    {
      gtk_widget_show_all(GTK_WIDGET(qow[QOWLTUNE]));
    }
  else saveTn(0);
}
void on_saveT1_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_saveT1_clicked\n" << flush;
#endif

  // Save the data
  // Save the data
  if ( checkRightTuneFileExists() )
    {
      gtk_widget_show_all(GTK_WIDGET(qow[QOWRTUNE]));
    }
  else saveTn(1);
}

// Apply changes to inputs resulting in new .cal files
void on_refRunPar0_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_refRunPar0_clicked\n" << flush;
#endif

  // Save the data
  if ( checkLeftCamFileExists() )
    gtk_widget_show_all(GTK_WIDGET(qow[QOWLCAM]));
  else saveAb(0);

  if ( checkLeftDatFileExists() )
    gtk_widget_show_all(GTK_WIDGET(qow[QOWLDAT]));
  else saveDat(0);
}
void on_refRunPar1_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_refRunPar1_clicked\n" << flush;
#endif

  // Save the data
  if ( checkRightCamFileExists() )
    gtk_widget_show_all(GTK_WIDGET(qow[QOWRCAM]));
  else saveAb(1);

  if ( checkRightDatFileExists() )
    gtk_widget_show_all(GTK_WIDGET(qow[QOWRDAT]));
  else saveDat(1);
}

// Run calibration
void on_recalibrate_clicked(GtkButton *button, gpointer app_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_recalibrate_clicked\n" << flush;
#endif

  GnomeApp *app    = GNOME_APP(app_ptr);
  recalibrate(app);

  return;
}

// Run sliceraw from fake button
void on_sliceraw_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_sliceraw_clicked\n" << flush;
#endif
  sliceraw();
  return;
}

// Run calibration from fake button
void on_endend_clicked(GtkButton *button, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_endend_clicked\n" << flush;
#endif
  endEnd();
  return;
}

// Run calibration
void recalibrate(GnomeApp *app)
{
#ifdef VERBOSE
  if ( verbose ) cout << "recalibrate\n" << flush;
#endif
  gint      status = 0;
  int checkCalExists[2] = {checkLeftCalFileExists(), checkLeftCalFileExists() };

  // Change directory to cal
  if ( 0 > (status = chdir(xwd)) )
    {
      cerr << "ERROR(blink) : recalibrate : couldn't open directory " << xwd
	   << endl << flush;
      return;
    }

  // Calibrate, create .cal file
  if ( 0 != Execute("extCal", 0) )
    cerr << "ERROR(Blink) :  recalibrate : exec of \'extCal\' failed\n" << flush;

  for( int i = 0; i<numDev; i++ )
    {
      if ( checkCalExists[i] ) 
	cout << "MESSAGE(Blink) : recalibrate : wrote " << calFile[i]
	     << endl << flush;
    }
  
  // Change back to pwd
  if ( 0 > (status = chdir(pwd)) )
    {
      cerr << "ERROR(blink) : recalibrate : couldn't open directory " << pwd
	   << endl << flush;
      return;
    }

  // re-display
  g_signal_emit_by_name(app, "realize");

  // update check buttons
#ifdef VERBOSE
  if ( verbose ) cout << "MESSAGE(recalibrate) : SM.update()\n"
		      << flush;
#endif
  SM.update();
}

// Run sliceraw
void sliceraw()
{
#ifdef VERBOSE
  if ( verbose ) cout << "sliceraw\n" << flush;
#endif

  gchar  *cmd;
  gint      status = 0;

  // Change directory to cal
  if ( 0 > (status = chdir(xwd)) )
    {
      cerr << "ERROR(blink) : sliceraw : couldn't open directory " << xwd
	   << endl << flush;
      return;
    }
  
  // Calibrate, create .cal file
  cmd = g_strdup_printf("rm -f cal?.???");
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	 << flush;

  float endTime = 1.5 + (float(NUMFRAMES)*2.0)/float(FPS);
  for( int i=STILL_WHITEA; i<NUM_STILL_BUTTONS/2; i++)
    {

      // make left ball short
      cmd = g_strdup_printf("sliceraw -iballL%s.raw -oballL%ss.raw \
                            -t1.5:%4.2f -T%d -v%d 2>&1",
			    stSnFileName[i], stSnFileName[i], endTime,
			    NUMFRAMES, verbose);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	       << flush;
	  return;
	}

      // make left blank short
      cmd = g_strdup_printf("sliceraw -iblankL%s.raw -oblankL%ss.raw \
                            -t1.5:%4.2f -T%d -v%d 2>&1",
			    stSnFileName[i], stSnFileName[i], endTime,
			    NUMFRAMES, verbose);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	       << flush;
	  return;
	}

      // shuffle left ball and blank
      cmd = g_strdup_printf("sliceraw -iballL%ss.raw -iblankL%ss.raw \
                            -ocalL%s.raw -v%d 2>&1",
			    stSnFileName[i], stSnFileName[i], stSnFileName[i],
			    verbose);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	       << flush;
	  return;
	}

      // make right ball short
      cmd = g_strdup_printf("sliceraw -iballR%s.raw -oballR%ss.raw \
                             -t1.5:%4.2f -T%d -v%d 2>&1",
			    stSnFileName[i], stSnFileName[i], endTime,
			    NUMFRAMES, verbose);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	       << flush;
	  return;
	}

      // make right blank short
      cmd = g_strdup_printf("sliceraw -iblankR%s.raw -oblankR%ss.raw \
                             -t1.5:%4.2f -T%d -v%d 2>&1",
			    stSnFileName[i], stSnFileName[i], endTime,
			    NUMFRAMES, verbose);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	       << flush;
	  return;
	}

      // shuffle right ball and blank
      cmd = g_strdup_printf("sliceraw -iballR%ss.raw -iblankR%ss.raw \
                             -ocalR%s.raw -v%d 2>&1",
			    stSnFileName[i], stSnFileName[i], stSnFileName[i],
			    verbose);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	       << flush;
	  return;
	}
    }

  // Shuffle left A and B
  cmd = g_strdup_printf("sliceraw -c -icalLA.raw -icalLB.raw \
                               -ocalLAB.raw -v%d 2>&1", verbose);
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  // shuffle left AB with left C
  cmd = g_strdup_printf("sliceraw -c -icalLAB.raw -icalLC.raw \
                               -ocalL.raw -v%d 2>&1", verbose);
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }
  
  // save left ABC
  cmd = g_strdup_printf("mv calL.raw cal0.raw");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  cmd = g_strdup_printf("mv calL.set cal0.set");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  cmd = g_strdup_printf("mv calL.tim cal0.tim");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  // Shuffle right A and B
  cmd = g_strdup_printf("sliceraw -c -icalRA.raw -icalRB.raw \
                               -ocalRAB.raw -v%d 2>&1", verbose);
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  // shuffle right AB with left C
  cmd = g_strdup_printf("sliceraw -c -icalRAB.raw -icalRC.raw \
                               -ocalR.raw -v%d 2>&1", verbose);
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  // save right ABC
  cmd = g_strdup_printf("mv calR.raw cal1.raw");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  cmd = g_strdup_printf("mv calR.set cal1.set");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  cmd = g_strdup_printf("mv calR.tim cal1.tim");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  cmd = g_strdup_printf("pwd");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) :  sliceraw : exec of \'" << cmd << "\' failed\n"
	   << flush;
      return;
    }

  sprintf(calLFile,     "%s/%s", xwd, calLRawName);
  sprintf(calRFile,     "%s/%s", xwd, calRRawName);

  if ( checkCalLExists() ) 
    cout << "MESSAGE(Blink) : sliceraw : wrote " << calLFile << endl << flush;

  if ( checkCalRExists() ) 
    cout << "MESSAGE(Blink) : sliceraw : wrote " << calRFile << endl << flush;

  SM.update("sliceraw run");
  
  // Change back to pwd
  if ( 0 > (status = chdir(pwd)) )
    {
      cerr << "ERROR(blink) : sliceraw : couldn't open directory " << pwd
	   << endl << flush;
      return;
    }

  // Update to get check buttons to update
#ifdef VERBOSE
  if ( verbose ) cout << "MESSAGE(Blink) : sliceraw : SM.update()\n"
		      << flush;
#endif
  SM.update();
}

// Check calibration end to end.  Needs all 6 masks, video*.cal, world.dat
void endEnd()
{
#ifdef VERBOSE
  if ( verbose ) cout << "endEnd\n" << flush;
#endif
  gint      status = 0;

  // Change directory to cal
  if ( 0 > (status = chdir(xwd)) )
    {
      cerr << "ERROR(blink) : endEnd : couldn't open directory " << xwd
	   << endl << flush;
      return;
    }

  // run end end check
  gchar  *cmd;

  // Synchronize time in data files
  cmd = g_strdup_printf("cp -f cal0.tim cal1.tim");
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd << "\' failed\n" << flush;

  // Save master masks
  for ( int i=0; i<numDev; i++ )
    {
      cmd = g_strdup_printf("mv video%d.mask video%d.mask.sav", i, i);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) : endEnd : exec of \'" << cmd
	     << "\' failed\n" << flush;
    }

  // Loop through all three balls
  int DM=RM-LM;
  for( int i=LMA; i<=LMC; i++)
    {
      cmd = g_strdup_printf("cp -f %s video0.mask", msFileName[i]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd << "\' failed\n"
	     << flush;

      cmd = g_strdup_printf("cp -f %s video1.mask", msFileName[i+DM]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd << "\' failed\n"
	     << flush;

      cmd = g_strdup_printf("ump.x -C%d -t%d", (i-LMA)*NUMFRAMES*2,NUMFRAMES*2);
      if ( 0 != Execute(cmd, 0) )
	{
	  cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd << "\' failed\n"
	       << flush;
	}
      else if ( verbose )
	{
	  cmd = g_strdup_printf("showppms -r5");
	  if ( 0 != Execute(cmd, 0) )
	    cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd
		 << "\' failed\n" << flush;
	}
      
      cmd = g_strdup_printf("cp temp temp%s", stSnFileName[i-LMA]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd << "\' failed\n"
	     << flush;

      cmd = g_strdup_printf("grep O\%s world.dat", stSnFileName[i-LMA]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) :  endEnd : exec of \'" << cmd << "\' failed\n"
	     << flush;
    }
  
  // Restore master masks
  for ( int i=0; i<numDev; i++ )
    {
      cmd = g_strdup_printf("mv video%d.mask.sav video%d.mask", i, i);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) : endEnd : exec of \'" << cmd
	     << "\' failed\n" << flush;
    }

  // Change back to pwd
  if ( 0 > (status = chdir(pwd)) )
    {
      cerr << "ERROR(blink) : endEnd : couldn't open directory " << pwd
	   << endl << flush;
      return;
    }

}

// Measure ball positions for .dat files from cal0/1.raw using 6 masks
void measureRaw(MRawId mRawId)
{
#ifdef VERBOSE
  if ( verbose ) cout << "measureRaw(" << mRawId << ")\n" << flush;
#endif
  gint   status = 0;
  gchar *cmd;
  int checkCamExists[2] = {checkLeftCamFileExists(),checkRightCamFileExists()};
  int checkCalExists[2] = {checkLeftCalFileExists(),checkRightCalFileExists()};

  // Check for sliceraw done
  if ( !SM.complete("sliceraw run") )
  {
    showErr("You must run sliceraw before valid measurement");
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook1), 6);
    return;
  }

  // Change directory to cal
  if ( 0 > (status = chdir(xwd)) )
    {
      cerr << "ERROR(blink) : measureRaw : couldn't open directory " << xwd 
	   << endl << flush;
      return;
    }

  // Make temporary .cal files if needed
  int tempCal[2] = {0, 0};
  for ( int i=0; i<numDev; i++ )
    {
      if ( !checkCalExists[i] )
	{
	  if ( checkCamExists[i] )
	    {
	      cmd = g_strdup_printf("cp %s/video%d.cam %s/video%d.cal",
				    xwd, i, xwd, i);
	      if ( 0 != Execute(cmd, 0) )
		{
		  cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd
		       << "\' failed\n" << flush;
		  return;
		}
#ifdef VERBOSE
	      if ( verbose )
		cout << "Blink : measureRaw : made temporary "
		     << calFile[i] << ".  To be removed.\n" << flush;
#endif
	      tempCal[i] = 1;
	    }
	  else
	    {
	      cerr << "ERROR(Blink) : measureRaw : "
		   << ".cal & .cam file missing.  Quitting.\n" << flush;
	      return;
	    }
	}
    }

  // run end end check
  int device = mRawId*2/NUM_MRAW_BUTTONS;  // map to device
  cmd = g_strdup_printf("cp -f %s video%d.mask",
			msFileName[device+1+mRawId], device);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd << "\' failed\n"
	 << flush;
      
#ifdef VERBOSE
  if ( verbose )
    {
      cmd = g_strdup_printf("echo \"(Blink) : calculating device %d, mask %s\"",
			    device, msFileName[device+1+mRawId]);
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd << "\' failed\n"
	     << flush;
    }
#endif

  cmd = g_strdup_printf(">%s ump -C -ical%d.raw -w%d -t%d -v%d 2>&1",
			tempFileName[mRawId], device,
			(mRawId - device*NUM_MRAW_BUTTONS/2)*NUMFRAMES*2,
			NUMFRAMES*2, verbose);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd << "\' failed\n"
	 << flush;
  
  cmd = g_strdup_printf("grep Found %s", tempFileName[mRawId]);
  if ( 0 != Execute(cmd, 0) )
    cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd << "\' failed\n"
	 << flush;

#ifdef VERBOSE
  if ( verbose )
    {
      cmd = g_strdup_printf("showppms -r4");
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd << "\' failed\n"
	     << flush;
    }
#endif

  // Remove temporary .cal files if needed
  for ( int i=0; i<numDev; i++ )
    {
      if ( tempCal[i] )
	{
	  cmd = g_strdup_printf("rm -f %s/video%d.cal", xwd, i);
	  if ( 0 != Execute(cmd, 0) )
	    {
	      cerr << "ERROR(Blink) : measureRaw : exec of \'" << cmd
		   << "\' failed\n" << flush;
	      return;
	    }
#ifdef VERBOSE
	  if ( verbose )
	    cout << "Blink : measureRaw : removed temporary "
		 << calFile[i] << ".\n" << flush;
#endif
	}
    }

  // Change back to pwd
  if ( 0 > (status = chdir(pwd)) )
    {
      cerr << "ERROR(blink) : measureRaw : couldn't open directory " << pwd 
	   << endl << flush;
      return;
    }
}

// Apply changes in dialog to change main directory
void on_mainDirApply_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDirApply_clicked\n" << flush;
#endif
  g_signal_emit_by_name(entry, "activate");
}

// Apply changes in dialog to change calibration directory
void on_calDirApply_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDirApply_clicked\n" << flush;
#endif
  g_signal_emit_by_name(entry, "activate");
}

// Apply changes in dialog to change main directory and hide
void on_mainDirOk_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDirOk_clicked\n" << flush;
#endif
  g_signal_emit_by_name(entry, "activate");
  g_signal_emit_by_name(entry, "hide");
}

// Apply changes in dialog to change calibration directory and hide
void on_calDirOk_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDirOk_clicked\n" << flush;
#endif
  g_signal_emit_by_name(entry, "activate");
  g_signal_emit_by_name(entry, "hide");
}

// Cancel changes in dialog to change main directory and hide
void on_mainDirCancel_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDirCancel_clicked\n" << flush;
#endif
  g_signal_emit_by_name(entry, "hide");
}

// Cancel changes in dialog to change calibration directory and hide
void on_calDirCancel_clicked(GtkButton *button, gpointer entry_ptr)
{
  GnomeFileEntry *entry = GNOME_FILE_ENTRY(entry_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDirCancel_clicked\n" << flush;
#endif
  g_signal_emit_by_name(entry, "hide");
}

// Show the calculator window
void on_calculator_show(GtkWindow *calculator, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_calculator_show\n" << flush;
#endif
  gtk_widget_show_all(GTK_WIDGET(calculator));
}

// Hide the calculator window
void on_calculator_hide(GtkWindow *calculator, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_calculator_hide\n" << flush;
#endif
  gtk_widget_hide(GTK_WIDGET(calculator));
}

// Show the about
void on_about1_activate(GtkImageMenuItem *about, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_about1_activate\n" << flush;
#endif
  GnomeAbout *aboutBlink = GNOME_ABOUT(entry_ptr);
  gtk_widget_show_all(GTK_WIDGET(aboutBlink));
}

// Hide the about window
void on_aboutBlink_hide(GnomeAbout *aboutBlink, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_aboutBlink_hide\n" << flush;
#endif
  gtk_widget_hide(GTK_WIDGET(aboutBlink));
}

// Hide the about window
void on_about1_hide(GtkImageMenuItem *about1, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_about1_hide\n" << flush;
#endif
  GnomeAbout *aboutBlink = GNOME_ABOUT(entry_ptr);  
  gtk_widget_hide(GTK_WIDGET(aboutBlink));
}

// Show the graphical masks window
void on_showM0_clicked(GtkButton *showM0, gpointer m0_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_showM0_clicked\n" << flush;
#endif
  GtkWindow *maskWindowL = GTK_WINDOW(m0_ptr);
  gtk_widget_hide(GTK_WIDGET(maskWindowL));
  gtk_widget_show_all(GTK_WIDGET(maskWindowL));
}

// Show the graphical masks window
void on_showM1_clicked(GtkButton *showM1, gpointer m1_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_showM1_clicked\n" << flush;
#endif
  GtkWindow *maskWindowR = GTK_WINDOW(m1_ptr);
  gtk_widget_hide(GTK_WIDGET(maskWindowR));
  gtk_widget_show_all(GTK_WIDGET(maskWindowR));
}

// Show the graphical masks window
void on_maskWindowL_show(GtkWindow *maskWindowL, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_maskWindowL_show\n" << flush;
#endif
  gtk_widget_show_all(GTK_WIDGET(maskWindowL));
}

// Hide the graphical masks window
void on_maskWindowL_hide(GtkWindow *maskWindowL, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_maskWindowL_hide\n" << flush;
#endif
  gtk_widget_hide(GTK_WIDGET(maskWindowL));
}

// Show the graphical masks window
void on_maskWindowR_show(GtkWindow *maskWindowR, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_maskWindowR_show\n" << flush;
#endif
  gtk_widget_show_all(GTK_WIDGET(maskWindowR));
}

// Hide the graphical masks window
void on_maskWindowR_hide(GtkWindow *maskWindowR, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_maskWindowR_hide\n" << flush;
#endif
  gtk_widget_hide(GTK_WIDGET(maskWindowR));
}

// Show the main directory window
void on_mainDirEntry_show(GtkButton *file_widget, gpointer changer_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDirEntry_show\n" << flush;
#endif
  GtkDialog *changer = GTK_DIALOG(changer_ptr);
  gtk_widget_show_all(GTK_WIDGET(changer));
}

// Hide the main directory window
void on_mainDirEntry_hide(GnomeFileEntry *file_widget, gpointer changer_ptr)
{
  GtkDialog *changer = GTK_DIALOG(changer_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_mainDirEntry_hide\n" << flush;
#endif
  gtk_widget_hide(GTK_WIDGET(changer));
}

// Show the calibration directory window
void on_calDirEntry_show(GtkButton *file_widget, gpointer changer_ptr)
{
  GtkDialog *changer = GTK_DIALOG(changer_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDirEntry_show\n" << flush;
#endif
  gtk_widget_show_all(GTK_WIDGET(changer));
}

// Hide the calibration directory window
void on_calDirEntry_hide(GnomeFileEntry *file_widget, gpointer changer_ptr)
{
  GtkDialog *changer = GTK_DIALOG(changer_ptr);
#ifdef VERBOSE
  if ( verbose ) cout << "on_calDirEntry_hide\n" << flush;
#endif
  gtk_widget_hide(GTK_WIDGET(changer));
}

// Utility to status a file
int checkFileExists(const char *filePath)
{
  int               fileExists = 0;
  gchar            *fileURI;
  GnomeVFSFileInfo *info;
  GnomeVFSResult    result;
 
#ifdef VERBOSE
 if ( 2 < verbose ) cout << "checkFileExists : " << filePath;
#endif
  fileURI = gnome_vfs_get_uri_from_local_path(filePath);

  info   = gnome_vfs_file_info_new();
  result = gnome_vfs_get_file_info(fileURI, info,
				   GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
  if ( GNOME_VFS_OK == result )
    {
      if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
	{
	  if ( 0 <  atoi(gnome_vfs_format_file_size_for_display(info->size)) )
	    fileExists = 1;
	}
  }
#ifdef VERBOSE
  if ( 2 < verbose ) cout << " : " << fileExists << endl << flush;
#endif
  return fileExists;
}

// Utility to compare modify times of two files
int compareFileMtime(const char *filePathNew, const char *filePathOld)
{
  time_t            fileMtimeNew  = 0;
  time_t            fileMtimeOld  = 0;
#ifdef VERBOSE
  if ( 2 < verbose ) cout << "compareFileMtime : ";
#endif

  // New file
  fileMtimeNew = getFileMtime(filePathNew);

  // Old file
  fileMtimeOld = getFileMtime(filePathOld);

  if ( fileMtimeNew < fileMtimeOld ) 
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << filePathNew << " older than " << filePathOld
			      << endl << flush;
#endif
      return 0;  // filePathNew older than filePathOld
    }
  else
    {
#ifdef VERBOSE
      if ( 2 < verbose ) cout << filePathNew << " not older than " 
			      << filePathOld << endl << flush;
#endif
      return 1;  // filePathNew newer than filePathOld
    }
}

// Utility to get the modify time of file
time_t getFileMtime(const char *filePath)
{
#ifdef VERBOSE
  if ( 2 < verbose ) cout << "getFileMtime : " << filePath 
			  << " : " << flush;
#endif
  time_t            fileMtime  = 0;
  int               fileExists = 0;
  gchar            *fileURI;
  GnomeVFSFileInfo *info;
  GnomeVFSResult    result;

  fileURI = gnome_vfs_get_uri_from_local_path(filePath);
  info   = gnome_vfs_file_info_new();
  result = gnome_vfs_get_file_info(fileURI, info,
				   GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
  if ( GNOME_VFS_OK == result )
    {
      if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
	{
	  if ( 0 <  atoi(gnome_vfs_format_file_size_for_display(info->size)) )
	    {
	      fileExists = 1;
	      if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_MTIME)
		fileMtime = info->mtime;
	    }
	}
    }
#ifdef VERBOSE
  if ( 2 < verbose ) cout << fileMtime << endl << flush;
#endif
  return fileMtime;
}

// Utility to status a file
int checkFileExistsIn(const char *fileName, const char *path)
{
  int               fileExists = 0;
  gchar             filePath[MAX_CANON];
  gchar            *fileURI;
  GnomeVFSFileInfo *info;
  GnomeVFSResult    result;

  strcpy(filePath, path);
  strcat(filePath, "/");
  strcat(filePath, fileName);
  fileURI = gnome_vfs_get_uri_from_local_path(filePath); 
#ifdef VERBOSE
  if ( 2 < verbose ) cout << "checkFileExistsIn : " << filePath << flush;
#endif

  info   = gnome_vfs_file_info_new();
  result = gnome_vfs_get_file_info(fileURI, info,
				   GNOME_VFS_FILE_INFO_GET_MIME_TYPE);
  if ( GNOME_VFS_OK == result )
    {
      if (info->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
	{
	  if ( 0 <  atoi(gnome_vfs_format_file_size_for_display(info->size)) )
	    fileExists = 1;
	}
  }
#ifdef VERBOSE
  if ( 2 < verbose ) cout << " : " << fileExists << endl << flush;
#endif
  return fileExists;
}

// Colors for masks
void assignRGB(const int request, int *RED, int *GREEN, int *BLUE)
{
  switch (request)
    {
    case 0: *RED = 255; *GREEN = 255; *BLUE =   0; break; // Yellow
    case 1: *RED = 255; *GREEN =   0; *BLUE =   0; break; // Red
    case 2: *RED =   0; *GREEN =   0; *BLUE = 255; break; // Blue
    case 3: *RED =   0; *GREEN = 255; *BLUE =   0; break; // Lime
    case 4: *RED = 255; *GREEN = 165; *BLUE =   0; break; // Orange
    case 5: *RED =  80; *GREEN =   0; *BLUE =   0; break; // Maroon 
    case 6: *RED = 255; *GREEN =   0; *BLUE = 255; break; // Fuchsia, Magenta 
    case 7: *RED =   0; *GREEN = 255; *BLUE = 255; break; // Aqua, Cyan
    case 8: *RED =   0; *GREEN =  80; *BLUE =  80; break; // Teal 
    case 9: *RED =  80; *GREEN =  80; *BLUE =   0; break; // Olive
    case 10:*RED =   0; *GREEN =  80; *BLUE =  80; break; // Navy
    case 11:*RED =   0; *GREEN =  80; *BLUE =   0; break; // Green
    case 12:*RED =  80; *GREEN =  80; *BLUE =  80; break; // Gray
    case 13:*RED = 192; *GREEN = 192; *BLUE = 192; break; // Silver
    case 14:*RED = 154; *GREEN = 205; *BLUE =  32; break; // Yellow Green
    case 15:*RED = 139; *GREEN =   0; *BLUE =   0; break; // Dark Red
    default:*RED = 255; *GREEN = 160; *BLUE = 122;        // Light Salmon
    }
}

// check and set file paths
void checkSetFilePaths(const gchar *cwd)
{
#ifdef VERBOSE
  if ( verbose ) cout << "checkSetFilePaths\n" << flush;
#endif
  // Image files
  for( int i=0; i<NUM_INT_STILL_BUTTONS; i++ )
    sprintf(stillFile[i], "%s/%s", cwd, stImName[i]);
  for( int i=LM; i<NUM_M_IMAGES; i++ )
    {
      sprintf(mImgFile[i], "%s/%s", cwd, msImFileName[i]);
      sprintf(snapFile[i], "%s/%s", cwd, msSnFileName[i]);
      if ( 0 == strcmp(cwd, pwd) )
	{
	  mImgFileExistsPWD[i]= checkFileExists(mImgFile[i]);
	  snapFileExistsPWD[i]= checkFileExists(snapFile[i]);
	}
      if ( 0 == strcmp(cwd, xwd) )
	{
	  sprintf(msFile[i], "%s/%s", cwd, msFileName[i]);
	  mImgFileExists[i]   = checkFileExists(mImgFile[i]);
	  snapFileExists[i]   = checkFileExists(snapFile[i]);
	}
    }

  // Device files
  for( int i = 0; i < numDev; i++ )
    {
      if ( 0 == strcmp(cwd, pwd) )
	{
	  sprintf(tunFile[i],    "%s/video%c.tune", cwd, device_digit[i]);
	  sprintf(calFilePWD[i], "%s/video%c.cal",  cwd, device_digit[i]);
	}
      if ( 0 == strcmp(cwd, xwd) )
	{
	  sprintf(camFile[i],    "%s/video%c.cam",  cwd, device_digit[i]);
	  sprintf(datFile[i],    "%s/video%c.dat",  cwd, device_digit[i]);
	  sprintf(calFile[i],    "%s/video%c.cal",  cwd, device_digit[i]);
	}
    }

  // Worldly files
  if ( 0 == strcmp(cwd, pwd) )
    {
      sprintf(tuneFile,  "%s/zone.tune", cwd);
    }
  if ( 0 == strcmp(cwd, xwd) )
    {
      sprintf(worldFile,   "%s/world.dat",  cwd);
      sprintf(calLFile,    "%s/%s",         cwd, calLRawName);
      sprintf(calRFile,    "%s/%s",         cwd, calRRawName);
    }
  return;
}

// check and set file paths for main mask input only
void checkSetPathMainMask(const gchar *cwd)
{
#ifdef VERBOSE
  if ( verbose ) cout << "checkSetPathMainMask\n" << flush;
#endif
  sprintf(msFile[LM], "%s/%s", cwd, msFileName[LM]);
  sprintf(msFile[RM], "%s/%s", cwd, msFileName[RM]);
  return;
}

// Start the info help system
void on_info_activate(GtkImageMenuItem *info, gpointer entry_ptr)
{
#ifdef VERBOSE
  if ( verbose ) cout << "on_info_activate\n" << flush;
#endif
  gchar *cmd;
  cmd = g_strdup_printf("startUmpInfo");
  if ( 0 != Execute(cmd, 0) )
    {
      cerr << "ERROR(Blink) : on_info_activate : exec of \'" << cmd
	   << "\' failed\n" << flush;
    }
  // Show xml  TODO:  generate xml from texi?
  if ( !gnome_help_display("ump", NULL, NULL) )
    if ( 4 < verbose )
      cerr << "ERROR(Blink) : Unspecified error opening ump.xml\n" << flush;

  return;
}

int showErr(const char *msgLabel)
{
  GtkWindow      *msgWindow;
  GtkDialog      *msg;
  GtkHBox        *msgHbox;
  GtkImage       *msgIcon;
  GtkLabel       *msgText;
  msgWindow = GTK_WINDOW(g_object_new(GTK_TYPE_WINDOW,
				      "default_height", 200,
				      "default_width", 200,
				      "border-width", 12,
				      "title", "Query",
				      NULL));
  msg = GTK_DIALOG(gtk_dialog_new_with_buttons("Error",       // title
					       msgWindow,     // parent
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       "_Acknowledge", GTK_RESPONSE_OK,
					       NULL));
  gtk_dialog_set_default_response(msg, GTK_RESPONSE_OK);

  // Hide dialog box when it returns a response
  g_signal_connect_swapped(msg, "response", G_CALLBACK(gtk_widget_hide), msg);

  // Fill dialog window:  create HBox packed with icon and text
  msgHbox = GTK_HBOX(g_object_new(GTK_TYPE_HBOX, "border-width", 8, NULL));

  msgIcon = GTK_IMAGE(g_object_new(GTK_TYPE_IMAGE,
				   "stock", GTK_STOCK_DIALOG_WARNING,
				   "icon-size", GTK_ICON_SIZE_DIALOG,
				   "xalign", 0.5, "yalign", 1.0,
				   NULL));

  gtk_box_pack_start(GTK_BOX(msgHbox), GTK_WIDGET(msgIcon), FALSE, FALSE, 0);

  msgText = GTK_LABEL(g_object_new(GTK_TYPE_LABEL,
				   "wrap", TRUE,
				   "use-markup", TRUE,
				   "label", msgLabel,
				   NULL));

  gtk_box_pack_start(GTK_BOX(msgHbox), GTK_WIDGET(msgText), TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(msg->vbox), GTK_WIDGET(msgHbox), FALSE, FALSE, 0);

  gtk_widget_show_all(GTK_WIDGET(msg));

  return(0);
}

// Dialog to remove file
void query_remove(gchar *filePath)
{
  GtkWindow      *msgWindow;
  GtkDialog      *msg;
  GtkHBox        *msgHbox;
  GtkImage       *msgIcon;
  GtkLabel       *msgText;
  char msgLabel[MAX_CANON];
  sprintf(msgLabel, "<big><b>Do you want to remove or overwrite %s?</b></big>",
	  filePath);
  msgWindow = GTK_WINDOW(g_object_new(GTK_TYPE_WINDOW,
				      "default_height", 200,
				      "default_width", 200,
				      "border-width", 12,
				      "title", "Query",
				      NULL));
  msg = GTK_DIALOG(gtk_dialog_new_with_buttons("Query",       // title
					       msgWindow,     // parent
					       GTK_DIALOG_DESTROY_WITH_PARENT,
					       "_Remove/Overwrite",
					       GTK_RESPONSE_OK,
					       "_Cancel", GTK_RESPONSE_NO,
					       NULL));

  // connect qrm_action handler to the filePath and response
  g_signal_connect(msg, "response",
		   G_CALLBACK(qrm_action), filePath);

  gtk_dialog_set_default_response(msg, GTK_RESPONSE_NO);

  // Hide dialog box when it returns a response
  g_signal_connect_swapped(msg, "response", G_CALLBACK(gtk_widget_hide), msg);

  // Fill dialog window:  create HBox packed with icon and text
  msgHbox = GTK_HBOX(g_object_new(GTK_TYPE_HBOX, "border-width", 8, NULL));

  msgIcon = GTK_IMAGE(g_object_new(GTK_TYPE_IMAGE,
				   "stock", GTK_STOCK_DIALOG_WARNING,
				   "icon-size", GTK_ICON_SIZE_DIALOG,
				   "xalign", 0.5, "yalign", 1.0,
				   NULL));

  gtk_box_pack_start(GTK_BOX(msgHbox), GTK_WIDGET(msgIcon), FALSE, FALSE, 0);

  msgText = GTK_LABEL(g_object_new(GTK_TYPE_LABEL,
				   "wrap", TRUE,
				   "use-markup", TRUE,
				   "label", msgLabel,
				   NULL));

  gtk_box_pack_start(GTK_BOX(msgHbox), GTK_WIDGET(msgText), TRUE, TRUE, 0);

  gtk_box_pack_start(GTK_BOX(msg->vbox), GTK_WIDGET(msgHbox), FALSE, FALSE, 0);

  gtk_widget_show_all(GTK_WIDGET(msg));

}

// Remove action
void qrm_action(GtkDialog *dialog, gint response, gpointer id_ptr)
{
  char *filePath = (char *) id_ptr; 
  gchar *cmd = g_strdup_printf("rm -f %s", filePath);
#ifdef VERBOSE
  if ( verbose ) cout << "qrm_action : " << filePath << endl << flush;
#endif
  switch ( response )
    {
    case GTK_RESPONSE_OK:
#ifdef VERBOSE
      if ( verbose ) cout << "qrm_action : GTK_RESPONSE_OK\n" << flush;
#endif
      
      if ( 0 != Execute(cmd, 0) ||  checkFileExists(filePath) )
	{
	  cerr << "ERROR(Blink) :  qrm_action : exec of \'" << cmd
	       << "\' failed\n" << flush;
	  return;
	}
      cerr << "MESSAGE(Blink) : qrm_action : removed " << filePath << ".\n"
	   << flush;
      
      blockQueryRemove = 1;
      g_signal_emit_by_name(app, "realize");
      blockQueryRemove = 0;
      break;
    default:
#ifdef VERBOSE
      if ( verbose ) cout << "qrm_action : GTK_RESPONSE_NO\n" << flush;
#endif
      break;
    }
}
