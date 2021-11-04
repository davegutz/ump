// 	$Id: Image.cc,v 1.2 2010/01/10 11:52:46 davegutz Exp $	
// Image segmentation classes
// DA Gutz 17-Jun-01

// Includes section
using namespace std;
#include <common.h>
#include <fstream>     // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <stdio.h>     // sprintf
#include <sys/wait.h>  // waitpid
#include <sys/types.h> // open/read
#include <zap.h>       // local
extern "C"
{
 #include <Cwrappers.h> // wrappers
 #include <Cwrappthread.h>
 #include <mmio.h>      // MatrixMarket snapshots
}
#include "Image.h"
#include <checkMem.h>  // memory checking
#include <InFile.h>    // Input file class

// Globals
static const float BGAUSS       =   30;// Default pixel value weighting
static const float MINPROX      =  4.0;// Default minimum separation absorb
static const float MINBALLRATIO = 0.05;// Default minimum aspect ratio ball
static const float MAXBALLRATION=  1.5;// Default maximum aspect ratio ball
static const float MAXBALLRATIOC=  3.0;// Calibration maximum aspect ratio
static const float MINBALLDENSITYN = 0.05; // Default min density of ball
static const float MINBALLDENSITYC = 0.05; // Calibration min density of ball
static const float MINBALLAREAN =  6.0;// Default minimum pixels area of ball
static const float MINBALLAREAC =  1.2;// Calibration minimum pixels area
static const float MAXBALLAREA  = 60.0;// Default maximum pixels area of ball
extern timeval globalProcStart;        // Process start time
extern int     dump;
extern int     verbose;

// Initialization function
struct reading_t aClearReadingT()
{
  struct reading_t temp;
  temp.startV    = 0;
  temp.numV      = 0;
  temp.loadSec   = 0;
  temp.loadUsec  = 0;
  temp.ploadSec  = 0;
  temp.ploadUsec = 0;
  return temp;
}

/////////////// Image class functions///////////////////////
// Constructors
Image::Image()
  :  picture_(NULL), mask_(NULL), agauss_(0), bgauss_(0), nnodes_(0),
     nedges_(0), inodes_(NULL), W_(NULL), d_(NULL), fps_(0), m_(0), n_(0),
     bufsize_(0), time_(0), dtime_(0), num_(0), start_(aClearTimeval()),
     load_(aClearTimeval()), motion_(aClearTimeval()),
     printed_(aClearTimeval()), stop_(aClearTimeval()), imageNumber_(0),
     ballCrit_(ballTune()), numMoved_(0), deviceDigit_(0), maxEdgePix_(0),
     dev_(0), presLoad_(aClearTimeval()), segs_(NULL),
     eigVector_(Vector<float>()), distance_(NULL), device_(NULL), neutral_(0),
     warmupTime_(0), imageR_(NULL), minEdgePix_(MINPIX+NUMCORNERS),
     numToCheck_(0), scalar_(0), sub_(0), calibrate_(0), loadTest_(0),
     scalar4_(0), sub4_(0), nSub_(0), mSub_(0), bufsizeSub_(0),
     writeCompact_(0), still_(0), CAL_(0), nslips(0) {}
Image::Image(const int n, const int m, const int fps, const char device_digit,
	     const int maxedgepix, const int dev, char *device,
	     const int neutral, const float warmupTime, const float agauss,
	     const int sub, const int calibrate, const int loadTest,
	     const int writeCompact, const int still, Calibration *cal)
  :  picture_(NULL), mask_(NULL), agauss_(0), bgauss_(0), nnodes_(0),
     nedges_(0), inodes_(NULL), W_(NULL), d_(NULL), fps_(fps), m_(m), n_(n),
     bufsize_(n*m), time_(0), dtime_(0), num_(0), start_(aClearTimeval()),
     load_(aClearTimeval()), motion_(aClearTimeval()),
     printed_(aClearTimeval()), stop_(aClearTimeval()), imageNumber_(0),
     ballCrit_(ballTune()), numMoved_(0), deviceDigit_(device_digit),
     maxEdgePix_((int)((float)(maxedgepix/sub))), dev_(dev),
     presLoad_(aClearTimeval()), segs_(NULL), eigVector_(Vector<float>()),
     distance_(NULL), device_(device), neutral_(neutral),
     warmupTime_(warmupTime), imageR_(NULL), minEdgePix_(MINPIX+NUMCORNERS),
     numToCheck_(n*m), scalar_((float)n_/(float)160), sub_(sub),
     calibrate_(calibrate), loadTest_(loadTest),
     scalar4_(scalar_*scalar_*scalar_*scalar_), sub4_(sub_*sub_*sub_*sub_),
     nSub_(n_/sub_), mSub_(m_/sub_), bufsizeSub_(nSub_*mSub_),
     writeCompact_(0), still_(still), CAL_(cal), nslips(0)
{
  // pixel loading scalar; originally calibrated for 160x120.
  if ( 1 > sub_ )
    {
      cerr << "ERROR(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	   << " sub size =" << sub_ 
	   << " is less than 1." << endl << flush;
      exit(1);
    }
  if ( calibrate_ && still_ )
    {
      cerr << "WARNING(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	   << " both calibrate and still are requested.  Still is meant \n"
	   << "for non-moving regular balls." << endl << flush;
    }
  if ( writeCompact && 1 < sub_ ) writeCompact_ = 1; 
  if ( bufsizeSub_*sub_*sub_ != bufsize_ )
    {
      cerr << "ERROR(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	   << " sub size =" << sub_ 
	   << " not integrally divisible into dimensions of image =" 
	   << n_ << "x" << m_ << endl << flush;
      exit(1);
    }
  char maskFile[MAX_CANON];
  char tunFile[MAX_CANON];
  inodes_   = new int [bufsize_];
  imageR_   = new int[bufsize_];
  distance_ = new Hypotenuse(MAX(n_, m_));
  segs_     = new SegmentSet(maxEdgePix_, distance_, m_, n_, sub_, this);
    //  cerr << "new picturePtrSegmentSet=" << segs_ << " mxn=" << m_ << " x " << n_ << endl;

  // Get mask
  sprintf(maskFile,"video%c.mask", deviceDigit_);
  mask_ = new Mask(maskFile, n_, m_, maxEdgePix_);

  // Get tuning parameters
  float minballdensity = MINBALLDENSITYN;
  float minballarea    = MINBALLAREAN;
  float maxballratio   = MAXBALLRATION;
  if ( calibrate_ || still_ )
    {
      minballdensity = MINBALLDENSITYC;
      minballarea    = MINBALLAREAC;
      maxballratio   = MAXBALLRATIOC;
    }
  sprintf(tunFile,"video%c.tune", deviceDigit_);
  Tune *TUN = new Tune(tunFile);
  agauss_                   = agauss         * TUN->agaussScalar();
  bgauss_                   = BGAUSS         * TUN->bgaussScalar();
  ballCrit_.minBallDensity  = minballdensity * TUN->minBallDensityScalar();
  ballCrit_.minBallArea     = minballarea    * TUN->minBallAreaScalar();
  ballCrit_.maxBallArea     = MAXBALLAREA    * TUN->maxBallAreaScalar();
  ballCrit_.minBallRatio    = MINBALLRATIO   * TUN->minBallRatioScalar();
  ballCrit_.maxBallRatio    = maxballratio   * TUN->maxBallRatioScalar();
  ballCrit_.minProx         = MINPROX        * TUN->minProxScalar();
  zap(TUN);

  // Increase for loading test to avoid rejecting full screen
  if ( loadTest_ )
    {
#ifdef VERBOSE
      if ( 5 < verbose )
	cout << "MESSAGE(Image) :performing load test."
	     << "  Doubling maxBallArea and maxBallRatio\n" << flush;
#endif
      ballCrit_.maxBallArea  *= 2.0;
      ballCrit_.maxBallRatio *= 2.0;
    }

  // Normalize and check tuning parameters
  agauss_               *= scalar_ * MAX( scalar_/6.0 + 0.85, 1.0 );
  ballCrit_.minBallArea *= scalar_*scalar_;
  ballCrit_.maxBallArea *= scalar_*scalar_;
  ballCrit_.minProx     *= scalar_;
  if ( 0 > agauss_ || 0 > bgauss_
       || 0 > ballCrit_.minBallArea  || 0 > ballCrit_.maxBallArea
       || 0 > ballCrit_.minBallRatio || 0 > ballCrit_.maxBallRatio
       || ballCrit_.minBallArea  >= ballCrit_.maxBallArea
       || ballCrit_.minBallRatio >= ballCrit_.maxBallRatio
       || 0 > ballCrit_.minProx)
    {
      cerr << "WARNING(Image) : " << tunFile << " has unusual values.\n"
	   << flush;
    }
#ifdef VERBOSE
  if ( 3 < verbose )
    cout << "MESSAGE(Image) :tuning parameters for device " << device << ":\n"
	 << "agauss_="        << agauss_
	 << " bgauss_="       << bgauss_
	 << " minBallDensity="<< ballCrit_.minBallDensity
	 << " minBallArea="   << ballCrit_.minBallArea
	 << " maxBallArea="   << ballCrit_.maxBallArea
	 << "\nminBallRatio=" << ballCrit_.minBallRatio
	 << " maxBallRatio="  << ballCrit_.maxBallRatio
	 << " minProx="       << ballCrit_.minProx
	 << " maxEdgePix_="   << maxEdgePix_
	 << endl << flush;
#endif

  // Lens radial aberration correction applied when Line instantiated
  CAL_->initCorrectRx(n_, m_);
}
Image::Image(const Image & I)
  :  picture_(NULL), mask_(NULL), agauss_(I.agauss_), bgauss_(I.bgauss_),
     nnodes_(I.nnodes_),
     nedges_(I.nedges_), inodes_(NULL), W_(NULL), d_(NULL), fps_(I.fps_),
     m_(I.m_), n_(I.n_), bufsize_(I.bufsize_), time_(I.time_),
     dtime_(I.dtime_), num_(I.num_), start_(I.start_), load_(I.load_),
     motion_(I.motion_), printed_(I.printed_), stop_(I.stop_),
     imageNumber_(I.imageNumber_), ballCrit_(ballTune()),
     numMoved_(I.numMoved_),deviceDigit_(I.deviceDigit_),
     maxEdgePix_(I.maxEdgePix_), dev_(I.dev_), presLoad_(aClearTimeval()),
     segs_(NULL), eigVector_(Vector<float>()), distance_(NULL), device_(NULL),
     neutral_(I.neutral_), warmupTime_(I.warmupTime_), imageR_(NULL),
     minEdgePix_(I.minEdgePix_), numToCheck_(I.numToCheck_),
     scalar_(I.scalar_), sub_(I.sub_), calibrate_(I.calibrate_),
     loadTest_(I.loadTest_), scalar4_(I.scalar4_), sub4_(I.sub4_),
     nSub_(I.nSub_), mSub_(I.mSub_), bufsizeSub_(I.bufsizeSub_),
     writeCompact_(I.writeCompact_), still_(I.still_), CAL_(I.CAL_),
     nslips(0)
{
  inodes_   = new int [bufsize_];
  imageR_   = new int[bufsize_];
  distance_ = new Hypotenuse(*I.distance_);
  segs_     = new SegmentSet(*I.segs_);
  mask_     = new Mask(*I.mask_);
}
Image & Image::operator=(const Image & I)
{
  if ( this == &I ) return *this;  // Allows self-assignment.
  // TODO:  write operator= function
  return *this;                    // Allows stacked assignments.
}

Image::~Image()
{
  zap(mask_);
  zaparr(imageR_);
  zaparr(inodes_);
  if ( 0 < nedges_ ) zaparr(W_);
  if ( 0 < nnodes_ ) zaparr(d_);
  zap(distance_);
  zap(segs_);
}

// Cleanup files
void Image::removeFiles()
{
  char input_line[MAX_CANON];
  sprintf(input_line,
	  "find / -maxdepth 1 -name 'tempRC%c*.*' -exec rm '{}' ';'",
	  deviceDigit_);
  if ( 0 == Fork() )
    {
      execlp("/bin/sh", "sh", "-c", input_line, (char *) 0);
      exit (0);
    }
  wait( NULL );
  sprintf(input_line,
	  "find / -maxdepth 1 -name 'tempR%c*.*' -exec rm '{}' ';'",
	  deviceDigit_);
  if ( 0 == Fork() )
    {
      execlp("/bin/sh", "sh", "-c", input_line, (char *) 0);
      exit (0);
    }
  wait( NULL );
}

// Access the time summary information
producerTimes Image::timeSummary()
{
  producerTimes summary;
  summary.deviceDigit = deviceDigit_;
  summary.start = start_;
  summary.load = load_;
  summary.motion = motion_;
  summary.printed = printed_;
  summary.stop =   stop_;
  summary.presLoad = presLoad_;
  summary.procStart = globalProcStart;
  summary.numMoved = numMoved_;
  summary.numNodes =  nnodes_;
  summary.nslips = nslips;
  summary.fps = fps_;
  return summary;
}

// Make edges for non-zero nodes
int  Image::makeEdges()
{
  ofstream outf;
  // TUNE PIX_THRESH to -255 to use dark images
  const int PIX_THRESH = 0;
  int i, j, indexj;
  if ( 0 < nedges_ ) zaparr(W_);
  if ( 0 < nnodes_ ) zaparr(d_);
  nnodes_ = 0;
  for (j=0; j<numToCheck_; j++)
    {
      if ( PIX_THRESH < picture_[j]->velrate() )
	{
	  inodes_[nnodes_++] = j;
	}
    }
  // Return if too full for some reason.
  if ( maxEdgePix_ <= nnodes_ )
    {
      nnodes_ = 0;
#ifdef VERBOSE
      if ( 3 < verbose ) 
	cout << "MESSAGE(Image) :too many nodes in " << device_ << endl
	     << flush;
#endif
      return nnodes_;
    }
  nedges_ = nnodes_*nnodes_;
  W_      = new float[nedges_]; 
  d_      = new float[nnodes_];
  for (j=0; j<nnodes_; j++)
    {
      int indexij = inodes_[j];
      for (i=j; i<nnodes_; i++)
	{
	  int indexiijj = inodes_[i];
	  W_[j*nnodes_+i] = gaussian(*picture_[indexij], *picture_[indexiijj],
				     agauss_, bgauss_);
	}
    }
  
  // Fill in lower half
  for (j=0; j<nnodes_; j++)
    {
      indexj = j*nnodes_;
      for (i=0; i<j; i++)
	{
	  W_[indexj+i] = W_[i*nnodes_+j];
	}
    }

  // Debug
  if (dump)
    {
      outf.open("makeEdges.dmp", ios::out);
      outf << "before delete W_=" << endl << flush;
      for (j=0; j<nnodes_; j++)
	{
	  indexj = j*nnodes_;
	  for (i=0; i<nnodes_; i++)
	    {
	      outf << W_[indexj+i] << " ";
	    }
	  outf << endl << flush;
	}
    }
  

  // Delete the loners by assuming equal intensity but distance so
  // great that using min of exponential, times number of edges.
  // Apply scale factor determined by tuning.
  float EDGE_THRESH;
  //float EDGE_THRESH = nedges_ * curvex(-10000.0, expo)
  //  * 0.1 * (float) ( 160 * 160) / (float) ( n_ * n_ ); 
  if ( n_ > 160 ) 
    EDGE_THRESH = nedges_ * curvex(-10000.0, expo)
      * 0.1 * (float) ( 160 * 160) / (float) ( n_ * n_ ) / float(sub_ * sub_);
  else
    EDGE_THRESH = nedges_ * curvex(-10000.0, expo)
      * 0.1 * float(n_ * n_) / (160.0 * 160.0) / float(sub_ * sub_);
  int *loners = new int[nnodes_];
  int nloners = 0;
  if ( dump ) outf << "thresh=" << EDGE_THRESH << endl << flush;
  for (j=0; j<nnodes_; j++)
    {
      indexj = j*nnodes_;
      float sum = 0.0;
      for (i=0; i<nnodes_; i++)
	{
	  sum += W_[indexj+i];
	}
      sum -= 1;
      if ( dump ) outf << "sum(" << j << ")=" << sum << endl << flush;
      if ( EDGE_THRESH > sum && !picture_[inodes_[j]]->onCorner)
	{
	  loners[nloners++] = j;
	}
    }
  // Debug
  if ( dump )
    {
      outf << "loners=";
      for (i=0;i<nloners;i++) outf << loners[i] << " ";
      outf << endl << flush;
    }
  for (j=0; j<nloners; j++)
    {
      if (deleteNode_(loners[j]))
	for (int i=j+1; i<nloners; i++) loners[i] -= 1;
    }
  // Debug
  if ( dump )
    {
      outf << "after delete W_=" << endl << flush;
      for (j=0; j<nnodes_; j++)
	{
	  indexj = j*nnodes_;
	  for (i=0; i<nnodes_; i++)
	    {
	      outf << W_[indexj+i] << " ";
	    }
	  outf << endl << flush;
	}
      outf.close();
    }
 
  // Sum up
  for (j=0; j<nnodes_; j++)
    {
      indexj = j*nnodes_;
      d_[j] = 0.0;
      for (i=0; i<nnodes_; i++)
	{
	  d_[j] += W_[indexj+i];
	}
    }
  
  zaparr(loners);
  return (nnodes_);
}

// Compare camera geometry
int Image::cameraSameAs(Image & O)
{
  if ( !(&O) ) return 0;
  for (int i=0; i<3; i++)
    {
      for (int j=0; j<3; j++)
	{
	  if ( (*(CAL_->ACPW()))[i][j] != O.ACPW(i, j) ) return 0;
	}
      if ( P(i) != O.P(i) ) return 0;
    }
  if( CAL_->focalCalEx() != O.focalCalEx() ) return 0;
  return 1;
}

// Calculate minimum segmentation cut eigenvector - second eigenvector
int Image::spectralCalc()
{
  int returnVal = 0;
  ofstream outf;

  // octave/matlab commands:
  // fiedler_image4; printmat(Image'); fiedler(makeW(Image,4,1000))

  // Spectrally organize image.  Greater than 2 pixels for meaningful spectrum
  if ( 2 < nnodes_ )
    {
      // Calculate d & D matrices
      Matrix <float> D(nnodes_,   nnodes_, 0.0);
      Matrix <float> DMH(nnodes_, nnodes_, 0.0); 
      Vector <float> z0d(nnodes_); 
      for (int j=0; j<nnodes_; j++)
	{
	  z0d[j]    = sqrt(d_[j]);
	  D[j][j]   = d_[j];
	  DMH[j][j] = 1./sqrt(d_[j]);
	}
      norm(z0d);

      Matrix <float> W (nnodes_, nnodes_, W_);
      Matrix <float> A (nnodes_, nnodes_, 0.0);
      A = DMH * (( D - W ) * DMH );

      // Initialize the eigenvector guess.  (z0d is exact - apriori).
      Vector <float> z1(nnodes_, 1.0);
      z1[0]=0.5; 
      
      // Calculate transformed eigenvectors
      Vector <float> eigValue(2);

      if(dump)
	{
	  outf.open("cut.dmp", ios::out | ios::app);
	  outf << A << z0d << z1;
	  outf.close();
	}

      // Calculate the second eigenvector (Fiedler vector)
      eigVector_ = second(A, eigValue, z0d, z1, 0);
      if ( 0 == eigVector_.size() )
	{                   // Failed
	  returnVal = -1;
	}
      else
	{
	  // normalize 
	  for (int i=0; i<nnodes_; i++) eigVector_[i] /= z0d[i];
	  norm(eigVector_);
	}
    }
  return returnVal;
}


// Determine segments from eigenvector
int Image::segmentCut()
{
  int returnVal = 0;
  location *locs = new location[nnodes_];

  const float DELTA_FACTOR    = 0.02;  // Segmentation discretion
  // TUNE DELTA_FACTOR to half if using logic where PIX_THRESH < 0 
  // (half for twice as many pixels)
  float delta_factor = DELTA_FACTOR / scalar4_ * (float) ( sub4_ );

  const float GRAN_FACTOR     = 100.;  // Granulation factor
  float gran_factor  = GRAN_FACTOR;

  // Pair eigenvector with pixel
  for (int i=0; i<nnodes_; i++)
    {
      locs[i].y = inodes_[i];
    }

  // granulate
  for (int i=0; i<nnodes_; i++) eigVector_[i] *= gran_factor;

  // Sort
  for (int i=0; i<nnodes_; i++)
    {
      locs[i].x = eigVector_[i];
    }
  qsort(locs, nnodes_, sizeof(location), (cmp_func)comparelocs);

  // Organize cuts
  float refS = locs[0].x;
  int segNo = 0; 
  segs_->loadNode(segNo, pixelPtr(locs[0].y) );
  for (int node=1; node<nnodes_; node++)
    {
      if ( locs[node].x > refS + delta_factor ) segNo++;
      refS = locs[node].x;
      segs_->loadNode(segNo, pixelPtr(locs[node].y) );
    }
  segs_->calcProperties();
  // Consolidation probably not needed since added addCorners().
  segs_->consolidate(ballCrit_);
  
  // Sort
  segs_->sort();

  zaparr(locs);

  return returnVal;
}

// Delete node
int Image::deleteNode_(const int duck)
{

  // Delete element from inodes_
  for (int j=duck; j < nnodes_-1; j++)
    {
      inodes_[j] = inodes_[j+1];
    }

  // Delete row from W_
  for (int j=duck; j < nnodes_-1; j++)
    for (int i=0; i < nnodes_; i++)
      W_[j*nnodes_+i] = W_[(j+1)*nnodes_+i];


  // Delete column from W_
  int count, i, j;
  for (i=duck, j=duck+1, count=0; j< nedges_; i++, j++, count++)
    {
    if (count == nnodes_-1)
      {
	j++;
	count = 0;
      }
    W_[i] = W_[j];
    }

  nnodes_--;
  nedges_ = nnodes_*nnodes_;

  return(1);
}


// Label file
void Image::writeRlbl(char *filename)
{
  FILE *fp;
  if ( (fp = fopen(filename, "w")) == NULL)
    cerr << "WARNING(Image) : Cannot open " << filename << " for writing\n"
	 << flush;
  if ( 0 == imageNumber_ )
    fprintf(fp, "%.3d %8.4f %4.4f\n", imageNumber_, -999., dtime_);
  else
    fprintf(fp, "%.3d %8.4f %4.4f\n", imageNumber_, time_, dtime_);

  fclose(fp);
}

///////////////Class RealImage//////////////////////////////////////////
// Constructors
RealImage::RealImage()
  :  Image(), startV_(0), numV_(0), smptr_(NULL), writeSnap_(0),
     talk_(NULL), reading_(aClearReadingT()), imageV_(NULL),
     prevLoad_(aClearTimeval()){}
RealImage::RealImage(const int dev, const int n, const int m, const int fps,
		     int *smptr, const char device_digit, const int writeSnap,
		     const int maxedgepix, int *talk,
		     char *device, const float warmupTime, const float agauss,
		     const int sub, const int calibrate, const int loadTest,
		     const int writeCompact, Calibration *cal)
  :  Image(n, m, fps, device_digit, maxedgepix, dev, device, 0, warmupTime,
	   agauss, sub, calibrate, loadTest, writeCompact, 0, cal),
     startV_(0), numV_(0), smptr_(smptr), writeSnap_(writeSnap), talk_(talk),
     reading_(aClearReadingT()), imageV_(NULL), prevLoad_(aClearTimeval())
{
  imageV_    = new unsigned char[bufsize_];
  picture_   = new Pixel*[bufsize_];
  for (int j=0; j<bufsize_; j++)
    {
      picture_[j]     = new Pixel(&(imageR_[j]), &(imageV_[j]));
    }
  char maskFile[MAX_CANON];
  sprintf(maskFile, "video%c.mask", deviceDigit_);
  mask_->loadMasksFrom(maskFile);
  mask_->applyMasksTo(picture_, n_, m_, sub_);

  // Cleanup output files
  if ( writeSnap_ )
    {
#ifdef VERBOSE
      if ( verbose )
	cout << "MESSAGE(Image) : Cleaning up image files .mm, .lbl, and .ppm";
#endif
      removeFiles();
#ifdef VERBOSE
      if ( verbose ) cout << ".\n" << flush;
#endif
    }
}
RealImage::RealImage(const RealImage & R)
  : startV_(R.startV_), numV_(R.numV_), smptr_(R.smptr_),
    writeSnap_(R.writeSnap_), talk_(R.talk_), reading_(aClearReadingT()), 
    imageV_(NULL), prevLoad_(R.prevLoad_)
{
  imageV_    = new unsigned char[bufsize_];
  picture_   = new Pixel*[bufsize_];
  for (int j=0; j<bufsize_; j++)
    {
      picture_[j]     = new Pixel(&(imageR_[j]), &(imageV_[j]));
    }
  mask_->applyMasksTo(picture_, n_, m_, sub_);
}
RealImage & RealImage::operator=(const RealImage & R)
{
  if ( this == &R ) return *this;  // Allows self-assignment.
  startV_    = R.startV_;
  numV_      = R.numV_;
  smptr_     = R.smptr_;
  writeSnap_ = R.writeSnap_;
  talk_      = R.talk_;
  if ( imageV_  ) zaparr(imageV_);
  for (int j=0; j<bufsize_; j++)
    {
      if ( picture_[j] ) zap(picture_[j]);
    }
  if ( picture_ ) zaparr(picture_); 
  imageV_    = new unsigned char[bufsize_];
  picture_   = new Pixel*[bufsize_];
  for (int j=0; j<bufsize_; j++)
    {
      picture_[j]     = new Pixel(&(imageR_[j]), &(imageV_[j]));
    }
  mask_->applyMasksTo(picture_, n_, m_, sub_);
  return *this;                    // Allows stacked assignments.
}
RealImage::~RealImage()
{
  for (int j=0; j<bufsize_; j++) if (picture_[j]) zap(picture_[j]);
  if ( picture_ ) zaparr(picture_);
  if ( imageV_  ) zaparr(imageV_);
  smptr_ = NULL;
}

// Send mask to child
void RealImage::sendMask()
{

  // Store the values
  if ( 0 < mask_->numInputLines() )
    {
      for (int i=0; i<mask_->numInputLines(); i++)
	{
	  int index = i*7;
	  smptr_[index    ] = mask_->Sx(i);
	  smptr_[index + 1] = mask_->Sy(i);
	  smptr_[index + 2] = mask_->Dx(i);
	  smptr_[index + 3] = mask_->DDx(i);
	  smptr_[index + 4] = mask_->Dy(i);
	  smptr_[index + 5] = mask_->DDy(i);
	  smptr_[index + 6] = mask_->add(i);
	}
    }
  
  // Send number of expected values, writer will block even for 0
  int *numInputLines = new int(mask_->numInputLines());
  if ( -1 == write(*talk_, numInputLines, sizeof(int)) ) exit(1);
}

// Load a snapshot image
int RealImage::loadOne()
{
  int  returnVal = 0;
  int  cmd = 1;
  int  iread;

  // Note start time
  gettimeofday(&start_, NULL);

  // Request new image from streamery, get back time.
  if( -1 == write(*talk_, &cmd, sizeof(int)) ) exit(1);
  iread = read (*talk_, &reading_, sizeof(struct reading_t));
  if( 0 >= iread )
    {
      returnVal = 1;              // EOF
      gettimeofday(&load_, NULL);
      return returnVal;
    }
  presLoad_.tv_sec   = reading_.loadSec;
  presLoad_.tv_usec  = reading_.loadUsec;
  prevLoad_.tv_sec   = reading_.ploadSec;
  prevLoad_.tv_usec  = reading_.ploadUsec;
  if ( !timerisset(&globalProcStart) ) globalProcStart = presLoad_;
  startV_           = (int)reading_.startV;
  numV_             = MIN((int)reading_.numV, maxEdgePix_);
  
  // Update the counters.
  dtime_ = presLoad_ - prevLoad_;
  time_  = presLoad_ - globalProcStart;
  num_++;
  gettimeofday(&load_, NULL);
  return returnVal;
}

// Reset timers in preparation for a restart
void RealImage::resetTimers()
{
  timerclear(&globalProcStart);
  num_ = 0;
}

// Add corners to force clean images to segment properly
void RealImage::addCorners()
{
  int scalesub = MAX( (int) roundf ( scalar_ / (float) sub_ ), 1 );
  int rowNum   = 0;
  int i        = 0;
  int numInRow = 0;
  int index    = 0;
  while ( i < scalesub )
    {
      rowNum++;
      numInRow = rowNum + 1;
      for ( int j=0; j<numInRow && i<scalesub; j++ )
	{
	  picture_[numV_]->y        = m_-(rowNum + 1 - j);
	  picture_[numV_]->x        = n_-j;
	  picture_[numV_]->onCorner = 1;
	  picture_[numV_]->onEdge   = 1;
	  *picture_[numV_]->valp    = 0;
	  *picture_[numV_++]->velp  = 255;
	  index = bufsize_ - 1 - ( (rowNum - j) * n_ + j );
	  i++;
	}
    }
}

// Assign edges if not calibrating.  For calibration, ignore so
// that can evaluate all the objects
void RealImage::assignEdges()
{
  if ( !calibrate_ ) for (int i=0; i<numV_; i++)
    {
      int xm = picture_[i]->x - sub_;
      int xp = picture_[i]->x + sub_;
      int ym = picture_[i]->y - sub_;
      int yp = picture_[i]->y + sub_;
      if ( 0>xm || n_-1<xp || 0>ym || m_-1<yp )
	picture_[i]->onEdge = 1;
      else if (  !picture_[xm + ym*n_]->inMask
	      || !picture_[xm + yp*n_]->inMask 
              || !picture_[xp + ym*n_]->inMask
              || !picture_[xp + yp*n_]->inMask )
	picture_[i]->onEdge = 1;
      else
	picture_[i]->onEdge = 0;
    }
}

// Convert image to motion
int RealImage::move()
{
  int indexV;
  if(time_ <= warmupTime_) numV_  = 0;
  else for (int i=0; i<numV_; i++)
    {
      indexV                = startV_ + i*4;
      picture_[i]->y        = smptr_[indexV  ];
      picture_[i]->x        = smptr_[indexV+1];
      *picture_[i]->velp    = smptr_[indexV+2];
      *picture_[i]->valp    = (unsigned char) smptr_[indexV+3];
      picture_[i]->onCorner = 0;
    }
  assignEdges();
  addCorners();
  gettimeofday(&motion_, NULL);
  numToCheck_ = numMoved_ = numV_;
  return numMoved_;
}

// Restart snapshots
void  RealImage::startSnap()
{
  cerr << "\a" << flush;
  sleep(1);
  cerr << "\a" << flush;
  writeSnap_   = 1;
  imageNumber_ = -1;
}

// Segmentation executive
int RealImage::segment()
{
  int returnVal = 0;
  int numnodes;

  // Clear memory from previous pass
  segs_->resetNodes();
  if ( time_ > warmupTime_ )
    {
      if ( minEdgePix_ < numMoved_ && numMoved_ < maxEdgePix_ )
	{
	  numnodes = makeEdges();
	  if( minEdgePix_ < numnodes && numnodes < maxEdgePix_ )
	    {
	      if      ( -1 == spectralCalc() ) returnVal = -1;
	      else if ( -1 == segmentCut()   ) returnVal = -1;
	      else
		{
		  gettimeofday(&stop_, NULL);
		  imageNumber_++;
		  returnVal = 1;
		}
	      writeToFiles(*this);
	    }
	}
      else if ( numMoved_ >= maxEdgePix_ )
	{
#ifdef VERBOSE
	  if ( 2 < verbose )
	    cout << "MESSAGE(Image) :too many pixels changed :"
		 << numMoved_ << endl << flush;
#endif
	}
    }

  // Always write to files when calibrating
  if ( !returnVal && calibrate_ )
    {
      imageNumber_++;
      writeToFiles(*this);
      gettimeofday(&stop_, NULL);
    }
  return returnVal;
}

// MatrixMarket io file write
void RealImage::writeRmm(char *filename)
{
  FILE *fp;

  // Open file
  if ( (fp = fopen(filename, "w")) == NULL)
    cerr << "WARNING(Image) : Cannot open " << filename << " for writing\n"
	 << flush;

  // Write to file
  // print banner followed by typecode
  MM_typecode matcode;                        
  mm_initialize_typecode(&matcode);
  mm_set_matrix(&matcode);
  mm_set_coordinate(&matcode);
  mm_set_integer(&matcode);
  mm_write_banner(fp, matcode);
  mm_write_mtx_crd_size(fp, n_, m_, nnodes_);
  for (int j=0; j<segs_->nSeg(); j++)
    {
      for (int i=0; i<segs_->nSegLoc(j); i++)
	{
	  fprintf(fp, "%d %d %d\n",
		  segs_->segLocX(j, i)+1, segs_->segLocY(j, i)+1, j); 
	}
    }
  // Close file
  fclose(fp);
}

// Save to file
void writeToFiles(RealImage & image)
{
  char      filename[MAX_CANON];
  const int NUMPIPEIMAGES = 100;

  // write files
  if( image.writeSnap_ && NUMPIPEIMAGES > image.imageNumber_ )
    {
      sprintf(filename,"tempR%c%.3i.mm", image.deviceDigit_,
	      image.imageNumber_);
      image.writeRmm(filename);
      sprintf(filename,"tempR%c%.3i.lbl", image.deviceDigit_,
	      image.imageNumber_);
      image.writeRlbl(filename);
    }
  if( image.writeSnap_ && NUMPIPEIMAGES == (image.imageNumber_+1)  )
    {
      cerr << "MESSAGE(Image) : done snapping\n" << "\a" << flush;
    }

  // mark time
  gettimeofday(&image.printed_, NULL);
}


/////////////// FileImage //////////////////////////////////////////
// Constructors
FileImage::FileImage()
 : Image(), imageZ_(NULL), ifp_(0), tfp_(0), pastLoad_(aClearTimeval()),
   deadband_(0), showUndistort_(0),
   endTime_(0), absframes_(0), image_(NULL)
{}
FileImage::FileImage(const int dev, int ifp[2], const char device_digit,
		     const int n, const int m, const int fps,
		     const int maxedgepix, char *device, 
		     const float warmupTime, const float agauss, const int sub,
		     const int deadband, const int calibrate,
		     const int loadTest, const int showUndistort,
		     const int writeCompact, const int still, Calibration *cal,
		     const float endTime, const int absframes)
  : Image(n, m, fps, device_digit, maxedgepix, dev, device, 0, warmupTime,
	  agauss, sub, calibrate, loadTest, writeCompact, still, cal),
    imageZ_(NULL), ifp_(ifp[1]), tfp_(ifp[0]), pastLoad_(aClearTimeval()),
    deadband_(deadband), showUndistort_(showUndistort), endTime_(endTime),
    absframes_(absframes), image_(NULL)
{
  int i, j, index, indexj;

  timerclear(&pastLoad_);
  picture_  = new Pixel*[bufsize_];
  image_    = new unsigned char [bufsize_];
  imageZ_   = new unsigned char [bufsize_];
  for (j=0; j<m_; j++)
    {
      indexj = j*n_;
      for (i=0; i<n_; i++)
	{
	  index = indexj+i;
	  picture_[index] = new Pixel(&(image_[index]), &(imageR_[index]),
				      &(imageZ_[index]), i, j);
	}
    }
  char maskFile[MAX_CANON];
  sprintf(maskFile, "video%c.mask", deviceDigit_);
  mask_->loadMasksFrom(maskFile);
  mask_->applyMasksTo(picture_, n_, m_, sub_);
  assignEdges();

  // Cleanup output files
#ifdef VERBOSE
  if ( verbose )
    cout << "MESSAGE(Image) : Cleaning up image files .mm, .lbl, and .ppm";
#endif
  removeFiles();
#ifdef VERBOSE
  if ( verbose ) cout << ".\n" << flush;
#endif
}
FileImage::FileImage(const FileImage & F)
  :  imageZ_(NULL), ifp_(F.ifp_), tfp_(F.tfp_),
     pastLoad_(aClearTimeval()), deadband_(F.deadband_),
     showUndistort_(F.showUndistort_), endTime_(F.endTime_),
     absframes_(F.absframes_), image_(NULL)
{
  picture_  = new Pixel*[bufsize_];
  image_    = new unsigned char [bufsize_];
  imageZ_   = new unsigned char [bufsize_];
  int i=0, j=0, indexj=0, index=0;
  for (j=0; j<m_; j++)
    {
      indexj = j*n_;
      for (i=0; i<n_; i++)
	{
	  index = indexj+i;
	  picture_[index] = new Pixel(&(image_[index]), &(imageR_[index]),
				      &(imageZ_[index]), i, j);
	}
    }
}
FileImage & FileImage::operator=(const FileImage & F)
{
  if ( this == &F ) return *this;  // Allows self-assignment.
  ifp_          = F.ifp_;
  tfp_          = F.tfp_;
  deadband_     = F.deadband_;
  showUndistort_= F.showUndistort_;
  endTime_      = F.endTime_;
  absframes_    = F.absframes_;
  picture_  = new Pixel*[bufsize_];
  image_    = new unsigned char [bufsize_];
  imageZ_   = new unsigned char [bufsize_];
  int i=0, j=0, indexj=0, index=0;
  for (j=0; j<m_; j++)
    {
      indexj = j*n_;
      for (i=0; i<n_; i++)
	{
	  index = indexj+i;
	  picture_[index] = new Pixel(&(image_[index]), &(imageR_[index]),
				      &(imageZ_[index]), i, j);
	}
    }
  return *this;                    // Allows stacked assignments.
}
FileImage::~FileImage()
{
  for (int j=0, n=m_*n_; j<n; j++) if ( picture_[j] ) zap(picture_[j]);
  if ( picture_ ) zaparr(picture_);
  if ( image_   ) zaparr(image_);
  if ( imageZ_  ) zaparr(imageZ_);
}

// Load a snapshot image
int FileImage::loadOne()
{

  // Note start time and save past image
  gettimeofday(&start_, NULL);
  memcpy(imageZ_, image_, bufsize_);

  // Load image into buffer
  int            bytes_read;
  unsigned char *bp           = &image_[0];
  unsigned       bytes_needed = bufsize_;
  long           imageCount   = 0;
  while ( bytes_needed )
    {
      bytes_read = read(ifp_, bp, bytes_needed);
      if ( -1 == bytes_read && EAGAIN != errno )
	{
	  perror("LoadOne");
	  cerr << "ERROR(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	       << "cannot read .raw file\n" << flush;
	  return(1);
	}
      if ( 20000000 < imageCount )
	{
	  cerr << "ERROR(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	       << "device " << deviceDigit_
	       << " timed out reading .raw file\n" << flush;
	  return(1);
	}
      if ( !bytes_read  && num_ == absframes_ )
	{
#ifdef VERBOSE
	  if ( verbose )
	    {
	      if ( 1 == dev_ ) cout << "              " << flush;
	      cout << "EOF" << endl << flush;
	    }
#endif
	  gettimeofday(&load_, NULL);
	  return(1);
	}
      if ( 0 < bytes_read )
	{
	  bp           += bytes_read;
	  bytes_needed -= bytes_read;
	}
      imageCount++;
    }
  
  // Read time stamp
  long  readTime[2];
  long *rtP       = &readTime[0];
  long  timeCount = 0;
  bytes_needed    = 2*sizeof(long);
  while ( bytes_needed )
    {
      bytes_read = read(tfp_, rtP, bytes_needed);
      if ( -1 == bytes_read && EAGAIN != errno )
	{
	  perror("LoadOne");
	  cerr << "ERROR(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	       << "cannot read .tim file\n" << flush;
	  return(1);
	}
      if ( 2000000 < timeCount )
	{
	  cerr << "ERROR(Image) :" << __FILE__ << " : " << __LINE__ << " : "  
	       << "timed out reading .tim file\n" << flush;
	  cerr << deviceDigit_ << ":  time=" << time_ << ", dtime_=" << dtime_
	       << ", endTime_=" << endTime_ << ", num_=" << num_ 
	       << ", absframes_=" << absframes_ << endl << flush;
	  return(1);
	}
      if ( !bytes_read  && num_ == absframes_ )
	{
#ifdef VERBOSE
	  if ( verbose )
	    { 
	      if ( 1 == dev_ ) cout << "              " << flush;
	      cout << "EOF" << endl << flush;
	    }
#endif
	  return(1);
	}
      if ( 0 < bytes_read )
	{
	  rtP          += bytes_read;
	  bytes_needed -= bytes_read;
	}
      timeCount++;
    }
  if ( !timerisset(&pastLoad_) )
    {
      presLoad_.tv_sec =  readTime[0];
      presLoad_.tv_usec=  readTime[1];
      pastLoad_ = presLoad_;
    }
  else
    {
      pastLoad_.tv_sec  = presLoad_.tv_sec;
      pastLoad_.tv_usec = presLoad_.tv_usec;
      presLoad_.tv_sec =  readTime[0];
      presLoad_.tv_usec=  readTime[1];
    }
  if ( !timerisset(&globalProcStart) ) globalProcStart = presLoad_;

  // Update the counters.
  float dtimeN = presLoad_ - pastLoad_;
  if      ( dtimeN  ) dtime_ = dtimeN;
  else if ( !dtime_ ) dtime_ = 1.0/fps_;
  float timeP = time_;                                 // Check for 0 dtime
  time_  = presLoad_ - globalProcStart;
  // Shuffled time possible
  if ( num_ && time_ - timeP < 1e-5 ) time_ = timeP + dtime_;
  num_++;

  // Quit if run over the -t | --absframes time
  if ( absframes_ < num_ ) return(1);

  gettimeofday(&load_, NULL);
  return 0;
}

// Add corners to force clean images to segment properly
void FileImage::addCorners()
{
  int scalesub = MAX( (int) roundf ( scalar_ / (float) sub_ ), 1 );
  int rowNum   = 0;
  int i        = 0;
  int numInRow = 0;
  int index    = 0;
  while ( i < scalesub )
    {
      rowNum++;
      numInRow = rowNum + 1;
      for ( int j=0; j<numInRow && i<scalesub; j++ )
	{
	  index = bufsize_ - 1 - ( (rowNum - j) * n_ + j );
	  *picture_[index]->valp    =   0;
	  *picture_[index]->ratep   = 255;
	  picture_[index]->onCorner =   1;
	  numMoved_++;
	  i++;
	}
    }
}

// Assign edges.  If calibrating, do not, because it is difficult
// to see if the whole ball is inside the mask.
void FileImage::assignEdges()
{
  if ( !calibrate_ ) for (int i=0; i<mask_->numMaskVal(); i++)
    {
      int index = mask_->maskVal(i);
      int x =   index % n_;
      int y = ( index - x ) / n_;
      int xm =  x - sub_;
      int xp =  x + sub_;
      int ym =  y - sub_;
      int yp =  y + sub_;
      if ( 0>xm || n_-1<xp || 0>ym || m_-1<yp ) picture_[index]->onEdge = 1;
      else
	{
	  if (    !picture_[xm + ym*n_]->inMask
	       || !picture_[xm + yp*n_]->inMask 
	       || !picture_[xp + ym*n_]->inMask
	       || !picture_[xp + yp*n_]->inMask )
	    {
	      picture_[index]->onEdge = 1;
	    }
	}
    }
}

// Convert image to motion
int FileImage::move()
{
  numMoved_ = 0;
  if ( warmupTime_ >= time_ )
    {
      gettimeofday(&motion_, NULL);
      return numMoved_;
    }
  
  // Initialize new rate to large negative character
  memset(imageR_, -100, bufsize_*sizeof(imageR_[0]));
  int delta, sign_delta;
  for (int j=0, *i=mask_->maskValptr(); j<mask_->numMaskVal(); j++, i++)
    {
      delta      = ((int)*(picture_[*i]->valp)
		    - (int)*(picture_[*i]->valpastp));
      sign_delta = SIGN(delta);
      delta      = abs(delta) - deadband_;
      if ( 0 < delta )
	{
	  numMoved_++;
	  *picture_[*i]->ratep = (delta + deadband_) * sign_delta;
	}
    }
  addCorners();
  gettimeofday(&motion_, NULL);
  return numMoved_;
}

// Segmentation executive
int FileImage::segment()
{
  int returnVal = 0;
  int numnodes;

  // Clear memory from previous pass
  segs_->resetNodes();

  if ( warmupTime_ < time_ )
    {
      if ( minEdgePix_ < numMoved_ && numMoved_ < maxEdgePix_ )
	{
	  numnodes = makeEdges();
	  if ( minEdgePix_ < numnodes && numnodes < maxEdgePix_ )
	    {
	      if      ( -1 == spectralCalc() )
		{
		  returnVal = -1;
		}
	      else if ( -1 == segmentCut()   )
		{
		  returnVal = -1;
		}
	      else
		{
		  gettimeofday(&stop_, NULL);
		  imageNumber_++;
		  returnVal = 1;
		}
	      writeToFiles(*this);
	    }
	}
      else if ( numMoved_ >= maxEdgePix_ )
	{
#ifdef VERBOSE
	  if ( 2 < verbose ) 
	    {
	      if ( 0 < dev_ ) cerr << "              " << flush;
	      cout << "MESSAGE(Image) :too many pixels changed :"
		   << numMoved_ << endl << flush;
	    }
#endif
	}
    }

  // Always write to files when calibrating
  if ( !returnVal && calibrate_ )
    {
      imageNumber_++;
      writeToFiles(*this);
      gettimeofday(&stop_, NULL);
    }

  return returnVal;
}

// MatrixMarket io file write
void FileImage::writeRmm(char *filename)
{
  FILE *fp;

  // Open file
  if ( (fp = fopen(filename, "w")) == NULL)
    cerr << "WARNING(Image) : Cannot open " << filename << " for writing\n"
	 << flush;

  // Write to file
  // print banner followed by typecode
  MM_typecode matcode;                        
  mm_initialize_typecode(&matcode);
  mm_set_matrix(&matcode);
  mm_set_coordinate(&matcode);
  mm_set_integer(&matcode);
  mm_write_banner(fp, matcode); 

  // Write binary data (MatrixMarket in Fortran format)
  if ( writeCompact_ )
    {
      mm_write_mtx_crd_size(fp, nSub_, mSub_, nnodes_);
      for (int j=0; j<segs_->nSeg(); j++)
	{
	  for (int i=0; i<segs_->nSegLoc(j); i++)
	    {
	      fprintf(fp, "%d %d %d\n", picture_[segs_->segLocSub(j, i)]->x+1, 
		      picture_[segs_->segLocSub(j, i)]->y+1, j); 
	    }
	}
    }
  else
    {
      mm_write_mtx_crd_size(fp, n_, m_, nnodes_);
      for (int j=0; j<segs_->nSeg(); j++)
	{
	  for (int i=0; i<segs_->nSegLoc(j); i++)
	    {
	      fprintf(fp, "%d %d %d\n", picture_[segs_->segLoc(j, i)]->x+1, 
		      picture_[segs_->segLoc(j, i)]->y+1, j); 
	    }
	}
    }

  // Close file
  fclose(fp);
}

// Color rate pnm file (image with motion colored)
void FileImage::writeRppm(char *filename)
{
  void assignRGB(const int request, int *RED, int *GREEN, int *BLUE);

  FILE *fp;
  // Open file
  if ( NULL == (fp = fopen(filename, "wb")) )
    cerr << "WARNING(Image) : Cannot open " << filename << " for writing\n"
	 << flush;

  // Write header
  fprintf(fp, "P6\n");
  if ( writeCompact_ ) fprintf(fp, "%d %d\n", nSub_, mSub_);
  else                 fprintf(fp, "%d %d\n", n_,    m_   );
  fprintf(fp, "255\n");
  
  // Assign data
  unsigned char *img; 
  int bufsize = bufsize_;
  if ( writeCompact_ ) bufsize = bufsizeSub_;
  img = new unsigned char[bufsize*3];
  memset(img, 0, bufsize*3);
  for (int i = 0; i < mask_->numMaskVal() ; i++)
    {
      int index    = mask_->maskVal(i);
      int indexSub = mask_->maskSubVal(i);
      int indexRGB = 3*index;
      if ( writeCompact_ ) indexRGB = 3*indexSub;
      if ( indexRGB+2 >= bufsize*3 )
	{
	  cerr << "ERROR(Image) : " << __FILE__ << " : " << __LINE__ << " : "
	       << "Value of mask[" << i << "]=" << index << " or\n"
	       << "maskSubVal[" << i << "]=" << indexSub
	       << " is too long compared to length of image ="
	       << bufsize_ << endl << flush;
	  exit(1);
	}
      img[      indexRGB] = *picture_[index]->valp;
      img[  1 + indexRGB] = img[indexRGB];
      img[  2 + indexRGB] = img[indexRGB];
    }

  int RED, GREEN, BLUE;
  // Assign edges
  assignRGB(4, &RED, &GREEN, &BLUE);
  for (int i = 0; i < mask_->numMaskVal() ; i++)
    {
      int index    = mask_->maskVal(i);
      int indexSub = mask_->maskSubVal(i);
      int indexRGB = 3*index;
      if ( writeCompact_ ) indexRGB = 3*indexSub;
      if ( picture_[index]->onEdge )
	{
	  img[      indexRGB] = RED;
	  img[  1 + indexRGB] = GREEN;
	  img[  2 + indexRGB] = BLUE;
	}
    }

  // Assign segment values, cover edges with detected motion
  int indexRGB = 0;
  if (nnodes_ > 1)
    {
      for (int j = 0; j < segs_->nSeg(); j++)
	{
	  assignRGB(j, &RED, &GREEN, &BLUE);
	  for (int i = 0; i < segs_->nSegLoc(j); i++)
	    {
	      indexRGB = 3 * segs_->segLoc(j, i);
	      if ( writeCompact_ ) indexRGB = 3 * segs_->segLocSub(j, i);
	      img[      indexRGB ] = RED;
	      img[  1 + indexRGB ] = GREEN;
	      img[  2 + indexRGB ] = BLUE;
	    }
	}
    }

  unsigned char *imgp;
  // Correct
  // Assign data
  if ( showUndistort_ )
    {
      unsigned char *imgU; 
      imgU = new unsigned char[bufsize_*3];
      memset(imgU, 0, bufsize_*3);
      int indexU, indexD;
      float x, y;
      for (int i = 0; i < n_; i++)
	{
	  for (int j = 0; j < m_; j++)
	    {
	      x = float(i);
	      y = float(j);
	      indexU = 3 * ( j*n_ + i );
	      CAL_->CameraData::distort(&x, &y);
	      //	      distort(&x, &y);
	      indexD = 3 * ( int(y)*n_ + int(x) );
	      imgU[     indexU ] = img[     indexD ];
	      imgU[ 1 + indexU ] = img[ 1 + indexD ];
	      imgU[ 2 + indexU ] = img[ 2 + indexD ];
	    }
	}
      imgp = imgU;
    }
  else
    {
      imgp = img;
    }

  // Write file
  int n = n_, m = m_;
  if ( writeCompact_ ){ m = mSub_; n = nSub_; }
  for (int j = 0 ; j < m ; j++)
    {
      fwrite(imgp, n, 3, fp);
      imgp += 3*n;
    }

  // Cleanup
  zaparr(img);
  fclose(fp);
}

///////////////Class MemImage//////////////////////////////////////////
// Constructors
MemImage::MemImage()
  :  RealImage(), ifd_(0), ofd_(0)
{
}

MemImage::MemImage(const int dev, const int n, const int m, const int fps,
		   int *smptr, const char device_digit, const int writeSnap,
		   const int maxedgepix, const int ifd, const int ofd,
		   char *device, const float warmupTime, const float agauss,
		   const int sub, const int calibrate, const int loadTest,
		   const int writeCompact, Calibration *cal)
  :  RealImage(dev, n, m, fps, smptr, device_digit, writeSnap, maxedgepix,
	       NULL, device, warmupTime, agauss, sub, calibrate, loadTest,
	       writeCompact, cal),
     ifd_(ifd), ofd_(ofd){}

MemImage::~MemImage(){}

// Load a snapshot image
int MemImage::loadOne()
{
  int  returnVal = 0;
  int  cmd = 1;
  int  iread;

  // Note start time
  gettimeofday(&start_, NULL);

  // Request new image from streamery, get back time.
  if( -1 == write(ofd_, &cmd, sizeof(int)) ) exit(1);
  iread = read (ifd_, &reading_, sizeof(struct reading_t));
  if( 0 >= iread )
    {
      returnVal = 1;              // EOF
      gettimeofday(&load_, NULL);
      return returnVal;
    }
  presLoad_.tv_sec   = reading_.loadSec;
  presLoad_.tv_usec  = reading_.loadUsec;
  prevLoad_.tv_sec   = reading_.ploadSec;
  prevLoad_.tv_usec  = reading_.ploadUsec;
  if ( !timerisset(&globalProcStart) ) globalProcStart = presLoad_;
  startV_           = (int)reading_.startV;
  numV_             = MIN((int)reading_.numV, maxEdgePix_);
  
  // Update the counters.
  dtime_ = presLoad_ - prevLoad_;
  time_  = presLoad_ - globalProcStart;
  num_++;
  /*cerr << device_ << " load times="
       << time_ << " " <<dtime_ << " " << num_
       << " " << numV_ << ".\n" << flush;
  */
  gettimeofday(&load_, NULL);
  return returnVal;
}

// Load a snapshot image
void MemImage::sendMask()
{

  // Store the values
  if ( 0 < mask_->numInputLines() )
    {
      for (int i=0; i<mask_->numInputLines(); i++)
	{
	  int index = i*7;
	  smptr_[index    ] = mask_->Sx(i);
	  smptr_[index + 1] = mask_->Sy(i);
	  smptr_[index + 2] = mask_->Dx(i);
	  smptr_[index + 3] = mask_->DDx(i);
	  smptr_[index + 4] = mask_->Dy(i);
	  smptr_[index + 5] = mask_->DDy(i);
	  smptr_[index + 6] = mask_->add(i);
	}
    }
  
  // Send number of expected values, writer will block even for 0
  int *numInputLines = new int(mask_->numInputLines());
  if ( -1 == write(ofd_, numInputLines, sizeof(int)) ) exit(1);
}

// cout
ostream & operator<< (ostream & out, const Image & I)
{
  out << "Image for device " << I.device_ << ":"
      << "\nagauss_=     " << I.agauss_      << ", nnodes_=    "<< I.nnodes_
      << "\nnedges_=     " << I.nedges_      << ", fps_=       "<< I.fps_
      << "\nm_=          " << I.m_           << ", n_=         "<< I.n_
      << "\nbufsize_=    " << I.bufsize_     << ", time_=      "<< I.time_
      << "\ndtime_=      " << I.dtime_       << ", num_=       "<< I.num_
      << "\nimageNumber_=" << I.imageNumber_ << ", numMoved_=  "<< I.numMoved_
      << "\ndeviceDigit_=" << I.deviceDigit_ << ", maxEdgePix_="<< I.maxEdgePix_
      << "\ndev_=        " << I.dev_         << ", device_    ="<< I.device_
      << "\nneutral_=    " << I.neutral_     << ", warmupTime_="<< I.warmupTime_
      << "\nminEdgePix_= " << I.minEdgePix_  << ", numToCheck_="<< I.numToCheck_
      << "\nsub_=        " << I.sub_         << ", calibrate_= "<< I.calibrate_
      << "\nloadTest_=   " << I.loadTest_    << ", still_=     "<< I.still_
      << "\nwriteCompact_=" << I.writeCompact_
      << "\nmask_=       " << I.mask_        << endl;
  return out;
}

/////////////////////// Miscellaneous functions//////////////////

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

void writeToFiles(FileImage & image)
{
  char    filename[MAX_CANON];

  // write files
  sprintf(filename,"tempRC%c%.3i.ppm", image.deviceDigit_, image.imageNumber_);
  image.writeRppm(filename);
  sprintf(filename,"tempRC%c%.3i.lbl", image.deviceDigit_, image.imageNumber_);
  image.writeRlbl(filename);

  sprintf(filename,"tempR%c%.3i.mm", image.deviceDigit_, image.imageNumber_);
  image.writeRmm(filename);
  sprintf(filename,"tempR%c%.3i.lbl", image.deviceDigit_,image.imageNumber_);
  image.writeRlbl(filename);

  // mark time
  gettimeofday(&image.printed_, NULL);
}

// Substring substitution.
void gsub(string *expr, const string target, const string replace)
{
  unsigned int j=0;
  while ( (j=expr->find(target)) < expr->length() )
    {
      expr->replace(j, target.length(), replace);
    }
}

// Make .set file name
string makeSetFileName(char *filename)
{
  string setfilename(filename);
  gsub(&setfilename, ".raw", ".set");
  return setfilename;
}

// Make .tim file name
string makeTimFileName(char *filename)
{
  string timfilename(filename);
  gsub(&timfilename, ".raw", ".tim");
  return timfilename;
}


