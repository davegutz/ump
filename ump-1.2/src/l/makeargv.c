// Make arguments for exec functions
// DA Gutz 17-Jun-01
// 	$Id: makeargv.c,v 1.1 2006/08/16 22:07:53 davegutz Exp $	

#include <common.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <makeargv.h>

extern int verbose;

// Make argv array (*argvp) for tokens in s which are separated by
// delimiters.   Return -1 on error or the number of tokens otherwise.
int makeargv(char *s, char *delimiters, char ***argvp)
{
  char *t;
  char *snew;
  int numtokens;
  int i;
  // snew is real start of string after skipping leading delimiters
  snew = s + strspn(s, delimiters);
  // create space for a copy of snew in t
  if ( NULL == (t = calloc(strlen(snew) + 1, sizeof(char))) )
    {
      *argvp = NULL;
      numtokens = -1;
    }
  else
    {
      strcpy(t, snew);
      if ( strtok(t, delimiters) == NULL )
	numtokens = 0;
      else
	for (numtokens = 1; strtok(NULL, delimiters) !=NULL; numtokens++);
      if ( NULL == (*argvp = calloc(numtokens + 1, sizeof(char *))) )
	{
	  free(t);
	  numtokens = -1;
	}
      /* insert pointers to tokens into the array */
      else
	{
	  if ( 0 < numtokens )
	    {
	      strcpy(t, snew);
	      **argvp = strtok(t, delimiters);
	      for (i = 1; i < numtokens + 1; i++)
		*( (*argvp) + i ) = strtok(NULL, delimiters);
	    }
	  else
	    {
	      **argvp = NULL;
	      free(t);
	    }
	}
    }
  return numtokens;
}

// The redirect function redirects standard  output to outfilename and
// standard input to infilename. If either infilename or outfilename is
// NULL, the corresponding redirection does not occur.
// It returns 0 if successful and -1 if unsuccessful.
int redirect(char *infilename, char *outfilename)
{
   int indes;
   int outdes;

   // redirect standard in to infilename 
   if ( NULL != infilename )
     {
       if ( -1 == (indes = open(infilename, O_RDONLY, STDMODE)) ) return -1;
       if ( -1 == dup2(indes, STDIN_FILENO) )
	 {
	   close(indes);
	   return -1;
	 }  
       close(indes);
     }
   // redirect standard out to outfilename 
   if (outfilename != NULL)
     {
       if ( -1 == (outdes = open(outfilename, O_WRONLY|O_CREAT, STDMODE)) )
         return -1;
       if ( -1 == dup2(outdes, STDOUT_FILENO) )
	 {
	   close(outdes);
	   return -1;
	 }
       close(outdes);
     }   
   return 0;
}

int  parsefile(char *s, char delimiter, char **v)
{
   char *p;
   char *q;
   int offset;
   int error = 0;

   // Find position of the delimiting character 
   *v = NULL;
   if ((p = strchr(s, delimiter)) != NULL)
     {
       // Split off the token following delimiter 
       if ((q = (char *)malloc(strlen(p + 1) + 1)) == NULL)
	 error = -1;
       else
	 {
	   strcpy(q, p + 1);
	   if ((*v = strtok(q, DELIMITERSET)) == NULL)
	     error = -1;
	   offset = strlen(q);
	   strcpy(p, p + offset + 1);
	 } 
     }

   return error;
}

// The executecmdline function parses incmd for possible redirection.
// It calls redirect to perform the redirection and makeargv to create
// the command  argument array. It then execvp's the command executable.
// The function exits on error so it never returns to the caller. 
void executecmdline(char *incmd)
{
  char **chargv;
  char *infilename;
  char *outfilename;
  
  if (      -1 == parsefile(incmd, IN_REDIRECT_SYMBOL , &infilename) )
    fprintf(stderr, "Incorrect input redirection\n");
  
  else if ( -1 == parsefile(incmd, OUT_REDIRECT_SYMBOL, &outfilename) )
    fprintf(stderr, "Incorrect output redirection\n");
  
  else if ( -1 == redirect(infilename, outfilename) )
    fprintf(stderr, "Redirection failed\n");
  
  else if (  0 <  makeargv(incmd, BLANK_STRING, &chargv) )
    {
      if ( -1 == execvp(chargv[0], chargv) )
	perror("Invalid command");
    }
  exit(1);
}

// Execute cmd and return it's exit status
int Execute(char *cmd, int spawn)
{
  int status = 0;
  int returnVal = 0;
  pid_t   child_pid, wait_pid;
  if ( 0 == ( child_pid = fork()) )
    {
      if ( spawn && -1 == setpgid(getpid(), getpid()) ) exit(1);
      if ( 1< verbose )
	{
	  printf("%s\n", cmd);
	  fflush(stdout);
	}
      executecmdline(cmd);
      return(-1);
    }
  else if ( 0 < child_pid )
    {
      if ( !spawn )
	{
	  while( 0 < ( wait_pid = waitpid(-1, &status, 0) ) )
	    {
	      if ( wait_pid == child_pid ) break;
	    }
	  if ( WIFEXITED(status) ) returnVal =  WEXITSTATUS(status);
	}
      while ( 0 < waitpid(-1, NULL, WNOHANG) );
    }
  if ( 1< verbose )
    {
      printf("%s returned %d\n", cmd, returnVal);
      fflush(stdout);
    }
  return(returnVal);
}








