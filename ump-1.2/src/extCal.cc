// Use geometry data and readings of three ball positions in each image
// to determine coordinate transformation properties and write them
// to a configuration file video?.cal for each camera device.
// Author:  D. Gutz
// 	$Id: extCal.cc,v 1.1 2007/02/10 02:25:26 davegutz Exp davegutz $	


// Includes section
using namespace std;
#include <common.h>
#ifdef HAVE_GETOPT_LONG
 #include <getopt.h>   // Argument list handling
#endif
#include <math.h>      // math
#include <string>      // string classes
#include <string.h>    // strings
#include <fstream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <new>         // assertion
#include <sys/param.h> // built-in constants and MIN/MAX
#include <assert.h>    // assertion
#include <limits.h>    // access built-in constants
#include <second.h>    // local brings in TNT stuff
#include <stdio.h>     // sprintf
#include "PixelLoc.h"  // rotate, center, etc
#include "Tune.h"      // World class, Segment class
#include <sys/time.h>  // timing
#include <InFile.h>    // Input file class
#include <zap.h>       // zap

// Globals
timeval globalProcStart;           // Process start time
int   verbose           =   0;     // print lots of information
int   dump              =   0;     // Dump debug command
int   calibrate         =   1;     // Doing calibration
int   force             =   0;     // Over-ride goodness check
char  path[MAX_CANON] = "\0";

// Program usage
void usage(char *prog)
{
    char *h;

    if ( NULL != (h = strrchr(prog,'/')) )
	prog = h+1;
    
    fprintf(stdout,
	    "\'%s\' Calibrates umpire system.\n"
	    "Reads world.dat, video?.dat, video?.cam."
	    "Writes video?.cal."
	    "usage:\n %s [ options ]\n"
	    "\n"
	    "options:\n"
	    "  -h, --help         this screen                     \n"
	    "  -f, --force        over-ride goodness checking           [%d]\n"
	    "  -p, --path         path to triangulate program           [%s]\n"
	    "  -v, --verbose      print lots of information             [%d]\n"
	    "                                                               \n"
	    "examples:                                                      \n"
	    "\'%s\'\n"
	    "     normal mode\n"
	    "\'%s -f\'\n"
	    "     over-ride goodness checking\n"
	    "--                                                             \n"
	    "(c) 2000-2005 Dave Gutz <davegutz@alum.mit.edu>\n",
	    prog,prog,force,path,verbose,prog,prog);
}
/* ---------------------------------------------------------------------- */


/////////////////////main/////////////////////////////////////////
int main(int argc, char **argv)
{
  ofstream outf;
  ifstream inf;
  char c;
  int badCompare = 0;

  // parse options
  int num_args=0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      int option_index = 0;
      static struct option long_options[] =
      {
	{"force",       0, 0, 'f'},
	{"help",        0, 0, 'h'},
	{"path",        1, 0, 'p'},
	{"version",     0, 0, 'V'},
	{"verbose",     1, 0, 'v'},
	{0, 0, 0, 0}
      };
      c = getopt_long(argc, argv, "fhp:Vv:",
		      long_options, &option_index);
#else
      c = getopt(argc, argv, "fhp:Vv:");
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
	case 'f':
	  force = 1;
	  break;
	case 'p':
	  strcpy(path, optarg);
	  break;
	case 'V':
	  cout << "extCal: $Id: extCal.cc,v 1.1 2007/02/10 02:25:26 davegutz Exp davegutz $" << endl
	       << "written by Dave Gutz Copyright (C) 2005"   << endl;
	  exit(0);
	  break;
	case 'v':
	  verbose = atoi(optarg);
	  if ( 0 > verbose )
	    {
	      cerr << "ERROR(ump):  " << __FILE__ << " : " << __LINE__<< " : "  
		   << "bad verbose specified\n";
	      exit(1);
	    }
#ifndef VERBOSE
	  if ( verbose ) cerr << "WARNING(ump) : recompile \'-DVERBOSE\' to "
			      << "get full verbose messages.\n" << flush;
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
  
  // Camera focal length in pixels (for 1/4" CCD, f=3.8 mm, w=3.63 mm so
  // if image width in pixels is 160 the focal_length=3.8/3.63*160=167.5 pix.

  // Read in world geometry from world.dat file.
  World *map = new World("world.dat");

  // Read in camera measurements from .cam and .dat files.
  MyCamData *caml = new MyCamData("video0.cam", "video0.dat", "world.dat");
  MyCamData *camr = new MyCamData("video1.cam", "video1.dat", "world.dat");

  // Check that files all opened
  int filesOK = 1;
  if ( !map->geoFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file world.dat" << endl;
      filesOK = 0;
    }      
  if ( !caml->camFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << caml->camFile() << endl;
      filesOK = 0;
    }      
  if ( !caml->datFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << caml->datFile() << endl;
      filesOK = 0;
    }      
  if ( !caml->geoFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << caml->geoFile() << endl;
      filesOK = 0;
    }      
  if ( !camr->camFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << camr->camFile() << endl;
      filesOK = 0;
    }      
  if ( !camr->datFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << camr->datFile() << endl;
      filesOK = 0;
    }      
  if ( !camr->geoFileExists() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << camr->geoFile() << endl;
      filesOK = 0;
    }      
  if ( !filesOK )
    {
      cerr << "ERROR(extCal):  quitting\n";
      exit (1);
    }

  // Correct the pixel measurements for aberration
  caml->initCorrectRx();
  camr->initCorrectRx();
  PixelLoc3 rawlA  = caml->OAP();
  PixelLoc3 rawrA  = camr->OAP();
  PixelLoc3 rawlB  = caml->OBP();
  PixelLoc3 rawrB  = camr->OBP();
  PixelLoc3 rawlC  = caml->OCP();
  PixelLoc3 rawrC  = camr->OCP();
  if ( verbose )
    {
      cout << "RAW:\n";
      cout << caml->OAP() << ", " << caml->OBP() << ", " << caml->OCP() << endl;
      cout << camr->OAP() << ", " << camr->OBP() << ", " << camr->OCP() << endl;
      cout << rawlA       << ", " << rawlB       << ", " << rawlC       << endl;
      cout << rawrA       << ", " << rawrB       << ", " << rawrC       << endl;
    }
  caml->correctRx();
  camr->correctRx();
  PixelLoc3 corrlA = caml->OAP();
  PixelLoc3 corrrA = camr->OAP();
  PixelLoc3 corrlB = caml->OBP();
  PixelLoc3 corrrB = camr->OBP();
  PixelLoc3 corrlC = caml->OCP();
  PixelLoc3 corrrC = camr->OCP();

  if ( verbose )
    {
      cout << "CORRECTED:\n";
      cout << caml->OAP() << ", " << caml->OBP() << ", " << caml->OCP() << endl;
      cout << camr->OAP() << ", " << camr->OBP() << ", " << camr->OCP() << endl;
      cout << corrlA      << ", " << corrlB      << ", " << corrlC      << endl;
      cout << corrrA      << ", " << corrrB      << ", " << corrrC      << endl;
    }

  if ( verbose )
    {
      cout << "RE-REGISTERED:\n";
      cout << caml->OAP() << ", " << caml->OBP() << ", " << caml->OCP() << endl;
      cout << camr->OAP() << ", " << camr->OBP() << ", " << camr->OCP() << endl;
      cout << "WORLD OBJECT LOCATIONS:\n";
      for (int i=0; i<3; i++) cout << map->OA()[i] << "   ";
      cout << endl;
      for (int i=0; i<3; i++) cout << map->OB()[i] << "   ";
      cout << endl;
      for (int i=0; i<3; i++) cout << map->OC()[i] << "   ";
      cout << endl;
      cout << endl;
      cout << *map << endl;
    }

  // Verification data
  PixelLoc3 Oalp, Oblp, Oclp, Oarp, Obrp, Ocrp;
  Oalp = caml->OAP();
  Oblp = caml->OBP();
  Oclp = caml->OCP();
  Oarp = camr->OAP();
  Obrp = camr->OBP();
  Ocrp = camr->OCP();

  // Vectors from cameras to objects in world coordinates
  PixelLoc3 OAl, OBl, OCl, OAr, OBr, OCr;

  // Difference between cameras
  Vector <float> dP(2);

  // Calibration
  // Translate measurement pixels to camera centered coordinates
  caml->center();
  camr->center();
  if ( verbose )
    {
      cout << "CENTERED:\n";
      cout << caml->OAP() << ", " << caml->OBP() << ", " << caml->OCP() << endl;
      cout << camr->OAP() << ", " << camr->OBP() << ", " << camr->OCP() << endl;
    }

  // Calculate other measurements
  dP[0] = map->Pl()[0] - map->Pr()[0];
  dP[1] = map->Pl()[1] - map->Pr()[1];
  OAl   = map->OA() - map->Pl();
  OBl   = map->OB() - map->Pl();
  OCl   = map->OC() - map->Pl();
  OAr   = map->OA() - map->Pr();
  OBr   = map->OB() - map->Pr();
  OCr   = map->OC() - map->Pr();

  if ( verbose ) 
    {
      cout << "OAl= " << OAl << endl;
      cout << "OBl= " << OBl << endl;
      cout << "OCl= " << OCl << endl;
      cout << "OAr= " << OAr << endl;
      cout << "OBr= " << OBr << endl;
      cout << "OCr= " << OCr << endl;
    }

  // Normalize measurements
  caml->norm();
  camr->norm();
  norm(OAl);
  norm(OBl);
  norm(OCl);
  norm(OAr);
  norm(OBr);
  norm(OCr);

  if ( verbose )
    {
      cout << "NORMED:\n";
      cout << caml->OAP() << ", " << caml->OBP() << ", " << caml->OCP() << endl;
      cout << camr->OAP() << ", " << camr->OBP() << ", " << camr->OCP() << endl;
    }
  if ( verbose ) 
    {
      cout << "OAl= " << OAl << endl;
      cout << "OBl= " << OBl << endl;
      cout << "OCl= " << OCl << endl;
      cout << "OAr= " << OAr << endl;
      cout << "OBr= " << OBr << endl;
      cout << "OCr= " << OCr << endl;
    }

  // Group measurements
  Matrix <float> Oxlp(3,3);
  Matrix <float> Oxrp(3,3);
  Matrix <float> Oxl (3,3);
  Matrix <float> Oxr (3,3);
  for (int j=0; j<3; j++)
    {
      Oxlp[j][0] = caml->OAP()[j];
      Oxlp[j][1] = caml->OBP()[j];
      Oxlp[j][2] = caml->OCP()[j];
      Oxrp[j][0] = camr->OAP()[j];
      Oxrp[j][1] = camr->OBP()[j];
      Oxrp[j][2] = camr->OCP()[j];
      Oxl[j][0]  = OAl[j];
      Oxl[j][1]  = OBl[j];
      Oxl[j][2]  = OCl[j];
      Oxr[j][0]  = OAr[j];
      Oxr[j][1]  = OBr[j];
      Oxr[j][2]  = OCr[j];
    }
  
  // Calculate rotation matrix using library functions:
  // Solve Equation ACPW*Oxp=Ox where A is unknown 3x3, Oxp is pixel vectors
  // arranged in columns for 3x3 matrix and Ox is object vector arranged
  // in columns for 3x3 matrix.  Transpose the equation so that
  // (Oxp'*ACPW')=Ox' then split Ox and ACPW to solve separately for columns
  // of ACPW.
  Matrix <float> OxlpM(3,3);
  Matrix <float> OxrpM(3,3); OxrpM = transpose(Oxrp);
  // Transpose implicit in following assignments
  Vector <float> Oxl1 (3);
  Vector <float> Oxl2 (3);
  Vector <float> Oxl3 (3);
  Vector <float> Oxr1 (3);
  Vector <float> Oxr2 (3);
  Vector <float> Oxr3 (3);
  for (int i=0; i<3; i++)
    {
      Oxl1[i] = Oxl[0][i];
      Oxl2[i] = Oxl[1][i];
      Oxl3[i] = Oxl[2][i];
      Oxr1[i] = Oxr[0][i];
      Oxr2[i] = Oxr[1][i];
      Oxr3[i] = Oxr[2][i];
    }
  Matrix <float> ACPWl(3,3);
  Matrix <float> ACPWr(3,3);
  Matrix <float> Vl(3,3);
  Matrix <float> Vr(3,3);
  Matrix <float> Tl(3,3);
  Matrix <float> Tr(3,3);
  Matrix <float> ATl(3,3);
  Matrix <float> ATr(3,3);
  for (int j=0; j<3; j++)
    {
      Vl[0][j] = caml->OAP()[j];
      Vl[1][j] = caml->OBP()[j];
      Vl[2][j] = caml->OCP()[j];
      Vr[0][j] = camr->OAP()[j];
      Vr[1][j] = camr->OBP()[j];
      Vr[2][j] = camr->OCP()[j];
      Tl[0][j] = OAl[j];
      Tl[1][j] = OBl[j];
      Tl[2][j] = OCl[j];
      Tr[0][j] = OAr[j];
      Tr[1][j] = OBr[j];
      Tr[2][j] = OCr[j];
    }
  ATl = inv(Vl) * Tl;
  ATr = inv(Vr) * Tr;
  if ( verbose )
    {
      cout << "Vl="  << Vl  << "Vr="  << Vr  << endl 
	   << "Tl="  << Tl  << "Tr="  << Tr  << endl
	   << "ATl=" << ATl << "ATr=" << ATr << endl;
    }
  ACPWl = transpose(ATl);
  ACPWr = transpose(ATr);
  normM(ACPWl);
  normM(ACPWr);

  *(caml->P())    = (map->Pl());
  *(caml->ACPW()) = (ACPWl);
  *(camr->P())    = (map->Pr());
  *(camr->ACPW()) = (ACPWr);

  if ( verbose )
    {
      cout << "CALIBRATION RESULTS:\n";
      cout << "PL=" << map->Pl() << endl << "PR=" << map->Pr() << endl;
      cout << "ACPWl=" << ACPWl << "ACPWr=" << ACPWr;
    }

  // Write cal result files
  char calFile[NAME_MAX];
  strcpy(calFile, "video0.cal");
  outf.open(calFile, ios::out);
  if ( outf.fail() )
    {
      cerr << "extCal: " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << calFile << endl;
      return(1);
    }
  outf << *caml;
  if ( outf.fail() )
    {
      cerr << "ERROR(extCal): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't write file " << calFile << endl;
      exit(1);
    }
  else
    cout << "MESSAGE(extCal): Wrote " << calFile << ".\n";
  outf.close();
  strcpy(calFile, "video1.cal");
  outf.open(calFile, ios::out);
  if ( outf.fail() )
    {
      cerr << "ERROR(extCal): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't open file " << calFile << endl;
      return(1);
    }
  outf << *camr;
  if ( outf.fail() )
    {
      cerr << "ERROR(extCal): " << __FILE__ << " : " << __LINE__ << " : "  
	   << "couldn't write file " << calFile << endl;
      exit(1);
    }
  else
    cout << "MESSAGE(extCal): Wrote " << calFile << ".\n";
  outf.close();



  // Sample Calculation and check
  if ( verbose )
    cout << "\n\nVERIFY:\n";

  Matrix <float> Oelp(3,3);
  Matrix <float> Oerp(3,3);
  Matrix <float> OelT(3,3);
  Matrix <float> OerT(3,3);
  Matrix <float> vT(2,2);
  Vector <float> RL(2);
  Matrix <float> Oel(3,3);
  Matrix <float> Oer(3,3);

  // Bias to center
  if ( verbose )
    {
      cout << "Corrected: " << endl;
      cout << "O0p=" << Oalp << ", " << Oblp << ", " << Oclp << endl;
      cout << "O1p=" << Oarp << ", " << Obrp << ", " << Ocrp << endl;
    }
  center(Oalp, caml->imWidthCalEx(), caml->imHeightCalEx(), caml->ARC());
  center(Oblp, caml->imWidthCalEx(), caml->imHeightCalEx(), caml->ARC());
  center(Oclp, caml->imWidthCalEx(), caml->imHeightCalEx(), caml->ARC());
  center(Oarp, camr->imWidthCalEx(), camr->imHeightCalEx(), camr->ARC());
  center(Obrp, camr->imWidthCalEx(), camr->imHeightCalEx(), camr->ARC());
  center(Ocrp, camr->imWidthCalEx(), camr->imHeightCalEx(), camr->ARC());
  if ( verbose )
    {
      cout << "Centered: " << endl;
      cout << "O0p=" << Oalp << ", " << Oblp << ", " << Oclp << endl;
      cout << "O1p=" << Oarp << ", " << Obrp << ", " << Ocrp << endl;
    }

  // Normalize
  norm(Oalp);
  norm(Oblp);
  norm(Oclp);
  norm(Oarp);
  norm(Obrp);
  norm(Ocrp);

  if ( verbose )
    {
      cout << "Normed: " << endl;
      cout << "O0p=" << Oalp << ", " << Oblp << ", " << Oclp << endl;
      cout << "O1p=" << Oarp << ", " << Obrp << ", " << Ocrp << endl;
    }

  // Group
  for (int j=0; j<3; j++)
    {
      Oelp[j][0] = Oalp[j];
      Oelp[j][1] = Oblp[j];
      Oelp[j][2] = Oclp[j];
      Oerp[j][0] = Oarp[j];
      Oerp[j][1] = Obrp[j];
      Oerp[j][2] = Ocrp[j];
    }
  OelT = ACPWl*Oelp;
  OerT = ACPWr*Oerp;
  for (int i=0; i<3; i++)
    {
      dP[0] = map->Pl()[0] - map->Pr()[0];
      dP[1] = map->Pl()[1] - map->Pr()[1];
      for (int j=0; j<2; j++)
	{
	  vT[j][0] =  OerT[j][i];
	  vT[j][1] = -OelT[j][i];
	}
      
      // Use library to calculate
      // Solve the equation vT*RL=dP where vT is 2x2 matrix of the first two
      // elements of predicted left and right direction vectors in world space,
      // dP is 1x2 matrix of the first two elements of left-right postions of
      // camera lens, and RL is to be calculated as the right and left camera
      // scalars on direction vectors to produce an intersection at the real
      // world object.  The problem is inherently over-specified with 4
      // coordinates (two directions in each direction vector ) predicting a 
      // 3 coordinate point in space.  This is used to advantage because lines
      // usually don't intersect allowing calculation of zone.  Assume average
      // of solutions is center of zone. Discard solutions whose determinant 
      // is small which means the projections of the two camera rays in that 
      // plane are nearly parallel.

      PixelLoc3 location;
      PixelLoc3 uncert;
      int combos[3][2] = { { 0, 1 },
			   { 0, 2 },
			   { 1, 2 } };
      float weight[3]  = { 0.,
			   0.,
			   0. }; // Weight to apply to each combo

      // Loop through all three combinations and average
      for ( int ii=0; ii<3; ii++ )
	{
	  int I = combos[ii][0];
	  int J = combos[ii][1];


	  // Difference between cameras
	  dP[0] = map->Pl()[I] - map->Pr()[I];
	  dP[1] = map->Pl()[J] - map->Pr()[J];

	  vT[0][0] =  OerT[I][i];
	  vT[0][1] = -OelT[I][i];
	  vT[1][0] =  OerT[J][i];
	  vT[1][1] = -OelT[J][i];
	  if ( verbose ) cout << "vT=" << vT;


	  // Predict location and add to weighted average subtotal
	  if ( 0.00001 < std::abs(dP[0]) || 0.00001 < std::abs(dP[1]) ) 
	    {
	      Vector <Subscript> indexT;
	      RL = dP;
	      LU_factor(vT, indexT);
	      LU_solve (vT, indexT, RL);
	      
	      weight[I] += 1.;
	      weight[J] += 1.;
	      float locI = map->Pl()[I] + RL[1]*OelT[I][i];
	      float locJ = map->Pl()[J] + RL[1]*OelT[J][i];
	      location[I] += locI;
	      location[J] += locJ;
	      if ( verbose )
		{
		  cout << "RL=" << RL << endl;
		  cout << "location(" 
		       << I << "," << J << ")=" << locI << ", " << locJ << endl;
		}
	      if ( 0 == uncert[I] ) uncert[I]  = locI;
	      else                  uncert[I] -= locI;
	      if ( 0 == uncert[J] ) uncert[J]  = locJ;
	      else                  uncert[J] -= locJ;
	    }
	  else
	    {
	      if ( verbose )
		cout << "Dropping indeterminant solution.\n" <<
		  "Cameras in line in this plane.\n";
	    }
	}
      // Weighted average
      for (int ii=0; ii<3; ii++)
	{
	  if ( 0.0 < weight[ii] )
	    location[ii] /= weight[ii];
	  else 
	    location[ii] = 0.0;
	  if ( 1.0 == weight[ii] ) 
	    uncert[ii] = 0.0;
	  else
	    uncert[ii] /= weight[ii];
	}

      cout << "\nMESSAGE(extCal):  Calculated location       = [";
      for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				   << location[k];
      cout << "]\n";
      cout << "MESSAGE(extCal):  Expected location         = [";
      if ( 0 == i )
	for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				     << map->OA()[k];
      if ( 1 == i )
	for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				     << map->OB()[k];
      if ( 2 == i )
	for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				     << map->OC()[k];
      cout << "]\n";
      cout << "MESSAGE(extCal):  Error                     = [";
      if ( 0 == i )
	for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				     << location[k]-map->OA()[k];
      if ( 1 == i )
	for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				     << location[k]-map->OB()[k];
      if ( 2 == i )
	for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				     << location[k]-map->OC()[k];
      cout << "]\n";
      cout << "MESSAGE(extCal):  Uncertainty (world units) = [";
      for (int k=0; k<3; k++) cout << setprecision(4) << setw(11) 
				   << uncert[k];
      cout << "]\n";
      
      // Check against triangulate
      char cmd[MAX_CANON];

      sprintf(cmd, "rm -f triangulater.out");
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(extCal) :  exec of \'" << cmd << "\' failed\n" << flush;
      
      if ( 0 == i )
	sprintf(cmd,
      "%striangulate -s%dx%d -r %9.5f %9.5f %9.5f %9.5f -v%d >triangulater.out",
		path, caml->imWidthCalEx(), caml->imHeightCalEx(),
		rawlA[0], rawlA[1], rawrA[0], rawrA[1], verbose);
      else if ( 1 == i )
	sprintf(cmd,
      "%striangulate -s%dx%d -r %9.5f %9.5f %9.5f %9.5f -v%d >triangulater.out",
		path, caml->imWidthCalEx(), caml->imHeightCalEx(),
		rawlB[0], rawlB[1], rawrB[0], rawrB[1], verbose);
      else if ( 2 == i )
	sprintf(cmd, 
      "%striangulate -s%dx%d -r %9.5f %9.5f %9.5f %9.5f -v%d >triangulater.out",
		path, caml->imWidthCalEx(), caml->imHeightCalEx(),
		rawlC[0], rawlC[1], rawrC[0], rawrC[1], verbose);
      
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(extCal) :  exec of \'" << cmd << "\' failed\n" << flush;

      if ( 1 < verbose )
	{
	  cout << "MESSAGE(ump):  path=" << path << endl << flush;
	}
      
      sprintf(cmd, "rm -f triangulatec.out");
      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(extCal) :  exec of \'" << cmd << "\' failed\n" << flush;
      
      if ( 0 == i )
	sprintf(cmd,
        "%striangulate -s%dx%d %9.5f %9.5f %9.5f %9.5f -v%d >triangulatec.out",
		path, caml->imWidthCalEx(), caml->imHeightCalEx(),
		corrlA[0], corrlA[1], corrrA[0], corrrA[1], verbose);
      else if ( 1 == i )
	sprintf(cmd,
        "%striangulate -s%dx%d %9.5f %9.5f %9.5f %9.5f -v%d >triangulatec.out",
		path, caml->imWidthCalEx(), caml->imHeightCalEx(),
		corrlB[0], corrlB[1], corrrB[0], corrrB[1], verbose);
      else if ( 2 == i )
	sprintf(cmd,
	"%striangulate -s%dx%d %9.5f %9.5f %9.5f %9.5f -v%d >triangulatec.out",
		path, caml->imWidthCalEx(), caml->imHeightCalEx(),
		corrlC[0], corrlC[1], corrrC[0], corrrC[1], verbose);


      if ( 0 != Execute(cmd, 0) )
	cerr << "ERROR(extCal) :  exec of \'" << cmd << "\' failed\n" << flush;
      
      // Check raw result
      InFile *ptrTriOutr = new InFile("triResultr", "triangulater.out",
				      128, MAX_CANON);
      ptrTriOutr->load();
      ptrTriOutr->stripComments("#");
      ptrTriOutr->stripComments("%");
      ptrTriOutr->stripBlankLines();
      if ( 0 == ptrTriOutr->loadVars() )
	{
	  if ( verbose ) 
	    {
	      cerr << "WARNING(extCal): " <<__FILE__<<" : "<<__LINE__<<" : " 
		   << "No triangulater.out file or  empty.\n";
	    }
	}
      else
	{
	  int numVal;
	  PixelLoc3 triResultr;
	  PixelLoc3 triUncertr;
	  numVal = ptrTriOutr->getPar("TRIPOSITION", triResultr);
	  if ( 1 != numVal )
	    {
	      cerr << "WARNING(extCal): "<< __FILE__<<" : "<< __LINE__ << " : " 
		   << "Bad position data in triangulater.out file.\n";
	    }
	  else
	    {
	      numVal = ptrTriOutr->getPar("UNCERTAINTY", triUncertr);
	      if ( 1 != numVal )
		{
		  cerr << "WARNING(extCal): "<<__FILE__<<" : "<<__LINE__<<" : " 
		       << "Bad uncertainty data in triangulater.out file.\n";
		}
	      else
		{
		  cout << "MESSAGE(extCal):  Calculated Triangulate    = [";
		  for (int k=0; k<3; k++)
		    {
		      cout << setprecision(4) << setw(11) << triResultr[k];
		    }
		  cout << "]\n";
		  cout << "MESSAGE(extCal):  Tri Disagree              = [";
		  for (int k=0; k<3; k++)
		    {
		      float err = triResultr[k]-location[k];
		      cout << setprecision(4) << setw(11) << err;
		      if ( std::abs(err) > 1e-3 ) badCompare = 1;
		    }
		  cout << "]\n";
		  cout << "MESSAGE(extCal):  Tri Uncert  (world units) = [";
		  for (int k=0; k<3; k++)
		    {
		      cout << setprecision(4) << setw(11) << triUncertr[k];
		      if ( triUncertr[k] > 1e-3 ) badCompare = 1;
		    }
		  cout << "]\n";
		}
	    }
	}
      zap(ptrTriOutr);

      // Check corr result
      InFile *ptrTriOutc = new InFile("triResultc",
				      "triangulatec.out", 128, MAX_CANON);
      ptrTriOutc->load();
      ptrTriOutc->stripComments("#");
      ptrTriOutc->stripComments("%");
      ptrTriOutc->stripBlankLines();
      if ( 0 == ptrTriOutc->loadVars() )
	{
	  if ( verbose ) 
	    {
	      cerr << "WARNING(extCal): " << __FILE__ << " : " << __LINE__ << " : " 
		   << "No triangulatec.out file or  empty.\n";
	    }
	}
      else
	{
	  int numVal;
	  PixelLoc3 triResultc;
	  PixelLoc3 triUncertc;
	  numVal = ptrTriOutc->getPar("TRIPOSITION", triResultc);
	  if ( 1 != numVal )
	    {
	      cerr << "WARNING(extCal): " << __FILE__ << " : "<<__LINE__<<" : " 
		   << "Bad position data in triangulatec.out file.\n";
	    }
	  else
	    {
	      numVal = ptrTriOutc->getPar("UNCERTAINTY", triUncertc);
	      if ( 1 != numVal )
		{
		  cerr << "WARNING(extCal): "<<__FILE__<<" : "<<__LINE__<<" : " 
		       << "Bad uncertainty data in triangulatec.out file.\n";
		}
	      else
		{


		  cout << "MESSAGE(extCal):  Calc Triangulate Corrected= [";
		  for (int k=0; k<3; k++)
		    {
		      cout << setprecision(4) << setw(11) << triResultc[k];
		    }
		  cout << "]\n";
		  cout << "MESSAGE(extCal):  Tri Corrected Disagree    = [";
		  for (int k=0; k<3; k++)
		    {
		      float err = triResultc[k]-location[k];
		      cout << setprecision(4) << setw(11) << err;
		      if ( std::abs(err) > 1e-3 ) badCompare = 1;
		    }
		  cout << "]\n";
		  cout << "MESSAGE(extCal):  Tri Corrected Uncert      = [";
		  for (int k=0; k<3; k++)
		    {
		      cout << setprecision(4) << setw(11) << triUncertc[k];
		      if ( triUncertc[k] > 1e-3 ) badCompare = 1;
		    }
		  cout << "]\n";
		}
	    }
	}
      zap(ptrTriOutc);
    }
  
  zap(map);
  zap(caml);
  zap(camr);

  if ( badCompare )
    {
      cerr << "ERROR(extCal): triangulation results not consistent\n" << flush;
      exit(-1);
    }
}


