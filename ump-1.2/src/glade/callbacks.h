// C++-callbacks for blink functions
// DA Gutz 10-Jan-06
// 	$Id: callbacks.h,v 1.1 2006/09/15 22:47:32 davegutz Exp $ 
#ifndef callbacks_h
#define callbacks_h 

extern "C" {
 #include <gnome.h>                  // gnome
 #include <glade/glade.h>            // gnome
 #include <unistd.h>                 // gnome
 #include <libgnome/libgnome.h>      // gnome
 #include <libgnomevfs/gnome-vfs.h>  // gnome
 #include <makeargv.h>               // local execution functions
}

typedef enum
{
  BALLA, BALLB, BALLC,
  NUM_BALLS
} Ball;

typedef enum
  {
    REVERT_ZONE,    REVERT_WORLD,
    AB0_REVERT,     CAM0_REVERT,
    MASKD0_REVERT,
    MASKD0A_REVERT, MASKD0B_REVERT, MASKD0C_REVERT, 
    AB1_REVERT,     CAM1_REVERT,
    MASKD1_REVERT,
    MASKD1A_REVERT, MASKD1B_REVERT, MASKD1C_REVERT, 
    T0_REVERT,
    T1_REVERT,
    NUM_REVERT_BUTTONS
  } RevertId;

typedef enum
  {
    MRAWA0, MRAWB0, MRAWC0,
    MRAWA1, MRAWB1, MRAWC1,
    NUM_MRAW_BUTTONS
  } MRawId;

typedef enum
  {
    QRMCALLINK,
    NUM_QRM_FILES
  } QrmId;

typedef enum
  {
    QSVWORLD, QSVLCAM, QSVRCAM, QSVLDAT, QSVRDAT,
    QSVTUNE, QSVLTUNE, QSVRTUNE, QSVLDM, QSVRDM,
    NUM_QSV_FILES
  } QsvId;

typedef enum
  {
    QOWWORLD, QOWLCAM, QOWRCAM, QOWLDAT, QOWRDAT,
    QOWTUNE, QOWLTUNE, QOWRTUNE, QOWLDM, QOWRDM,
    NUM_QOW_FILES
  } QowId;

typedef enum
  {
    LM,
    LMA, LMB, LMC,
    RM,
    RMA, RMB, RMC,
    NUM_M_IMAGES
  } MsImId;

typedef enum
  {
    CHOSE_WORKING_DIRECTORY, CHOSE_CAL_DIRECTORY,
    WORLD_DAT_SAVED, GUIDE_TUNE_SAVED, 
    EXT_CAL_RUN, 
    GUIDE_D0_SAVED,
    LEFT_MA_SAVED,  LEFT_MB_SAVED,  LEFT_MC_SAVED, 
    GUIDE_D1_SAVED,
    RIGHT_MA_SAVED, RIGHT_MB_SAVED, RIGHT_MC_SAVED,
    LEFT_ABERRATION_SAVED, RIGHT_ABERRATION_SAVED,
    LEFT_PIXEL_DATA_SAVED, RIGHT_PIXEL_DATA_SAVED,
    SNAPS_DONE,
    SLICERAW_RUN,
    END_END_READY,
    LEFT_TUNE_SAVED, RIGHT_TUNE_SAVED,
    WHITEA,     WHITEB,     WHITEC, 
    BLACKA,     BLACKB,     BLACKC,
    STILLLEFT,  STILLRIGHT,
    NUM_GUIDE_BUTTONS
  } GdBuId;

typedef enum
  {
    STILL_WHITEA, STILL_WHITEB, STILL_WHITEC,
    STILL_BLACKA, STILL_BLACKB, STILL_BLACKC,
    NUM_STILL_BUTTONS
  } StId;

typedef enum
  {
    LEFT_STILL_BUTTON, RIGHT_STILL_BUTTON,
    NUM_INT_STILL_BUTTONS
  } StIntId;

typedef enum
  {
    SXD0SP,  SYD0SP,  DXD0SP,  DYD0SP,  DDXD0SP,  DDYD0SP,  VALD0SP, 
    SX10SP,  SY10SP,  DX10SP,  DY10SP,  DDX10SP,  DDY10SP,  VAL10SP, 
    SX20SP,  SY20SP,  DX20SP,  DY20SP,  DDX20SP,  DDY20SP,  VAL20SP, 
    SX30SP,  SY30SP,  DX30SP,  DY30SP,  DDX30SP,  DDY30SP,  VAL30SP, 
    SX40SP,  SY40SP,  DX40SP,  DY40SP,  DDX40SP,  DDY40SP,  VAL40SP, 
    SX50SP,  SY50SP,  DX50SP,  DY50SP,  DDX50SP,  DDY50SP,  VAL50SP, 
    SXD0ASP, SYD0ASP, DXD0ASP, DYD0ASP, DDXD0ASP, DDYD0ASP, VALD0ASP, 
    SX10ASP, SY10ASP, DX10ASP, DY10ASP, DDX10ASP, DDY10ASP, VAL10ASP, 
    SX20ASP, SY20ASP, DX20ASP, DY20ASP, DDX20ASP, DDY20ASP, VAL20ASP, 
    SX30ASP, SY30ASP, DX30ASP, DY30ASP, DDX30ASP, DDY30ASP, VAL30ASP, 
    SX40ASP, SY40ASP, DX40ASP, DY40ASP, DDX40ASP, DDY40ASP, VAL40ASP, 
    SX50ASP, SY50ASP, DX50ASP, DY50ASP, DDX50ASP, DDY50ASP, VAL50ASP, 
    SXD0BSP, SYD0BSP, DXD0BSP, DYD0BSP, DDXD0BSP, DDYD0BSP, VALD0BSP, 
    SX10BSP, SY10BSP, DX10BSP, DY10BSP, DDX10BSP, DDY10BSP, VAL10BSP, 
    SX20BSP, SY20BSP, DX20BSP, DY20BSP, DDX20BSP, DDY20BSP, VAL20BSP, 
    SX30BSP, SY30BSP, DX30BSP, DY30BSP, DDX30BSP, DDY30BSP, VAL30BSP, 
    SX40BSP, SY40BSP, DX40BSP, DY40BSP, DDX40BSP, DDY40BSP, VAL40BSP, 
    SX50BSP, SY50BSP, DX50BSP, DY50BSP, DDX50BSP, DDY50BSP, VAL50BSP, 
    SXD0CSP, SYD0CSP, DXD0CSP, DYD0CSP, DDXD0CSP, DDYD0CSP, VALD0CSP, 
    SX10CSP, SY10CSP, DX10CSP, DY10CSP, DDX10CSP, DDY10CSP, VAL10CSP, 
    SX20CSP, SY20CSP, DX20CSP, DY20CSP, DDX20CSP, DDY20CSP, VAL20CSP, 
    SX30CSP, SY30CSP, DX30CSP, DY30CSP, DDX30CSP, DDY30CSP, VAL30CSP, 
    SX40CSP, SY40CSP, DX40CSP, DY40CSP, DDX40CSP, DDY40CSP, VAL40CSP, 
    SX50CSP, SY50CSP, DX50CSP, DY50CSP, DDX50CSP, DDY50CSP, VAL50CSP,
    SXD1SP,  SYD1SP,  DXD1SP,  DYD1SP,  DDXD1SP,  DDYD1SP,  VALD1SP, 
    SX11SP,  SY11SP,  DX11SP,  DY11SP,  DDX11SP,  DDY11SP,  VAL11SP, 
    SX21SP,  SY21SP,  DX21SP,  DY21SP,  DDX21SP,  DDY21SP,  VAL21SP, 
    SX31SP,  SY31SP,  DX31SP,  DY31SP,  DDX31SP,  DDY31SP,  VAL31SP, 
    SX41SP,  SY41SP,  DX41SP,  DY41SP,  DDX41SP,  DDY41SP,  VAL41SP, 
    SX51SP,  SY51SP,  DX51SP,  DY51SP,  DDX51SP,  DDY51SP,  VAL51SP, 
    SXD1ASP, SYD1ASP, DXD1ASP, DYD1ASP, DDXD1ASP, DDYD1ASP, VALD1ASP, 
    SX11ASP, SY11ASP, DX11ASP, DY11ASP, DDX11ASP, DDY11ASP, VAL11ASP, 
    SX21ASP, SY21ASP, DX21ASP, DY21ASP, DDX21ASP, DDY21ASP, VAL21ASP, 
    SX31ASP, SY31ASP, DX31ASP, DY31ASP, DDX31ASP, DDY31ASP, VAL31ASP, 
    SX41ASP, SY41ASP, DX41ASP, DY41ASP, DDX41ASP, DDY41ASP, VAL41ASP, 
    SX51ASP, SY51ASP, DX51ASP, DY51ASP, DDX51ASP, DDY51ASP, VAL51ASP, 
    SXD1BSP, SYD1BSP, DXD1BSP, DYD1BSP, DDXD1BSP, DDYD1BSP, VALD1BSP, 
    SX11BSP, SY11BSP, DX11BSP, DY11BSP, DDX11BSP, DDY11BSP, VAL11BSP, 
    SX21BSP, SY21BSP, DX21BSP, DY21BSP, DDX21BSP, DDY21BSP, VAL21BSP, 
    SX31BSP, SY31BSP, DX31BSP, DY31BSP, DDX31BSP, DDY31BSP, VAL31BSP, 
    SX41BSP, SY41BSP, DX41BSP, DY41BSP, DDX41BSP, DDY41BSP, VAL41BSP, 
    SX51BSP, SY51BSP, DX51BSP, DY51BSP, DDX51BSP, DDY51BSP, VAL51BSP, 
    SXD1CSP, SYD1CSP, DXD1CSP, DYD1CSP, DDXD1CSP, DDYD1CSP, VALD1CSP, 
    SX11CSP, SY11CSP, DX11CSP, DY11CSP, DDX11CSP, DDY11CSP, VAL11CSP, 
    SX21CSP, SY21CSP, DX21CSP, DY21CSP, DDX21CSP, DDY21CSP, VAL21CSP, 
    SX31CSP, SY31CSP, DX31CSP, DY31CSP, DDX31CSP, DDY31CSP, VAL31CSP, 
    SX41CSP, SY41CSP, DX41CSP, DY41CSP, DDX41CSP, DDY41CSP, VAL41CSP, 
    SX51CSP, SY51CSP, DX51CSP, DY51CSP, DDX51CSP, DDY51CSP, VAL51CSP, 
    NUM_M_BUTTONS
  } SpMId;

typedef enum
  {
    NSP,  MSP,  SXSP, SYSP, DXSP, DYSP, X0SP, Y0SP, X1SP, Y1SP, DNSP, DMSP,
    NUM_CA_BUTTONS
  } SpCaId;

typedef enum
  {
    IMW0SP, IMH0SP, FOC0SP, ABC0SP, ABR0SP, ABI0SP, ABJ0SP, ARC0SP, SY0SP,
    IMW1SP, IMH1SP, FOC1SP, ABC1SP, ABR1SP, ABI1SP, ABJ1SP, ARC1SP, SY1SP,
    NUM_AB_BUTTONS
  } SpAbId;

typedef enum
  {
    OAP00SP, OAP10SP, OBP00SP, OBP10SP, OCP00SP, OCP10SP,
    OAP01SP, OAP11SP, OBP01SP, OBP11SP, OCP01SP, OCP11SP,
    NUM_DAT_BUTTONS
  } SpDatId;

typedef enum
  {
    PLXSP, PLYSP, PLZSP, PRXSP, PRYSP, PRZSP,
    OAXSP, OAYSP, OAZSP, OBXSP, OBYSP, OBZSP, OCXSP, OCYSP, OCZSP,
    NUM_WORLD_BUTTONS
  } SpWorldId;

typedef enum
  {
    CALC0, CALC0A, CALC0B, CALC0C, CALC1, CALC1A, CALC1B, CALC1C,
    NUM_CALC_BUTTONS
  } calcId;

typedef enum
  {
    BALLDIASP, TRIANGULATIONSCALARSP, DXZONESP, DZZONESP,
    DXDMPHZONESP, DZDMPHZONESP,
    VELTHRESHSP, VELMAXSP, GRAVITYSP, LEFTSP, TOPSP,
    RIGHTSP, BOTTOMSP, SPEEDDISPLAYSCALARSP, VERBOSESP, AUTOSP,
    AGS0SP, BGS0SP, MBDS0SP, MBAS0SP, XBAS0SP, MBRS0SP, XBRS0SP, MPS0SP,
    AGS1SP, BGS1SP, MBDS1SP, MBAS1SP, XBAS1SP, MBRS1SP, XBRS1SP, MPS1SP,
    NUM_TUNE_BUTTONS
  } SpTuneId;

typedef enum
  {
    JUMP_TO_MAIN,      MAIN_GO_GUIDE,
    JUMP_TO_CAL,       CAL_GO_GUIDE,
    JUMP_TO_WORLD,     WORLD_GO_GUIDE,
    GUIDE_GO_TUNE,     TUNE_GO_GUIDE,
    GUIDE_GO_MASK_D0,
    JUMP_TO_LEFT_MA,   LEFT_MA_GO_GUIDE,
    JUMP_TO_LEFT_MB,   LEFT_MB_GO_GUIDE,
    JUMP_TO_LEFT_MC,   LEFT_MC_GO_GUIDE,
    GUIDE_GO_MASK_D1,
    JUMP_TO_RIGHT_MA,  RIGHT_MA_GO_GUIDE,
    JUMP_TO_RIGHT_MB,  RIGHT_MB_GO_GUIDE,
    JUMP_TO_RIGHT_MC,  RIGHT_MC_GO_GUIDE,
    JUMP_TO_LEFT_ABERRATION,      LEFT_ABERRATION_GO_GUIDE,
    JUMP_TO_RIGHT_ABERRATION,     RIGHT_ABERRATION_GO_GUIDE,
    JUMP_TO_LEFT_PIXEL_DATA,
    JUMP_TO_RIGHT_PIXEL_DATA,
    JUMP_TO_SNAPS,     SNAPS_GO_GUIDE,
    JUMP_TO_LEFT_TUNE, JUMP_TO_RIGHT_TUNE,
    NUM_JUMP_BUTTONS
  } JumpId;

int  checkBallLRawExists(int ballId);
int  checkBallLRawExistsA();
int  checkBallLRawExistsB();
int  checkBallLRawExistsC();
int  checkBallRRawExists(int ballId);
int  checkBallRRawExistsA();
int  checkBallRRawExistsB();
int  checkBallRRawExistsC();
int  checkBlankLRawExists(int ballId);
int  checkBlankLRawExistsA();
int  checkBlankLRawExistsB();
int  checkBlankLRawExistsC();
int  checkBlankRRawExists(int ballId);
int  checkBlankRRawExistsA();
int  checkBlankRRawExistsB();
int  checkBlankRRawExistsC();
int  checkCalLExists();
int  checkCalRExists();
int  checkFileExists(const char *filePath);
int  checkFileExistsIn(const char *fileName, const char *path);
int  checkFileExistsInPwd(const char *fileName);
int  checkIntStillFileExistsL();
int  checkIntStillFileExistsR();
int  checkLeftCalFileExists();
int  checkLeftCalFileExistsPWD();
int  checkLeftCamFileExists();
int  checkLeftDatFileExists();
int  checkLeftTuneFileExists();
void checkMe(GdBuId gdBuId, GtkCheckButton *checkButton, int testPass,
	     const char *tag, const char *failMsg);
int  checkMFileExistsLM();
int  checkMFileExistsLMA();
int  checkMFileExistsLMB();
int  checkMFileExistsLMC();
int  checkMFileExistsRM();
int  checkMFileExistsRMA();
int  checkMFileExistsRMB();
int  checkMFileExistsRMC();
int  checkNewCals();
int  checkOldCamFile0();
int  checkOldCamFile1();
int  checkOldDatFile0();
int  checkOldDatFile1();
int  checkOldSnapFile0();
int  checkOldSnapFile0A();
int  checkOldSnapFile0B();
int  checkOldSnapFile0C();
int  checkOldSnapFile1();
int  checkOldSnapFile1A();
int  checkOldSnapFile1B();
int  checkOldSnapFile1C();
int  checkOldWorldFile();
int  checkPwdNotHome();
int  checkRawFileExists0();
int  checkRawFileExists1();
int  checkRightCalFileExists();
int  checkRightCalFileExistsPWD();
int  checkRightCamFileExists();
int  checkRightDatFileExists();
int  checkRightTuneFileExists();
int  checkTuneFileExists();
int  checkWorldFileExists();
int  checkXwdNotHome();
void checkSetFilePaths(const gchar *cwd);
void checkSetPathMainMask(const gchar *cwd);
int  compareFileMtime(const char *filePathNew, const char *filePathOld);
gint delete_event(GtkWidget *widget, GdkEvent event, gpointer data);
void end_program(GtkWidget *widget, gpointer entry_ptr);
void endEnd();
int  filesSaved();
time_t getFileMtime(const char *filePath);
void loadCam(int i);
void loadMask(int i);
void loadTune(int i);
void loadWorld(int i);
void loadZone();
void measureRaw(MRawId mRawId);
void on_about1_activate(GtkImageMenuItem *about, gpointer entry_ptr);
void on_about1_hide(GtkImageMenuItem *widget, gpointer entry_ptr);
void on_aboutBlink_hide(GnomeAbout *widget, gpointer entry_ptr);
void on_app_realize(GnomeApp *app, gpointer entry_ptr);
void on_auto_clicked(GtkButton *button, gpointer entry_ptr);
void on_calDirApply_clicked(GtkButton *button, gpointer entry_ptr);
void on_calDirCancel_clicked(GtkButton *button, gpointer entry_ptr);
void on_calDirEntry_activate(GnomeFileEntry *file_widget, gpointer entry_ptr);
void on_calDirEntry_hide(GnomeFileEntry *file_widget, gpointer entry_ptr);
void on_calDirEntry_show(GtkButton *button, gpointer entry_ptr);
void on_calDirOk_clicked(GtkButton *button, gpointer entry_ptr);
void on_calDir_clicked(GtkButton *button, gpointer entry_ptr);
void on_calLink_clicked(GtkCheckButton *button, gpointer entry_ptr);
void on_calcButton_clicked(GtkButton *button, gpointer entry_ptr);
void on_calculator_hide(GtkWindow *calculator, gpointer entry_ptr);
void on_calculator_show(GtkWindow *calculator, gpointer entry_ptr);
void on_continuous_clicked(GtkButton *saveTheWorldButton, gpointer entry_ptr);
void on_dialog_clicked(GtkButton *button, gpointer entry_ptr);
void on_endend_clicked(GtkButton *button, gpointer entry_ptr);
void on_guide_clicked(GtkCheckButton *guideButton, gpointer entry_ptr);
void on_info_activate(GtkImageMenuItem *about1, gpointer entry_ptr);
void on_intStill_clicked(GtkButton *button, gpointer entry_ptr);
void on_kill_clicked(GtkButton *button, gpointer entry_ptr);
void on_leftShowStill_clicked(GtkButton *button, gpointer entry_ptr);
void on_mainDirApply_clicked(GtkButton *button, gpointer entry_ptr);
void on_mainDirCancel_clicked(GtkButton *button, gpointer entry_ptr);
void on_mainDirEntry_activate(GnomeFileEntry *file_widget, gpointer entry_ptr);
void on_mainDirEntry_hide(GnomeFileEntry *file_widget, gpointer entry_ptr);
void on_mainDirEntry_show(GtkButton *button, gpointer entry_ptr);
void on_mainDirOk_clicked(GtkButton *button, gpointer entry_ptr);
void on_mainDir_clicked(GtkButton *button, gpointer entry_ptr);
void on_maskImage_realize(GtkImage *maskImage, gpointer entry_ptr);
void on_maskWindowL_hide(GtkWindow *maskWindowR, gpointer entry_ptr);
void on_maskWindowL_show(GtkWindow *maskWindowR, gpointer entry_ptr);
void on_maskWindowR_hide(GtkWindow *maskWindowR, gpointer entry_ptr);
void on_maskWindowR_show(GtkWindow *maskWindowR, gpointer entry_ptr);
void on_mRaw_clicked(GtkButton *mRaw, gpointer id_ptr);
void on_jump_clicked(GtkButton *present, gpointer id_ptr);
void on_plot_clicked(GtkButton *button, gpointer entry_ptr);
void on_recalibrate_clicked(GtkButton *button, gpointer app_ptr);
void on_refRunPar0_activate(GtkButton *button, gpointer entry_ptr);
void on_refRunPar0_clicked(GtkButton *button, gpointer entry_ptr);
void on_refRunPar0_clicked(GtkButton *button, gpointer entry_ptr);
void on_refRunPar1_activate(GtkButton *button, gpointer entry_ptr);
void on_refRunPar1_clicked(GtkButton *button, gpointer entry_ptr);
void on_refRunPar1_clicked(GtkButton *button, gpointer entry_ptr);
void on_replay_clicked(GtkButton *button, gpointer entry_ptr);
void on_revert_clicked(GtkButton *revert, gpointer id_ptr);
void on_rightShowStill_clicked(GtkButton *button, gpointer entry_ptr);
void on_saveT0_clicked(GtkButton *button, gpointer entry_ptr);
void on_saveT1_clicked(GtkButton *button, gpointer entry_ptr);
void on_saveM_clicked(GtkButton *button, gpointer entry_ptr);
void on_saveTheWorld_clicked(GtkButton *button, gpointer entry_ptr);
void on_saveZoneTune_clicked(GtkButton *button, gpointer entry_ptr);
void on_showM0_clicked(GtkButton *button, gpointer entry_ptr);
void on_showM1_clicked(GtkButton *button, gpointer entry_ptr);
void on_showM_clicked(GtkButton *button, gpointer entry_ptr);
void on_single_clicked(GtkButton *button, gpointer entry_ptr);
void on_sliceraw_clicked(GtkButton *button, gpointer entry_ptr);
void on_still_clicked(GtkButton *button, gpointer entry_ptr);
void on_store_clicked(GtkButton *button, gpointer entry_ptr);
int  printError(GnomeVFSResult code, const gchar *uri);
int  print_error(GnomeVFSResult code, const gchar *uri);
void recalibrate(GnomeApp *app);
void qsv_action(GtkDialog *dialog, gint response, gpointer id_ptr);
void qow_action(GtkDialog *dialog, gint response, gpointer id_ptr);
void qrm_action(GtkDialog *dialog, gint response, gpointer id_ptr);
void query_remove(char *filePath);
int  saveAb(int i);
int  saveDat(int i);
int  saveM(int i);
int  saveTn(int i);
int  saveTune();
int  saveWorld();
int  showErr(const char *msgLabel);
void sliceraw();
void spAb_changed(GtkSpinButton *spin_button, gpointer entry_ptr);
void spCa_changed(GtkSpinButton *spin_button, gpointer entry_ptr);
void spDat_changed(GtkSpinButton *spin_button, gpointer entry_ptr);
void spM_changed(GtkSpinButton *spin_button, gpointer entry_ptr);
void spTune_changed(GtkSpinButton *spin_button, gpointer entry_ptr);
void spWorld_changed(GtkSpinButton *spin_button, gpointer entry_ptr);

#endif
