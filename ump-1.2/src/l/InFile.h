// Test file open and string capabilities.
/* 	$Id: InFile.h,v 1.1 2007/12/16 21:39:31 davegutz Exp davegutz $	 */
#ifndef InFile_h
#define InFile_h 1
using namespace std;
#include <common.h>
#include <stdio.h>
#include <math.h>       // For fmod
#ifdef BACKWARD
#  include <fstream.h>    // cerr etc
#  include <iomanip.h>    // setprecision etc
#else
#  include <fstream>      // cerr etc
#  include <iomanip>      // setprecision etc
#endif
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>  // For pipe
#include <unistd.h>     // For fork and dup2 and close
#include <sys/param.h>  // access built-in constants and MIN/MAX
#ifdef UMP
 #include "second.h"
 #include "../PixelLoc.h"// PixelLoc3 class
#endif
#include <stringset.h>
#include <list>
//#include <String.h>
#include <zap.h>
#include <vector>
#include <algorithm>
#ifdef UMP
extern "C"
{
 #include "makeargv.h"
}
#else
 #include "makeargv.h"
#endif

class Coord
{
 public:
  int row;
  int pos;
  Coord() : row(0), pos(0) {}
  Coord(const int rowIn, const int posIn)
    : row(rowIn), pos(posIn) {}
  Coord(const Coord & C) : row(C.row), pos(C.pos) {}
  Coord & operator=(const Coord & C)
    {
      if ( this == &C ) return *this;
      row = C.row;
      pos = C.pos;
      return *this;
    }
  virtual ~Coord() {}
};

class Datax
{
 public:
  string         name;
  int            length;
  vector <Coord> locV;
  Datax() : name(""), length(0), locV(vector <Coord> ()) {}
  Datax(const string nam, const int len)
    : name(nam), length(len), locV(vector <Coord> ()) {}
  Datax(const Datax & D) : name(D.name), length(D.length), locV(D.locV) {}
  Datax & operator=(const Datax & D)
    {
      if ( this == &D ) return *this;
      name   = D.name;
      length = D.length;
      locV   = D.locV;
      return *this;
    }
  virtual ~Datax() {}
};

class InFile
{
 public:
  InFile();
  InFile(const char pname[], const char ifile[],
	 const long maxfilelines, const int maxline);
  InFile(InFile &IF);
  InFile & operator=(InFile & IF);
  ~InFile();

  // element access
  inline string  FileExt()                {return fileextension_;};
  //  inline String  FileRoot()               {return fileroot_;};
  //String Line(const int i);
  inline string  FileRoot()               {return fileroot_;};
  string Line(const int i);
  int            numLines();
  int            maxLineLength();
  inline string  name()                   {return inFile_;};
  inline string  nameStr()                {return string(inFile_);};
  stringset      LineS(const int i);
  stringset     *LineSptr(const int i);
  string         token(const int i, const int j);

  // operators
  friend ostream & operator<< (ostream & out, InFile & InFile);

  // functions
  void shortenDelim(const int L, int i);   // Delete last char of i'th
  // delim of line L
  int  loadVars();
  void deleteLine(const int L);
  void addLine(const int L, const string S);
  void downcase();
  void downcase(int startline, int endline);
  int  findStr(string target, int startline);
  int  isAlpha(string strData);
  int  isNum(string strData);
  int  load();
  int  load(int skips, int after, int includeLast=1);
  int  getPar(string target, float *par);
  int  getPar(string target, int   *par);
#ifdef UMP
  int  getPar(string target, PixelLoc3 &par);
  int  getPar(string target, PixelLoc3 &par, int nP);
  int  getPar(string target, Vector <float> &V);
  int  getPar(string target, Matrix <float> &M);
#endif
  int  gsub(string *expr, const string target, const string replace);
  int  gsub(list<string>::iterator & iExpr,
	    const string target, const string replace);
  int  gsub(const string target, const string replace);
  int  gsub(const string target, const string replace,
	   int startline, int endline);
  int  gsubDelims(const string target, const string replace);
  void reconstruct();
  void sort();
  int  stripBlankLines();
  int  stripComments(const string &comment_delim);
  int  stripComments(const string &comment_delim, int startline, int endline);
  char *sureShort(const string source);
  int  tokenize(const string &token_delims);
  void upcase();
  void upcase(int startline, int endline);

protected:
  vector<Datax>   varVX_;
  list<string>    vA_;
  list<stringset> vS_;
  string        token_delims_;
  string        fileextension_;
  //String        fileroot_;
  string        fileroot_;
  string        inFile_;
  int           maxfilelines_;
  int           maxline_;
  int           maxLineLength_;
  int           num_lines_;
  int           nvars_;             // Number of variable data
  string        programName_;
  int           reconstructed_;     // Show original delims
  int           tokenized_;         // Lines tokenized
  int           counted_;
  inline string getLine(string &inbuf, ifstream &inf);
#ifdef BACKWARD
  inline string getLine(string &inbuf, _IO_istream_withassign &inf);
#else
  inline string getLine(string &inbuf, istream &inf);
#endif //BACKWARD  
  void          swap(void *v[], int i, int j);
};

#endif // InFile_h
