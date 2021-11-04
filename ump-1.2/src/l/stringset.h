// Define stringset input class.
/* 	$Id: stringset.h,v 1.2 2010/01/10 12:10:55 davegutz Exp $	 */
#ifndef stringset_h
#define stringset_h 1
#include <stdlib.h>
#ifdef BACKWARD
#  include <iostream.h>
#else
#  include <iostream>
#endif
#include <string>
#include <algorithm>
#include <vector>
#include <zap.h>
using namespace std;

//////////////////////////class stringset///////////////////////////////
class stringset
{
 public:

  // Constructors
  stringset();
  stringset(const string& str, const string& delimiters = "\0");
  stringset(const int maxsize, const string instring='\0',
	    const char *token_delims='\0');
  stringset(const char *str='\0', const char *token_delims='\0');
  stringset(const string str, const char *token_delims='\0');
  void initialize(const string& delimiters = "\0");
  stringset(const stringset &sS);
  stringset & operator=(const stringset & sS);
  virtual ~stringset();

  // Operators
  const   int  operator ==(const stringset & sS) const;
  const   int  operator !=(const stringset & sS) const {return !(this == &sS);}
  inline       operator string(void) {return string_;};
  //  inline       operator String(void) {return string_;};
  inline       operator char*(void);

  // Functions
  void shortenDelim(int i);
  int gsub(const string target, const string replace);
  int gsubDelims(const string target, const string replace);
  int contains(const string & expr)
    {
      return( string_.find(expr) < string_.length() );
    }

  // Access
  inline const string data() const
    {
      if ( 0 < size_ ) return( string_ );
      else return( "\0" );;
    }
  inline const string delim(int i = 0) const
    {
      return (i < sized_ ? delims_[i] : string("\0") );
    }
  inline const string token(int i = 0) const
    {
      return (i < size_ && 0 <= i ? tokens_[i] : string("\0") );
    }
  inline const char *ctoken(int i = 0) const
    {
      return (i < size_ ? tokens_[i].data() : '\0');
    }
  inline const char *cdelim(int i = 0) const
    {
      return (i < sized_ ? delims_[i].data() : '\0');
    }
  inline const int     size()  const {return size_;};
  inline const int     sized() const {return sized_;};
  inline const int     len()   const {return size_;};
  friend ostream & operator<<(ostream & out, const stringset & sS);

 private:
  string         string_;

 protected:
  int            size_;
  int            sized_;
  vector<string> tokens_;
  vector<string> delims_;

};
#endif // stringset_h










