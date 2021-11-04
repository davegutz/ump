// 	$Id: InFile.cc,v 1.3 2007/02/14 23:08:43 davegutz Exp davegutz $	
// Read input file into ram.
#include "InFile.h"
extern int verbose;

// Constructors.
InFile::InFile()
  : varVX_(vector<Datax>()), vA_(list<string>()), vS_(list<stringset>()),
    token_delims_(""), fileextension_(""), fileroot_(""),
    inFile_(""), maxfilelines_(1), maxline_(1), maxLineLength_(0),
    num_lines_(0), nvars_(0), programName_(""), reconstructed_(0),
    tokenized_(0), counted_(0)
{}

InFile::InFile(const char pname[], const char ifile[], const long maxfilelines,
	       const int maxline)
  : varVX_(vector<Datax>()), vA_(list<string>()), vS_(list<stringset>()),
    token_delims_(""), fileextension_(""), fileroot_(""),
    inFile_(""), maxfilelines_(maxfilelines), maxline_(maxline),
    maxLineLength_(0), num_lines_(0), nvars_(0), programName_(""),
    reconstructed_(0), tokenized_(0), counted_(0)
{
  if ( maxline_ < 2 )
    cerr << "InFile: " << __FILE__ << " : " << __LINE__ << " : "
	 << "only " << maxline_ << " characters reserved for maxline\n";
  programName_ = string(pname);
  if ( !ifile )
    {
      cerr << "InFile: " << __FILE__ << " : " << __LINE__ << " : "
	   << "filename not specified\n";
      exit(1);
    }
  inFile_ = string(ifile);
  
  // Determine file root.
  const char *delims = "/.";
  stringset in_fileS(inFile_, delims);
  fileextension_ = in_fileS.token(in_fileS.size()-1);
  fileroot_      = in_fileS.token(0);
}

// Copy constructor
InFile::InFile(InFile &IF)
  : varVX_(IF.varVX_), vA_(list<string>()), vS_(list<stringset>()),
    token_delims_(IF.token_delims_),
    fileextension_(IF.fileextension_), fileroot_(IF.fileroot_),
    inFile_(IF.inFile_), maxfilelines_(IF.maxfilelines_), maxline_(IF.maxline_),
    maxLineLength_(IF.maxLineLength_), num_lines_(IF.num_lines_),
    nvars_(IF.nvars_), programName_(IF.programName_),
    reconstructed_(IF.reconstructed_), tokenized_(IF.tokenized_),
    counted_(IF.counted_)
{
  int i=0;
  list<string>::iterator iA = IF.vA_.begin();
  for ( ; i<num_lines_&&iA!=IF.vA_.end(); i++,iA++ )
    {
      vA_.push_back(*iA);
    }
  int j=0;
  list<stringset>::iterator iS = IF.vS_.begin();
  for ( ; j<num_lines_&&iS!=IF.vS_.end(); j++,iS++ )
    {
      vS_.push_back(*iS);
    }
}

// Assignment operator
InFile & InFile::operator=(InFile & IF)
{
  if ( this == &IF ) return *this;
  varVX_         = IF.varVX_;
  token_delims_  = IF.token_delims_;
  fileextension_ = IF.fileextension_;
  fileroot_      = IF.fileroot_;
  inFile_        = IF.inFile_;
  maxfilelines_  = IF.maxfilelines_;
  maxline_       = IF.maxline_;
  maxLineLength_ = IF.maxLineLength_;
  num_lines_     = IF.num_lines_;
  nvars_         = IF.nvars_;
  programName_   = IF.programName_;
  reconstructed_ = IF.reconstructed_;
  tokenized_     = IF.tokenized_;
  counted_       = IF.counted_;
  int i=0;
  list<string>::iterator    aS = IF.vA_.begin();
  list<stringset>::iterator iS = IF.vS_.begin();
  for ( ; i<num_lines_&&iS!=IF.vS_.end(); i++,iS++,aS++ )
    {
      vA_.push_back(*aS);
      vS_.push_back(*iS);
    }
  return *this;   // Allow stacked assignments
}



InFile::~InFile()
{
}


// cout
ostream & operator<< (ostream & out, InFile & InFile)
{
  int i=0;
  if ( !InFile.tokenized_ )
    {
      out << "num_lines_= " << InFile.num_lines_ << endl;
      list<string>::iterator    aS = InFile.vA_.begin();
      for ( ; i<InFile.num_lines_&&aS!=InFile.vA_.end(); i++,aS++ )
	{
	  out << setw(4) << i << ":" << *aS << endl;
	}
    }
  else
    {
      out << "num_lines_= " << InFile.num_lines_ << endl;
      list<stringset>::iterator iS = InFile.vS_.begin();
      for ( iS=InFile.vS_.begin(); iS!=InFile.vS_.end(); iS++,i++ )
	{
	  out << setw(4) << i << ":" << *iS << endl;
	}
    }
  return out;
}


// Access to data.
int    InFile::numLines() {return num_lines_;};
//String InFile::Line(const int i)
string InFile::Line(const int i)
{
  int j;
  list<string>::iterator aS;
  for ( j=0, aS=vA_.begin(); j<i&&aS!=vA_.end(); j++,aS++ );
  if ( i > -1 )
    {
      if ( i<num_lines_ ) return *aS;
      else                return *(aS--);
    }
  else 
    {
      aS=vA_.begin();
      return *aS;
    }
}
stringset InFile::LineS(const int i)
{
  if ( i > -1 )
    {
      int j=0;
      list<stringset>::iterator iS = vS_.begin();
      for (  ; j<i && j<num_lines_-1; j++,iS++ );
      return *(iS);
    }
  else return *(vS_.begin());
}
int  InFile::findStr(string target, int line)
{
  int j=0;
  list<string>::iterator aS = vA_.begin();
  for (  ; j<line && j<num_lines_-1; j++,aS++ );
  while ( line < num_lines_
	  && !(aS->find(target) < aS->length()) )
    {
      line++;
      aS++;
    }
  return line;
}
string InFile::token(const int i, const int j)
{
  if ( 0 > i || 0 > j )
    return "";
  else
    return LineS(i).token(j);
}

// Count variable data
int InFile::loadVars()
{
  if ( tokenized_ )
    {
      cerr << "Must not be tokenized.\n";
      return 1;
    }
  this->tokenize(" =!\t\n\a\b\r\f\v;%<>/,&^?#|$@*():{}\\\'[]\"");

  // Examine each location and load arrays
  vector <Coord> locV;
  int    loadingVar = 0;
  Datax   tempX;
  for ( int i=0; i<num_lines_; i++ ) for ( int j=0; j<LineS(i).size(); j++ )
    {
      // Find first variable
      if ( !loadingVar && isNum( token(i,j) ) ) continue;
      int lastTok = ( num_lines_==i+1 && LineS(i).size()==j+1 );

      if ( isNum( token(i,j) ) )
	{
	  Coord temp(i, j);
	  locV.push_back(temp);
	}

      if ( !isNum( token(i,j) ) || lastTok )
	{
	  tempX.length = locV.size();
	  tempX.locV   = locV;
	  varVX_.push_back(tempX);
	  loadingVar = 0;
	  if ( !lastTok )
	    {
	      nvars_++;
	      tempX.name = token(i,j);
	      loadingVar = 1;
	      locV.clear();
	    }
	}
    }
  return nvars_;
}

 // Delete last char of i'th
void InFile::shortenDelim(const int L, int i)
{
  int j=0;
  list<stringset>::iterator iS = vS_.begin();
  for (  ; j<L && j<num_lines_-1; j++,iS++ ){};
  iS->shortenDelim(i);
}

void InFile::downcase()
{
  int i=0;
  list<string>::iterator aS = vA_.begin();
  for (  ; i<num_lines_&&aS!=vA_.end(); i++,aS++ )
    {
      for ( unsigned int j=0; j < aS->length(); j++ )
	{
	  (*aS)[j] = tolower( (*aS)[j] );
	}
    }
}

void InFile::downcase(int startline, int endline)
{
  endline   = MAX( MIN( endline,   num_lines_-1), 0);
  startline = MAX( MIN( startline, endline     ), 0);
  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<startline&&aS!=vA_.end(); i++,aS++ );
  for (                   ; i<endline+1&&aS!=vA_.end(); i++,aS++ )
    {
      for ( unsigned int j=0; j < aS->length(); j++ )
	{
	  (*aS)[j] = tolower( (*aS)[j] );
	}
    }
}

// Global replace.  Return number of replacements.
int InFile::gsub(string *expr, const string target, const string replace){
  unsigned int  j     = 0;
  int           count = 0;
  while ( (j=expr->find(target)) < expr->length() ){
    expr->replace(j, target.length(), replace);
    count++;
  }
  return count;
}
int InFile::gsub(list<string>::iterator & iExpr,
		 const string target, const string replace){
  unsigned int  j     = 0;
  int           count = 0;
  while ( (j=iExpr->find(target)) < iExpr->length() ){
    iExpr->replace(j, target.length(), replace);
    count++;
  }
  return count;
}
int InFile::gsub(const string target, const string replace)
{
  int count = 0;
  if ( !tokenized_ )
    {
      int i;
      list<string>::iterator aS;
      if ( target != replace )
	for ( i=0,aS=vA_.begin(); i<num_lines_&&aS!=vA_.end(); i++,aS++ )
	  {
	    count += gsub(aS, target, replace);
	  }
    }
  else
    {
      int i;
      list<stringset>::iterator iS;
      if ( target != replace )
	for ( i=0,iS=vS_.begin(); i<num_lines_&&iS!=vS_.end(); i++,iS++ )
	    count += iS->gsub(target, replace);
    }
  return count;
}  
int InFile::gsub(const string target, const string replace,
		 int startline, int endline)
{
  int count = 0;
  if ( !tokenized_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << ": " << __LINE__ 
	   << "  : must first tokenize() before gsub().\n";
      exit(1);
    }
  if ( target != replace )
    {
      endline   = MAX( MIN( endline,   num_lines_-1 ), 0 );
      startline = MAX( MIN( startline, endline      ), 0 );

      int i;
      list<stringset>::iterator iS;
      for ( i=0,iS=vS_.begin(); i<startline&&iS!=vS_.end(); i++,iS++ );
      for (                   ; i<endline+1&&iS!=vS_.end(); i++,iS++ )
	{
	  count += iS->gsub(target, replace);
	}
    }
  return count;
}  
int  InFile::gsubDelims(const string target, const string replace)
{
  int count = 0;
  if ( !tokenized_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << ": " << __LINE__ 
	   << "  : must first tokenize() before gsubDelims().\n";
      exit(1);
    }
  int i;
  list<stringset>::iterator iS;
  if ( target != replace )
    for ( i=0,iS=vS_.begin(); i<num_lines_&&iS!=vS_.end(); i++,iS++ )
      {
	count += iS->gsubDelims(target, replace);
      }
  return count;
}
void InFile::reconstruct()
{
  if ( !tokenized_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << ": " << __LINE__ 
	   << "  : must first tokenize() before reconstruct().\n";
      exit(1);
    }
  if ( reconstructed_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << ": " << __LINE__ 
	   << "  : already reconstructed.\n";
      exit(1);
    }

  vA_.clear();
  int i;
  list<stringset>::iterator iS;
  for ( i=0,iS=vS_.begin(); i<num_lines_&&iS!=vS_.end(); i++,iS++ )
    {
      // Dangling initial delimiter
      if ( iS->sized() && !iS->size() )
	{
	  vA_.push_back(iS->delim(0));
	}
      else
	{
	  vA_.push_back("");
	}
      // Some tokens exist
      for ( int j=0; j<iS->size(); j++ )
	{
	  vA_.back() = vA_.back() + iS->delim(j) + iS->token(j);
	}

      // Dangling final delimiter
      if ( iS->size() && iS->sized()>iS->size() )
	{
	  vA_.back() = vA_.back() + iS->delim(iS->sized()-1);
	}
    }
  reconstructed_ = 1;
  tokenized_     = 0;
}

void InFile::upcase()
{
  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<num_lines_&&aS!=vA_.end(); i++,aS++ )
    {
      for ( unsigned int j=0; j < aS->length(); j++ )
	{
	  (*aS)[j] = toupper((*aS)[j]);
	}
    }
}

void InFile::upcase(int startline, int endline)
{
  endline   = MAX(MIN(endline,   num_lines_-1), 0);
  startline = MAX(MIN(startline, endline),      0);

  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<startline&&aS!=vA_.end(); i++,aS++ );
  for (                   ; i<endline+1&&aS!=vA_.end(); i++,aS++ )
    {
      for ( unsigned int j=0; j < aS->length(); j++ )
	{
	  (*aS)[j] = toupper((*aS)[j]);
	}
    }
}

// Strip to end of line following comment_delim
int InFile::stripComments(const string & comment_delim)
{
  int numComS = 0;
  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<num_lines_&&aS!=vA_.end(); i++,aS++ )
    {
      unsigned int j=0;
      if ( (j = (aS->find(comment_delim))) < aS->length() )
	{
	  numComS++;
	  (*aS) = aS->substr(0, j);
	}
    }
  return numComS;
}
int InFile::stripComments(const string & comment_delim,
			   int startline, int endline)
{
  int numComS = 0;
  endline   = MAX(MIN(endline, num_lines_-1), 0);
  startline = MAX(MIN(startline, endline), 0);


  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<startline&&aS!=vA_.end(); i++,aS++ );
  for (                   ; i<endline+1&&aS!=vA_.end(); i++,aS++ )
    {
      unsigned int j=0;
      if ( (j = (aS->find(comment_delim))) < aS->length() )
	{
	  numComS++;
	  (*aS) = aS->substr(0, j);
	}
    }
  return numComS;
}

// Strip lines containing only white space.  Return number of lines left.
int InFile::stripBlankLines()
{
  int i;
  list<string>::iterator aS;

  for ( i=0,aS=vA_.begin(); i<num_lines_&&aS!=vA_.end(); i++,aS++ )
    {
      int j=0;
      int len = aS->length();
      while ( j < len && isspace( (*aS)[j]) )
	{
	  j++;
	}
      if ( j == len )
	{
	  vA_.erase(aS--);
	  i--;
	  num_lines_--;
	}
    }

  if ( num_lines_ == 0 )
    {
#ifdef VERBOSE
      if ( verbose )
	cerr << "WARNING(InFile): stripBlankLines : " << __FILE__ << " : " 
	     << __LINE__ << " : " << "file " << inFile_ 
	     << " is empty after stripping white space\n";
#endif
    }
  return (num_lines_);
}

// Determine length of longest line, in tokens
int InFile::maxLineLength()
{
  if ( counted_ )         return (maxLineLength_);
  else if ( !tokenized_ ) return (1);
  else
    {
      counted_ = 1;
      int i;
      list<stringset>::iterator iS;
      for ( i=0,iS=vS_.begin(); i<num_lines_&&iS!=vS_.end(); i++,iS++ )
	{
	  maxLineLength_ = MAX(maxLineLength_, iS->size());
	}
      return (maxLineLength_);
    }
}
    
// Tokenize each line of file into a stringset
int InFile::tokenize(const string &tokenDelims)
{
  vS_.clear();
  int numTokens = 0;
  token_delims_  = string(tokenDelims);
  if ( num_lines_ )
    {
      tokenized_     = 1;
      reconstructed_ = 0;
    }
  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<num_lines_&&aS!=vA_.end(); i++,aS++ )
    {
      vS_.push_back(stringset( aS->data(), tokenDelims.data()));
      list<stringset>::iterator iS=--vS_.end();
      numTokens += iS->size();
    }
  return(numTokens);
}

// Delete line and readjust array
void InFile::deleteLine(const int L)
{
  int i;
  list<string>::iterator    aS;
  list<stringset>::iterator iS;
  for ( i=0,aS=vA_.begin(); i<L&&aS!=vA_.end();	i++,aS++ );
  for ( i=0,iS=vS_.begin(); i<L&&iS!=vS_.end();	i++,iS++ );

  vA_.erase(aS);
  if ( tokenized_ )
    {
      vS_.erase(iS);
    }
  num_lines_--;
}
void InFile::addLine(const int L, const string S)
{
  // Check size.
  if ( num_lines_ >= maxfilelines_-1 )
    {
      cerr << "InFile: " << __FILE__ << " : " << __LINE__ << " : "
	   << "basic block full "
	   << "on block line " << num_lines_ <<".\n"
	   << "Check maxfilelines=" << maxfilelines_ << " is "
	   << "greater than " << num_lines_
	   << " and recompile if necessary\n";
      exit(1);
    }

  int i;
  list<string>::iterator aS;
  for ( i=0,aS=vA_.begin(); i<L&&aS!=vA_.end(); i++,aS++ );
  vA_.insert(aS, string(S));
  aS--;
  num_lines_++;
  if ( tokenized_ )
    {
      int i;
      list<stringset>::iterator iS;
      for ( i=0, iS=vS_.begin(); i<L&&iS!=vS_.end(); i++, iS++ );
	vS_.insert(iS, stringset(aS->data(), token_delims_.data()));
    }
}
// Load a line.
inline string InFile::getLine(string &inbuf, ifstream &inf)
{
  char *line = new char [maxline_];
  strcpy(line, "\0");  // Need to pre-load for some reason
  if ( inf )
    {
      inf.getline(line, maxline_, '\n');  //Gets text a line at a time
      if ( maxline_-2 < (int)strlen(line) )
	{
	  cerr << "WARNING(InFile): getLine: " << __FILE__ << " : "
	       << __LINE__ << " : " << "line buffer size " << maxline_ 
	       << " may have overflowed.  Increase maxline_\n";
	}
    }
  if ( !inf )
    {
      strcpy(line, "\0");
    }
  inbuf = line;
  zaparr(line);
  return inbuf;
}

// Load a line.
inline string InFile::getLine(string &inbuf, istream &inf)
{
  char *line = new char [maxline_];
  strcpy(line, "\0");  // Need to pre-load for some reason
    if ( inf )
    {
      inf.getline(line, maxline_, '\n');  //Gets text a line at a time
      if ( maxline_-2 < (int)strlen(line) )
	{
	  cerr << "WARNING(InFile): getLine " << __FILE__ << " : " << __LINE__ 
	       << " : " << "line buffer size " << maxline_ 
	       << " may have overflowed.  Increase maxline_\n";
	}
    }
  if ( !inf )
    {
      strcpy(line, "\0");
    }
  inbuf = line;
  zaparr(line);
  return inbuf;
}

//Load InFile from file.
int InFile::load()
{

  // If compressed, automatically load it
  if ( "Z" == fileextension_ )
    {
      int    fildes[2];
      char **execvparg;             // Argument for execvp
      char   delim[] = " \t";       // Delimiter to make argument
      char   command[MAX_CANON];    // Desired execvp command
      int    numtokens;
      pipe(fildes);
      switch ( fork() )
	{
	case -1:
	  perror("InFile: load : The fork failed");
	  exit(1);
	case 0:    // child
	  if ( (dup2(fildes[1], STDOUT_FILENO)) == -1 )
	    {
	      perror("InFile: load : Child could not dup stdout");
	      exit(1);
	    }
	  if ( (close(fildes[0]) == -1) || (close(fildes[1]) == -1) )
	    {
	      perror("InFile: load : Child could not close file descriptors");
	      exit(1);
	    }
	  sprintf(command, "zcat %s", inFile_.data());
	  if ( (numtokens = makeargv(command, delim, &execvparg)) < 0 )
	    {
	      cerr << "ERROR(InFile): load:" << __FILE__ << " : " << __LINE__
		   << " : " << "could not construct argument array for " 
		   << command << endl;
	      exit (1);
	    }
	  // Start zcat.
	  if ( (execvp(execvparg[0], &execvparg[0])) < 0 )
	    {
	      perror("InFile : load : The exec of zcat failed");
	      exit (1);
	    }
	  cerr << command << endl;
	  break;
	default:  // Parent
	  if ( (dup2(fildes[0], STDIN_FILENO)) == -1)
	    {
	      perror("InFile : load : Child could not dup stdin");
	      exit(1);
	    }
	  if ( (close(fildes[0]) == -1) || (close(fildes[1]) == -1) )
	    {
	      perror("InFile : load : Parent could not close file descriptors");
	      exit(1);
	    }
	}
      string inbuf;
      getLine(inbuf, cin);
      if ( !cin )
	{
	  cerr << "InFile: " << __FILE__ << " : " << __LINE__ << " : "
	       << "file may be empty.\n"
	       << "Also check for line longer than " << maxline_
	       << " characters."
	       << "Recompile program with new maxline value if needed\n";
	  return(0);
	}
      // Strip out data.
      do
	{
	  // Load the block into array.
	  vA_.push_back(string(inbuf));
	  num_lines_++;

	  // Check size.
	  if ( num_lines_ >= maxfilelines_ )
	    {
	      cerr << "ERROR(InFile): load : " << __FILE__ << " : " << __LINE__
		   << " : " << "basic block full "
		   << "on block line " << num_lines_ <<".\n"
		   << "Check maxfilelines=" << maxfilelines_ << " is "
		   << "greater than " << num_lines_
		   << " and recompile if necessary\n";
	      exit(1);
	    }

	}
      while ( cin
	      && getLine(inbuf, cin)  != "EOF"
	      && cin
	      && num_lines_ < maxfilelines_ );
      return(num_lines_); 
    }
  
  // Uncompressed file
  ifstream inf;               // Input file
  inf.open(inFile_.data(), ios::in);
  if ( !inf )
    {
#ifdef VERBOSE
      if ( verbose )
	cout << "InFile: load : " << __FILE__ << " : " << __LINE__ << " : "
	     << "cannot open file " << inFile_ << ".\n";
#endif
      return(0);
    }
  string inbuf;
  getLine(inbuf, inf);
  if ( !inf )
    {
#ifdef VERBOSE
      if ( verbose ) 
	cout << "WARNING(InFile): load : " << __FILE__ << " : " << __LINE__ 
	     << " : " << "file may be empty.\n"
	     << "Also check for line longer than " << maxline_ << " characters."
	     << "Recompile parent program with new maxline value if needed\n";
#endif
      return(0);
    }
  // Strip out data.
  do
    {
      // Load the block into array.
      vA_.push_back(string(inbuf));
      num_lines_++;

      // Check size.
      if ( num_lines_ >= maxfilelines_ )
	{
	  cerr << "ERROR(InFile): load :  " << __FILE__ << " : " << __LINE__ 
	       << " : " << "basic block full "
	       << "on block line " << num_lines_ <<".\n"
	       << "Check maxfilelines=" << maxfilelines_ << " is "
	       << "greater than " << num_lines_
	       << " and recompile if necessary\n";
	  exit(1);
	}

    }
  while ( inf
	  && (getLine(inbuf,inf) != "EOF")
	  && inf
	  && num_lines_ < maxfilelines_ );

  inf.close();
  if ( num_lines_ == 0 )
    {
    cerr << "WARNING(InFile): load : " << __FILE__ << " : " << __LINE__ << " : "
	 << "file " << inFile_ << " is empty\n";
    }
  return(num_lines_); 
}


//Load InFile from file.
int InFile::load(int skips, int after, int includeLast)
{

  // Screen input commands
  if ( after < 1 )
    {
      cerr << "WARNING(InFile): load : " << __FILE__ << " : " << __LINE__
	   << " : " << "bad value for after=" << after << endl;
    }

  // If compressed, automatically load it
  if ( "Z" == fileextension_ )
    {
      int fildes[2];
      char **execvparg;           // Argument for execvp
      char delim[] = " \t";       // Delimiter to make argument
      char command[MAX_CANON];    // Desired execvp command
      int numtokens;
      pipe(fildes);
      switch (fork())
	{
	case -1:
	  perror("InFile : load : The fork failed");
	  exit(1);
	case 0:    // child
	  if ( (dup2(fildes[1], STDOUT_FILENO)) == -1)
	    {
	      perror("InFile : load : Child could not dup stdout");
	      exit(1);
	    }
	  if ( (-1 == close(fildes[0])) || (-1 == close(fildes[1])) )
	    {
	      perror("InFile : load : Child could not close file descriptors");
	      exit(1);
	    }
	  sprintf(command, "zcat %s", inFile_.data());
	  if ( 0 > (numtokens = makeargv(command, delim, &execvparg)) )
	    {
	      cerr << "ERROR(InFile): load : Could not construct argument "
		   << "array for " << command << endl;
	      exit (1);
	    }
	  // Start zcat.
	  if ( 0 > execvp(execvparg[0], &execvparg[0]) ) {
	    perror("InFile : load : The exec of zcat failed");
	    exit (1);
	  }
	  cerr << command << endl;
	  break;
	default:  // Parent
	  if ( -1 == dup2(fildes[0], STDIN_FILENO) )
	    {
	      perror("InFile : load : Child could not dup stdin");
	      exit(1);
	    }
	  if ( (-1 == close(fildes[0])) || (-1 == close(fildes[1])) )
	    {
	      perror("InFile : load : Parent could not close file descriptors");
	      exit(1);
	    }
	}
      if ( !cin )
	{
	  cerr << "ERROR(InFile): load : " << __FILE__ << " : " << __LINE__ 
	       << " : " << "cannot open file " << inFile_ << endl;
	  exit(1);
	}
      string inbuf;
      getLine(inbuf, cin);
      if ( !cin )
	{
	  cerr << "WARNING(InFile): load : " << __FILE__ << " : " << __LINE__
	       << " : " << "file may be empty.\n"
	       << "Also check for line longer than " << maxline_
	       << ".  Recompile program with new maxline value if needed.\n";
	  return(0);
	}
      // Strip out data.
      int lineNumber=0;
      do
	{
	  // Load the block into array.
	  if ( ++lineNumber < after+1
	       ||
	       fmod((float)(lineNumber-after), (float)MAX(skips+1, 1)) == 0 )
	    { 
	      vA_.push_back(string(inbuf));
	      num_lines_++;
	      // Check size.
	      if ( num_lines_ >= maxfilelines_ )
		{
		  cerr << "ERROR(InFile): load : " << __FILE__ << " : "
		       << __LINE__ << " : " << "basic block full "
		       << "on block line " << num_lines_ <<".\n"
		       << "Check maxfilelines=" << maxfilelines_ << " is "
		       << "greater than " << num_lines_
		       << " and recompile if necessary\n";
		  exit(1);
		}
	    }
	}
      while ( cin
	      && (getLine(inbuf, cin) != "EOF")
	      && cin
	      && num_lines_ < maxfilelines_ );

      // Attempt to tack on last line
      // doesn't work if file padded with blank line
      if ( includeLast && (vA_.back() != inbuf) )
	{
	  vA_.push_back(string(inbuf));
	  num_lines_++;
	  if ( num_lines_ >= maxfilelines_ )
	    {
	      cerr << "ERROR(InFile): load : " << __FILE__ << " : "
		   << __LINE__ << " : " << "basic block full "
		   << "on block line " << num_lines_ <<".\n"
		   << "Check maxfilelines=" << maxfilelines_ << " is "
		   << "greater than " << num_lines_ 
		   << " and recompile if necessary\n";
	      exit(1);
	    }
	}
      if ( 0 == num_lines_ )
	{
	  cerr << "InFile: " << __FILE__ << " : " << __LINE__ << " : "
	       << "file " << inFile_ << " is empty\n";
	}
      return(num_lines_); 
    }
  
  // Uncompressed file
  ifstream inf;
  inf.open(inFile_.data(), ios::in);
  if ( !inf )
    {
      cerr << "ERROR(InFile): load : " << __FILE__ << " : " << __LINE__ << " : "
	   << "cannot open file " << inFile_ << endl;
      exit(1);
    }
  string inbuf;
  getLine(inbuf,inf);
  if ( !inf )
    {
      cerr << "InFile: " << __FILE__ << " : " << __LINE__ << " : "
	   << "file may be empty.\n"
	   << "Also check for line longer than " << maxline_ << " characters."
	   << "Recompile parent program with new maxline value if needed\n";
      return(0);
    }
  // Strip out data.
  int lineNumber = 0;
  do
    {
      // Load the block into array.
      if ( ++lineNumber < after+1
	   || fmod((float)(lineNumber-after), (float)MAX(skips+1, 1)) == 0 )
	{ 
	  vA_.push_back(string(inbuf));
	  num_lines_++;
	  // Check size.
	  if ( num_lines_ >= maxfilelines_ )
	    {
	      cerr << "ERROR(InFile): load :" << __FILE__ << " : " << __LINE__
		   << " : " << "basic block full "
		   << "on block line " << num_lines_ <<".\n"
		   << "Check maxfilelines=" << maxfilelines_ << " is "
		   << "greater than " << num_lines_ 
		   << " and recompile if necessary\n";
	      exit(1);
	    }
	}
    }
  while ( inf
	  && (getLine(inbuf,inf) != "EOF")
	  && inf
	  && num_lines_ < maxfilelines_ );
  
  // Attempt to tack on last line (doesn't work if file padded with blank line
  if ( includeLast && vA_.back() != inbuf )
    {
      vA_.push_back(string(inbuf));
      num_lines_++;
      if ( num_lines_ >= maxfilelines_ )
	{
	  cerr << "ERROR(InFile): load : " << __FILE__ << " : " << __LINE__ 
	       << " : " << "basic block full "
	       << "on block line " << num_lines_ <<".\n"
	       << "Check maxfilelines=" << maxfilelines_ << " is "
	       << "greater than " << num_lines_ 
	       << " and recompile if necessary\n";
	  exit(1);
	}
    }
    

  inf.close();
  if ( 0 == num_lines_ )
    {
      cerr << "WARNING(InFile): load : " << __FILE__ << " : " << __LINE__
	   << " : " << "file " << inFile_ << " is empty\n";
    }
  return(num_lines_); 
}


// Overcome bug in string class
char *InFile::sureShort(const string source)
{
  const int len = source.length();
  char *shortData = new char[len+1];
  strncpy(shortData, source.data(), len);
  shortData[len]='\0';
  return shortData;
}  

// Find parameters
int InFile::isAlpha(string strData)
{
  char *shortData = sureShort(strData);
  char *k = &shortData[0];
  for ( unsigned int kk=0; kk<strlen(shortData); kk++, k++ )
    {
      if ( isalpha(*k) )
	{
	  if ( shortData ) zaparr(shortData);
	  return 1;
	}
    }
  if ( shortData ) zaparr(shortData);
  return 0;
}
int InFile::isNum(string strData)
{
  int numE = 0;
  int nump = 0;
  int numm = 0;
  int numd = 0;
  char *shortData = sureShort(strData);
  char *k = &shortData[0];
  for ( unsigned int kk=0; kk<strlen(shortData); kk++, k++ )
    {
      if ( !isdigit(*k) )
	{
	  if      ( 'e' == *k || 'E' == *k ) ++numE;
	  else if ( '+' == *k  ) ++nump;
	  else if ( '-' == *k  ) ++numm;
	  else if ( '.' == *k  ) ++numd;
	  else
	    {
	      if ( shortData ) zaparr(shortData);
	      return 0;
	    }
	  if ( 1 < numE || (1 < numm && 1 != numE) || 1 < numd 
	       || (1 < nump && 1 != numE) || ( (1 < nump) && (1 < numm) ) )
	    {
	      if ( shortData ) zaparr(shortData);
	      return 0;
	    }
	}
    }
  if ( shortData ) zaparr(shortData);
  return 1;
}

// Find paramters in var_
// float
int InFile::getPar(string target, float *par)
{
  if ( !nvars_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << " : " << __LINE__
	   << " : Must have run tokenize and loadVars\n";
      exit(1);
    }

  int num = 0;
  // Find last occurence of target
  vector<Datax>::iterator ivar      = varVX_.begin();
  vector<Datax>::iterator ivarMatch = varVX_.end();
  while ( ivar !=varVX_.end() )
    {
      if ( ivar->name == target )
	{
	  ivarMatch = ivar;
	  num++;
	}
      ivar++;
    }
  if ( ivarMatch == varVX_.end() ){}
  else
    { 
      // Convert and return
      if ( 0 < num )
	{
	  *par
	    = atof(token((ivarMatch->locV)[0].row,
			 (ivarMatch->locV)[0].pos ).data());
	  if ( 1 < num )
	    {
#ifdef VERBOSE
	      if ( verbose )
		cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
		     << "Multiple occurences of " << target << ".\n";
#endif
	    }
	}
    }
  return num;
}
// integer
int InFile::getPar(string target, int *par)
{
  if ( !nvars_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << " : " << __LINE__
	   << " : Must have run tokenize and loadVars\n";
      exit(1);
    }

  int num = 0;
  // Find last occurence of target
  vector<Datax>::iterator ivar      = varVX_.begin();
  vector<Datax>::iterator ivarMatch = varVX_.end();
  while ( ivar !=varVX_.end() )
    {
      if ( ivar->name == target )
	{
	  ivarMatch = ivar;
	  num++;
	}
      ivar++;
    }
  if ( ivarMatch == varVX_.end() ){}
  else
    { 
      // Convert and return
      if ( 0 < num )
	{
	  *par
	    = atoi(token((ivarMatch->locV)[0].row,
			 (ivarMatch->locV)[0].pos ).data());
	  if ( 1 < num )
	    {
#ifdef VERBOSE
	      if ( verbose )
		cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
		     << "Multiple occurences of " << target << ".\n";
#endif
	    }
	}
    }
  return num;
}


#ifdef UMP
// Vector
int InFile::getPar(string target, Vector <float> &V)
{
  if ( !nvars_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << " : " << __LINE__
	   << " : Must have run tokenize and loadVars\n";
      exit(1);
    }

  int num = 0;
  // Find last occurence of target
  vector<Datax>::iterator ivar      = varVX_.begin();
  vector<Datax>::iterator ivarMatch = varVX_.end();
  while ( ivar !=varVX_.end() )
    {
      if ( ivar->name == target )
	{
	  ivarMatch = ivar;
	  num++;
	}
      ivar++;
    }
  if ( ivarMatch == varVX_.end() ){}
  else
    { 
      // Convert and return
      if ( 0 < num )
	{
	  if ( V.size() == ivarMatch->length )
	    {
	      for ( int j=0; j<V.size(); j++ )
		{
		  V[j]
		    = atof(token((ivarMatch->locV)[j].row,
				 (ivarMatch->locV)[j].pos ).data());
		}
	      if ( 1 < num )
		{
#ifdef VERBOSE
		  if ( verbose )
		    cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
			 << " multiple occurences of " << target << ".\n";
#endif
		}
	    }
	  else
	    {
#ifdef VERBOSE
	      if ( verbose )
		cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
		     << " Bad input for Vector " << target << ".\n";
#endif
	    }
	}
    }
  return num;
}
// Matrix
int InFile::getPar(string target, Matrix <float> &M)
{
  if ( !nvars_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << " : " << __LINE__
	   << " : Must have run tokenize and loadVars\n";
      exit(1);
    }

  int num = 0;
  // Find last occurence of target
  vector<Datax>::iterator ivar      = varVX_.begin();
  vector<Datax>::iterator ivarMatch = varVX_.end();
  while ( ivar !=varVX_.end() )
    {
      if ( ivar->name == target )
	{
	  ivarMatch = ivar;
	  num++;
	}
      ivar++;
    }
  if ( ivarMatch == varVX_.end() ){}
  else
    { 
      // Convert and return
      if ( 0 < num )
	{
	  if ( M.size() == ivarMatch->length )
	    {
	      for ( int j=0; j<M.num_rows(); j++ )
		{
		  for ( int k=0; k<M.num_cols(); k++ )
		    {
		      string valToken =
			token((ivarMatch->locV)[k+j*M.num_cols()].row,
			      (ivarMatch->locV)[k+j*M.num_cols()].pos );
		      if ( valToken == "nan" )
			{
#ifdef VERBOSE
			  if ( verbose )
			    cout << "WARNING(InFile): " << __FILE__ << ": "
				 << __LINE__ << " nan for Matrix " << target
				 << ".\n" << flush;
#endif
			  return -1;
			}
		      M[j][k] = atof(valToken.data());
		    }
		}
	      if ( 1 < num )
		{
#ifdef VERBOSE
		  if ( verbose )
		    cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
			 << " multiple occurences of " << target << ".\n";
#endif
		}
	    }
	  else
	    {
#ifdef VERBOSE
	      if ( verbose )
		cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
		     << " bad input for Matrix " << target << ".\n";
#endif
	      return -1;
	    }
	}
    }
  return num;
}
// PixelLoc3
int InFile::getPar(string target, PixelLoc3 &P)
{
  if ( !nvars_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << " : " << __LINE__
	   << " : Must have run tokenize and loadVars\n";
      exit(1);
    }
  
  int num = 0;
  // Find last occurence of target
  vector<Datax>::iterator ivar      = varVX_.begin();
  vector<Datax>::iterator ivarMatch = varVX_.end();
  while ( ivar !=varVX_.end() )
    {
      if ( ivar->name == target )
	{
	  ivarMatch = ivar;
	  num++;
	}
      ivar++;
    }
  if ( ivarMatch == varVX_.end() ){}
  else
    { 
      // Convert and return
      if ( 0 < num )
	{
	  if ( P.size() == ivarMatch->length )
	    {
	      for ( int j=0; j<P.size(); j++ )
		{
		  P[j]
		    = atof(token((ivarMatch->locV)[j].row,
				 (ivarMatch->locV)[j].pos ).data());
		}
	      if ( 1 < num )
		{
#ifdef VERBOSE
		  if ( verbose )
		    cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
			 << " multiple occurences of " << target << ".\n";
#endif
		}
	    }
	  else
	    {
#ifdef VERBOSE
	      if ( verbose )
		cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
		     << " Bad input for Vector " << target << ".\n";
#endif
	    }
	}
    }
  return num;
}

int InFile::getPar(string target, PixelLoc3 &P, int nP)
{
  if ( !nvars_ )
    {
      cerr <<  "ERROR(InFile): " << __FILE__ << " : " << __LINE__
	   << " : Must have run tokenize and loadVars\n";
      exit(1);
    }

  int num = 0;
  // Find last occurence of target
  vector<Datax>::iterator ivar      = varVX_.begin();
  vector<Datax>::iterator ivarMatch = varVX_.end();
  while ( ivar !=varVX_.end() )
    {
      if ( ivar->name == target )
	{
	  ivarMatch = ivar;
	  num++;
	}
      ivar++;
    }
  if ( ivarMatch == varVX_.end() ){}
  else
    { 
      // Convert and return
      if ( 0 < num )
	{
	  if ( nP == ivarMatch->length && nP <= P.size() )
	    {
	      for ( int j=0; j<nP; j++ )
		{
		  P[j]
		    = atof(token((ivarMatch->locV)[j].row,
				 (ivarMatch->locV)[j].pos ).data());
		}
	      if ( 1 < num )
		{
#ifdef VERBOSE
		  if ( verbose )
		    cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
			 << " multiple occurences of " << target << ".\n";
#endif
		}
	    }
	  else
	    {
#ifdef VERBOSE
	      if ( verbose )
		cout << "WARNING(InFile): " << __FILE__ << ": " << __LINE__ 
		     << " Bad input for Vector " << target << ".\n";
#endif
	    }
	}
    }
  return num;
}

#endif // UMP


// Sort by line.
void InFile::sort()
{
  vA_.sort();
}

//Swap pointers.
void InFile::swap(void *v[], int i, int j)
{
  void *temp;
  temp = v[i];
  v[i] = v[j];
  v[j] = temp;
}
