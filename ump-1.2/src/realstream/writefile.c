/*
 * save pictures to disk (ppm,pgm,jpeg)
 *
 *  (c) 1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include "config.h"

#include "writefile.h"

#ifdef HAVE_LIBJPEG
# include "jpeglib.h"
#endif

/* ---------------------------------------------------------------------- */

/*
 * count up the latest block of digits in the passed string
 * (used for filename numbering
 */
int
patch_up(char *name)
{
    char *ptr;
    
    for (ptr = name+strlen(name); ptr >= name; ptr--)
	if (isdigit(*ptr))
	    break;
    if (ptr < name)
	return 0;
    while (*ptr == '9' && ptr >= name)
	*(ptr--) = '0';
    if (ptr < name)
	return 0;
    if (isdigit(*ptr)) {
	(*ptr)++;
	return 1;
    }
    return 0;
}

char*
snap_filename(char *base, char *channel, char *ext)
{
    static time_t last = 0;
    static int count = 0;
    static char *filename = NULL;
    
    time_t now;
    struct tm* tm;
    char timestamp[32];
    
    time(&now);
    tm = localtime(&now);
    
    if (last != now)
	count = 0;
    last = now;
    count++;
    
    if (filename != NULL)
	free(filename);	
    filename = malloc(strlen(base)+strlen(channel)+strlen(ext)+32);
    
    strftime(timestamp,31,"%Y%m%d-%H%M%S",tm);
    sprintf(filename,"%s-%s-%s-%d.%s",
	    base,channel,timestamp,count,ext);
    return filename;
}
int write_ppm(char *filename, char *data, int width, int height)
{
    FILE *fp;
    
    if (NULL == (fp = fopen(filename,"w"))) {
	fprintf(stderr,"grab: can't open %s: %s\n",filename,strerror(errno));
	return -1;
    }
    fprintf(fp,"P6\n%d %d\n255\n",width,height);
    fwrite(data,height,3*width,fp);
    fclose(fp);

    return 0;
}

