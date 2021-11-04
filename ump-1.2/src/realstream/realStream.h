/* Real time streaming driver */
/* 	$Id: realStream.h,v 1.1 2006/08/16 22:05:26 davegutz Exp $	 */
char v4l_conf[] = "";
int   debug             =   0;     // Debug for streamer stuff
int have_dga            =   0;     // Keep v4l2 happy
struct video_capability
{
  char name[32];
  int type;
  int channels;	// Num channels
  int audios;	// Num audio devices
  int maxwidth;	// Supported width
  int maxheight;// And height
  int minwidth;	// Supported width
  int minheight;// And height
};
int *realStreamerFork(int *width, int *height, char *device, int grab_fps,
		      int absframes, int deadband, int *smid, int *childpid,
		      int *talk, int maxedgepix, int *smptr, int sub,
		      int loadTest, int loadLevel, int *format, char *input,
		      char *tvnorm);
int *pipeStreamerFork(int width, int height, char *device, int fps,
		      int absframes, int deadband, int *smid, int *childpid,
		      int ifd[], int ofd[], const int maxedgepix, int *smptr,
		      int sub, int loadTest, int loadLevel);
int *readStreamerFork(char *device_digit, char *inFile, int *width,
		      int *height, int *fps, int *absframes, int limitRead);
int writeStreamerFork(char *device_digit, char *outFile, int *width,
		      int *height, char *device, int *fps, int *absframes);
