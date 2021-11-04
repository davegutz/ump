// Sort and triangulate ball positions

#include "Ball.h"

/////////////////class BallStats////////////////////////////////////
int compareBalls(const BallStats *left, const BallStats *right)
{
  float diff =  timerdiff(left->load, right->load);
  if      ( diff > 0 )                     return -1;
  else if ( diff < 0 )                     return  1;
  else if ( left->device > right->device ) return -1;
  else if ( left->device < right->device ) return  1;
  else if ( left->light  > right->light  ) return -1;
  else if ( left->light  < right->light  ) return  1;
  else if ( left->size   > right->size   ) return -1;
  else if ( left->size   < right->size   ) return  1;
  else                                     return  0;
}

// Sort the balls into useful order, defined by compareBalls.
void sortPastBalls(BallStats *balls, int & numBalls, const timeval startFrame,
		   const float deltaFrame)
{
  // Sort basic balls.
  qsort(balls, numBalls, sizeof(BallStats), (cmp_func)compareBalls);
  // Cull out old ones
  for(int i=0; i<numBalls; i++)
    {
      if ( deltaFrame < (startFrame - balls[i].load) )
	{
	  numBalls = i;
	  break;
	}
    }
}

// Average ball velocities, accounting for gravity in z-direction but use
// at most last numpast+2 samples for z.
PixelLoc3 avgBalls(BallStats & presentBall, BallStats & previousBall,
		   BallStats *pastBalls, const int numpast, const float grav)
{
  PixelLoc3 avg;
  timeval tRef = presentBall.load;
  float   deltaT;
  int     numAvg  = 0;
  int     numZAvg = 0;
  if ( 0 == sqnorm(presentBall.velo) ) return avg;
  else
    {
      avg = presentBall.velo;
      numAvg++;
      numZAvg++;
    }
  if ( 0 < sqnorm(previousBall.velo) )
    {
      avg[0] += previousBall.velo[0];
      avg[1] += previousBall.velo[1];
      deltaT  = tRef - previousBall.load;
      avg[2] += (previousBall.velo[2] + grav*deltaT);
      numAvg++;
      numZAvg++;
    }
  // Limit use of vx & vy past to numpast + previous
  for (int i=0; i<numpast; i++)
    {
      if ( 0 < sqnorm(pastBalls[i].velo) )
	{
	  avg[0] += pastBalls[i].velo[0];
	  avg[1] += pastBalls[i].velo[1];
	  numAvg++;
	}
    }
#ifdef VERBOSE
  if ( 1 < verbose )
    cout << "BALL NUM AVG=" << numAvg << " ";
#endif
  avg[0] /= (float) numAvg;
  avg[1] /= (float) numAvg;
  avg[2] /= (float) numZAvg;
  return avg;
}
// Average ball velocities, accounting for gravity in z-direction but use
// at most last numpast+2 samples for z.
pixelLocf avgZone(BallStats & presentBall, BallStats & previousBall,
		  BallStats *pastBalls, const int numpast, const float grav,
		  PixelLoc3 & avgV, pixelLocf & avgU, PixelLoc3 & avgVU)
{
  //  PixelLoc3 avgV;
  pixelLocf avgZone;
  timeval tRef = presentBall.load;
  float   deltaT;
  float   numAvg = 0;      // Sum of weighted balls
  float   wtPres = 3;      // Weight of present ball
  float   wtPrev = 2;      // Weight of previous ball
  float   wtPast = 1;      // Weight of past balls
  int     numZBallsAvg = 0; // Number of Z balls averaged
  int     numBallsAvg = 0; // Number of balls averaged, for reporting only
  if ( 0 == sqnorm(presentBall.velo) ) return avgZone;
  else
    {
      avgV  = presentBall.velo;
      avgV[0]  *= wtPres;
      avgV[1]  *= wtPres;
      avgV[2]  *= 1.0;
      for ( int i=0; i<3; i++ )
	avgVU[i] = wtPres * presentBall.VeloErr[i]* presentBall.VeloErr[i];
      numAvg  += wtPres;
      numZBallsAvg++;
      numBallsAvg++;
    }
  if ( 0 < sqnorm(previousBall.velo) )
    {
      avgV[0] += wtPrev * ( previousBall.velo[0] );
      avgV[1] += wtPrev * ( previousBall.velo[1] );
      deltaT   = tRef - previousBall.load;
      avgV[2] += (previousBall.velo[2] + grav*deltaT);
      for ( int i=0; i<3; i++ )
	avgVU[i] += wtPrev * previousBall.VeloErr[i]* previousBall.VeloErr[i];
      numAvg  += wtPrev;
      numZBallsAvg++;
      numBallsAvg++;
    }
  // Limit use of vx & vy past to numpast + previous
  for (int i=0; i<numpast; i++)
    {
      if ( 0 < sqnorm(pastBalls[i].velo) )
	{
	  avgV[0]  += wtPast * ( pastBalls[i].velo[0]    );
	  avgV[1]  += wtPast * ( pastBalls[i].velo[1]    );
	  for ( int j=0; j<3; j++ )
	    avgVU[j] += wtPast*pastBalls[i].VeloErr[j]*pastBalls[i].VeloErr[j];
	  numAvg   += wtPast;
	  numBallsAvg++;
	}
    }
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "BALL NUM AVG=" << numBallsAvg << " ";
#endif
  avgV[0]  /= (float) numAvg;
  avgV[1]  /= (float) numAvg;
  avgV[2]  /= (float) numZBallsAvg;
  for ( int i=0; i<3; i++ )
    avgVU[i] = sqrt(avgVU[i] / (float) numAvg);
#ifdef VERBOSE
  if ( 1 < verbose ) cout << "BALL AVG             =" << avgV 
			  << endl << flush;
#endif

  // Now have average velocity.  Next calculate average position.
  float timeZone;
  numAvg    = 0;
  timeZone  =  fabs(presentBall.pos[1] / avgV[1]);
  avgZone.x = wtPres * ( presentBall.pos[0] + timeZone*avgV[0] );
  avgZone.y = wtPres * ( presentBall.pos[2] + timeZone*avgV[2]
                                   - grav*timeZone*timeZone/2 );

  // Weight present ball as wtPres
  numAvg  += wtPres;
  
  if ( 0 < sqnorm(previousBall.velo) )
    {
      timeZone   = fabs(previousBall.pos[1] / avgV[1]);
      avgZone.x += wtPrev * ( previousBall.pos[0] + timeZone*avgV[0] );
      avgZone.y += wtPrev * ( previousBall.pos[2] + timeZone*avgV[2]
                                         - grav*timeZone*timeZone/2  );
      numAvg    += wtPrev;
    }
  
  // Limit use of vx & vy past to numpast + previous
  for (int i=0; i<numpast; i++)
    {
      if ( 0 < sqnorm(pastBalls[i].velo) )
	{
	  timeZone   = fabs(pastBalls[i].pos[1] / avgV[1]);
	  avgZone.x += wtPast * ( pastBalls[i].pos[0] + timeZone*avgV[0] );
	  avgZone.y += wtPast * ( pastBalls[i].pos[2] + timeZone*avgV[2]
	                                     - grav*timeZone*timeZone/2  );
	  numAvg    += wtPast;
	}
    }
  avgZone.x /= (float) numAvg;
  avgZone.y /= (float) numAvg;
  avgU.x    = timeZone * avgVU[0];
  avgU.y    = timeZone * avgVU[2];

  return avgZone;
}

// Triangulate ball position
PixelLoc3 triangulate(BallStats &T)
{
  PixelLoc3 location;
  PixelLoc3 uncert;
  PixelLoc3 O0p(T.centroid, T.pixP->focalCalEx());
  PixelLoc3 O1p(T.centroidOther, T.pixPOther->focalCalEx());
  // Transform to calibration scaling, focal already initialized correctly
  float SXN = float( T.pixP->width()  ) / float( T.pixP->widthCalEx()  );
  float SYM = float( T.pixP->height() ) / float( T.pixP->heightCalEx() );
  O0p[0] /= SXN;
  O0p[1] /= SYM;
  O1p[0] /= SXN;
  O1p[1] /= SYM;
  Vector <float> dP(2);
  Matrix <float> vT(2,2);
  Vector <float> RL(2);

  // Shift camera pixel coordinates to camera center coordinates and normalize
#ifdef VERBOSE
  if ( 4 < verbose )
    {
      cout << "Raw: " << endl;
      cout << "O0p=" << O0p << flush << endl;
      cout << "O1p=" << O1p << flush << endl;
    }
#endif
  O0p[0] = O0p[0] - T.pixP->imCalEx();
  O0p[1] = T.pixP->jmCalEx() - O0p[1];
  O1p[0] = O1p[0] - T.pixPOther->imCalEx();
  O1p[1] = T.pixPOther->jmCalEx() - O1p[1];
#ifdef VERBOSE
  if ( 4 < verbose )
    {
      cout << "Centered: " << endl;
      cout << "O0p=" << O0p << flush << endl;
      cout << "O1p=" << O1p << flush << endl;
    }
#endif
  norm(O0p);
  norm(O1p);
#ifdef VERBOSE
  if ( 4 < verbose )
    {
      cout << "Normed: " << endl;
      cout << "O0p=" << O0p << flush << endl;
      cout << "O1p=" << O1p << flush << endl;
    }
#endif

  // Translate measurement pixels to real world coordinates
  T.pixP->rotate(O0p);
  T.pixPOther->rotate(O1p);
  norm(O0p);
  norm(O1p);
#ifdef VERBOSE
  if ( 4 < verbose )
    {
      cout << "Rotated: " << endl;
      cout << "O0p=" << O0p << flush << endl;
      cout << "O1p=" << O1p << flush << endl;
    }
#endif

  // Use library to
  // solve the equation vT*RL=dP where vT is 2x2 matrix of the first two
  // elements of predicted left and right direction vectors in world space,
  // dP is 1x2 matrix of the first two elements of left-right postions of
  // camera lens, and RL is to be calculated as the right and left camera
  // scalars on direction vectors to produce an intersection at the real
  // world object.  The problem is inherently over-specified with 4
  // coordinates (two directions in each direction vector ) predicting a 
  // 3 coordinate point in space.  This is used to advantage because lines
  // usually don't intersect allowing calculation of zone.  Assume average
  // of solutions is center of zone.  Discard solutions whose determinant 
  // is small which means the projections of the two camera rays in that 
  // plane are nearly parallel.

  int combos[3][2] = { { 0, 1 },
		       { 0, 2 },
		       { 1, 2 } };
  float weight[3]  = { 0.,
		       0.,
		       0. }; // Weight to apply to each combo

  // Loop through all three combinations and average
  for ( int i=0; i<3; i++ )
    {
      int I = combos[i][0];
      int J = combos[i][1];


      // Difference between cameras
      dP[0] = T.pixP->P(I) - T.pixPOther->P(I);
      dP[1] = T.pixP->P(J) - T.pixPOther->P(J);
#ifdef VERBOSE
      if ( 4 < verbose ) cout << "dP=" << dP << flush << endl;
#endif

      vT[0][0] =  O1p[I];
      vT[0][1] = -O0p[I];
      vT[1][0] =  O1p[J];
      vT[1][1] = -O0p[J];
      float detvT = fabs(vT[0][0]*vT[1][1]-vT[0][1]*vT[1][0]);
#ifdef VERBOSE
      if ( 4 < verbose ) cout << "vT=" << vT;
#endif
      if ( 0.05 > detvT )
	{
#ifdef VERBOSE
	  if ( 4 < verbose )
	    {
	      cout << "MESSAGE(triangulate): skipping combo "
		   <<I << "," << J << flush << endl;
	    }
#endif
	  continue;
	}


      Vector <Subscript> indexT;
      RL = dP;
      LU_factor(vT, indexT);
      LU_solve (vT, indexT, RL);
#ifdef VERBOSE
      if ( 4 < verbose ) cout << "RL=" << RL;
#endif
    
      // Predict location and add to subtotal
      weight[I] += 1.;
      weight[J] += 1.;
      float locI = T.pixP->P(I) + RL[1]*O0p[I];
      float locJ = T.pixP->P(J) + RL[1]*O0p[J];
      location[I] += locI;
      location[J] += locJ;
      if ( 0 == uncert[I] ) uncert[I]  = locI;
      else                  uncert[I] -= locI;
      if ( 0 == uncert[J] ) uncert[J]  = locJ;
      else                  uncert[J] -= locJ;
#ifdef VERBOSE
      if ( 4 < verbose ) cout << "location=" << location << flush << endl;
#endif
    }
  // Weighted average
  for (int i=0; i<3; i++)
    {
      if ( 0.0 < weight[i] )
	location[i] /= weight[i];
      else 
	location[i] = 0.0;
      if ( 1.0 == weight[i] ) 
	uncert[i] = 0.0;
      else
	uncert[i] /= weight[i];
    }

  // Check error
#ifdef VERBOSE
  if ( 4 < verbose ) cout << "uncertainty = " << uncert << flush << endl;
#endif

  T.pos = location;
  T.err = sqrt(sqnorm(uncert));
  T.Err = uncert;
  return location;
}

// Operators
ostream & operator<< (ostream & out, const BallStats &BS)
{
  char outstr[MAX_CANON];
  sprintf(outstr,
	 "%9.6f size= %3.0f dens= %4.2f centroid= %5.1f,%5.1f other= %5.1f,%5.1f\n",
	  BS.load - BS.procStart, BS.size, BS.density,
	  BS.centroid.x, BS.centroid.y, BS.centroidOther.x,
	  BS.centroidOther.y);
  out << outstr;
  return out;
}


/////////////////////////class Ball/////////////////////////////////////

// Access the ball summary information
BallStats Ball::ballSummary()
{
  BallStats summary;
  summary.value       = value_;
  summary.light       = isWhiter();
  summary.device      = -1;
  summary.size        = area();
  summary.density     = density();
  summary.velocity.x  = 0.0;
  summary.velocity.y  = 0.0;
  timerclear(&summary.load);
  timerclear(&summary.procStart);
  summary.centroid    = centroid_;
  return summary;
}


/////////////////////////class BallSet//////////////////////////////////

// Add a ball to the array.
int BallSet::addBall(Ball *ballP, const int device, const foundStuff found,
		     Image *pixP, const timeval procStart, const int fps)
{
  int isWhite = 0;  // Return value
  if ( !timerisset(&procStart_) ) procStart_ = procStart;
  fps_  = fps;

  
  // Adjust array, automatically discarding last value by over-write
  if ( MAXBALLS == numBalls_ ) numBalls_--;
  for ( int i=numBalls_; i>0; i--) balls_[i] = balls_[i-1];

  // Add new value
  balls_[0]           = ballP->ballSummary();
  balls_[0].procStart = procStart;
  if ( balls_[0].light )
    {
      balls_[0].load    = found.presLoad;
      isWhite          = 1;
    }
  else
    {
      balls_[0].load    = found.prevLoad;
      isWhite          = 0;
    }
  balls_[0].device    = device;
  balls_[0].pixP      = pixP;
  numBalls_++;
  return isWhite;
}

// Sort the balls into useful order, defined by compareBalls.
void BallSet::sortBalls()
{
  // Sort basic balls.
  qsort(balls_, numBalls_, sizeof(BallStats), (cmp_func)compareBalls);
  // Reorder the light balls.
  numLightBalls_ = 0;
  for (int i=0; i<numBalls_; i++) if ( balls_[i].light )
    {
      lightBalls_[numLightBalls_] = &balls_[i];
      numLightBalls_++;
    }
}

// Operators
ostream & operator<< (ostream & out, const BallSet &BS)
{
  char outstr[MAX_CANON];
  for (int i=0; i<BS.numBalls_; i++)
    {
      sprintf(outstr,
	      "%d  %d %14.6f size=%3.0f dens=%4.2f centroid=%5.1f,%5.1f\n",
	      BS.balls_[i].device, BS.balls_[i].light, 
	      BS.balls_[i].load - BS.procStart_,  BS.balls_[i].size,
	      BS.balls_[i].density,
	      BS.balls_[i].centroid.x, BS.balls_[i].centroid.y);
      out << outstr;
    }
  return out;
}


////////////////////////class CamBallS/////////////////////////////////////

// Determine pixel position of latest identifiable ball in set.
// Find the loner among the last 3 balls and use it.
// There are three useful combinations:  X O X, X X 0,  and O X X 
BallStats CamBallS::latestBall(const float dTbias)
{
  int numStep;
  if ( memTime || realTime ) numStep = NUMSTEP;
  else            numStep = NUMFILESTEP;
  BallStats chosen = *lightBalls_[0];
  chosen.centroidOther.x = 0.0; 
  chosen.centroidOther.y = 0.0;
  chosen.hasOther        = 0;
  chosen.pixPOther       = NULL;
  float      epsilon    = 1.0 / (float)fps_ / 5.0;
  float      maxStep     = ( (float)numStep / (float)fps_ ) + epsilon;
  float      dTself, dTother;
  pixelLocf  dOther;

  // Not enough balls for combinations
  if ( numLightBalls_ < 3 ) return chosen;

  // O X X pattern
  if ( lightBalls_[2]->device == lightBalls_[1]->device
       &&
       lightBalls_[2]->device != lightBalls_[0]->device   )
    {
#ifdef VERBOSE
      if ( 1 < verbose )
	cout << "O X X latest is unique; "
	     << "extrapolate previous two to present\n";
#endif
      chosen          = *lightBalls_[0];
      chosen.centroidOther.x = 0.0; 
      chosen.centroidOther.y = 0.0;
      chosen.hasOther        = 0;
      chosen.pixPOther       = NULL;
      dTself          = lightBalls_[0]->load     - lightBalls_[2]->load;
      dTother         = lightBalls_[1]->load     - lightBalls_[2]->load;
      dOther          = lightBalls_[1]->centroid - lightBalls_[2]->centroid;
      if ( epsilon > dTother )
	{
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "MESSAGE(ump): small other time step = "
		 << dTother << endl;
#endif
	  goto endlocate;
	}
      if ( maxStep < dTself )
	{
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "MESSAGE(ump): large self time step = "
		 << dTself << " versus max step= " << maxStep << endl;
#endif
	  goto endlocate;
	}
      chosen.centroidOther = lightBalls_[2]->centroid
	+ dOther * ( dTself + dTbias ) / dTother;
      chosen.hasOther      = 1;
      chosen.pixPOther     = lightBalls_[2]->pixP;
    }

  // X O X pattern
  else if ( lightBalls_[2]->device != lightBalls_[1]->device
	    &&
	    lightBalls_[2]->device == lightBalls_[0]->device )
    {
#ifdef VERBOSE
      if ( 1 < verbose )
	cout << "X O X previous is unique; interplolate others to previous\n";
#endif
      chosen          = *lightBalls_[1];
      chosen.centroidOther.x = 0.0; 
      chosen.centroidOther.y = 0.0;
      chosen.hasOther        = 0;
      chosen.pixPOther       = NULL;
      dTself          = lightBalls_[1]->load     - lightBalls_[2]->load;
      dTother         = lightBalls_[0]->load     - lightBalls_[2]->load;
      dOther          = lightBalls_[0]->centroid - lightBalls_[2]->centroid;
      if ( epsilon > dTother )
	{
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "MESSAGE(ump): small other time step = "
		 << dTother << endl;
#endif
	  goto endlocate;
	}
      if ( maxStep < dTother )
	{
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "MESSAGE(ump): large others time step = "
		 << dTother << " versus max step= " << maxStep << endl;
#endif
	  goto endlocate;
	}
      chosen.centroidOther = lightBalls_[2]->centroid
	+ dOther * ( dTself + dTbias ) / dTother;
      chosen.hasOther      = 1;
      chosen.pixPOther     = lightBalls_[2]->pixP;
    }
  
  // X X O pattern
  else if ( lightBalls_[2]->device != lightBalls_[1]->device
	    &&
	    lightBalls_[2]->device != lightBalls_[0]->device )
    {
#ifdef VERBOSE
      if ( 1 < verbose )
	cout << "X X O two back is unique; extraplolate others back two\n";
#endif
      chosen          = *lightBalls_[2];
      chosen.centroidOther.x = 0.0; 
      chosen.centroidOther.y = 0.0;
      chosen.hasOther        = 0;
      chosen.pixPOther       = NULL;
      dTself          = lightBalls_[2]->load     - lightBalls_[0]->load;
      dTother         = lightBalls_[1]->load     - lightBalls_[0]->load;
      dOther          = lightBalls_[1]->centroid - lightBalls_[0]->centroid;
      if ( epsilon > fabs(dTother) )
	{
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "MESSAGE(ump): small other time step = "
		 << dTother << endl;
#endif
	  goto endlocate;
	}
      if ( maxStep < fabs(dTself) )
	{
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "MESSAGE(ump): large self time step = "
		 << dTself << " versus max step= " << maxStep << endl;
#endif
	  goto endlocate;
	}
      chosen.centroidOther = lightBalls_[0]->centroid
	+ dOther * ( dTself + dTbias ) / dTother;
      chosen.hasOther      = 1;
      chosen.pixPOther     = lightBalls_[0]->pixP;
    }
  
 endlocate:
  return chosen;
}

// Operators
ostream & operator<< (ostream & out, const CamBallS &BS)
{
  for (int i=0; i<BS.numLightBalls_; i++)
    {
      out << *BS.lightBalls_[i];
    }
  return out;
}

//////////////////////////class EqCamBallS////////////////////////////////

// Determine position of latest ball in set.
BallStats EqCamBallS::latestBall(const float dTbias)
{
  int numStep;
  if ( memTime || realTime )
    numStep = NUMSTEP;
  else
    numStep = NUMFILESTEP;
  BallStats  chosen    = *lightBalls_[0];
  float      epsilon  = 1.0 / (float)fps_ / 5.0;
  float      maxStep  = ( (float)numStep / (float)fps_ ) + epsilon;
  float      dTself, dTother;
  pixelLocf  dSelf, vSelf, dOther, vOther, predOther, predChosen;

  // Not enough balls for combinations
  if ( numLightBalls_ < 3 ) goto endlocate;

  // Various combinations
  if ( chosen.device == lightBalls_[1]->device )
    { // Last two same device
      int i = 2;
      while ( i<numLightBalls_
	      && lightBalls_[i]->device == chosen.device ) i++; 
      if ( i >= numLightBalls_
	   || lightBalls_[i]->device == chosen.device )
	goto endlocate;
#ifdef VERBOSE
      if ( 1 < verbose )
	cout << "Last two same device; back calc other then predict pres\n";
#endif
      dTself          = chosen.load - lightBalls_[1]->load;
      if ( epsilon > dTself || maxStep < dTself) goto endlocate;
      dSelf           = chosen.centroid - lightBalls_[1]->centroid;
      vSelf           = dSelf / dTself;
      dTother         = chosen.load - lightBalls_[i]->load;
      if ( maxStep < dTother) goto endlocate;
      predOther       = chosen.centroid - dSelf * dTother / dTself;
      predChosen      = lightBalls_[i]->centroid + dTother * vSelf;
      chosen.centroid = predChosen;
      chosen.velocity = vSelf;
    }
  else
    {
      if ( chosen.device == lightBalls_[2]->device )
	{
	  dTself        = chosen.load - lightBalls_[2]->load;
	  if ( epsilon > dTself || maxStep < dTself) goto endlocate;
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "Last two diff device, third same; "
		 << "interp other then predict pres\n";
#endif
	  dSelf           = chosen.centroid - lightBalls_[2]->centroid;
	  vSelf           = dSelf / dTself;
	  dTother         = chosen.load - lightBalls_[1]->load;
	  if ( maxStep < dTother) goto endlocate;
	  predOther       = chosen.centroid - dSelf * dTother / dTself;
	  predChosen      = lightBalls_[1]->centroid + dTother * vSelf;
	  chosen.centroid = predChosen;
	  chosen.velocity = vSelf;
#ifdef VERBOSE
	  if ( 1 < verbose )
	    {
	      cout << "dTself=" << dTself << " dSelf=" << dSelf << " vSelf="
		   << vSelf <<  endl;
	      cout << "dTother=" << dTother << " predOther=" << predOther
		   << " predChosen=" << predChosen << endl;
	    }
#endif
	}
      else
	{
	  dTself          = chosen.load - lightBalls_[2]->load;
	  dTother         = lightBalls_[1]->load - lightBalls_[2]->load;
	  if ( epsilon > dTother || maxStep < dTother ) goto endlocate;
#ifdef VERBOSE
	  if ( 1 < verbose )
	    cout << "Last two diff device, third diff; extrap other to pres\n";
#endif
	  dOther          = lightBalls_[1]->centroid
	    - lightBalls_[2]->centroid;
	  vOther          = dOther / dTother;
	  predOther       = lightBalls_[2]->centroid
	    + dOther * dTself / dTother;
	  chosen.centroid = predOther;
	  chosen.velocity = vOther;
	}
    }
#ifdef VERBOSE
  if ( 1 < verbose )
    cout << "Error Chosen = "
	 << magErr(chosen.centroid, lightBalls_[0]->centroid)/plane_*100
	 << endl;
#endif
 endlocate:
  return chosen;
}

// Operators
ostream & operator<< (ostream & out, const EqCamBallS &BS)
{
  for (int i=0; i<BS.numLightBalls_; i++)
    {
      out << *BS.lightBalls_[i];
    }
  return out;
}


