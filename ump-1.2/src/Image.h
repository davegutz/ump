// Image for segmentation
// DA Gutz 17-Jun-01
/* 	$Id: Image.h,v 1.1 2007/12/16 18:26:40 davegutz Exp davegutz $	 */
#ifndef Image_h
#define Image_h 1

#include <string>      // string classes
#include <string.h>    // strings
#include <myTime.h>    // timers
#include "Tune.h"
#include "Segment.h"
#include "Mask.h"

struct reading_t
{
  long startV;
  long numV;
  long loadSec;
  long loadUsec;
  long ploadSec;
  long ploadUsec;
};

struct reading_t aClearReadingT();

// Status structure
struct producerTimes {
  char    deviceDigit;     // Device indicator
  timeval start;           // Start throughput measure.
  timeval load;            // Load time throughput measure.
  timeval motion;          // Motion calculation throughput measure.
  timeval printed;         // Print time throughput measure.
  timeval stop;            // Stop throughput measure.
  timeval presLoad;        // Present image snapshot time
  timeval procStart;       // Original image snapshot time
  int     numMoved;        // Number of changed pixels
  int     numNodes;        // Number of accepted changes
  int     nslips;          // Number of discarded frames
  int     fps;             // Frame rate.
};

///////////////////////////class Image//////////////////////////////////
// Image class
class Image
{
protected:
  Pixel  **picture_;                    // Pointer to image pixels
  Mask    *mask_;                       // Pointer to the mask
  float    agauss_;                     // Distance edge weigth in gaussian.
  float    bgauss_;                     // Intensity edge weigth in gaussian.
  int      nnodes_;                     // Number of existing nodes
  int      nedges_;                     // Number of edges = nml_*nml_
  int     *inodes_;                     // Indeces of existing nodes
  float   *W_;                          // Edge weights.
  float   *d_;                          // Edge weight sum.
  int      deleteNode_(const int duck); // Function to delete a node
  int      fps_;                        // Frame rate, 1/sec
  int      m_;                          // Height
  int      n_;                          // Width
  int      bufsize_;                    // Buffer size, width x height x colors
  float    time_;                       // Time of image, sec
  float    dtime_;                      // Time since last image, sec
  int      num_;                        // Number image buffer loads
  timeval  start_, load_, motion_, printed_, stop_;  // Misc times for debug
  int      imageNumber_;                // Valid numbers images
  ballTune ballCrit_;                   // Ball selection criteria
  int      numMoved_;                   // Number of moved pixels since last
  char     deviceDigit_;                // Dev designation, 0 or 1 /dev/video
  int      maxEdgePix_;                 // Limit on motion pixels (throughput)
  int      dev_;                        // Dev designation
  timeval  presLoad_;                   // Present load time
  SegmentSet *segs_;                    // Segment management
  static const int   NUMCORNERS  =    2;// Number of bogus corners added
  static const int   MINPIX      =    2;// Minimum pixels in move for solution
  static const int   MAXPIX      =   50;// Maximum pixels to try eigensolution
  Vector <float>  eigVector_;           // Segmentation eigenvector
  Hypotenuse     *distance_;            // Class for pixel distance calculation
  char           *device_;              // Dev designation: /dev/video<device>
  int             neutral_;             // Specified neutral pixel value
  float           warmupTime_;          // Device warmup time
  int            *imageR_;              // Image rate storage
  int             minEdgePix_;          // Limit on motion pixels (resolution)
  int             numToCheck_;          // Number of pixels to check edges
  float           scalar_;              // Size compared to 160x120
  int             sub_;                 // Subsample multiple
  int             calibrate_;           // Doing calibration
  int             loadTest_;            // Doing load test
  float           scalar4_;             // 4th power of scalar_
  int             sub4_;                // 4th power of sub_
  int             nSub_;                // Subsample n_
  int             mSub_;                // Subsample m_
  int             bufsizeSub_;          // Subsample bufsize_
  int             writeCompact_;        // Command to compact subsampled image
  int             still_;               // Taking stills
  Calibration    *CAL_;                 // Calibration information
 public:
  Image();
  Image(const int n, const int m, const int fps,
	const char device_digit, const int maxedgepix, const int dev,
	char *device, const int neutral, const float warmupTime,
	const float agauss, const int sub, const int calibrate,
	const int loadTest, const int writeCompact, const int still,
	Calibration *cal);
  Image(const Image & I);
  Image & operator=(const Image & I);
  virtual ~Image();
  int      nslips;      // synchronization counter
  float    ACPW(int i, int j){return (*(CAL_->ACPW()))[i][j];}
  ballTune ballCrit()   {return ballCrit_;}
  int      imageNumber(){return imageNumber_;}
  producerTimes timeSummary();
  Calibration  *CAL()   {return CAL_;}
  timeval  presLoad()   {return presLoad_;}
  float    dtime()      {return dtime_;}
  float    time()       {return time_;}
  float    warmupTime() {return warmupTime_;}
  float    P(int i)     {return (*(CAL_->P()))[i];}
  char    *device()     {return device_;}
  //  void     distort(float *x, float *y);
  float    focalCalEx() {return CAL_->focalCalEx();}
  int      fps()        {return fps_;}
  float    imCalEx()    {return CAL_->imCalEx();}
  float    jmCalEx()    {return CAL_->jmCalEx();}
  int      width()      {return n_;}
  int      height()     {return m_;}
  int      heightCalEx(){return CAL_->imHeightCalEx();}
  int      loadTest()   {return loadTest_;}
  int      maxEdgePix() {return maxEdgePix_;}
  int      makeEdges();
  int      dev()        {return dev_;}
  int      numThings()  {return segs_->nSeg();}
  int      cameraSameAs(Image & O);
  void     removeFiles();
  void     rotate(Vector <float> &T){ T = (*(CAL_->ACPW()))*T;}
  void     rotate(Matrix <float> &T){ T = (*(CAL_->ACPW()))*T;}
  int      segmentCut();
  int      spectralCalc();
  float    scalar()     {return scalar_;}
  int      sub()        {return sub_;}
  TDO     *thingP(int i){return segs_->segP(i);}
  int      widthCalEx() {return CAL_->imWidthCalEx();}
  void     writeRlbl(char *filename);
  virtual  void assignEdges(){}
  virtual  void addCorners() {}
  virtual  int  loadOne()    {return 0;}
  virtual  int  move()       {return 0;}
  virtual  Pixel *pixelPtr(int i){return 0;}
  virtual  timeval prevLoad(){timeval temp; return temp;};
  virtual  void reassignSmptr(int *smptr)   {}
  virtual  void reassignIfd  (const int ifd){}
  virtual  void reassignOfd  (const int ofd){}
  virtual  void reassignTalk (int *talk){}
  virtual  void resetTimers(){}
  virtual  int  segment()    {return 0;}
  virtual  void sendMask()   {}
  virtual  void startSnap()  {}
  virtual  void writeRppm(char *filename){}
  virtual  void writeRmm(char *filename){}
  virtual  int     talk()    {return -1;};
 friend ostream & operator<< (ostream & out, const Image & image);
};

// Image constructed real time from shared memory
class RealImage : public Image
{
protected:
  int        startV_;
  int        numV_;
  int       *smptr_;
  int        writeSnap_;
  int       *talk_;
  reading_t  reading_;
  unsigned char *imageV_;// memory allocation for image rate information
  timeval    prevLoad_;
public:
  RealImage();
  RealImage(const int dev, const int n, const int m, const int fps, int *smptr,
	    const char device_digit, const int writeSnap, const int maxedgepix,
	    int *talk, char *device, const float warmupTime,
	    const float agauss, const int sub, const int calibrate,
	    const int loadTest, const int writeCompact, Calibration *cal);
  RealImage(const RealImage & R);
  RealImage & operator=(const RealImage & R);
  virtual ~RealImage();
  virtual int     talk()         {return *talk_;};
  virtual void    addCorners();
  virtual void    assignEdges();
  virtual int     loadOne();
  virtual int     move();
  virtual void    writeRmm(char *filename);
  friend  void    writeToFiles(RealImage & image);
  virtual Pixel  *pixelPtr(int i)              {return picture_[i];}
  virtual timeval prevLoad()                   {return prevLoad_;}
  virtual int     segment();
  virtual void    startSnap();
  virtual void    reassignIfd  (const int ifd) {}
  virtual void    reassignOfd  (const int ofd) {}
  virtual void    reassignSmptr(int *smptr)    {smptr_ = smptr;}
  virtual void    reassignTalk (int *talk)     {talk_  = talk;}
  virtual void    resetTimers();
  virtual void    sendMask();
};

// Image constructed real time from shared memory
class MemImage : public RealImage
{
  int        ifd_;
  int        ofd_;
protected:
public:
  MemImage();
  MemImage(const int dev, const int n, const int m, const int fps, int *smptr,
	   const char device_digit, const int writeSnap, const int maxedgepix,
	   const int ifd, const int ofd, char *device, const float warmupTime,
	   const float agauss, const int sub, const int calibrate,
	   const int loadTest, const int writeCompact, Calibration *cal);
  ~MemImage();
  virtual int     loadOne();
  virtual int     talk()       {return -1;}
  virtual void    reassignIfd  (const int ifd) {ifd_   = ifd;}
  virtual void    reassignOfd  (const int ofd) {ofd_   = ofd;}
  virtual void    reassignTalk (int *talk){}
  virtual void    sendMask();
};



// Image constructed off-line from saved files
class FileImage : public Image
{
protected:
  unsigned char *imageZ_;   // Past image
  int            ifp_;      // Data file handle
  int            tfp_;      // Time stamp file handle
  timeval        pastLoad_; // Past load time
  int            deadband_; // Rate calc deadband, pixels
  int            showUndistort_;//Write undistorted image to .ppm
  float          endTime_;  // Time limit specified by -t|--absframes
  int            absframes_;// file heading no. frames
public:
  FileImage();
  FileImage(const int dev, int ifp[2], const char device_digit, const int n,
	    const int m, const int fps, const int maxedgepix, char *device,
	    const float warmupTime, const float agauss, const int sub,
	    const int deadband, const int calibrate, const int loadTest,
	    const int showUndistort, const int writeCompact, const int still,
	    Calibration *cal, const float endTime, const int absframes);
  FileImage(const FileImage & F);
  FileImage & operator=(const FileImage & F);
  ~FileImage();
  unsigned char   *image_;
  virtual  void    addCorners();
  virtual void     assignEdges();
  virtual  int     loadOne();
  virtual  int     move();
  virtual  Pixel  *pixelPtr(int i)   {return picture_[i];}
  virtual  timeval prevLoad()        {return pastLoad_;}
  virtual  void    writeRppm(char *filename);
  virtual  void    writeRmm(char *filename);
  virtual  int     segment();
  virtual  void    sendMask()        {}
  friend   void    writeToFiles(FileImage & image);
};


// Utilities
string makeSetFileName(char *filename);
string makeTimFileName(char *filename);
void   assignRGB(const int request, int *RED, int *GREEN, int *BLUE);
void   writeToFiles(FileImage & image);
void   writeToFiles(RealImage & image);
void   gsub(string *expr, const string target, const string replace);

#endif // Image_h
