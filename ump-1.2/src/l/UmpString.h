// Manage coordinates for image maniupulation functions
// D Gutz
/* 	$Id: String.h,v 1.1 2007/12/16 21:11:42 davegutz Exp davegutz $	 */
#ifndef String_h
#define String_h 1
using namespace std;
#include <common.h>
#include <string>

////////////////////////class String////////////////////////////////////
// Extend the basic string class because the char* improperly terminated
class String : public string
{
 protected:
 public:
  String()                 : string()     {}
  String(char *csrc)       : string(csrc) {}   
  String(const char *csrc) : string(csrc) {}   
  String(string src)       : string(src)  {}
  ~String() {}
  inline const char *cstr() const
    {
      int   len  = this->length();
      char *cstr = new char[len+1];
      strncpy(cstr, this->data(), len);
      cstr[len] = '\0';
      return cstr;
    }
  inline operator const char*(void)
    {
      return this->cstr();
    }
  friend ostream & operator<< (ostream & out, const String S)
    {
      out << S.cstr();
      return out;
    }
};

// Miscellaneous string handlers
inline const char *cstr(const String str)
{
  int len = str.length();
  char *cstr = new char[len+1];
  strncpy(cstr, str.data(), len);
  cstr[len] = '\0';
  return cstr;
}

#endif // String_h
