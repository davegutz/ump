// Make arguments for exec functions
// DA Gutz 17-Jun-01
// 	$Id: makeargv.h,v 1.1 2006/08/16 22:07:21 davegutz Exp $ 
#ifndef makeargv_h
#define makeargv_h 1

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define STDMODE 0600
#define DELIMITERSET " ><|&"
#ifndef MAX_CANON
 #define MAX_CANON 256
#endif 
#define BLANK_STRING  " "
#define PROMPT_STRING ">>"
#define QUIT_STRING "q"
#define BACK_STRING "&"
#define PIPE_STRING "|"
#define NEWLINE_STRING "\n"
#define IN_REDIRECT_SYMBOL '<'
#define OUT_REDIRECT_SYMBOL '>'
#define NULL_SYMBOL '\0'
#define PIPE_SYMBOL '|'
#define BACK_SYMBOL '&'
#define NEWLINE_SYMBOL '\n'

int  parsefile(char *inbuf, char delimiter, char **v);
int  redirect(char *infilename, char *outfilename);
void executecmdline(char *cmd);
int  makeargv(char *s, char *delimiters, char ***argvp);
int  Execute(char *cmd, int spawn);

#endif // makeargv_h
