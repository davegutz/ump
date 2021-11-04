// 	$Id: stringset.cc,v 1.2 2007/01/27 01:16:28 davegutz Exp davegutz $	
//String token class.
#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif
#include "stringset.h"

/////////////////class stringset//////////////////////////

// Constructors
stringset::stringset()
  : string_(NULL), size_(0), sized_(0), tokens_(vector<string>()),
    delims_(vector<string>()) {}
stringset::stringset(const string& str, const string& delimiters)
  : string_(str), size_(0), sized_(0), tokens_(vector<string>()),
    delims_(vector<string>())
{
  initialize(delimiters);
}
stringset::stringset(const int maxsize, const string instring,
 		     const char *token_delims)
  :string_(instring), size_(0), sized_(0), tokens_(vector<string>()),
   delims_(vector<string>())
{
  string delimiters = token_delims;
  initialize(delimiters);
}
stringset::stringset(const char *str, const char *token_delims)
  : string_(str), size_(0), sized_(0), tokens_(vector<string>()),
    delims_(vector<string>())
{
  string delimiters = token_delims;
  initialize(delimiters);
}
stringset::stringset(const string str, const char *token_delims)
  : string_(str), size_(0), sized_(0), tokens_(vector<string>()),
    delims_(vector<string>())
{
  string delimiters = token_delims;
  initialize(delimiters);
}
void stringset::initialize(const string& delimiters)
{
  if ( "" == string_ )
    {
      delims_.push_back("");
      tokens_.push_back("");
      return;
    }
  // Skip delimiters at beginning.
  string::size_type lastPos = string_.find_first_not_of(delimiters, 0);
  string::size_type pos     = string_.find_first_of(delimiters, lastPos);
  if ( string::npos==lastPos && string::npos!=pos )
    delims_.push_back(string_);
  else
    delims_.push_back(string_.substr(0, lastPos));
  while (string::npos != pos || string::npos != lastPos)
    {
      tokens_.push_back(string_.substr(lastPos, pos-lastPos));
      lastPos = string_.find_first_not_of(delimiters, pos);

      if ( string::npos != lastPos )
	delims_.push_back(string_.substr(pos, lastPos-pos));
      else if ( string::npos !=pos ) 
	delims_.push_back(string_.substr(pos, string_.length()-pos));
      pos = string_.find_first_of(delimiters, lastPos);
    }
  sized_ = delims_.size();
  size_  = tokens_.size();
  if ( sized_ - size_ > 1 || sized_ < size_ ) 
    {
      cerr << "ERROR(stringset): __FILE__ : __LINE__ : " << " impossible\n";
      exit (1);
    }
}
stringset::stringset(const stringset &sS)
  : string_(sS.string_), size_(sS.size_), sized_(sS.sized_),
    tokens_(sS.tokens_), delims_(sS.delims_) {}
stringset & stringset::operator=(const stringset & sS)
{
  if ( this == &sS ) return *this;
  string_ = sS.string_;
  size_   = sS.size_;
  sized_  = sS.sized_;
  tokens_ = sS.tokens_;
  delims_ = sS.delims_;
  return *this;   // Allow stacked assignments
}
stringset::~stringset(){}

// Functions
// Shorten delimiter
void stringset::shortenDelim(int i)
{
  delims_[i] = delims_[i].substr(0, delims_[i].length()-1);
}

// Global replace.  Return number of replacements.
int stringset::gsub(const string target, const string replace)
{
  int count = 0;
  if ( target != replace ) for ( int i=0; i<size_; i++ )
    {
      if ( target == tokens_[i] )
	{
	  count++;
	  tokens_[i] = replace;
	}
    }
  return count;
}

int stringset::gsubDelims(const string target, const string replace)
{
  int count = 0;
  if ( target != replace ) for ( int i=0; i<sized_; i++ )
    {
      if ( target == delims_[i] )
	{
	  count++;
	  delims_[i] = replace;
	}
    }
  return count;
}

// Output operator.
ostream & operator<< (ostream & out, const stringset & sS)
{
  // Dangling initial delimiter
  if ( sS.sized_  && !sS.size_ ) out << "<0>" << sS.delims_[0];

  // Some tokens exist
  for ( int i=0; i<sS.size(); i++ )
    out <<  "<" << i << ">" << sS.delims_[i] << "|" << sS.tokens_[i];

  // Dangling final delimiter
  if ( sS.size_   && sS.sized_ > sS.size_ )
    out << "<" << sS.sized_-1 << ">" << sS.delims_[sS.sized_-1];

  return out;
}


// Equivalence operator.
const int stringset::operator== (const stringset & sS)const{
  if ( &sS == this ) return(1);   // Self evident.
  if ( string_!=sS.string_ || size_!=sS.size_ || sized_!=sS.sized_ )
    return(0);
  for (int i=0; i < sized_; i++) if ( delims_[i] != sS.delims_[i] )
    return(0);
  for (int i=0; i < int(tokens_.size()) && i < int(sS.tokens_.size());  i++)
    if ( tokens_[i] != sS.tokens_[i] ) return(0);
  return(1);
}
