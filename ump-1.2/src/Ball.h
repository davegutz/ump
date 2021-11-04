#ifndef ball_h
#define ball_h 1

// Include section
#include <stdio.h>     // sprintf
#include <myTime.h>    // timers
#include "PixelLoc.h"
#include "Image.h"
extern "C"
{
 #include <Cwrappthread.h>  // pthread_mutex_t and pthread_cond_t
}

// Globals
extern int memTime;
extern int realTime;
const  int NUMSTEP     = 4;            // Allowed real time gap
const  int NUMFILESTEP = 4;            // Allowed file image gap

////////////////////////////struct foundStuff////////////////////////////
class Ball;
struct foundStuff{
  pthread_mutex_t mutex;           // mutex
  pthread_cond_t  cond;            // mutex condition
  int             numThings;       // Number of candidate objects
  Ball          **thingP;          // Candidate balls
  Image          *pixP;            // Pointer to source image
  float           dtime;           // Time step since last, sec
  timeval         presLoad;        // Load time
  timeval         procStart;       // Original image snapshot time 
  timeval         prevLoad;        // Previous load time
};


/////////////////////////////////class BallStats//////////////////////////
class BallStats
{
 public:
  float     value;
  int       light;
  int       device;
  float     size;
  float     density;
  pixelLocf velocity;
  timeval   load;
  timeval   procStart;
  pixelLocf centroid;
  pixelLocf centroidOther; // Interpolated location other ball
  int       hasOther;  // Successfully interpolated other ball location
  Image    *pixP;      // These image pointers used to access
  Image    *pixPOther; // calibration information
  PixelLoc3 pos;
  float     err;       // uncertainty
  PixelLoc3 Err;       // uncertainty
  PixelLoc3 velo;
  float     veloErr;   // velocity uncertainty
  PixelLoc3 VeloErr;   // velocity uncertainty
  // Constructors
  BallStats()
    :  value(0), light(0), device(0), size(0), density(0),
    velocity(apixelLocf(0,0)), load(aClearTimeval()),
    procStart(aClearTimeval()), centroid(apixelLocf(0,0)),
    centroidOther(apixelLocf(0,0)), hasOther(0), pixP(NULL), pixPOther(NULL),
    pos(PixelLoc3()), err(0), Err(PixelLoc3()), velo(PixelLoc3()), veloErr(0),
    VeloErr(PixelLoc3()) {}
  BallStats(const BallStats & B)
    : value(B.value), light(B.light), device(B.device), size(B.size),
    density(B.density), velocity(B.velocity), load(B.load),
    procStart(B.procStart), centroid(B.centroid),
    centroidOther(B.centroidOther), hasOther(B.hasOther), pixP(B.pixP),
    pixPOther(B.pixPOther), pos(B.pos), err(B.err), Err(B.Err), velo(B.velo),
    veloErr(B.veloErr), VeloErr(B.VeloErr) {}
  BallStats & operator=(const BallStats & B)
    {
      if ( this == &B ) return *this;  // Allows self-assignment.
      /*
      *this = BallStats(B);
      */
      value     = B.value;
      light     = B.light;
      device    = B.device;
      size      = B.size;
      density   = B.density;
      velocity  = B.velocity;
      load      = B.load;
      procStart = B.procStart;
      centroid  = B.centroid;
      centroidOther= B.centroidOther;
      hasOther  = B.hasOther;
      pixP      = B.pixP;
      pixPOther = B.pixPOther;
      pos       = B.pos;
      err       = B.err;
      Err       = B.Err;
      velo      = B.velo;
      veloErr   = B.veloErr;
      VeloErr   = B.VeloErr;
      return *this;                    // Allows stacked assignments.
    }
  ~BallStats(){}
};
void sortPastBalls(BallStats *balls, int & numBalls, const timeval startFrame,
		   const float deltaFrame);
PixelLoc3 avgBalls(BallStats & presentBall, BallStats & previousBall,
		   BallStats *pastBalls, const int numpast, const float grav);
pixelLocf avgZone(BallStats & presentBall, BallStats & previousBall,
		  BallStats *pastBalls, const int numpast, const float grav,
		  PixelLoc3 & avgV, pixelLocf & avgU, PixelLoc3 & avgVU);
PixelLoc3 triangulate(BallStats &T);
ostream & operator<< (ostream & out, const BallStats &BS);


/////////////////////////class Ball/////////////////////////////////////
class Ball : public TDO
{
public:
  Ball() {};
  Ball(TDO   O) : TDO(O){}
  Ball(TDO *Op) : TDO(Op){}
  ~Ball(){}
  BallStats ballSummary();
};

/////////////////////////class BallSet//////////////////////////////////

class BallSet
{
protected:
  static const int MAXBALLS = 20;
  int        numBalls_;
  int        numLightBalls_;
  timeval    procStart_;
  int        fps_;
  BallStats  *balls_;
  BallStats **lightBalls_;
public:
  BallSet()
    : numBalls_(0), numLightBalls_(0), procStart_(aClearTimeval()), fps_(0),
    balls_(NULL), lightBalls_(NULL)
    {
      balls_      = new BallStats[MAXBALLS];
      lightBalls_ = new BallStats*[MAXBALLS];
     }
  BallSet(const BallSet & B)
    : numBalls_(B.numBalls_), numLightBalls_(B.numLightBalls_),
    procStart_(B.procStart_), fps_(B.fps_), balls_(NULL), lightBalls_(NULL)
    {
      balls_      = new BallStats[MAXBALLS];
      lightBalls_ = new BallStats*[MAXBALLS];
      for ( int i=0; i<MAXBALLS; i++ )
	{
	  balls_[i] = B.balls_[i];
	  lightBalls_[i] = B.lightBalls_[i];
	}
    }
  BallSet & operator=(const BallSet & B)
    {
      if ( this == &B ) return *this;  // Allows self-assignment.
      numBalls_      = B.numBalls_;
      numLightBalls_ = B.numLightBalls_;
      procStart_     = B.procStart_;
      fps_           = B.fps_;
      if ( balls_ ) zaparr(balls_);
      balls_      = new BallStats[MAXBALLS];
      if ( lightBalls_ ) zaparr(lightBalls_);
      lightBalls_ = new BallStats*[MAXBALLS];
      for ( int i=0; i<MAXBALLS; i++ )
	{
	  balls_[i] = B.balls_[i];
	  lightBalls_[i] = B.lightBalls_[i];
	}
      return *this;                    // Allows stacked assignments.
    }
  virtual ~BallSet()
    {
      if ( balls_      ) zaparr(balls_);
      if ( lightBalls_ ) zaparr(lightBalls_);
    }
  int      numLightBalls()       {return numLightBalls_;}
  timeval  procStart()           {return procStart_;}
  virtual  BallStats latestBall(const float dTbias){BallStats temp; return temp;}
  int      addBall(Ball *ballP, const int device, const foundStuff found,
		   Image *pixP, const timeval procStart, const int fps);
  void sortBalls();
  friend ostream & operator<< (ostream & out, const BallSet &BS);
};

//////////////////////////////////class CamBallS///////////////////////////
class CamBallS : public BallSet
{
protected:
  float plane_;       // Distance camera to plane of motion
		      // assumed perpendicular to camera axis
public:
  CamBallS() : BallSet(), plane_(0) {}
  CamBallS(float dist) : BallSet(), plane_(dist) {}
  ~CamBallS(){}
  virtual BallStats latestBall(const float dTbias);
  friend ostream & operator<< (ostream & out, const CamBallS &BS);
};


//////////////////////////////////class EqCamBallS/////////////////////////
class EqCamBallS : public BallSet
{
protected:
  float plane_;       // Distance camera to plane of motion
                      // assumed perpendicular to camera axis
public:
  EqCamBallS() : BallSet(), plane_(0){}
  EqCamBallS(float dist) : BallSet(), plane_(dist){}
  ~EqCamBallS(){}
  virtual BallStats latestBall(const float dTbias);
  friend ostream & operator<< (ostream & out, const EqCamBallS &BS);
};

#endif // ball_h
