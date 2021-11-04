// 	$Id: MyMask.cc,v 1.1 2007/12/16 21:38:07 davegutz Exp davegutz $	
// Extension of Mask for glade use 

// Includes section
using namespace std;
#include <common.h>
#include <string>      // string classes
#include <string.h>    // strings
#include <iostream>    // cerr etc
#include <iomanip>     // setprecision etc
#include <zap.h>
#include "MyMask.h"

extern int verbose;

MyMask::MyMask()
  : Mask(), picture_(NULL), picture1_(NULL), picture2_(NULL),
    image_(NULL), imageZ_(0), imageR_(0), n_(0), m_(0),
    bufsize_(0), sub_(1), useDefault_(0), useP5_(0), useP6_(0) {}
//MyMask::MyMask(const String name, int n, int m)
MyMask::MyMask(const string name, int n, int m)
  : Mask(name, n, m, 1000), picture_(NULL), picture1_(NULL),
    picture2_(NULL), image_(NULL), imageZ_(0), imageR_(0), n_(n),
    m_(m), bufsize_(n*m), sub_(1), useDefault_(0), useP5_(0),
    useP6_(0)
{
  // Load masks
  if ( verbose ) cout << "MyMask::MyMask : " << flush;
  loadMasksFrom(name_.data());
  
  // Check the file
  if ( MAXMASK < maskData_->numLines() )
    {
      cout << "WARNING(Blink) : MyMask : " << __FILE__
	   << " : " << __LINE__ << " : "  
	   << "more than " << MAXMASK << " masks in file\n"
	   << name_ << "." << endl;
    }
}
MyMask::MyMask(const MyMask & M)
  : picture_(NULL), picture1_(NULL), picture2_(NULL),
    image_(NULL), imageZ_(M.imageZ_), imageR_(M.imageR_), n_(M.n_), m_(M.m_),
    bufsize_(M.bufsize_), sub_(M.sub_), useDefault_(M.useDefault_),
    useP5_(M.useP5_), useP6_(M.useP6_)
{
  if ( useP5_ && !useDefault_ )  // Gray format
    {
      picture_ = new Pixel*[bufsize_];
      image_   = new unsigned char [bufsize_];
      for( int i=0; i<bufsize_; i++) image_[i] = M.image_[i];
    }
  else if ( useP6_ && !useDefault_ )  // color format
    {
      picture_  = new Pixel*[bufsize_];
      picture1_ = new Pixel*[bufsize_];
      picture2_ = new Pixel*[bufsize_];
      image_    = new unsigned char [bufsize_*3];
      for( int i=0; i<bufsize_*3; i++) image_[i] = M.image_[i];
    }
  else  // unknown image.  Use a blank
    {
      picture_ = new Pixel*[bufsize_];
      image_   = new unsigned char [bufsize_];
      cerr << "WARNING(MyMask::MyMask): "<< __FILE__ << " : "
	   << __LINE__  << " : "  << "using default mask."
	   << endl << flush;
    }
  int j=0, i=0, index=0, indexj=0;
  for( j=0; j<m_; j++)
    {
      indexj = j*n_;
      for( i=0; i<n_; i++)
	{
	  index = indexj+i;
	  if ( useP6_ )
	    {
	      picture_[index]   = new Pixel(*M.picture_[index]);
	      picture1_[index]  = new Pixel(*M.picture1_[index]);
	      picture2_[index]  = new Pixel(*M.picture2_[index]);
	    }
	  else
	    {
	      picture_[index]   = new Pixel(*M.picture_[index]);
	    }
	}
    }
}
MyMask & MyMask::operator=(const MyMask & M)
{
  if ( this == &M ) return *this;
  imageZ_     = M.imageZ_;
  imageR_     = M.imageR_;
  n_          = M.n_;
  m_          = M.m_;
  bufsize_    = M.bufsize_;
  sub_        = M.sub_;
  useDefault_ = M.useDefault_;
  useP5_      = M.useP5_;
  useP6_      = M.useP6_;
  if ( useP5_ && !useDefault_ )  // Gray format
    {
      picture_ = new Pixel*[bufsize_];
      image_   = new unsigned char [bufsize_];
      for( int i=0; i<bufsize_; i++) image_[i] = M.image_[i];
    }
  else if ( useP6_ && !useDefault_ )  // color format
    {
      picture_  = new Pixel*[bufsize_];
      picture1_ = new Pixel*[bufsize_];
      picture2_ = new Pixel*[bufsize_];
      image_    = new unsigned char [bufsize_*3];
      for( int i=0; i<bufsize_*3; i++) image_[i] = M.image_[i];
    }
  else  // unknown image.  Use a blank
    {
      picture_ = new Pixel*[bufsize_];
      image_   = new unsigned char [bufsize_];
      cerr << "WARNING(MyMask::MyMask): "<< __FILE__ << " : "
	   << __LINE__  << " : "  << "using default mask."
	   << endl << flush;
    }
  int j=0, i=0, index=0, indexj=0;
  for( j=0; j<m_; j++)
    {
      indexj = j*n_;
      for( i=0; i<n_; i++)
	{
	  index = indexj+i;
	  if ( useP6_ )
	    {
	      picture_[index]   = new Pixel(*M.picture_[index]);
	      picture1_[index]  = new Pixel(*M.picture1_[index]);
	      picture2_[index]  = new Pixel(*M.picture2_[index]);
	    }
	  else
	    {
	      picture_[index]   = new Pixel(*M.picture_[index]);
	    }
	}
    }
  return *this;   // Allow stacked assignments
}

// Write mask image to file
//int MyMask::writeMaskImage(const String snapName, const String name)
int MyMask::writeMaskImage(const string snapName, const string name)
{
  void assignRGB(const int request, int *RED, int *GREEN, int *BLUE);
  if ( verbose ) cout << "writeMaskImage " << name << endl << flush;
  if ( 4 < verbose ) cout << "snapName=" << snapName << endl << flush;
  
  FILE *sfp;
  int i, j, index, indexj;

  // Read file header and determine source image type
  if ( 0 == (sfp = fopen(snapName.data(), "r")) )
    {
      cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : " << __LINE__ 
	   << " : "  << "cannot open " << snapName.data()
	   << " for reading\n" << flush;
      useDefault_ = 1;
    }
  else
    {
      // Verify image size and strip the header.
      char *pstr = new char[2];
      int  nsnap = 0, msnap = 0, isat = 0;
      if ( 4 != fscanf(sfp,  "%2c %d %d %d ", pstr, &nsnap, &msnap, &isat))
	{
	  cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : " << __LINE__ 
		 << " : "  << "cannot read " << snapName.data() << endl;
	  useDefault_ = 1;
	}
      if ( nsnap != n_ || msnap != m_ )
	{
	  cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : " << __LINE__ 
	       << " : "  << "wrong size image " << nsnap << "x" << msnap 
	       << " in" << snapName.data() << endl << flush;
	  useDefault_ = 1;
	}
      if ( pstr[0] == 'P' )
	{
	  if      ( pstr[1] == '5' ) useP5_ = 1;
	  else if ( pstr[1] == '6' ) useP6_ = 1;
	  else
	    {
	      cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : " 
		   << __LINE__  << " : "  << "bad image type header " << pstr
		   << " in" << snapName.data() << endl << flush;
	      useDefault_ = 1;
	    }
	}
      else
	{
	      cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : " 
		   << __LINE__  << " : "  << "bad image type header " << pstr
		   << " in" << snapName.data() << endl << flush;
	      useDefault_ = 1;
	}
      zaparr(pstr);
    }

  // Initialize
  if ( useP5_ && !useDefault_ )  // Gray format
    {
      picture_ = new Pixel*[bufsize_];
      image_   = new unsigned char [bufsize_];
    }
  else if ( useP6_ && !useDefault_ )  // color format
    {
      picture_  = new Pixel*[bufsize_];
      picture1_ = new Pixel*[bufsize_];
      picture2_ = new Pixel*[bufsize_];
      image_    = new unsigned char [bufsize_*3];
    }
  else  // unknown image.  Use a blank
    {
      picture_ = new Pixel*[bufsize_];
      image_   = new unsigned char [bufsize_];
      cerr << "WARNING(writeMaskImage): "<< __FILE__ << " : "
	   << __LINE__  << " : "  << "using default mask."
	   << endl << flush;
    }


  // Load source image
  if ( useP5_ && !useDefault_ )
    {
      // Load image into buffer
      if (verbose ) cerr << "MESSAGE(writeMaskImage):  loading "
			 << snapName.data() << endl;
      int val;
      for( int i=0; i<bufsize_; i++)
	{
	  if ( 0 > (val = fgetc(sfp)) )
	    {
	      cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : "
		   << __LINE__  << " : "  << "trouble reading "
		   << snapName.data() << " at position " << i+1 << " of " 
		   << bufsize_ << endl << flush;
	      memset(image_,  127, bufsize_*sizeof(image_[0]));
	      break;
	    }
	  image_[i] = (unsigned char) val;
	}
    }
  else if ( useP6_ && !useDefault_ )
    {
      // Load image into buffer
      if (verbose ) cerr << "MESSAGE(writeMaskImage):  loading "
			 << snapName.data() << endl;
      int val;
      for( int i=0; i<bufsize_*3; i++)
	{
	  if ( 0 > (val = fgetc(sfp)) )
	    {
	      cerr << "MESSAGE(writeMaskImage): "<< __FILE__ << " : "
		   << __LINE__  << " : "  << "trouble reading "
		   << snapName.data() << " at position " << i+1 << " of " 
		   << bufsize_ << endl << flush;
	      memset(image_,  127, bufsize_*3*sizeof(image_[0]));
	      break;
	    }
	  image_[i] = (unsigned char) val;
	}
    }
  else  // unknown image.  Use a neutral color blank
    {
      memset(image_,  127, bufsize_*sizeof(image_[0]));
    }

  // Process mask onto image
  for( j=0; j<m_; j++)
    {
      indexj = j*n_;
      for( i=0; i<n_; i++)
	{
	  index = indexj+i;
	  if ( useP6_ )
	    {
	      picture_[index]   = new Pixel(&(image_[3*index+0]), &imageR_,
				       &imageZ_, i, j);
	      picture1_[index]  = new Pixel(&(image_[3*index+1]), &imageR_,
					      &imageZ_, i, j);
	      picture2_[index]  = new Pixel(&(image_[3*index+2]), &imageR_,
					      &imageZ_, i, j);
	    }
	  else
	    {
	      picture_[index]  = new Pixel(&(image_[index+0]), &imageR_,
					   &imageZ_, i, j);
	    }
	}
    }
  applyMasksTo(picture_, n_, m_, sub_);
  if ( useP6_ )
    {
      applyMasksTo(picture1_, n_, m_, sub_);
      applyMasksTo(picture2_, n_, m_, sub_);
    }
  

  // Write file
  FILE *fp;
  if ( NULL == (fp = fopen(name.data(), "wb")) )
    {
      cout << "WARNING(Blink) : MyMask::writeMaskImage : " << __FILE__
	   << " : " << __LINE__ << " : "  << " Cannot open " << name
	   << " for writing\n"
	   << flush;
      //TODO  memory leak with these returns.  need to cleanup
      return(-1);
    }
  
  // Write header
  fprintf(fp, "P6\n");
  fprintf(fp, "%d %d\n", n_,    m_   );
  fprintf(fp, "255\n");
  
  // Assign data to file image
  unsigned char *img; 
  int imgBufsize = n_*m_*3;
  img = new unsigned char[imgBufsize];
  memset(img, 0, imgBufsize);
  for( int i = 0; i < numMaskVal_; i++)
    {
      int index    = maskVal_[i];
      int indexRGB = 3*index;
      if ( indexRGB+2 >= imgBufsize )
	{
	    cerr << "ERROR(Image):  " << __FILE__ << " : " << __LINE__ << " : "
		 << "Value of mask[" << i << "]=" << index
		 << " is too long compared to length of image ="
		 << n_*m_ << endl << flush;
	    return(-1);
	}
      img[indexRGB] = *picture_[index]->valp;
      if ( useP6_ )
	{
	  img[1+indexRGB] = *picture1_[index]->valp;
	  img[2+indexRGB] = *picture2_[index]->valp;
	}
      else
	{
	  img[1+indexRGB] = img[indexRGB];
	  img[2+indexRGB] = img[indexRGB];
	}
    }
  
  // Assign edges (now it doesn't matter what image type)
  int RED, GREEN, BLUE;
  assignRGB(4, &RED, &GREEN, &BLUE);
  for( int i=0; i<numMaskVal_; i++)
    {
      int index    = maskVal_[i];
      int indexRGB = 3*index;
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
      if ( picture_[index]->onEdge )
	{
	  img[      indexRGB] = RED;
	  img[  1 + indexRGB] = GREEN;
	  img[  2 + indexRGB] = BLUE;
	}
    }
  
  // Write file
  int n = n_, m = m_;
  unsigned char *imgp = img;
  for( int j = 0 ; j < m ; j++)
    {
      fwrite(imgp, n, 3, fp);
      imgp += 3*n;
    }
  
  // Cleanup
  fclose(fp);
  if ( verbose ) cout << "MESSAGE(Blink): writeMaskImage : wrote mask image "
		      << name << ".\n";
  
  delete []img;
  for( int j=0; j<bufsize_; j++) delete picture_[j];
  delete []picture_;
  if ( useP6_ )
    {
      for( int j=0; j<bufsize_; j++)
	{
	  delete picture1_[j];
	  delete picture2_[j];
	}
      delete []picture1_;
      delete []picture2_;
    }
  delete []image_;
  return(0);
}   
