/* Image file writing functions */
/* DA Gutz 17-Jun-01 */
/* 	$Id: writefile.h,v 1.1 2006/08/16 22:07:08 davegutz Exp $	 */
int   patch_up(char *name);
char* snap_filename(char *base, char *channel, char *ext);
int   write_ppm(char *filename, char *data, int width, int height);
int   write_pgm(char *filename, char *data, int width, int height);
