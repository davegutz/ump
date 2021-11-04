// Calibration data
// 	$Id: Tune.h,v 1.1 2007/12/16 21:43:21 davegutz Exp davegutz $
// D Gutz
#ifndef Tune_h
#define Tune_h 1
#include <second.h>    // tnt matrix stuff
#include <PixelLoc.h>  // Pixel stuff
#include <Segment.h>   // Point, Line stuff
//#include <String.h>    // String class
#include <Mask.h>      // Mask class

///////////////////////World class///////////////////////////////
class World
{
 protected:
  string    geoFile_;
  PixelLoc3 Pl_;
  PixelLoc3 Pr_;
  PixelLoc3 OA_;
  PixelLoc3 OB_;
  PixelLoc3 OC_;
  int       geoFileExists_;
 public:
  World();
  World(const char geoFile[]);
  World(const World & W);
  World & operator=(const World & W);
  ~World() {}

  // Element access
  friend ostream & operator<< (ostream & out, const World & W);
  PixelLoc3 Pl()              {return Pl_;}
  PixelLoc3 Pr()              {return Pr_;}
  PixelLoc3 OA()              {return OA_;}
  PixelLoc3 OB()              {return OB_;}
  PixelLoc3 OC()              {return OC_;}
  int       geoFileExists()   {return geoFileExists_;}
};


///////////////////////Data class///////////////////////////////
class Data
{
 protected:
  string      datFile_;
  PixelLoc3       OAp_;                 // Normed pixel A
  PixelLoc3       OBp_;                 // Normed pixel B
  PixelLoc3       OCp_;                 // Normed pixel C
  PixelLoc3       OApo_;                // Saved original pixel A
  PixelLoc3       OBpo_;                // Saved original pixel B
  PixelLoc3       OCpo_;                // Saved original pixel C
  int             datFileExists_;       // If source file exists
 public:
  Data();
  Data(const char datFile[], const float focalPix);
  Data(const Data & D);
  Data & operator=(const Data & D);
  ~Data() {}

  // Element access
  friend ostream & operator<< (ostream & out, const Data & D);
  PixelLoc3 OAp()             {return OAp_;}
  PixelLoc3 OBp()             {return OBp_;}
  PixelLoc3 OCp()             {return OCp_;}
  PixelLoc3 *OApPtr()         {return &OAp_;}
  PixelLoc3 *OBpPtr()         {return &OBp_;}
  PixelLoc3 *OCpPtr()         {return &OCp_;}
  PixelLoc3 OApo()            {return OApo_;}
  PixelLoc3 OBpo()            {return OBpo_;}
  PixelLoc3 OCpo()            {return OCpo_;}
  PixelLoc3 OApo(const float p0, const float p1, const float p2)
    {
      OApo_[0] = OAp_[0] = p0;
      OApo_[1] = OAp_[1] = p1;
      OApo_[2] = OAp_[2] = p2;
      return OApo_;}
  PixelLoc3 OBpo(const float p0, const float p1, const float p2)
    {
      OBpo_[0] = OBp_[0] = p0;
      OBpo_[1] = OBp_[1] = p1;
      OBpo_[2] = OBp_[2] = p2;
      return OBpo_;
    }
  PixelLoc3 OCpo(const float p0, const float p1, const float p2)
    {
      OCpo_[0] = OCp_[0] = p0;
      OCpo_[1] = OCp_[1] = p1;
      OCpo_[2] = OCp_[2] = p2;
      return OCpo_;
    }
  int       datFileExists()    {return datFileExists_;}

  // Functions
  void center(int imWidthCalEx, int imHeightCalEx, float ARC);
  void center(PixelLoc3 &P, int imWidthCalEx, int imHeightCalEx, float ARC);
  void norm();
};


///////////////////////CameraData class///////////////////////////////
class CameraData
{
 private:
  void      initializeCorrectRx();                     // Worker function
 protected:
  int             derived_;             // object derived
  int             imWidthCalEx_;        // Calibration width,   pixels
  int             imHeightCalEx_;       // Calibration height,  pixels
  float           focalCalEx_;          // Camera focal length, pixels
  float           AbConstant_;          // Aberration intercept
  float           Abrqw2_;              // Aberration radial distortion
  float           AbipimR_;             // Aberration x linear correction
  float           AbjpjmR_;             // Aberration y linear correction
  float           ARC_;                 // Image aspect ratio
  float           RgSir_;               // Register width bias scalar
  float           RgSiri_;              // Register width*x bias scalar
  float           RgSjr_;               // Register height bias scalar
  float           RgSjrj_;              // Register height*y bias scalar
  string          fileroot_;            // Camera file root name
  string          camFile_;             // Camera file path
  int             camFileExists_;       // Whether data loaded from file
  int             usingGrid_;           // Whether colpGrid loaded OK
  int             usingSy_;             // If SY specified - overrides all else
  int             gridPrepped_;         // Whether arrays instantiated
  float           SY_;                  // Row scale correction, applied ctr
  float           Sy_;                  // Input SY_ from application
  int             nUser_;               // Calling routine raw image size
  int             mUser_;               // Calling routine raw image size
  float           im_;                  // Optical center, raw pixels
  float           jm_;                  // Optical center, raw pixels
  float           *xr_;                 // User aberration correction array
  float           *yr_;                 // User aberration correction array
  Matrix <float>  *ACPW_;               // Pixel to world rotation matrix
  Vector <float>  *P_;                  // World position of camera
  Matrix <float>  *colpGrid_;           // Horizontal raw pixel locations grid
  Matrix <float>  *rowpGrid_;           // Vertical raw pixel locations grid
  float           *ipCORR_;             // i pixel correction table values
  float           *jpCORR_;             // j pixel correction table values
  float           *iCORR_;              // Table breakpoints
  float           *jCORR_;              // Table breakpoints
  Curve2          *rowpcorr_;           // Grid corrections
  Curve2          *colpcorr_;           // Grid corrections
 public:
  CameraData();
  CameraData(const char *camFile);
  CameraData(const CameraData &C);
  CameraData & operator=(const CameraData & C);
  virtual ~CameraData()
    {
      if ( !derived_ )
	{
	  zap(ACPW_);
	  zap(P_);
	  zap(colpGrid_);
	  zap(rowpGrid_);
	  if ( usingGrid_ )
	    {
	      if ( gridPrepped_ )
		{
		  zap(colpcorr_);
		  zap(rowpcorr_);
		  zaparr(jpCORR_);
		  zaparr(ipCORR_);
		  zaparr(jCORR_);
		  zaparr(iCORR_);
		}
	    }
	  if ( 0<nUser_ && 0<mUser_ )
	    {
	      zaparr(xr_);
	      zaparr(yr_);
	    }
	}
    }

  // Functions
  void      prepGrid();
  float     colpCorr(const int col, const int row, const int n, const int n);
  float     rowpCorr(const int col, const int row, const int n, const int m);
  void      initCorrectRx();                           // Call before correctRx
  void      initCorrectRx(const int n, const int m);   // Call before correctRx
  void      correctRx(float *x, float *y);             // Fast correction
  void      correctRx(PixelLoc3 *pPtr);                // Fast correction
  void      correctRx(pixelLocf *pPtr);                // Fast correction
  void      distort(float *x, float *y);

  // Element access
  string    print();
  int       imWidthCalEx()    {return imWidthCalEx_;}
  int       imHeightCalEx()   {return imHeightCalEx_;}
  float     focalCalEx()      {return focalCalEx_;}
  float     AbConstant()      {return AbConstant_;}
  float     Abrqw2()          {return Abrqw2_;}
  float     AbipimR()         {return AbipimR_;}
  float     AbjpjmR()         {return AbjpjmR_;}
  float     ARC()             {return ARC_;}
  float     SY()              {return SY_;}
  float     Sy()              {return Sy_;}
  float     RgSir()           {return RgSir_;}
  float     RgSiri()          {return RgSiri_;}
  float     RgSjr()           {return RgSjr_;}
  float     RgSjrj()          {return RgSjrj_;}
  int       usingGrid()       {return usingGrid_;}
  int       usingSy()         {return usingSy_;}
  int       gridPrepped()     {return gridPrepped_;}
  inline    string  fileroot(){return fileroot_;};
  Matrix    <float> *ACPW()   {return ACPW_;}
  Vector    <float> *P()      {return P_;}
  Matrix    <float> *colpGrid() {return colpGrid_;}
  Matrix    <float> *rowpGrid() {return rowpGrid_;}
  string    camFile()         {return camFile_;}
  int       camFileExists()   {return camFileExists_;}
  int       imWidthCalEx(const int N)    {return imWidthCalEx_  = N;}
  int       imHeightCalEx(const int M)   {return imHeightCalEx_ = M;}
  float     focalCalEx(const float f)    {return focalCalEx_    = f;}
  float     AbConstant(const float A)    {return AbConstant_    = A;}
  float     Abrqw2(const float A)        {return Abrqw2_        = A;}
  float     AbipimR(const float A)       {return AbipimR_       = A;}
  float     AbjpjmR(const float A)       {return AbjpjmR_       = A;}
  float     ARC(const float A)           {return ARC_           = A;}
  float     SY(const float A)
    {
      if ( 0.0 < A )
	{
	  SY_ = Sy_ = A;
	  usingSy_  = 1;
	  if ( usingGrid_ )
	    cerr << "WARNING(Tune): " << __FILE__ << " : " << __LINE__ 
		 << " over-riding grid inputs with SY.\n" << flush;
	  usingGrid_ = 0;
	  ARC_ = (im_*2+1) / (jm_*2+1) / SY_;
	}
      else
	{
	  Sy_ = 0.0;
	  ARC_ = 1.33333;
	}
      return Sy_;
    }
  float     RgSir(const float R)         {return RgSir_         = R;}
  float     RgSiri(const float R)        {return RgSiri_        = R;}
  float     RgSjr(const float R)         {return RgSjr_         = R;}
  float     RgSjrj(const float R)        {return RgSjrj_        = R;}
};

class TsaiData;
class MyCamData : public CameraData
{
  string          datFile_;
  string          geoFile_;
  Data           *d_;                   // Data class
  World          *w_;                   // World class
 protected:
 public:
  MyCamData();
  MyCamData(const char camFile[], const char datFile[], const char geoFile[]);
  MyCamData(const TsaiData  & C);      // To compute equivalent
  MyCamData(const MyCamData &C);
  MyCamData & operator=(const MyCamData & C);
  ~MyCamData()
    {
      if ( !derived_ )
	{
	  zap(d_);
	  zap(w_);
	}
    }

  // Functions
  void correctRx();
  void correctRg(float *x, float *y, int n, int m, int imWidthCalEx,
		 int imHeightCalEx, float RgSir, float RgSiri,
		 float RgSjr, float RgSjrj);
  void center();
  void norm();
  int  checkCosines();

  // Element access
  friend ostream & operator<< (ostream & out, MyCamData &C);
  PixelLoc3 OAP()             {return d_->OAp();}
  PixelLoc3 OBP()             {return d_->OBp();}
  PixelLoc3 OCP()             {return d_->OCp();}
  PixelLoc3 OAp(const float p0, const float p1, const float p2)
    {return d_->OApo(p0, p1, p2);}
  PixelLoc3 OBp(const float p0, const float p1, const float p2)
    {return d_->OBpo(p0, p1, p2);}
  PixelLoc3 OCp(const float p0, const float p1, const float p2)
    {return d_->OCpo(p0, p1, p2);}
  PixelLoc3 Pl()              {return w_->Pl();}
  PixelLoc3 Pr()              {return w_->Pr();}
  PixelLoc3 OA()              {return w_->OA();}
  PixelLoc3 OB()              {return w_->OB();}
  PixelLoc3 OC()              {return w_->OC();}
  string    datFile()         {return datFile_;}
  string    geoFile()         {return geoFile_;}
  int datFileExists()         {return d_->datFileExists();}
  int geoFileExists()         {return w_->geoFileExists();}
};


class TsaiData : public CameraData
{
  char           *tamFile_;
 protected:
  float           Ncx_;                 // sensor elements camera x dir, sel
  float           Nfx_;                 // pixels in frame grabber x dir, pix
  float           dx_;                  // x dim of camera sensor, mm/sel
  float           dy_;                  // y dim of camera sensor, mm/sel
  float           dpx_;                 // frame grabber eff pix size, mm/pix
  float           dpy_;                 // frame grabber eff pix size, mm/pix
  float           Cx_;                  // image center, pix
  float           Cy_;                  // image center, pix
  float           sx_;                  // scalar to compensate for errors dpx
  float           f_;                   // focal length, mm
  float           kappa1_;              // aberration radial distortion, 1/mm^2
  float           Tx_;                  // translation world to camera, mm
  float           Ty_;                  // translation world to camera, mm
  float           Tz_;                  // translation world to camera, mm
  float           Rx_;                  // rotation world to camera, rad
  float           Ry_;                  // rotation world to camera, rad
  float           Rz_;                  // rotation world to camera, rad
  float           p1_;                  // spare
  float           p2_;                  // spare
  Matrix <float>  *R_;
  Vector <float>  *T_;
 public:
  TsaiData();
  TsaiData(const char tamFile[], int n, int m);
  TsaiData(const TsaiData & T);
  TsaiData(const MyCamData & C, int n, int m);      // To compute equivalent
  TsaiData & operator=(const TsaiData & T);
  ~TsaiData()
    {
      if ( !derived_ )
	{
	  zap(tamFile_);
	}
      zap(R_);
      zap(T_);
    }

  // Element access
  friend ostream & operator<< (ostream & out, TsaiData &C);
};

///////////////////////Calibration class///////////////////////////////
class Calibration : public CameraData
{
protected:
  char           *calFile_;             // Input file name
  float           imCalEx_;             // Calibration center, raw pixels
  float           jmCalEx_;             // Calibration center, raw pixels
  int             calFileExists_;       // .cal file exists
 public:
  Calibration();
  Calibration(const char calFile[], int *n, int *m);
  Calibration(const Calibration & C);
  Calibration & operator=(const Calibration & P);
  ~Calibration()
    {
      zaparr(calFile_);
    }

  // Functions
  void correctRx(pixelLocf *c);
  void correctRg(float *x, float *y, int n, int m);

  // Element access
  float           im()            {return im_;}
  float           jm()            {return jm_;}
  float           imCalEx()       {return imCalEx_;}
  float           jmCalEx()       {return jmCalEx_;}
  int             calFileExists() {return calFileExists_;}
  friend ostream & operator<< (ostream & out, Calibration & C);
};

///////////////////////Tune class///////////////////////////////
class Tune
{
protected:
  char *tunFile_;
  float agaussScalar_;
  float bgaussScalar_;
  float minBallDensityScalar_;
  float minBallAreaScalar_;
  float maxBallAreaScalar_;
  float minBallRatioScalar_;
  float maxBallRatioScalar_;
  float minProxScalar_;
 public:
  Tune();
  Tune(const int device);
  Tune(const char tunFile[]);
  Tune(const Tune & T);
  Tune & operator=(const Tune & T);
  ~Tune()
    {
      zaparr(tunFile_);
    }

  // Element access
  friend ostream & operator<< (ostream & out, const Tune & T);
  float agaussScalar()         {return agaussScalar_;}
  float bgaussScalar()         {return bgaussScalar_;}
  float minBallDensityScalar() {return minBallDensityScalar_;}
  float minBallAreaScalar()    {return minBallAreaScalar_;}
  float maxBallAreaScalar()    {return maxBallAreaScalar_;}
  float minBallRatioScalar()   {return minBallRatioScalar_;}
  float maxBallRatioScalar()   {return maxBallRatioScalar_;}
  float minProxScalar()        {return minProxScalar_;}
  void  agaussScalar(float val)         {agaussScalar_         = val;}
  void  bgaussScalar(float val)         {bgaussScalar_         = val;}
  void  minBallDensityScalar(float val) {minBallDensityScalar_ = val;}
  void  minBallAreaScalar(float val)    {minBallAreaScalar_    = val;}
  void  maxBallAreaScalar(float val)    {maxBallAreaScalar_    = val;}
  void  minBallRatioScalar(float val)   {minBallRatioScalar_   = val;}
  void  maxBallRatioScalar(float val)   {maxBallRatioScalar_   = val;}
  void  minProxScalar(float val)        {minProxScalar_        = val;}
};


///////////////////////Zone class///////////////////////////////
// Describe and handle activity around the strike zone.
class Zone
{
protected:
  char *zonFile_;              // input file name
  float dxZone_;               // adjustment to calculated x intersection
  float dzZone_;               // adjustment to calculated z intersection
  float dXdMphZone_;           // adjustment to calculated x intersection
  float dZdMphZone_;           // adjustment to calculated z intersection
  float velThresh_;            // lower limit screening speed for ball
  float velMax_;               // upper limit screening speed for ball
  float gravity_;              // gravity constant, match units of world
                               // e.g. 386 in/sec/sec or 32.2 ft/sec/sec
  float ballDia_;              // ball diameter, match units of world
  float triangulationScalar_;  // number ball dia uncert allowed
  float left_;                 // left side zone,  match units of world
  float top_;                  // top side zone,  match units of world
  float right_;                // right side zone,  match units of world
  float bottom_;               // bottom side zone,  match units of world
  float scale_;                // strike zone scaling
  float skew_;                 // skew to even vertical lines with horiz cols
  int   maxh_;                 // maximum lines drawn height
  int   maxw_;                 // maximum characters drawn width
  float dx_;                   // width of zone
  int   colL_;                 // column left side zone
  int   colR_;                 // column right side zone
  int   lineT_;                // line top zone
  int   lineB_;                // line bottom zone
  float speedDisplayScalar_;  // Scalar on speed display to get units
  Point <float> *llc_;         // lower left corner; umpire point of view
  Point <float> *ulc_;         // upper left corner; umpire point of view
  Point <float> *urc_;         // upper right corner; umpire point of view
  Point <float> *lrc_;         // lower right corner; umpire point of view
  Line  <float> *l_;           // left side
  Line  <float> *t_;           // top side
  Line  <float> *r_;           // right side
  Line  <float> *b_;           // bottom side
 public:
  Zone();
  Zone(const char zonFile[]);
  Zone(const Zone & Z);
  Zone & operator=(const Zone & Z);
  ~Zone()
    {
      zaparr(zonFile_);
      zap(llc_);
      zap(ulc_);
      zap(urc_);
      zap(lrc_);
      zap(l_);
      zap(t_);
      zap(r_);
      zap(b_);
    }

  // Functions
  string call(const float x, const float y);

  // Element access
  friend ostream & operator<< (ostream & out, const Zone & Z);
  float ballDia()        {return ballDia_;}
  float bottom()         {return bottom_;}
  float dxZone()         {return dxZone_;}
  float dzZone()         {return dzZone_;}
  float dXdMphZone()     {return dXdMphZone_;}
  float dZdMphZone()     {return dZdMphZone_;}
  float gravity()        {return gravity_;}
  float left()           {return left_;}
  float right()          {return right_;}
  float speedDisplayScalar() {return speedDisplayScalar_;}
  float top()            {return top_;}
  float triangulationScalar() {return triangulationScalar_;}
  float velMax()         {return velMax_;}
  float velThresh()      {return velThresh_;}
  void  ballDia(float val)             {ballDia_             = val;}
  void  bottom(float val)              {bottom_              = val;}
  void  dxZone(float val)              {dxZone_              = val;}
  void  dzZone(float val)              {dzZone_              = val;}
  void  dXdMphZone(float val)          {dXdMphZone_          = val;}
  void  dZdMphZone(float val)          {dZdMphZone_          = val;}
  void  gravity(float val)             {gravity_             = val;}
  void  left(float val)                {left_                = val;}
  void  right(float val)               {right_               = val;}
  void  speedDisplayScalar(float val)  {speedDisplayScalar_  = val;}
  void  top(float val)                 {top_                 = val;}
  void  triangulationScalar(float val) {triangulationScalar_ = val;}
  void  velMax(float val)              {velMax_              = val;}
  void  velThresh(float val)           {velThresh_           = val;}
};


//////////////////////////utility functions////////////////////////////
class InFile;
int GetPar(InFile &I, const string name, int *i, const int enWarn);
int GetPar(InFile &I, const string name, float *f, const int enWarn);
int GetPar(InFile &I, const string name, PixelLoc3 *P3, const int enWarn);
int GetPar(InFile &I, const string name, PixelLoc3 *P3, const int nP,
	   const int enWarn);
int GetPar(InFile &I, const string name, Vector <float> *V, const int enWarn);
int GetPar(InFile &I, const string name, Matrix <float> *M, const int enWarn);

#endif // Tune_h

