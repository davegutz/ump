// Calibration file input utilities
// 	$Id: Tune.cc,v 1.2 2007/02/13 21:47:43 davegutz Exp davegutz $	
// D Gutz

// Includes section
using namespace std;
#include <common.h>
#include <math.h>      // math
#include <string>      // string classes
#include <string.h>    // strings
#include <iostream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <sys/param.h> // built-in constants and MIN/MAX
#include <assert.h>    // assertion
#include <limits.h>    // access built-in constants
#include <second.h>    // local brings in TNT stuff
#include <stdio.h>     // sprintf
#include <InFile.h>    // Input file class
#include <stringset.h> // for fileroot
#include <curve.h>     // for curvex
#include "Tune.h"
#include <zap.h>
static const double pi= 3.14159265358979323846264338327950288419716939937510;
const int   nr   = 5;         // number rows colpGrid
const int   nc   = 7;         // number cols rowpGrid

// Global variables
extern int calibrate;
extern int verbose;

////////////////////////class World////////////////////////////////////
// Constructors.
World::World() : geoFile_(""), Pl_(PixelLoc3()), Pr_(PixelLoc3()),
		 OA_(PixelLoc3()), OB_(PixelLoc3()), OC_(PixelLoc3()),
		 geoFileExists_(0) {}
World::World(const char geoFile[])
  : geoFile_(string(geoFile)), Pl_(PixelLoc3()), Pr_(PixelLoc3()),
    OA_(PixelLoc3()), OB_(PixelLoc3()), OC_(PixelLoc3()), geoFileExists_(0)
{
  // Read file
  InFile *ptrGeoFile  = new InFile("World", geoFile_.data(), 128, MAX_CANON);
  ptrGeoFile->load();
  ptrGeoFile->stripComments("#");
  ptrGeoFile->stripComments("%");
  ptrGeoFile->stripBlankLines();
  if ( !ptrGeoFile->loadVars() ) 
    {
#ifdef VERBOSE
      if ( verbose )
	cerr << "World: " << __FILE__ << " : " << __LINE__ << " : " 
	     << "unrecognized format in " << geoFile_.data() 
	     << ".\nEnter vectors as follows, one per line,"
	     << " lines in any order:\n"
	     << "'Pl  = [float float float]' 3D position left cam, in\n"
	     << "'Pr  = [float float float]' 3D position right cam, in\n"
	     << "'OA  = [float float float]' 3D position first object, in\n"
	     << "'OB  = [float float float]' 3D position second object, in\n"
	     << "'OC  = [float float float]' 3D position third object, in\n"
	     << flush;
#endif
      Pl_[0] = Pl_[1] = Pl_[2] = 0.0;
      Pr_[0] = Pr_[1] = Pr_[2] = 0.0;
      OA_[0] = OA_[1] = OA_[2] = 0.0;
      OB_[0] = OB_[1] = OB_[2] = 0.0;
      OC_[0] = OC_[1] = OC_[2] = 0.0;
    }
  else
    {
      geoFileExists_ = 1;
      GetPar(*ptrGeoFile, "Pl", &Pl_, 1);
      GetPar(*ptrGeoFile, "Pr", &Pr_, 1);
      GetPar(*ptrGeoFile, "OA", &OA_, 1);
      GetPar(*ptrGeoFile, "OB", &OB_, 1);
      GetPar(*ptrGeoFile, "OC", &OC_, 1);
    }
  zap(ptrGeoFile);
}
World::World (const World & W)
  : geoFile_(W.geoFile_), Pl_(W.Pl_), Pr_(W.Pr_), OA_(W.OA_), OB_(W.OB_),
    OC_(W.OC_), geoFileExists_(W.geoFileExists_) {}
World & World::operator=(const World & W)
{
  if ( this==&W ) return *this;  // Allows self-assignment.
  geoFile_ = W.geoFile_;
  Pl_      = W.Pl_;
  Pr_      = W.Pr_;
  OA_      = W.OA_;
  OB_      = W.OB_;
  OC_      = W.OC_;
  geoFileExists_ = W.geoFileExists_;
  return *this;                  // Allows stacked assignments.
}

// Display World
ostream & operator<< (ostream & out, const World &W)
{
  out << "\n\n% MESSAGE(World): cal for device " << W.geoFile_.data() << ":\n";
  out << "Pl=   [";
  for (int i=0; i<3; i++) out << setprecision(6) << setw(9) << W.Pl_[i] << " ";
  out << "];\n";
  out << "Pr=   [";
  for (int i=0; i<3; i++) out << setprecision(6) << setw(9) << W.Pr_[i] << " ";
  out << "];\n";
  out << "OA=   [";
  for (int i=0; i<3; i++) out << setprecision(6) << setw(9) << W.OA_[i] << " ";
  out << "];\n";
  out << "OB=   [";
  for (int i=0; i<3; i++) out << setprecision(6) << setw(9) << W.OB_[i] << " ";
  out << "];\n";
  out << "OC=   [";
  for (int i=0; i<3; i++) out << setprecision(6) << setw(9) << W.OC_[i] << " ";
  out << "];\n";

  return out;
}

////////////////////////class Data////////////////////////////////////
// Constructors.
Data::Data()  : datFile_(""), OAp_(PixelLoc3()), OBp_(PixelLoc3()),
		OCp_(PixelLoc3()), OApo_(PixelLoc3()), OBpo_(PixelLoc3()),
		OCpo_(PixelLoc3()), datFileExists_(0) {}
Data::Data(const char datFile[], const float focalPix)
  : datFile_(string(datFile)),
    OAp_(PixelLoc3()), OBp_(PixelLoc3()), OCp_(PixelLoc3()),
    OApo_(PixelLoc3()), OBpo_(PixelLoc3()), OCpo_(PixelLoc3()),
    datFileExists_(0)
{

  // Read file
  InFile *ptrDatFile  = new InFile("Data", datFile_.data(), 128, MAX_CANON);
  ptrDatFile->load();
  ptrDatFile->stripComments("#");
  ptrDatFile->stripComments("%");
  ptrDatFile->stripBlankLines();
  if ( !ptrDatFile->loadVars() ) 
    {
#ifdef VERBOSE
      if ( verbose )
	cerr << "WARNING(Tune): Data::Data :  " << __FILE__ << ": " << __LINE__
	     << " no variables in " << datFile_.data() << " or empty.\n"
	     << "Use entry format as follows:     \n"
	     << "% Auto-generated video .dat file.\n"
	     << "% Enter coordinates as detected by ump or read off\n"
	     << "of a screen shot of same resolution.  \n"
	     << "% First  ball is OAp (mask A)         \n"
	     << "% Second ball is OBp (mask B)         \n"
	     << "% Third  ball is OCp (mask C)         \n"
	     << "OAp       = [  0      0   ];  \n"
	     << "OBp       = [  0      0   ];  \n"
	     << "OCp       = [  0      0   ];  \n" << flush;
#endif
      OAp_[0] = OAp_[1] = OAp_[2] = 0.0;
      OBp_[0] = OBp_[1] = OBp_[2] = 0.0;
      OCp_[0] = OCp_[1] = OCp_[2] = 0.0;
    }
  else
    {
      datFileExists_ = 1;
      GetPar(*ptrDatFile, "OAp", &OAp_, 2, 1);
      GetPar(*ptrDatFile, "OBp", &OBp_, 2, 1);
      GetPar(*ptrDatFile, "OCp", &OCp_, 2, 1);
    }

  // Leverage to 3D with focal length
  OAp_[2] = OBp_[2] = OCp_[2] = focalPix;
  OApo_ = OAp_;
  OBpo_ = OBp_;
  OCpo_ = OCp_;
  zap(ptrDatFile);
}
Data::Data(const Data & D)
  : datFile_(D.datFile_), OAp_(D.OAp_), OBp_(D.OBp_), OCp_(D.OCp_),
    OApo_(D.OApo_), OBpo_(D.OBpo_), OCpo_(D.OCpo_),
    datFileExists_(D.datFileExists_) {}
Data & Data::operator=(const Data & D)
{
  if ( this == &D ) return *this;  // Allows self-assignment.
  datFile_ = D.datFile_;
  OAp_     = D.OAp_;
  OBp_     = D.OBp_;
  OCp_     = D.OCp_;
  OApo_    = D.OApo_;
  OBpo_    = D.OBpo_;
  OCpo_    = D.OCpo_;
  datFileExists_ = D.datFileExists_;
  return *this;                    // Allows stacked assignments.
}

// Center pixel data
void Data::center(int imWidthCalEx, int imHeightCalEx, float ARC)
{
  center(OAp_,  imWidthCalEx, imHeightCalEx, ARC);
  center(OBp_,  imWidthCalEx, imHeightCalEx, ARC);
  center(OCp_,  imWidthCalEx, imHeightCalEx, ARC);
}

void Data::center(PixelLoc3 & P, int W, int H, float ARC)
{
  float im = float(W-1) / 2.0;
  float jm = float(H-1) / 2.0;
  P[0] = P[0]  - im;
  P[1] = jm  - P[1];
}

// Normalize
void Data::norm()
{
  OAp_.norm();
  OBp_.norm();
  OCp_.norm();
}

// Display Data
ostream & operator<< (ostream & out, const Data &D)
{
  out << "\n\n%MESSAGE(CameraData): data from " << D.datFile_.data() << ":\n"
      << "% First  ball is OAp (mask A)         \n"
      << "% Second ball is OBp (mask B)         \n"
      << "% Third  ball is OCp (mask C)         \n";
  out << "OApo=  [";
  for (int i=0; i<2; i++ ) out << setprecision(5) << setw(9) <<D.OApo_[i] <<" ";
  out << "];\n";
  out << "OBpo=  [";
  for (int i=0; i<2; i++ ) out << setprecision(5) << setw(9) <<D.OBpo_[i] <<" ";
  out << "];\n";
  out << "OCpo=  [";
  for (int i=0; i<2; i++ ) out << setprecision(5) << setw(9) <<D.OCpo_[i] <<" ";
  out << "];\n";
  out << "OAp =  [";
  for (int i=0; i<2; i++ ) out << setprecision(5) << setw(9) <<D.OAp_[i] <<" ";
  out << "];\n";
  out << "OBp =  [";
  for (int i=0; i<2; i++ ) out << setprecision(5) << setw(9) <<D.OBp_[i] <<" ";
  out << "];\n";
  out << "OCp =  [";
  for (int i=0; i<2; i++ ) out << setprecision(5) << setw(9) <<D.OCp_[i] <<" ";
  out << "];\n";

  return out;
}


////////////////////////class CameraData////////////////////////////////////
// Constructors.
CameraData::CameraData()
  : derived_(0),
    imWidthCalEx_(320), imHeightCalEx_(240), focalCalEx_(0), AbConstant_(1.0),
    Abrqw2_(0.0), AbipimR_(0.0), AbjpjmR_(0.0), ARC_(1.33333), RgSir_(0.0),
    RgSiri_(0.0), RgSjr_(0.0), RgSjrj_(0.0), fileroot_(""), camFile_(""),
    camFileExists_(0), usingGrid_(0), usingSy_(0), gridPrepped_(0), SY_(1.0),
    Sy_(0.0), nUser_(0), mUser_(0), im_(0.0), jm_(0.0), xr_(NULL), yr_(NULL),
    ACPW_(NULL), P_(NULL), colpGrid_(NULL), rowpGrid_(NULL), ipCORR_(NULL),
    jpCORR_(NULL), iCORR_(NULL), jCORR_(NULL), rowpcorr_(NULL), colpcorr_(NULL)
{
  ACPW_      = new Matrix <float> (3, 3, 0.0);
  P_         = new Vector <float> (3,    0.0);
  colpGrid_  = new Matrix <float> (5, 7, 0.0); 
  rowpGrid_  = new Matrix <float> (5, 7, 0.0); 
}
CameraData::CameraData(const char camFile[])
  : derived_(0),
    imWidthCalEx_(320), imHeightCalEx_(240), focalCalEx_(0), AbConstant_(1.0),
    Abrqw2_(0.0), AbipimR_(0.0), AbjpjmR_(0.0), ARC_(1.33333), RgSir_(0.0),
    RgSiri_(0.0), RgSjr_(0.0), RgSjrj_(0.0), fileroot_(""),
    camFile_(string(camFile)), camFileExists_(0), usingGrid_(0), usingSy_(0),
    gridPrepped_(0), SY_(1.0), Sy_(0.0), nUser_(0), mUser_(0), im_(0.0),
    jm_(0.0), xr_(NULL), yr_(NULL), ACPW_(NULL), P_(NULL), colpGrid_(NULL),
    rowpGrid_(NULL), ipCORR_(NULL), jpCORR_(NULL), iCORR_(NULL), jCORR_(NULL),
    rowpcorr_(NULL), colpcorr_(NULL)
{
  ACPW_      = new Matrix <float> (3, 3, 0.0);
  P_         = new Vector <float> (3,    0.0);
  colpGrid_  = new Matrix <float> (5, 7, 0.0);
  rowpGrid_  = new Matrix <float> (5, 7, 0.0);
  float colpGridCheckSum = 0.0;
  float rowpGridCheckSum = 0.0;

  // Read files
  InFile *ptrCamFile  = new InFile("MyCamData", camFile_.data(), 128, MAX_CANON);
  if ( 0 == ptrCamFile->load() ) goto defCam;
  ptrCamFile->stripComments("#");
  ptrCamFile->stripComments("%");
  ptrCamFile->stripBlankLines();
  if ( !ptrCamFile->loadVars() ) goto defCam;
  camFileExists_ = 1;
  usingGrid_     = 1;
  GetPar(*ptrCamFile, "imWidthCalEx", &imWidthCalEx_, 1);
  GetPar(*ptrCamFile, "imHeightCalEx",&imHeightCalEx_,1);
  GetPar(*ptrCamFile, "focalCalEx",   &focalCalEx_,   1);
  GetPar(*ptrCamFile, "AbConstant",   &AbConstant_,   1);
  GetPar(*ptrCamFile, "Abrqw2",       &Abrqw2_,       0);
  GetPar(*ptrCamFile, "AbipimR",      &AbipimR_,      0);
  GetPar(*ptrCamFile, "AbjpjmR",      &AbjpjmR_,      0);
  GetPar(*ptrCamFile, "ARC",          &ARC_,          1);
  GetPar(*ptrCamFile, "RgSir",        &RgSir_,        0);
  GetPar(*ptrCamFile, "RgSiri",       &RgSiri_,       0);
  GetPar(*ptrCamFile, "RgSjr",        &RgSjr_,        0);
  GetPar(*ptrCamFile, "RgSjrj",       &RgSjrj_,       0);
  if ( -1 == GetPar(*ptrCamFile, "P",      P_,          0) ) camFileExists_ = 0;
  if ( -1 == GetPar(*ptrCamFile, "ACPW",   ACPW_,       0) ) camFileExists_ = 0;
  if ( -1 == GetPar(*ptrCamFile, "colpGrid", colpGrid_, 0) ) usingGrid_     = 0;
  if ( -1 == GetPar(*ptrCamFile, "rowpGrid", rowpGrid_, 0) ) usingGrid_     = 0;
  for ( int i=0; i<5; i++) for ( int j=0; j<7; j++ )
    {
      colpGridCheckSum += (*(colpGrid_))[i][j];
      rowpGridCheckSum += (*(rowpGrid_))[i][j];
    }
  if ( 0 == colpGridCheckSum  || 0 == rowpGridCheckSum ) usingGrid_ = 0;
  if ( -1 == GetPar(*ptrCamFile, "Sy", &Sy_, 0) )
    usingSy_ = 0;
  else
    {
      usingSy_ = 1;
      SY_      = Sy_;
    }
  // SY_ overrides all other options
  if ( 0 == SY_ ) usingSy_   = 0;
  else
    {
      if ( usingGrid_ )
	cerr << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	     << " over-riding grid inputs with SY.\n" << flush;
      usingGrid_ = 0;
    }
  if ( usingGrid_ )
    {
#ifdef VERBOSE
      if ( 4 < verbose )
	{
	  cout << "% MESSAGE(Tune): using calibration grid." << endl << flush;
	}
#endif
      prepGrid();
    }
  goto endCam;
 defCam:
  camFileExists_ = 0;
  cerr << "WARNING(Tune): " << __FILE__ << ": " << __LINE__
       << " no variables in " << camFile_.data() << " or empty.\n";
#ifdef VERBOSE
  if ( verbose )
    cerr << "Use entry format as follows:\n"
	 << "% Auto-generated video .cam file.\n"
	 << "imWidthCalEx  = 320; % pixels image width, set by checkSetup \n"
	 << "imHeightCalEx = 240; % pixels image height, set by checkSetup \n"
	 << "focalCalEx    =   0; % =f/w*imWidthCalEx, pixels\n"
	 << "%  where w is camera ccd width, mm,(4.8 for 1/2 in CCD), \n"
	 << "%  f is camera focal length, mm, read off the lens or better\n"
	 << "%  from lens test (see the report and use the .xls spreadsheet\n"
	 << "%  supplied in the documentation.   \n"
	 << "AbConstant = 1.;  % internal calibration, see .xls spreadsheet.\n"
	 << "Abrqw2     = 0.;  % internal calibration, see .xls spreadsheet.\n"
	 << "AbipimR    = 0.;  % internal calibration, see .xls spreadsheet.\n"
	 << "AbjpjmR    = 0.;  % internal calibration, see .xls spreadsheet.\n"
	 << "ARC        = 1.3333; %internal cal,see .xls spreadsheet.\n"
	 << "RgSir      = 0;\n"
	 << "RgSiri     = 0;\n"
	 << "RgSjr      = 0;\n"
	 << "RgSjrj     = 0;\n"
         << "SY         = 1.00; % Amount to stretch j pixels to square image\n"
	 << "P          = [0 0 0];\n"
	 << "ACPW       = [0 0 0\n"
	 << "              0 0 0\n"
	 << "              0 0 0];\n"
	 << "colpGrid=[0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0];\n"
	 << "rowpGrid=[0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0\n"
	 << "          0 0 0 0 0 0 0];\n";
#endif
  cerr << "Using defaults and continuing...\n" << flush;
  imWidthCalEx_  = 320;
  imHeightCalEx_ = 240;
  focalCalEx_    =   0;
  AbConstant_    = 1.0;
  Abrqw2_        = 0.0;
  AbipimR_       = 0.0;
  AbjpjmR_       = 0.0;
  ARC_           = 1.3333;
  RgSir_         =   0;
  RgSiri_        =   0;
  RgSjr_         =   0;
  RgSjrj_        =   0;
  SY_            = 1.0;
  usingGrid_     =   1;
 endCam:
  zap(ptrCamFile);
  return;  
}
CameraData::CameraData(const CameraData &C)
  : derived_(C.derived_), imWidthCalEx_(C.imWidthCalEx_),
    imHeightCalEx_(C.imHeightCalEx_), focalCalEx_(C.focalCalEx_),
    AbConstant_(C.AbConstant_), Abrqw2_(C.Abrqw2_), AbipimR_(C.AbipimR_),
    AbjpjmR_(C.AbjpjmR_), ARC_(C.ARC_), RgSir_(C.RgSir_), RgSiri_(C.RgSiri_),
    RgSjr_(C.RgSjr_), RgSjrj_(C.RgSjrj_), fileroot_(C.fileroot_),
    camFile_(C.camFile_), camFileExists_(C.camFileExists_),
    usingGrid_(C.usingGrid_), usingSy_(C.usingSy_),
    gridPrepped_(C.gridPrepped_), SY_(C.SY_), Sy_(C.Sy_), nUser_(C.nUser_),
    mUser_(C.mUser_), im_(C.im_), jm_(C.jm_), xr_(NULL), yr_(NULL), ACPW_(NULL),
    P_(NULL), colpGrid_(NULL), rowpGrid_(NULL),ipCORR_(NULL), jpCORR_(NULL),
    iCORR_(NULL), jCORR_(NULL), rowpcorr_(NULL), colpcorr_(NULL)
{
  ACPW_      = new Matrix <float> (*C.ACPW_);
  P_         = new Vector <float> (*C.P_);
  colpGrid_  = new Matrix <float> (*C.colpGrid_); 
  rowpGrid_  = new Matrix <float> (*C.rowpGrid_); 
  xr_ = new float[nUser_*mUser_];
  yr_ = new float[nUser_*mUser_];
  for (int col=0; col<nUser_; col++)
    {
      for (int row=0; row<mUser_; row++)
	{
	  int index  = col*mUser_ + row;
	  xr_[index] = C.xr_[index];
	  yr_[index] = C.yr_[index];
	}
    }
  if ( C.ipCORR_ && C.jpCORR_ )
    {
      ipCORR_ = new float[nr*nc];
      jpCORR_ = new float[nr*nc];
      int index;
      for ( int r=0; r<nr; r++ )
	{
      for ( int c=0; c<nc; c++ )
	{
	  index = nc*r + c;
	  ipCORR_[index] = C.ipCORR_[index];
	  jpCORR_[index] = C.jpCORR_[index];
	}
	}
    }
  if ( C.iCORR_ && C.jCORR_ )
    {
      iCORR_ = new float[nc];
      for ( int c=0; c<nc; c++ ) iCORR_[c] = C.iCORR_[c];
      jCORR_ = new float[nr];
      for ( int r=0; r<nr; r++ ) jCORR_[r] = C.jCORR_[r];
    }
  if ( C.rowpcorr_ ) rowpcorr_ = new Curve2(iCORR_, jCORR_, jpCORR_, nc, nr);
  if ( C.colpcorr_ ) colpcorr_ = new Curve2(iCORR_, jCORR_, ipCORR_, nc, nr);
}
CameraData & CameraData::operator=(const CameraData & C)
{
  if ( this == &C ) return *this;  // Allows self-assignment.
  derived_        =  C.derived_;
  imWidthCalEx_   =  C.imWidthCalEx_;
  imHeightCalEx_  =  C.imHeightCalEx_;
  focalCalEx_     =  C.focalCalEx_;
  AbConstant_     =  C.AbConstant_;
  Abrqw2_         =  C.Abrqw2_;
  AbipimR_        =  C.AbipimR_;
  AbjpjmR_        =  C.AbjpjmR_;
  ARC_            =  C.ARC_;
  RgSir_          =  C.RgSir_;
  RgSiri_         =  C.RgSiri_;
  RgSjr_          =  C.RgSjr_;
  RgSjrj_         =  C.RgSjrj_;
  fileroot_       =  C.fileroot_;
  camFile_        =  C.camFile_;
  camFileExists_  =  C.camFileExists_;
  usingGrid_      =  C.usingGrid_;
  usingSy_        =  C.usingSy_;
  gridPrepped_    =  C.gridPrepped_;
  SY_             =  C.SY_;
  Sy_             =  C.Sy_;
  nUser_          =  C.nUser_;
  mUser_          =  C.mUser_;
  im_             =  C.im_;
  jm_             =  C.jm_;
  xr_             =  NULL;
  yr_             =  NULL;
  ACPW_           =  NULL;
  P_              =  NULL;
  colpGrid_       =  NULL;
  rowpGrid_       =  NULL;
  ipCORR_         =  NULL;
  jpCORR_         =  NULL;
  iCORR_          =  NULL;
  jCORR_          =  NULL;
  rowpcorr_       =  NULL;
  colpcorr_       =  NULL;
  ACPW_      = new Matrix <float> (*C.ACPW_);
  P_         = new Vector <float> (*C.P_);
  colpGrid_  = new Matrix <float> (*C.colpGrid_); 
  rowpGrid_  = new Matrix <float> (*C.rowpGrid_); 
  xr_ = new float[nUser_*mUser_];
  yr_ = new float[nUser_*mUser_];
  for (int col=0; col<nUser_; col++)
    {
      for (int row=0; row<mUser_; row++)
	{
	  int index  = col*mUser_ + row;
	  xr_[index] = C.xr_[index];
	  yr_[index] = C.yr_[index];
	}
    }
  if ( C.ipCORR_ && C.jpCORR_ )
    {
      ipCORR_ = new float[nr*nc];
      jpCORR_ = new float[nr*nc];
      int index;
      for ( int r=0; r<nr; r++ )
	{
	  for ( int c=0; c<nc; c++ )
	    {
	  index = nc*r + c;
	  ipCORR_[index] = C.ipCORR_[index];
	  jpCORR_[index] = C.jpCORR_[index];
	    }
	}
    }
  if ( C.iCORR_ && C.jCORR_ )
    {
      iCORR_ = new float[nc];
      for ( int c=0; c<nc; c++ ) iCORR_[c] = C.iCORR_[c];
      jCORR_ = new float[nr];
      for ( int r=0; r<nr; r++ ) jCORR_[r] = C.iCORR_[r];
    }
  if ( C.rowpcorr_ ) rowpcorr_ = new Curve2(iCORR_, jCORR_, jpCORR_, nc, nr);
  if ( C.colpcorr_ ) colpcorr_ = new Curve2(iCORR_, jCORR_, ipCORR_, nc, nr);
  return *this;  // Allows stacked assignments.
}

// Prepare grid shot correction for aberration
void CameraData::prepGrid()
{
  const float L    = 1.0;       // units real world grid spacing
  const int   midr = (nr+1)/2-1;// midpoint
  const int   midc = (nc+1)/2-1;// midpoint
  Matrix <float> ipmim (nr, nc, 0.0);
  Matrix <float> jmmjp (nr, nc, 0.0);

  if ( !usingGrid_)
    {
      cerr <<  "ERROR(Tune): prepGrid  : " << __FILE__ << " : " << __LINE__ 
	   << " cannot call prepGrid with usingGrid_ = " << usingGrid_
	   << ".  Quitting\n" << endl << flush;
      exit(1);
    }

  // Initialize
  for ( int r=0; r<nr; r++ )
    {
      for ( int c=0; c<nc; c++ )
	{
	  ipmim[r][c] = (*colpGrid_)[r][c]  - float(imWidthCalEx_-1)/2.0;
	  jmmjp[r][c] = float(imHeightCalEx_-1)/2.0 - (*rowpGrid_)[r][c];
	}
    }

  // Discover scaling factor  
  float Li = 
    std::abs( ( ipmim[midr-1][midc+1] - ipmim[midr-1][midc  ] ) +
	      ( ipmim[midr  ][midc+1] - ipmim[midr  ][midc  ] ) +
	      ( ipmim[midr+1][midc+1] - ipmim[midr+1][midc  ] ) +
	      ( ipmim[midr-1][midc  ] - ipmim[midr-1][midc-1] ) +
	      ( ipmim[midr  ][midc  ] - ipmim[midr  ][midc-1] ) +
	      ( ipmim[midr+1][midc  ] - ipmim[midr+1][midc-1] ) ) / 6.0 / L;

  float Lj =
    std::abs( ( jmmjp[midr+1][midc-1] - jmmjp[midr  ][midc-1] ) +
	      ( jmmjp[midr+1][midc  ] - jmmjp[midr  ][midc  ] ) +
	      ( jmmjp[midr+1][midc+1] - jmmjp[midr  ][midc+1] ) +
	      ( jmmjp[midr  ][midc-1] - jmmjp[midr-1][midc-1] ) +
	      ( jmmjp[midr  ][midc  ] - jmmjp[midr-1][midc  ] ) +
	      ( jmmjp[midr  ][midc+1] - jmmjp[midr-1][midc+1] ) ) / 6.0 / L;

  SY_  = Li / Lj;

  // Calculate the theoretical pixel readings for the grid
  Matrix <float> xR(nr, nc, 0.0);
  Matrix <float> yR(nr, nc, 0.0);
  xR[midr][midc] = ipmim[midr][midc] / Li;
  yR[midr][midc] = jmmjp[midr][midc] / Lj;
  for ( int r=0; r<nr; r++ )
    {
      for ( int c=0; c<nc; c++ )
	{
	  if ( midr != r || midc != c )
	    {
	      xR[r][c] = xR[midr][midc] - L * (midc -c);
	      yR[r][c] = yR[midr][midc] + L * (midr -r);
	    }
	}
    }
  // Calculate real grid rotation alpha
  float theta = 0.0;
  int num = 0;
  for ( int c=0; c<nc; c++ ) 
    {
      if ( midc > c )
	{
	  theta += atan2( SY_*(jmmjp[midr][midc] - jmmjp[midr][c]),
			  ipmim[midr][midc] - ipmim[midr][c] );
	  num ++;
	}
      if ( midc < c )
	{
	  theta += atan2( SY_*(jmmjp[midr][c] - jmmjp[midr][midc]),
			  ipmim[midr][c] - ipmim[midr][midc] );
	  num ++;
	}
    }
  theta /= num;
  float phi = 0.0;
  num = 0;
  for ( int r=0; r<nr; r++ ) 
    {
      if ( midr > r )
	{
	  phi += atan2( SY_*(jmmjp[r][midc] - jmmjp[midr][midc]),
			ipmim[r][midc] - ipmim[midr][midc] );
	  num ++;
	}
      if ( midr < r )
	{
	  phi += atan2( SY_*(jmmjp[midr][midc] - jmmjp[r][midc]),
			ipmim[midr][midc] - ipmim[r][midc] );
	  num ++;
	}
    }
  phi /= num;
  phi -= pi / 2;
  float alpha = ( theta + phi ) / 2.0;

  Matrix <float> ipmimT (nr, nc, 0.0);
  Matrix <float> jmmjpT (nr, nc, 0.0);
  Matrix <float> R (nr, nc, 0.0);
  Matrix <float> gamma (nr, nc, 0.0);
  Matrix <float> di (nr, nc, 0.0);
  Matrix <float> dj (nr, nc, 0.0);
  Matrix <float> ipmimTC (nr, nc, 0.0);
  Matrix <float> jmmjpTC (nr, nc, 0.0);
  ipCORR_ = new float[nr*nc];
  jpCORR_ = new float[nr*nc];
  int index;
  for ( int r=0; r<nr; r++ )
    {
      for ( int c=0; c<nc; c++ )
	{
	  ipmimT[r][c]  =   xR[r][c] * Li;
	  jmmjpT[r][c]  =   yR[r][c] * Lj * SY_;
	  R[r][c]       =   sqrt(ipmimT[r][c]*ipmimT[r][c] +
				 jmmjpT[r][c]*jmmjpT[r][c]);
	  gamma[r][c]   =   atan2(jmmjpT[r][c], ipmimT[r][c]);
	  di[r][c]      =  -R[r][c] * alpha * sin(gamma[r][c]);
	  dj[r][c]      =   R[r][c] * alpha * cos(gamma[r][c]);
	  ipmimTC[r][c] =   ipmimT[r][c] + di[r][c];
	  jmmjpTC[r][c] =   jmmjpT[r][c] + dj[r][c];
	  index = nc*r + c;
	  ipCORR_[index]  =   ipmimTC[r][c] - ipmim[r][c];
	  jpCORR_[index]  = -(jmmjpTC[r][c] - jmmjp[r][c]);
	}
    }

  if ( 3<verbose ) 
    {
      cout << "ipCORR_=" << nc << "x" << nr << endl;
      for ( int row=0; row<nr; row++ )
	{
	  for ( int col=0; col<nc; col++ )
	    {
	      int index = row * nc + col;
	      cout << ipCORR_[index] << " ";
	    }
	  cout << "\n\n";
	}
      cout << "jpCORR_=" << nc << "x" << nr << endl;
      for ( int row=0; row<nr; row++ )
	{
	  for ( int col=0; col<nc; col++ )
	    {
	      int index = row * nc + col;
	      cout << jpCORR_[index] << " ";
	    }
	  cout << "\n\n";
	}

    }

  // Construct 2-d table breakpoints (raw pixels)
  iCORR_ = new float[nc];
  for ( int c=0; c<nc; c++ ) iCORR_[c] = Li * (c - midc);
  jCORR_ = new float[nr];
  for ( int r=0; r<nr; r++ ) jCORR_[r] = Lj * (r - midr);

  if ( 7 < verbose )
    {
      cout << "iCORR_=";
      for ( int c=0; c<nc; c++ )
	{
	  cout << iCORR_[c];
	  if ( c<nc-1 ) cout << ",";
	}
      cout << endl << flush;
      cout << "jCORR_=";
      for ( int r=0; r<nr; r++ )
	{
	  cout << jCORR_[r];
	  if ( r<nr-1 ) cout << ",";
	}
      cout << endl << flush;
    }


  rowpcorr_ = new Curve2(iCORR_, jCORR_, jpCORR_, nc, nr);
  colpcorr_ = new Curve2(iCORR_, jCORR_, ipCORR_, nc, nr);

  // Check
  float *JPCORR = new float[imHeightCalEx_*imWidthCalEx_];
  float *IPCORR = new float[imHeightCalEx_*imWidthCalEx_];
  float colpm, rowpm;
  for ( int row=0; row<imHeightCalEx_; row++ )
    {
      for ( int col=0; col<imWidthCalEx_; col++ )
	{
	  index = row*imWidthCalEx_ + col;
	  colpm  = col - float(imWidthCalEx_ -1) / 2.0;
	  rowpm  = row - float(imHeightCalEx_-1) / 2.0;
	  JPCORR[index] = curvex(colpm, rowpm, *rowpcorr_);
	  IPCORR[index] = curvex(colpm, rowpm, *colpcorr_);
	}
    }

  if ( 3 < verbose )
    {
      cout << "IPCORR\n";
      for ( int row=62; row<65; row++ )
	{
	  for ( int col=38; col<41; col++ )
	    {
	      int index = row * imWidthCalEx_ + col;
	      cout << IPCORR[index] << " ";
	    }
	  cout << "\n";
	}
      cout << "\n\n";
      
      cout << "IPCORR\n";
      for ( int row=0; row<120; row++ )
	{
	  for ( int col=38; col<41; col++ )
	    {
	      int index = row * imWidthCalEx_ + col;
	      cout << IPCORR[index] << " ";
	    }
	  cout << "\n";
	}
      cout << "\n\n";
      
      cout << "JPCORR\n";
      for ( int row=62; row<65; row++ )
	{
	  for ( int col=38; col<41; col++ )
	    {
	      int index = row * imWidthCalEx_ + col;
	      cout << JPCORR[index] << " ";
	    }
	  cout << "\n";
	}
      cout << "\n\n\n\n";

      cout << "JPCORR\n";
      for ( int row=0; row<120; row++ )
	{
	  for ( int col=38; col<41; col++ )
	    {
	      int index = row * imWidthCalEx_ + col;
	      cout << JPCORR[index] << " ";
	    }
	  cout << "\n" << row << ": ";
	}
      cout << "\n\n\n\n";
    }

  gridPrepped_ = 1;
  zaparr(JPCORR);
  zaparr(IPCORR);
}

// Correction to column coordinate for aberration
float CameraData::colpCorr(const int col, const int row, const int n,
			   const int m)
{
  if ( m<=row || n<=col || 0>row || 0>col )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : requested position " << row << "," << col 
	   << " outside valid range " << m << "x"
	   << n << "  Check and try again.\n" << flush;
      exit(1);
    }
  // Breakpoints are raw pixels
  float colpm, rowpm;
  rowpm = (float(row) - float(m-1)/2.0) / (float)m * (float)imHeightCalEx_;
  colpm = (float(col) - float(n-1)/2.0) / (float)n * (float)imWidthCalEx_;
  return curvex(colpm, rowpm, *colpcorr_);
}

// Setup aberration fast lookup
void CameraData::initCorrectRx()
{
  if ( 0!=nUser_ || 0!=mUser_ )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : invalid user sizes, or already initialized : "
	   << nUser_ << "x" << mUser_ << flush;
      exit(1);
    }
  nUser_ = imWidthCalEx_;
  mUser_ = imHeightCalEx_;
  initializeCorrectRx();
}

// Setup aberration fast lookup
void CameraData::initCorrectRx(const int n, const int m)
{
  if ( 0!=nUser_ || 0!=mUser_ )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : invalid user sizes, or already initialized : "
	   << nUser_ << "x" << mUser_ << flush;
      exit(1);
    }
  nUser_ = n;
  mUser_ = m;
  initializeCorrectRx();
}

// Initialize aberration fast lookup, used by all
void CameraData::initializeCorrectRx()
{
  xr_ = new float[nUser_*mUser_];
  yr_ = new float[nUser_*mUser_];
  
  int index;
  float rho, rat, iim, jmj;
  for (int col=0; col<nUser_; col++)
    {
      for (int row=0; row<mUser_; row++)
	{
	  index      = col*mUser_ + row;

	  // Adjust for Aberration Error
	  // colpCorr and rowpCorr assume n/2 and m/2 raw pixels = corrected
	  // pixel center point.  Therefore, centering may be done after
	  // applying correction and furthermore both correction and
	  // centering may use raw pixel input.
	  if ( usingGrid_ && gridPrepped_ ) //  Using Grid
	    {
	      xr_[index] = (float)col + colpCorr(col, row, nUser_, mUser_);
	      yr_[index] = (float)row + rowpCorr(col, row, nUser_, mUser_);
	    }
	  else                                    // Spherical aberration
	    {
	      iim = float(col)   - im_;
	      jmj = jm_ -   float(row);
	      rho = sqrt( iim*iim + jmj*jmj );
	      if ( usingSy_ )
		rat = 1.0;                       // Using Sy_; no aberration
	      else
		rat = AbConstant_
		  + Abrqw2_  * ( rho*rho / float(nUser_*nUser_) )
		  + AbipimR_ *   iim     / float(nUser_)     
		  + AbjpjmR_ *   jmj     / float(nUser_); 
	      xr_[index] = rat *  iim         +  im_;
	      yr_[index] = rat *(-jmj) * SY_  +  jm_;
	    }
	  
	  // Adjust for Register Error
	  xr_[index] += ( float(nUser_) / float(imWidthCalEx_)  - 1.0 ) *
	    (RgSir_  + RgSiri_*xr_[index]);
	  
	  yr_[index] += ( float(mUser_) / float(imHeightCalEx_) - 1.0 ) *
	    (RgSjr_  + RgSjrj_*yr_[index]);

	}
    }  

  if ( 99 < verbose )
    {
      cout << "xr=\n" << nUser_ << "x" << mUser_ << " 38:41x62:65"
	   << endl << flush;
      for ( int row=62; row<65; row++ )
	{
	  for ( int col=38; col<41; col++ )
	    {
	      int index = col * mUser_ + row;
	      cout << xr_[index] << " ";
	    }
	  cout << "\n";
	}
      cout << "\n\n";

      cout << "yr=\n" << nUser_ << "x" << mUser_  << " 38:41x62:65"
	   << endl << flush;
      for ( int row=62; row<65; row++ )
	{
	  for ( int col=38; col<41; col++ )
	    {
	      int index = col*mUser_ + row;
	      cout << yr_[index] << " ";
	    }
	  cout << "\n";
	}
      cout << "\n\n\n\n";
    }

}

// Correct for aberration
void CameraData::correctRx(float *x, float *y)
{
  if ( 0>=nUser_ || 0>=mUser_ )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : invalid user sizes, or uninitialized (initCorrectRx) : "
	   << nUser_ << "x" << mUser_ << flush;
      exit(1);
    }
  int index = int(*x) * mUser_ + int(*y);
  *x = xr_[index] + (*x - int(*x));
  *y = yr_[index] + (*y - int(*y))*SY_;
}

// Correct for aberration, used in extCal
void CameraData::correctRx(PixelLoc3 *pPtr)
{
  if ( 0>=nUser_ || 0>=mUser_ )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : invalid user sizes, or uninitialized (initCorrectRx) : "
	   << nUser_ << "x" << mUser_ << flush;
      exit(1);
    }
  
  float *x = &((*pPtr)[0]);
  float *y = &((*pPtr)[1]);
  int index = int(*x) * mUser_ + int(*y);
  if ( 99 < verbose )
    {
      cout << "x=" << *x << ", y=" << *y << endl << flush;
      cout << "xr_[" << index << "]= " << xr_[index]
	   << ", yr_[" << index << "]= " << yr_[index] << endl << flush;
    }
  *x = xr_[index] + (*x - int(*x));
  *y = yr_[index] + (*y - int(*y))*SY_;
}

// Correct for aberration, used in triangulate
void CameraData::correctRx(pixelLocf *pPtr)
{
  if ( 0>=nUser_ || 0>=mUser_ )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : invalid user sizes, or uninitialized (initCorrectRx) : "
	   << nUser_ << "x" << mUser_ << flush;
      exit(1);
    }
  
  float *x = &(pPtr->x);
  float *y = &(pPtr->y);
  int index = int(*x) * mUser_ + int(*y);
  *x = xr_[index] + (*x - int(*x));
  *y = yr_[index] + (*y - int(*y))*SY_;
}

// Uncorrect for aberration
void CameraData::distort(float *x, float *y)
{
  if ( 0==nUser_ || 0==mUser_ || 0>nUser_ || 0>mUser_ )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : invalid user sizes, or uninitialized (initCorrectRx) : "
	   << nUser_ << "x" << mUser_ << flush;
      exit(1);
    }

  float iim, jmj, rho, rat;
  // Unadjust for Register Error
  *x =
    ( *x  - ( float(nUser_)/float(imWidthCalEx_)  - 1.0 ) * RgSir_ ) /
    ( 1.0 + ( float(nUser_)/float(imWidthCalEx_)  - 1.0 ) * RgSiri_);
  *y =
    ( *y  - ( float(mUser_)/float(imHeightCalEx_) - 1.0 ) * RgSjr_ ) /
    ( 1.0 + ( float(mUser_)/float(imHeightCalEx_) - 1.0 ) * RgSjrj_);

  // Uncorrect for radial aberration
  if ( usingGrid_ )
    {
      int   countx =   0;
      int   county =   0;
      float xpast =  0.0;
      float ypast =  0.0;
      float xinit =   *x;
      float yinit =   *y;
      float tolx  = 0.02;    // 0.1 resolution at user
      float toly  =0.001;    // inner loop
      int   row, col, index;
      float dx    =  0.0;
      float dy    =  0.0;
      float dxpast, dypast;
      do
	{
	  xpast  = *x;
	  dxpast = dx;
	  county =  0;
	  col = min(max((int) (*x + 0.5), 0), nUser_-1);
	  do
	    {
	      ypast  = *y;
	      dypast = dy;
	      row    = min(max((int) (*y + 0.5), 0), mUser_-1);
	      index  = col * mUser_ + row;
	      dx     = xr_[index] - (float)col;
	      dy     = yr_[index] - (float)row;
	      *x     = xinit - (dx + dxpast) / 2.0;  // Avg to prevent cycling
	      *y     = yinit - (dy + dypast) / 2.0;  // Avg to prevent cycling
	      if ( 2 < verbose )
		{
		  cout << "county,yinit,*y,col,row,index,yr,dy= "
		       << county << ", " << yinit << ", " << *y    << ", "
		       << col    << ", " << row   << ", " << index << ", " 
		       << yr_[index] << ", " << yr_[index]-(float)row
		       <<  endl << flush;
		}
	    } while ( ++county<10 && std::abs(ypast-*y)>toly );
	  if ( 2 < verbose )
	    {
	      cout << "countx,xinit,*x,col,row,index,xr,dx,*x*y= "
		   << countx << ", " << xinit << ", " << *x       << ", "
		   << col    << ", " << row   << ", " << index    << ", " 
		   << xr_[index] << ", " << xr_[index]-(float)col << ", "
		   << *x      << ", " << *y << endl << flush;
	    }
	} while ( ++countx<10 && std::abs(xpast-*x)>tolx );
    }
  else
    {
      iim = *x  - im_;
      jmj = jm_ -  *y;
      rho = sqrt(iim*iim + jmj*jmj);
      if ( usingSy_ )
	rat = 1.0;
      else
	rat = AbConstant_ + Abrqw2_ * ( rho*rho / float(nUser_*nUser_) )
	  + AbipimR_ *   iim     / float(nUser_) 
	  + AbjpjmR_ *   jmj     / float(nUser_); 
      iim /= rat;
      jmj /= rat;
      *x  =  iim + im_;
      *y  =  jm_ - jmj/SY_;
    }
}

// Correction to row coordinate for aberration
float CameraData::rowpCorr(const int col, const int row, const int n,
			   const int m)
{
  if ( m<=row || n<=col || 0>row || 0>col )
    {
      cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	   << " : requested position " << row << "," << col 
	   << " outside valid range " << m << "x"
	   << n << "  Check and try again.\n" << flush;
      exit(1);
    }
  // Breakpoints are raw pixels
  float colpm, rowpm;
  rowpm = (float(row) - float(m-1)/2.0) / (float)m * (float)imHeightCalEx_;
  colpm = (float(col) - float(n-1)/2.0) / (float)n * (float)imWidthCalEx_;
  return curvex(colpm, rowpm, *rowpcorr_);
}

// Display CameraData
string CameraData::print()
{
  Vector <float> P      ( *P_    );
  Matrix <float> ACPW   ( *ACPW_ );
  Matrix <float> colpGrid ( *colpGrid_ );
  Matrix <float> rowpGrid ( *rowpGrid_ );
  ostringstream out;

  if ( !derived_ )
    {
      out << "% MESSAGE(CameraData): cal for device " << camFile_.data() << ":\n";
    }
  else
    {
      out << "% MESSAGE(CameraData): cal for device " << fileroot_ << ":\n";
    }
  out  <<   "imWidthCalEx = " << setprecision(4) << setw(9) << imWidthCalEx_
       << ";\nimHeightCalEx= " << setprecision(4) << setw(9) << imHeightCalEx_
       << ";\nfocalCalEx   = " << setprecision(4) << setw(9) << focalCalEx_;
  if ( !usingGrid_ && !usingSy_ )
    out  << ";\nAbConstant   = " << setprecision(4) << setw(9) << AbConstant_
	 << ";\nAbrqw2       = " << setprecision(4) << setw(9) << Abrqw2_
	 << ";\nAbipimR      = " << setprecision(4) << setw(9) << AbipimR_
	 << ";\nAbjpjmR      = " << setprecision(4) << setw(9) << AbjpjmR_
	 << ";\nARC          = " << setprecision(5) << setw(9) << ARC_
	 << ";\nRgSir        = " << setprecision(4) << setw(9) << RgSir_
	 << ";\nRgSiri       = " << setprecision(4) << setw(9) << RgSiri_
	 << ";\nRgSjr        = " << setprecision(4) << setw(9) << RgSjr_
	 << ";\nRgSjrj       = " << setprecision(4) << setw(9) << RgSjrj_;
  if ( usingSy_ ) 
    out << ";\nSy           = " << setprecision(4) << setw(9) << Sy_;
  out << ";\nP          = [ ";
  for (int i=0; i<3; i++) out << setprecision(6) << setw(9) << P[i] << "  ";
  out << "]"
      << ";\nACPW       = [";
  for (int i=0; i<3; i++)
    {
      for (int j=0; j<3; j++)
	{
	  out << setprecision(5) << setw(10) << ACPW[i][j] << " ";
	}
      if ( i < 2 ) out << "\n              ";
    }
  out << " ]";
  out << ";\ncolpGrid=[";
  for (int i=0; i<5; i++)
    {
      for (int j=0; j<7; j++)
	{
	  out << setprecision(6) << setw(10) << colpGrid[i][j] << " ";
	}
      if ( i < 4 ) out << "\n          ";
    }
  out << " ]"
      << ";\nrowpGrid=[";
  for (int i=0; i<5; i++)
    {
      for (int j=0; j<7; j++)
	{
	  out << setprecision(6) << setw(10) << rowpGrid[i][j] << " ";
	}
      if ( i < 4 ) out << "\n          ";
    }
  out << " ]";
  out << ";\n";

  return out.str();
}


///////////////////// MyCamData //////////////////////////////////////////////
MyCamData::MyCamData()
  : CameraData(), datFile_(""), geoFile_(""), d_(NULL), w_(NULL) {}
MyCamData::MyCamData(const char camFile[], const char datFile[],
		     const char geoFile[])
  : CameraData(camFile), datFile_(""), geoFile_(""), d_(NULL), w_(NULL)
{
  int *n = new int(0);
  int *m = new int(0);
  camFile_ = string(camFile);
  datFile_ = string(datFile);
  geoFile_ = string(geoFile);

  // Determine file root.
  const char *delims = "/.";
  stringset   in_fileS(camFile_, delims);
  fileroot_ = in_fileS.token(in_fileS.size()-2);

  d_ = new Data(datFile_.data(), focalCalEx_);
  w_ = new World(geoFile_.data());
  
  if ( camFileExists_ )
    {
      if ( imWidthCalEx_ < *n || imHeightCalEx_ < *m )
	{
#ifdef VERBOSE
	  if ( 4 < verbose )
	    cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
		 << " :  calibration image size= " << imWidthCalEx_ << "x"
		 << imHeightCalEx_ << " for device " << camFile_.data() 
		 << "\nis at lower resolution than present setting of " << *n
		 << "x" << *m << ".\n" << flush;
#endif
	}
      if ( 0 == *n && 0 == *m )
	{
	  *n = imWidthCalEx_;
	  *m = imHeightCalEx_;
	}
      if ( 0 == *n || 0 == *m )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " :  size for device " << camFile_.data() 
	       << " is " << *n << "x" << *m << ".\n" << flush;
	  exit(1);
	}
    }
  else
    {
      *n             = MAX(imWidthCalEx_, 1);
      *m             = MAX(imHeightCalEx_, 1);
      imWidthCalEx_  =     *n;
      imHeightCalEx_ =     *m;
      focalCalEx_    =      0;
      AbConstant_    =    1.0;
      Abrqw2_        =    0.0;
      AbipimR_       =    0.0;
      AbjpjmR_       =    0.0;
      ARC_           = 1.3333;
      RgSir_         =      0;
      RgSiri_        =      0;
      RgSjr_         =      0;
      RgSjrj_        =      0;
    }
  
  // Conversions
  if ( !usingGrid_ && !usingSy_ ) SY_ = float(*n)  / float(*m) / ARC_;
  else ARC_ = float(*n)  / float(*m) / SY_;

  // Corrections assume centering at n/2 and m/2 points.  This allows
  // both correction and centering to use raw pixels as lookups.
  im_         = float(*n-1) / 2.0;
  jm_         = float(*m-1) / 2.0;
#ifdef VERBOSE
  if ( 4 < verbose )
    {
      cout << "% MESSAGE(Tune):  device=" << camFile_.data() << " ARC=" << ARC_
	   << " SY=" << SY_ << " im=" << im_ << " jm="
	   << jm_ << endl << flush;
    }
#endif

  zap(n);
  zap(m);
  return;  
}

// To compute equivalent
MyCamData::MyCamData(const TsaiData  & C)
  : CameraData(C), datFile_(""), geoFile_(""), d_(NULL), w_(NULL)
{
  derived_ = 1;
}
MyCamData::MyCamData(const MyCamData &C)
  : CameraData(C), datFile_(C.datFile_), geoFile_(C.geoFile_), d_(NULL),
    w_(NULL)
{
  d_ = new Data(*C.d_);
  w_ = new World(*C.w_);
}
MyCamData & MyCamData::operator=(const MyCamData & C)
{
  if ( this == &C ) return *this;  // Allows self-assignment.
  datFile_ = C.datFile_;
  geoFile_ = C.geoFile_;
  d_       = new Data(*C.d_);
  w_       = new World(*C.w_);
  return *this;                    // Allows stacked assignments.
}

// Correct for aberration, used in extCal
void MyCamData::correctRx()
{
  CameraData::correctRx( d_->OApPtr() );
  CameraData::correctRx( d_->OBpPtr() );
  CameraData::correctRx( d_->OCpPtr() );
}

// Center all data to optical center
void MyCamData::center()
{
  d_->center(imWidthCalEx_, imHeightCalEx_, ARC_);
}

// Normalize all data
void MyCamData::norm()
{
  d_->norm();
}

// Display calibration
// Check ACPW
int MyCamData::checkCosines ()
{
  for (int i=0; i<3; i++)
    {
      for (int j=0; j<3; j++)
	{
	  if ( 1.0 < fabs((*(ACPW_))[i][j]) ) return(1);
	}
    }
  return(0);
}  

ostream & operator<< (ostream & out, MyCamData &C)
{
  // Print CameraData
  out << C.print();

  // Display pixel data
  if ( !C.derived_ ) out << *(C.d_) << endl;

  // Display World
  if ( !C.derived_ ) out << *(C.w_) << endl;

  return out;
}

////////////////////// TsaiData //////////////////////////////////////////////
TsaiData::TsaiData(const char tamFile[], int n, int m)
  : CameraData(), tamFile_(NULL),
    Ncx_(0), Nfx_(0), dx_(0), dy_(0), dpx_(0), dpy_(0), Cx_(0),
    Cy_(0), sx_(0), f_(0), kappa1_(0), Tx_(0), Ty_(0), Tz_(0), Rx_(0), Ry_(0),
    Rz_(0), p1_(0), p2_(0), R_(NULL), T_(NULL)
{
  derived_ = 0;
  T_       = new Vector <float> (3, 0.0);
  R_       = new Matrix <float> (3, 3, 0.0);
  tamFile_ = new char[MAX_CANON];
  strcpy(tamFile_, tamFile);

  // Determine file root.
  const char *delims = "/.";
  stringset   in_fileS(tamFile_, delims);
  fileroot_ = in_fileS.token(in_fileS.size()-2);

  // Read files
  InFile *ptrTamFile  = new InFile("TsaiData", tamFile_, 128, MAX_CANON);
  ptrTamFile->load();
  ptrTamFile->stripComments("#");
  ptrTamFile->stripComments("%");
  ptrTamFile->stripBlankLines();
  ptrTamFile->tokenize(" ,=\t\n()[];");
  if ( 19 == ptrTamFile->numLines()
       && 1 == ptrTamFile->maxLineLength() )  // Read in Tsai format
    {
      double sa, ca, sb, cb, sg, cg;
      Matrix <float> Rinv(3, 3);
      float SXN, SYM, im, jm, imCalEx, jmCalEx;

      Ncx_ = atof(ptrTamFile->token(0, 0).data());
      Nfx_ = atof(ptrTamFile->token(1, 0).data());
      dx_  = atof(ptrTamFile->token(2, 0).data());
      dy_  = atof(ptrTamFile->token(3, 0).data());
      dpx_ = atof(ptrTamFile->token(4, 0).data());
      dpy_ = atof(ptrTamFile->token(5, 0).data());
      Cx_  = atof(ptrTamFile->token(6, 0).data());
      Cy_  = atof(ptrTamFile->token(7, 0).data());
      sx_  = atof(ptrTamFile->token(8, 0).data());
      f_   = atof(ptrTamFile->token(9, 0).data());
      kappa1_ = atof(ptrTamFile->token(10, 0).data());
      Tx_  = atof(ptrTamFile->token(11, 0).data());
      Ty_  = atof(ptrTamFile->token(12, 0).data());
      Tz_  = atof(ptrTamFile->token(13, 0).data());
      Rx_  = atof(ptrTamFile->token(14, 0).data());
      Ry_  = atof(ptrTamFile->token(15, 0).data());
      Rz_  = atof(ptrTamFile->token(16, 0).data());
      p1_  = atof(ptrTamFile->token(17, 0).data());
      p2_  = atof(ptrTamFile->token(18, 0).data());
      imWidthCalEx_  = int(Nfx_);  // Assumed
      imHeightCalEx_ = int(float(imWidthCalEx_*m)/float(n)); // Assumed
      if ( n != imWidthCalEx_ ) 
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " : requested size " << n << " does not match stored size "
	       << imWidthCalEx_ << "  Check and try again.\n" << flush;
	  zap(ptrTamFile);
	  exit(1);
	}
#ifdef VERBOSE
      if ( 4 < verbose )
	cout <<  "WARNING(ump): " << __FILE__ << " : " << __LINE__ 
	     << " :  assuming Tsai calibration was "
	     << imWidthCalEx_ << "x" << imHeightCalEx_ 
	     << ".\nIf not, modify Nfx, n, or m\n"
	     << flush;
#endif
      SXN     = float(n) / float(imWidthCalEx_);
      SYM     = float(m) / float(imHeightCalEx_);
      focalCalEx_= f_ / dpx_*sx_;
      AbConstant_= 1.0;
      Abrqw2_ = kappa1_ * float(n*n) * dpx_* dpx_ / SXN / SXN / sx_ / sx_;
      ARC_    = float(imWidthCalEx_) * dpx_ / float(imHeightCalEx_) / dpy_ /sx_;
      SY_     = dpy_ / dpx_ * sx_ * SXN / SYM;
      im      = Cx_*SXN;
      jm      = Cy_*SY_*SYM;
      imCalEx = im / SXN;
      jmCalEx = jm / SYM;
      (*T_)[0] = Tx_ / (-25.4);
      (*T_)[1] = Ty_ / (-25.4);
      (*T_)[2] = Tz_ / (-25.4);
      sa = sin(Rx_);
      ca = cos(Rx_);
      sb = sin(Ry_);
      cb = cos(Ry_);
      sg = sin(Rz_);
      cg = cos(Rz_);
      (*R_)[0][0] = cb * cg;
      (*R_)[0][1] = cg * sa * sb - ca * sg;
      (*R_)[0][2] = sa * sg + ca * cg * sb;
      (*R_)[1][0] = cb * sg;
      (*R_)[1][1] = sa * sb * sg + ca * cg;
      (*R_)[1][2] = ca * sb * sg - cg * sa;
      (*R_)[2][0] =-sb;
      (*R_)[2][1] = cb * sa;
      (*R_)[2][2] = ca * cb;
      Rinv     = inv(*R_);
      *(P_)    = Rinv * (*T_);
      *(ACPW_) = Rinv;
      for (int i=0; i<3; i++)(*(ACPW_))[i][1] *= -1.0;
      for (int i=0; i<5; i++)for ( int j=0; j<7; j++)
	{
	  (*(colpGrid_))[i][j] = 0.0;
	  (*(rowpGrid_))[i][j] = 0.0;
	}
    }
  else
    {
      cerr << "ERROR(TsaiData): " << __FILE__ << " : " << __LINE__ << " : " 
	   << "unrecognized format in " << tamFile_ 
	   << ".\nEnter vectors as follows, one per line,"
	   << " lines in any order:\n" << flush;
      zap(ptrTamFile);
      exit(1);
    }
  zap(ptrTamFile);
}
TsaiData::TsaiData(const TsaiData & T)
  : CameraData(), tamFile_(NULL),
    Ncx_(0), Nfx_(0), dx_(0), dy_(0), dpx_(0), dpy_(0), Cx_(0),
    Cy_(0), sx_(0), f_(0), kappa1_(0), Tx_(0), Ty_(0), Tz_(0), Rx_(0), Ry_(0),
    Rz_(0), p1_(0), p2_(0), R_(NULL), T_(NULL)
{
  derived_ = T.derived_;
  T_       = new Vector <float> (*T.T_);
  R_       = new Matrix <float> (*T.R_);
}
TsaiData::TsaiData(const MyCamData & C, int n, int m)
  : CameraData(C), tamFile_(NULL),
    Ncx_(0), Nfx_(0), dx_(0), dy_(0), dpx_(0), dpy_(0), Cx_(0),
    Cy_(0), sx_(0), f_(0), kappa1_(0), Tx_(0), Ty_(0), Tz_(0), Rx_(0), Ry_(0),
    Rz_(0), p1_(0), p2_(0), R_(NULL), T_(NULL)
{
  derived_ = 1;
  T_       = new Vector <float> (3, 0.0);
  R_       = new Matrix <float> (3, 3, 0.0);
  double sa, ca, sb, cb, sg, cg;
  float SXN, SYM, SY, im, jm, imCalEx, jmCalEx;

  // Conversions
  SY     = float(n) / float(m)    / ARC_;
  SXN    = float(n) / float(imWidthCalEx_);
  SYM    = float(m) / float(imHeightCalEx_);
  im     = float(n-1) / 2.0;
  jm     = float(m-1) / 2.0;
  imCalEx= im / SXN;
  jmCalEx= jm / SYM;
  Cx_    = im / SXN;
  Cy_    = jm / SYM;
  *R_    = inv(*(ACPW_));
  *T_    = (*R_) * (*P_);
  Tx_    =-25.4 * ((*T_)[0]);
  Ty_    = 25.4 * ((*T_)[1]);
  Tz_    =-25.4 * ((*T_)[2]);
  sb     =-(*R_)[2][0];
  Ry_    = asin(sb);
  cb     = cos(Ry_);
  cg     = (*R_)[0][0]/cb;
  Rz_    = -acos(cg);
  ca     = (*R_)[2][2]/cb;
  Rx_    = acos(ca);
  sa     = sin(Rx_);
  sg     = sin(Rz_);
}

// Display calibration
ostream & operator<< (ostream & out, TsaiData &C)
{
  if ( !C.derived_ )
    {
      out << "#MESSAGE(Tune): cal for device " << C.fileroot_ << " :\n"
	  << "\nTsai:\n"
	  << "       Ncx    =  " << C.Ncx_ << endl
	  << "       Nfx    =  " << C.Nfx_ << " assumed\n"
	  << "       dx     =  " << C.dx_  << endl
	  << "       dy     =  " << C.dy_  << endl
	  << "       dpx    =  " << C.dpx_ << endl
	  << "       dpy    =  " << C.dpy_ << endl
	  << "       f      =  " << C.f_      << "  [mm]\n"
	  << "       kappa1 =  " << C.kappa1_ << "  [1/mm^2]\n"
	  << "       Tx     =  "
	  << C.Tx_
	  << ",  Ty = " << C.Ty_ 
	  << ",  Tz = " << C.Tz_
	  << "  [mm]\n"
	  << "       Rx     = "
	  <<  C.Rx_*180/3.1415926
	  << ",  Ry = "    <<  C.Ry_*180/3.1415926
	  << ",  Rz = "    <<  C.Rz_*180/3.1415926
	  << "  [deg]\n"
	  << "       R=   [";
      for (int i=0; i<3; i++)
	{
	  for (int j=0; j<3; j++)
	    {
	      out << setprecision(6) << setw(10) << (*C.R_)[i][j] << " ";
	    }
	  if ( i < 2 ) out << "\n            ";
	}
      out << " ]\n";
      out << "       sx = " << C.sx_ << endl
	  << "       Cx = " << C.Cx_ << ",  Cy = " << C.Cy_ << "  [pixels]\n"
	  << "       Tz / f = " << C.Tz_ / C.f_ << endl << flush;
    }
  else  // derived
    {
      out << "#MESSAGE(Tune): cal for device " << C.fileroot_ << " :\n"
	  << "\nTsai Equivalent:\n"
	  << "Ncx=   " << "unknown" << endl
	  << "Nfx=   " << C.imWidthCalEx_ << " assumed\n"
	  << "dx=    " << "unknown" << endl
	  << "dy=    " << "unknown" << endl
	  << "dpx=   " << "unknown" << endl
	  << "dpy=   " << "unknown" << endl
	  << "Cx=    " << C.Cx_ << endl
	  << "Cy=    " << C.Cy_ << endl
	  << "sx=    " << "unknown" << endl
	  << "f=     " << "unknown" << endl
	  << "kappa1=" << "unknown" << endl
	  << "Tx=    " << C.Tx_ << endl
	  << "Ty=    " << C.Ty_ << endl
	  << "Tz=    " << C.Tz_ << endl
	  << "Rx=    " << C.Rx_ << endl
	  << "Ry=    " << C.Ry_ << endl
	  << "Rz=    " << C.Rz_ << endl
	  << "p1=    " << "unknown" << endl
	  << "p2=    " << "unknown" << endl << flush;
    }

  return out;
}


/////////////////////////class Calibration/////////////////////////////

// Constructors.
Calibration::Calibration()
  : calFile_(NULL), imCalEx_(0), jmCalEx_(0), calFileExists_(0){}

Calibration::Calibration(const char calFile[], int *n, int *m)
  : CameraData(calFile), calFile_(NULL), imCalEx_(0), jmCalEx_(0),
    calFileExists_(0)
{
  // Initialize
  calFile_ = new char[MAX_CANON];
  strcpy(calFile_, calFile);
  calFileExists_ = camFileExists_;
  if ( calFileExists_ )
    {
      if ( imWidthCalEx_ < *n || imHeightCalEx_ < *m )
	{
#ifdef VERBOSE
	  if ( 4 < verbose )
	    cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
		 << " :  calibration image size= " << imWidthCalEx_ << "x"
		 << imHeightCalEx_ << " for device " << calFile_ 
		 << "\nis at lower resolution than present setting of " << *n
		 << "x" << *m << ".\n" << flush;
#endif
	}
      if ( 0 == *n && 0 == *m )
	{
	  *n = imWidthCalEx_;
	  *m = imHeightCalEx_;
	}
      if ( 0 == *n || 0 == *m )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " :  size for device " << calFile_ 
	       << " is " << *n << "x" << *m << ".\n" << flush;
	  exit(1);
	}
    }
  else
    {
      *n             = MAX(*n, 1);
      *m             = MAX(*m, 1);
      imWidthCalEx_  =     *n;
      imHeightCalEx_ =     *m;
      focalCalEx_    =      0;
      AbConstant_    =    1.0;
      Abrqw2_        =    0.0;
      AbipimR_       =    0.0;
      AbjpjmR_       =    0.0;
      ARC_           = 1.3333;
      RgSir_         =      0;
      RgSiri_        =      0;
      RgSjr_         =      0;
      RgSjrj_        =      0;
    }
  
  // Conversions
  float SXN   = float(*n)  / float(imWidthCalEx_);
  float SYM   = float(*m)  / float(imHeightCalEx_);
  if ( !usingGrid_ && !usingSy_ ) SY_ = float(*n)  / float(*m) / ARC_;
  else ARC_ = float(*n)  / float(*m) / SY_;

  // Corrections assume centering at n/2 and m/2 points.  This allows
  // both correction and centering to use raw pixels as lookups.
  im_         = float(*n-1) / 2.0;
  jm_         = float(*m-1) / 2.0;
  imCalEx_    = im_ / SXN;
  jmCalEx_    = jm_ / SYM;
#ifdef VERBOSE
  if ( 4 < verbose )
    {
      cout << "% MESSAGE(Tune):  device=" << calFile_ << " ARC=" << ARC_
	   << " SY=" << SY_ << endl << "imCalEx=" << imCalEx_ << " jmCalEx="
	   << jmCalEx_ << endl << flush;
    }
#endif
}
Calibration::Calibration(const Calibration & C)
  : calFile_(NULL), imCalEx_(C.imCalEx_), jmCalEx_(C.jmCalEx_),
    calFileExists_(C.calFileExists_)
{
  calFile_ = new char[MAX_CANON];
  strcpy(calFile_, C.calFile_);
}
Calibration & Calibration::operator=(const Calibration & P)
{
  if ( this==&P ) return *this;  // Allows self-assignment.
  calFile_ = new char[MAX_CANON];
  strcpy(calFile_, P.calFile_);
  imCalEx_       = P.imCalEx_;
  jmCalEx_       = P.jmCalEx_;
  calFileExists_ = P.calFileExists_;
  return *this;  // Allows stacked assignments.
}

// Correct for aberration
void Calibration::correctRx(pixelLocf *c)
{
  CameraData::correctRx( c );
}

// Correct for register error
void Calibration::correctRg(float *x, float *y, int n, int m)
{
          *x += ( float(n) / float(imWidthCalEx_)  - 1.0 ) * 
	    (RgSir_  + RgSiri_* *x);

	  *y += ( float(m) / float(imHeightCalEx_) - 1.0 ) *
	    (RgSjr_  + RgSjrj_* *y);
}


// Display calibration
ostream & operator<< (ostream & out, Calibration &C)
{
  out << C.print();
  return out;
}

/////////////////////////class Tune/////////////////////////////

// Constructors.
Tune::Tune()
  : tunFile_(NULL),
    agaussScalar_(1.0), bgaussScalar_(1.0), minBallDensityScalar_(1.0),
    minBallAreaScalar_(1.0), maxBallAreaScalar_(1.0),
    minBallRatioScalar_(1.0), maxBallRatioScalar_(1.0),
    minProxScalar_(1.0)
{
  tunFile_ = new char[MAX_CANON];
  strcpy(tunFile_, "video?.tune");
}

Tune::Tune(int device)
  : tunFile_(NULL),
    agaussScalar_(1.0), bgaussScalar_(1.0), minBallDensityScalar_(1.0),
    minBallAreaScalar_(1.0), maxBallAreaScalar_(1.0),
    minBallRatioScalar_(1.0), maxBallRatioScalar_(1.0),
    minProxScalar_(1.0)
{
  tunFile_ = new char[MAX_CANON];
  sprintf(tunFile_, "video%d.tune", device);
}

Tune::Tune(const char tunFile[])
  : tunFile_(NULL),
    agaussScalar_(1.0), bgaussScalar_(1.0), minBallDensityScalar_(1.0),
    minBallAreaScalar_(1.0), maxBallAreaScalar_(1.0),
    minBallRatioScalar_(1.0), maxBallRatioScalar_(1.0),
    minProxScalar_(1.0)
{
  tunFile_ = new char[MAX_CANON];
  strcpy(tunFile_, tunFile);

  // Read file
  InFile *ptrTunFile  = new InFile("Tune", tunFile_, 128, MAX_CANON);
  if ( 0 == ptrTunFile->load() )
    {
#ifdef VERBOSE
      if ( (!calibrate && verbose) || (calibrate && 2 < verbose) ) 
	cerr << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ << " : " 
	     << " : No " << tunFile_ << " or  empty.\n"
	     << "Use entry format as follows:\n"
	     << "agaussScalar         = float distance clumping scalar\n"
	     << "bgaussScalar         = float intensity clumping scalar\n"
	     << "minBallDensityScalar = float min threshold density scalar\n"
	     << "minBallAreaScalar    = float min threshold area scalar\n"
	     << "maxBallAreaScalar    = float max threshold area scalar\n"
	     << "minBallRatioScalar   = float min threshold ratio scalar\n"
	     << "maxBallRatioScalar   = float max threshold ratio scalar\n"
	     << "minProxScalar        = float min distance clump join scalar\n"
	     << "Using defaults and continuing...\n" << flush;
#endif
      goto defaults;
    }
  else
    {
      ptrTunFile->stripComments("#");
      ptrTunFile->stripComments("%");
      ptrTunFile->stripBlankLines();
      if ( !ptrTunFile->loadVars() ) 
	{
	  cerr << "WARNING(Tune): " << __FILE__ << " : " << __LINE__
	       << " no variables in " << tunFile_ << endl
	       << "Using defaults and continuing...\n" << flush;
	  goto defaults;
	}
      GetPar(*ptrTunFile, "agaussScalar",         &agaussScalar_,        1);
      GetPar(*ptrTunFile, "bgaussScalar",         &bgaussScalar_,        1);
      GetPar(*ptrTunFile, "minBallDensityScalar", &minBallDensityScalar_,1);
      GetPar(*ptrTunFile, "minBallAreaScalar",    &minBallAreaScalar_,   1);
      GetPar(*ptrTunFile, "maxBallAreaScalar",    &maxBallAreaScalar_,   1);
      GetPar(*ptrTunFile, "minBallRatioScalar",   &minBallRatioScalar_,  1);
      GetPar(*ptrTunFile, "maxBallRatioScalar",   &maxBallRatioScalar_,  1);
      GetPar(*ptrTunFile, "minProxScalar",        &minProxScalar_,       1);
    }
  
 defaults:

  zap(ptrTunFile);
  return;
}
Tune::Tune(const Tune & T)
  : tunFile_(NULL),
    agaussScalar_(T.agaussScalar_), bgaussScalar_(T.bgaussScalar_),
    minBallDensityScalar_(T.minBallDensityScalar_),
    minBallAreaScalar_(T.minBallAreaScalar_),
    maxBallAreaScalar_(T.maxBallAreaScalar_),
    minBallRatioScalar_(T.minBallRatioScalar_),
    maxBallRatioScalar_(T.maxBallRatioScalar_), minProxScalar_(T.minProxScalar_)
{
  tunFile_ = new char[MAX_CANON];
  strcpy(tunFile_, T.tunFile_);
}
Tune & Tune::operator=(const Tune & T)
{
  if ( this==&T ) return *this;  // Allows self-assignment.
  tunFile_ = new char[MAX_CANON];
  strcpy(tunFile_, T.tunFile_);
  agaussScalar_         = T.agaussScalar_;
  bgaussScalar_         = T.bgaussScalar_;
  minBallDensityScalar_ = T.minBallDensityScalar_;
  minBallAreaScalar_    = T.minBallAreaScalar_;
  maxBallAreaScalar_    = T.maxBallAreaScalar_;
  minBallRatioScalar_   = T.minBallRatioScalar_;
  maxBallRatioScalar_   = T.maxBallRatioScalar_;
  minProxScalar_        = T.minProxScalar_;
  return *this;  // Allows stacked assignments.
}

// Display tune
ostream & operator<< (ostream & out, const Tune &T)
{
  out << "% MESSAGE(Tune): tune for device " << T.tunFile_ << " :\n"
      << "agaussScalar         = " << T.agaussScalar_         << ";\n"
      << "bgaussScalar         = " << T.bgaussScalar_         << ";\n"
      << "minBallDensityScalar = " << T.minBallDensityScalar_ << ";\n"
      << "minBallAreaScalar    = " << T.minBallAreaScalar_    << ";\n"
      << "maxBallAreaScalar    = " << T.maxBallAreaScalar_    << ";\n"
      << "minBallRatioScalar   = " << T.minBallRatioScalar_   << ";\n"
      << "maxBallRatioScalar   = " << T.maxBallRatioScalar_   << ";\n"
      << "minProxScalar        = " << T.minProxScalar_        << ";\n" << flush;
  return out;
}

/////////////////////////class Zone/////////////////////////////
// Describe and handle activity around the strike zone.

// Constructors.
Zone::Zone()
  : zonFile_(NULL),
    dxZone_(0), dzZone_(0), dXdMphZone_(0), dZdMphZone_(0), velThresh_(-500),
    velMax_(-2290), gravity_(386), ballDia_(2.75), triangulationScalar_(1.0),
    left_(-6), top_(100000), right_(6), bottom_(-100000), scale_(0),
    skew_(0), maxh_(22), maxw_(40), dx_(0), colL_(0), colR_(0), lineT_(0),
    lineB_(0), speedDisplayScalar_(17.6), llc_(NULL),  ulc_(NULL), urc_(NULL),
    lrc_(NULL), l_(NULL), t_(NULL), r_(NULL), b_(NULL) {}
Zone::Zone(const char zonFile[])
  :
  zonFile_    (NULL),
  dxZone_        (0),           // defaults
  dzZone_        (0), 
  dXdMphZone_    (0),
  dZdMphZone_    (0), 
  velThresh_(-500.0),           // 28  mph toward plate, in/sec 
  velMax_  (-2290.0),           // 130 mph toward plate, in/sec 
  gravity_   (386.0),           // in/sec/sec
  ballDia_    (2.75),           // in
  triangulationScalar_  (1.0),  // scalar on number of balls uncertainty allowed
  left_       (-6.0),           // in left
  top_    (100000.0),           // disable
  right_      (+6.0),           // in right
  bottom_(-100000.0),           // disable
  scale_         (0),
  skew_          (0),
  maxh_         (22),           // height of drawn window
  maxw_         (40),           // width of drawn window
  dx_            (0),
  colL_          (0),
  colR_          (0),
  lineT_         (0),
  lineB_         (0),
  speedDisplayScalar_(17.6),    // in/sec to mph
  llc_        (NULL),
  ulc_        (NULL),
  urc_        (NULL),
  lrc_        (NULL),
  l_          (NULL),
  t_          (NULL),
  r_          (NULL),
  b_          (NULL)
{
  zonFile_ = new char[MAX_CANON];
  strcpy(zonFile_, zonFile);

  // Read file
  InFile *ptrZonFile  = new InFile("Zone", zonFile_, 128, MAX_CANON);
  if ( 0 == ptrZonFile->load() )
    {
#ifdef VERBOSE
      if ( (!calibrate && verbose) || (calibrate && 2 < verbose) ) 
	cerr << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ << " : " 
	     << "No " << zonFile_ << " or  empty.\n"
	     << "Use entry format as follows, match units of world from \n"
	     << "extCal phase of operation and umpire's point of view:\n"
	     << "dxZone     = float adjustment to calculated ball x impact\n"
	     << "dzZone     = float adjustment to calculated ball z impact\n"
	     << "dXdMphZone = float adjustment to calculated ball x impact\n"
	     << "dXdMphZone = float adjustment to calculated ball z impact\n"
	     << "velThresh  = float lower limit screening speed for ball\n"
	     << "velMax     = float upper limit screening speed for ball\n"
	     << "gravity    = float gravity constant\n"
	     << "ballDia    = float ball diameter\n"
	     << "triangulationScalar =float number of ball dia uncert allowed\n"
	     << "left       = float left side strike zone\n"
	     << "top        = float top side strike zone\n"
	     << "right      = float right side strike zone\n"
	     << "bottom     = float bottom side strike zone\n"
	     << "speedDisplayScalar = float conversion velocity to speed\n"
	     << "Using defaults and continuing...\n" << flush;
#endif
      goto defaults;
    }
  else
    {
      ptrZonFile->stripComments("#");
      ptrZonFile->stripComments("%");
      ptrZonFile->stripBlankLines();
      if ( !ptrZonFile->loadVars() ) 
	{
	  cerr << "WARNING(Tune): " << __FILE__ << " : " << __LINE__
	       << " no variables in " << zonFile_ << endl
	       << "Using defaults and continuing...\n" << flush;
	  goto defaults;
	}
      GetPar(*ptrZonFile, "dxZone",    &dxZone_,    1);
      GetPar(*ptrZonFile, "dzZone",    &dzZone_,    1);
      GetPar(*ptrZonFile, "dXdMphZone",&dXdMphZone_,1);
      GetPar(*ptrZonFile, "dZdMphZone",&dZdMphZone_,1);
      GetPar(*ptrZonFile, "velThresh", &velThresh_, 1);
      GetPar(*ptrZonFile, "velMax",    &velMax_,    1);
      GetPar(*ptrZonFile, "gravity",   &gravity_,   1);
      GetPar(*ptrZonFile, "ballDia",   &ballDia_,   1);
      GetPar(*ptrZonFile, "triangulationScalar",   &triangulationScalar_, 1);
      GetPar(*ptrZonFile, "left",      &left_,      1);
      GetPar(*ptrZonFile, "top",       &top_,       1);
      GetPar(*ptrZonFile, "right",     &right_,     1);
      GetPar(*ptrZonFile, "bottom",    &bottom_,    1);
      GetPar(*ptrZonFile, "speedDisplayScalar", &speedDisplayScalar_, 1);
    }
 defaults:
  if ( ptrZonFile ) zap(ptrZonFile);

  // Setup.  Edge of ball touches for strike - account for ball dia.
  if ( left() >= right() )
    {
      cerr << "MESSAGE(Tune): zone in file " << zonFile_ << " :\n"
	   << "left side wrong side of right side.\n" << flush;
      exit(1);
    }
  if ( bottom() >= top() )
    {
      cerr << "MESSAGE(Tune): zone in file " << zonFile_ << " :\n"
	   << "top side wrong side of bottom side.\n" << flush;
      exit(1);
    }
  llc_  = new Point <float> (left_  - ballDia_/2.0, bottom_ - ballDia_/2.0);
  ulc_  = new Point <float> (left_  - ballDia_/2.0, top_    + ballDia_/2.0);
  urc_  = new Point <float> (right_ + ballDia_/2.0, top_    + ballDia_/2.0);
  lrc_  = new Point <float> (right_ + ballDia_/2.0, bottom_ - ballDia_/2.0);
  l_    = new Line  <float> (*llc_, *ulc_);
  t_    = new Line  <float> (*ulc_, *urc_);
  r_    = new Line  <float> (*lrc_, *urc_);
  b_    = new Line  <float> (*llc_, *lrc_);
  skew_ = maxw_ / float(maxh_); 
  dx_   = right_ - left_;
  scale_= float(maxw_) / dx_ / 2.0 / skew_;
  maxh_ = int( MIN( 1.2*top_, 5*dx_ ) * scale_ ) + 1;
  colL_ = int( (dx_ +  left_) * scale_ * skew_ );
  colR_ = int( (dx_ + right_) * scale_ * skew_ );
  lineT_= maxh_ - MIN( int( top_ *    scale_ ), maxh_ );
  lineB_= maxh_ - MAX( int( bottom_ * scale_ ),    0 );
}
Zone::Zone(const Zone & Z)
  : zonFile_(NULL),
    dxZone_(Z.dxZone_), dzZone_(Z.dzZone_), dXdMphZone_(Z.dXdMphZone_),
    dZdMphZone_(Z.dZdMphZone_), velThresh_(Z.velThresh_), velMax_(Z.velMax_),
    gravity_(Z.gravity_), ballDia_(Z.ballDia_),
    triangulationScalar_(Z.triangulationScalar_), left_(Z.left_), top_(Z.top_),
    right_(Z.right_), bottom_(Z.bottom_), scale_(Z.scale_), skew_(Z.skew_),
    maxh_(Z.maxh_), maxw_(Z.maxw_), dx_(Z.dx_), colL_(Z.colL_), colR_(Z.colR_),
    lineT_(Z.lineT_), lineB_(Z.lineB_),
    speedDisplayScalar_(Z.speedDisplayScalar_), llc_(NULL), ulc_(NULL),
    urc_(NULL), lrc_(NULL), l_(NULL), t_(NULL), r_(NULL), b_(NULL)
{
  zonFile_ = new char[MAX_CANON];
  strcpy(zonFile_, Z.zonFile_);
  llc_  = new Point <float> (*Z.llc_);
  ulc_  = new Point <float> (*Z.ulc_);
  urc_  = new Point <float> (*Z.urc_);
  lrc_  = new Point <float> (*Z.lrc_);
  l_    = new Line  <float> (*Z.l_);
  t_    = new Line  <float> (*Z.t_);
  r_    = new Line  <float> (*Z.r_);
  b_    = new Line  <float> (*Z.b_);
}
Zone & Zone::operator=(const Zone & Z)
{
  if ( this == &Z ) return *this;  // Allows self-assignment.
  zonFile_ = NULL;
  dxZone_  = Z.dxZone_;
  dzZone_  = Z.dzZone_;
  dXdMphZone_ = Z.dXdMphZone_;
  dZdMphZone_ = Z.dZdMphZone_;
  velThresh_  = Z.velThresh_;
  velMax_     = Z.velMax_;
  gravity_    = Z.gravity_;
  ballDia_    = Z.ballDia_;
  triangulationScalar_ = Z.triangulationScalar_;
  left_    = Z.left_; 
  top_     = Z.top_;
  right_   = Z.right_;
  bottom_  = Z.bottom_;
  maxh_    = Z.maxh_;
  maxw_    = Z.maxw_;
  dx_      = Z.dx_;
  colL_    = Z.colL_;
  colR_    = Z.colR_;
  lineT_   = Z.lineT_;
  lineB_   = Z.lineB_;
  speedDisplayScalar_ = Z.speedDisplayScalar_;
  llc_ = NULL; 
  ulc_ = NULL;
  urc_ = NULL;
  lrc_ = NULL;
  l_   = NULL;
  t_   = NULL;
  r_   = NULL;
  b_   = NULL;
  zonFile_ = new char[MAX_CANON];
  strcpy(zonFile_, Z.zonFile_);
  llc_  = new Point <float> (*Z.llc_);
  ulc_  = new Point <float> (*Z.ulc_);
  urc_  = new Point <float> (*Z.urc_);
  lrc_  = new Point <float> (*Z.lrc_);
  l_    = new Line  <float> (*Z.l_);
  t_    = new Line  <float> (*Z.t_);
  r_    = new Line  <float> (*Z.r_);
  b_    = new Line  <float> (*Z.b_);
  return *this;                    // Allows stacked assignments.
}

// Call strike
string Zone::call(const float x, const float y)
{
  string returnVal = "CALLED BALL";
  if      ( l_->horizDist(x, y) > 0 ) returnVal = "CALLED BALL OUTSIDE LEFT";
  else if ( r_->horizDist(x, y) < 0 ) returnVal = "CALLED BALL OUTSIDE RIGHT";
  else if ( t_->vertDist(x, y)  < 0 ) returnVal = "CALLED BALL HIGH";
  else if ( b_->vertDist(x, y)  > 0 ) returnVal = "CALLED BALL LOW";
  else                                returnVal = "CALLED STRIKE";
  int   linePt   = maxh_ - MAX( int( y * scale_ ), 0);
  int   colPt    = int( (dx_ + x) * scale_ * skew_ );
  int   lineK    = 1;
  if ( linePt < maxh_ / 2 ) lineK = maxh_-1;
  for (int i=0; i<=maxh_; i++)
    {
      if ( lineK == i )
	{
	  cout << "KEY:  X = OBJECT, BOX = STRIKE ZONE\n" << flush;
	}
      else
	{
	  for (int j=0; j<maxw_; j++)
	    {
	      if ( linePt == i && colPt == j ) cout << "X";
	      else if ( ( colL_ < j && colR_ > j )
			&& ( lineT_ == i || lineB_ == i) ) cout << "-";
	      else if ( ( lineB_ >= i && lineT_ <= i ) 
			&& ( colL_  == j || colR_  == j ) ) cout << "|";
	      else cout << " ";
	    }
	  cout << endl << flush;
	}
    }
  return returnVal;
}

// Display zone
ostream & operator<< (ostream & out, const Zone &Z)
{
  out << "% MESSAGE(Tune): zone for device " << Z.zonFile_ << " :\n"
      << "dxZone               = "  << setprecision(6) << setw(11) 
      << Z.dxZone_         << ";\n"
      << "dzZone               = "  << setprecision(6) << setw(11) 
      << Z.dzZone_         << ";\n"
      << "dXdMphZone           = "  << setprecision(6) << setw(11) 
      << Z.dXdMphZone_     << ";\n"
      << "dZdMphZone           = "  << setprecision(6) << setw(11) 
      << Z.dZdMphZone_     << ";\n"
      << "velThresh            = "  << setprecision(6) << setw(11) 
      << Z.velThresh_      << ";\n"
      << "velMax               = "  << setprecision(6) << setw(11) 
      << Z.velMax_         << ";\n"
      << "gravity              = "  << setprecision(6) << setw(11) 
      << Z.gravity_        << ";\n"
      << "ballDia              = "  << setprecision(6) << setw(11) 
      << Z.ballDia_        << ";\n"
      << "triangulationScalar  = "  << setprecision(6) << setw(11) 
      << Z.triangulationScalar_ << ";\n"
      << "left                 = "  << setprecision(6) << setw(11) 
      << Z.left_           << ";\n"
      << "top                  = "  << setprecision(6) << setw(11) 
      << Z.top_            << ";\n"
      << "right                = "  << setprecision(6) << setw(11) 
      << Z.right_          << ";\n"
      << "bottom               = "  << setprecision(6) << setw(11) 
      << Z.bottom_         << ";\n"
      << "speedDisplayScalar   = "  << setprecision(6) << setw(11) 
      << Z.speedDisplayScalar_ << ";\n" << flush;
  return out;
}

/////////////////////////////////utilities//////////////////////////

int GetPar(InFile &I, const string name, int *i, const int enWarn)
{
  int numVal = 0;
  if ( (numVal = I.getPar(name, i)) )
    {
      if ( 1 < numVal )
	{
	  cout << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " using last " << name << ".\n" << flush;
	}
      else if ( 0 > numVal )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " bad int value in " << I.name() << ".  Quitting"
	       << endl << flush;
	  return(-1);
	}
    }
  else if ( enWarn )
    cout << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	 << " Using default " << name << endl << flush;
  return (0);
}

int GetPar(InFile &I, const string name, float *f, const int enWarn )
{
  int numVal = 0;
  if ( (numVal = I.getPar(name, f)) )
    {
      if ( 1 < numVal )
	{
	  cout << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " using last " << name << "=" << *f << ".\n" << flush;
	  return (-1);
	}
      else if ( 0 > numVal )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " bad float value in " << I.name() << ".  Quitting"
	       << endl << flush;
	  return(-1);
	}
    }
  else
    {
      if ( enWarn ) cout << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
			 << " Using default " << name << "=" << *f
			 << endl << flush;
      return (-1);
    }

  return (0);
}

int GetPar(InFile &I, const string name, PixelLoc3 *P3, const int enWarn)
{
  int numVal = 0;
  if ( (numVal = I.getPar(name, *P3)) )
    {
      if ( 1 < numVal )
	{
	  cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " using last " << name << ".\n" << flush;
	}
      else if ( 0 > numVal )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " bad PixelLoc3 value in " << I.name() << ".  Quitting"
	       << endl << flush;
	  return(-1);
	}
    }
  else if ( enWarn )
    cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	 << " Using default " << name << endl << flush;
  return (0);
}

int GetPar(InFile &I, const string name, PixelLoc3 *P3, const int nP,
	   const int enWarn)
{
  int numVal = 0;
  if ( (numVal = I.getPar(name, *P3, nP)) )
    {
      if ( 1 < numVal )
	{
	  cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " using last " << name << ".\n" << flush;
	}
      else if ( 0 > numVal )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " bad PixelLoc3 value in " << I.name() << ".  Quitting"
	       << endl << flush;
	  return(-1);
	}
    }
  else if ( enWarn )
    cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	 << " Using default " << name << endl << flush;
  return (0);
}

int GetPar(InFile &I, const string name, Vector <float> *V, const int enWarn)
{
  int numVal = 0;
  if ( (numVal = I.getPar(name, *V)) )
    {
      if ( 1 < numVal )
	{
	  cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " using last " << name << ".\n" << flush;
	}
      else if ( 0 > numVal )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " bad Vector value in " << I.name() << ".  Quitting"
	       << endl << flush;
	  return(-1);
	}
    }
  else if ( enWarn )
    cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	 << " Using default " << name << endl << flush;
  return (0);
}

int GetPar(InFile &I, const string name, Matrix <float> *M, const int enWarn)
{
  int numVal = 0;
  if ( (numVal = I.getPar(name, *M)) )
    {
      if ( 1 < numVal )
	{
	  cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " using last " << name << ".\n" << flush;
	}
      else if ( 0 > numVal )
	{
	  cerr <<  "ERROR(Tune): " << __FILE__ << " : " << __LINE__ 
	       << " bad Matrix value in " << I.name() << endl << flush;
	  return(-1);
	}
    }
  else if ( enWarn )
    cerr <<  "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
	 << " Using default " << name << endl << flush;
  return (0);
}






