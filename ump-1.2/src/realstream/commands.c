#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>

#include <X11/Intrinsic.h>

#include "config.h"

#include "commands.h"
#include "writefile.h"
#include "grab.h"
#include "channel.h"
#include "frequencies.h"


/* ----------------------------------------------------------------------- */

/* feedback for the user */
void (*update_title)(char *message);
void (*display_message)(char *message);

/* for updating GUI elements / whatever */
void (*norm_notify)(void);
void (*input_notify)(void);
void (*attr_notify)(int id);
void (*freqtab_notify)(void);
void (*setfreqtab_notify)(void);
void (*setstation_notify)(void);

/* gets called _before_ channel switches */
void (*channel_switch_hook)();

/* capture overlay/grab/off */
void (*set_capture_hook)(int old, int new);

/* toggle fullscreen */
void (*fullscreen_hook)();
void (*exit_hook)();
void (*reconfigure_hook)();

int do_overlay;
char *snapbase = "snap";
int x11_pixmap_format;
int grab_width = 1536, grab_height = 1024;

/* ----------------------------------------------------------------------- */

static int setfreqtab_handler(char *name, int argc, char **argv);
static int setstation_handler(char *name, int argc, char **argv);
static int setchannel_handler(char *name, int argc, char **argv);

static int norm_handler(char *name, int argc, char **argv);
static int input_handler(char *name, int argc, char **argv);
static int capture_handler(char *name, int argc, char **argv);

static int volume_handler(char *name, int argc, char **argv);
static int attr_handler(char *name, int argc, char **argv);

static int snap_handler(char *name, int argc, char **argv);
static int fullscreen_handler(char *name, int argc, char **argv);
static int msg_handler(char *name, int argc, char **argv);
static int exit_handler(char *name, int argc, char **argv);

static int keypad_handler(char *name, int argc, char **argv);

static struct COMMANDS {
    char  *name;
    int    min_args;
    int   (*handler)(char *name, int argc, char **argv);
} commands[] = {
    { "setstation", 0, setstation_handler },
    { "setchannel", 0, setchannel_handler },
    { "setfreqtab", 1, setfreqtab_handler },

    { "setnorm",    1, norm_handler       },
    { "setinput",   1, input_handler      },
    { "capture",    1, capture_handler    },

    { "volume",     0, volume_handler     },
    { "color",      0, attr_handler       },
    { "hue",        0, attr_handler       },
    { "bright",     0, attr_handler       },
    { "contrast",   0, attr_handler       },

    { "snap",       0, snap_handler       },
    { "fullscreen", 0, fullscreen_handler },
    { "msg",        1, msg_handler        },
    { "message",    0, msg_handler        },
    { "exit",       0, exit_handler       },
    { "quit",       0, exit_handler       },
    { "bye",        0, exit_handler       },

    { "keypad",     1, keypad_handler     },

    { NULL, 0, NULL }
};

static struct PICT_ATTR {
    int    id;
    char  *name;
    int   *val;
} pict_attr [] = {
    { GRAB_ATTR_COLOR,    "color",    &cur_color },
    { GRAB_ATTR_BRIGHT,   "bright",   &cur_bright },
    { GRAB_ATTR_HUE,      "hue",      &cur_hue },
    { GRAB_ATTR_CONTRAST, "contrast", &cur_contrast }
};

#define NUM_ATTR (sizeof(pict_attr)/sizeof(struct PICT_ATTR))

static int keypad_state = -1;

/*------------------------------------------------------------------------*/

void
missing_feature(int id)
{
    static int displayed = 0;
    char message[128];

    /* seeing the same message twice is annonying ... */
    if (displayed & id)
	return;
    displayed |= id;

    switch (id) {
    case MISSING_CAPTURE:
	sprintf(message,"Grabbing is not supported by \"%s\".",
		grabber->name);
	break;
#ifndef HAVE_LIBJPEG
    case MISSING_JPEG:
	sprintf(message,"This binary has no JPEG support.");
	break;
#endif
    default:
	strcpy(message,"Oops, you should'nt see this message.");
	break;
    }
#if 0
    tell_user(app_shell,"Notice",message);
#else
    if (display_message)
	display_message(message);
#endif
}

/* ----------------------------------------------------------------------- */

int
do_va_cmd(int argc, ...)
{
    va_list ap;
    int  i;
    char *argv[32];
    
    va_start(ap,argc);
    for (i = 0; i < argc; i++)
	argv[i] = va_arg(ap,char*);
    argv[i] = NULL;
    va_end (ap);
    return do_command(argc,argv);
}

int
do_command(int argc, char **argv)
{
    int i;
    
    if (argc == 0) {
	fprintf(stderr,"do_command: no argument\n");
	return -1;
    }
    if (debug) {
	fprintf(stderr,"cmd:");
	for (i = 0; i < argc; i++) {
	    fprintf(stderr," \"%s\"",argv[i]);
	}
	fprintf(stderr,"\n");
    }

    for (i = 0; commands[i].name != NULL; i++)
	if (0 == strcasecmp(commands[i].name,argv[0]))
	    break;
    if (commands[i].name == NULL) {
	fprintf(stderr,"no handler for %s\n",argv[0]);
	return -1;
    }
    if (argc-1 < commands[i].min_args) {
	fprintf(stderr,"no enouth args for %s\n",argv[0]);
	return -1;
    } else {
	return commands[i].handler(argv[0],argc-1,argv+1);
    }
}

char**
split_cmdline(char *line, int *count)
{
    static char cmdline[1024];
    static char *argv[32];
    int  argc,i;

    strcpy(cmdline,line);
    for (argc=0, i=0; argc<31;) {
	argv[argc++] = cmdline+i;
	while (cmdline[i] != ' ' &&
	       cmdline[i] != '\t' &&
	       cmdline[i] != '\0')
	    i++;
	if (cmdline[i] == '\0')
	    break;
	cmdline[i++] = '\0';
	while (cmdline[i] == ' ' ||
	       cmdline[i] == '\t')
	    i++;
	if (cmdline[i] == '\0')
	    break;
    }
    argv[argc] = NULL;

    *count = argc;
    return argv;
}

/* ----------------------------------------------------------------------- */

/* sharing code does'nt work well for this one ... */
static void
set_capture(int capture)
{
    static int last_on = 0;

    if (set_capture_hook) {
	if (capture == CAPTURE_ON)
	    capture = last_on;
	
	if (capture == CAPTURE_OVERLAY) {
	    /* can we do overlay ?? */
	    if (NULL == grabber->grab_overlay)
		capture = CAPTURE_GRABDISPLAY;
	    if (!do_overlay)
		capture = CAPTURE_GRABDISPLAY;
	}

	if (cur_capture != capture) {
	    set_capture_hook(cur_capture,capture);
	    cur_capture = capture;
	}
	
	if (cur_capture != CAPTURE_OFF)
	    last_on = cur_capture;
    }
}

static void
set_norm(int j)
{
    int cap = cur_capture;

    if (cap != CAPTURE_OFF)
	set_capture(CAPTURE_OFF);

    cur_norm = j;
    if (grabber->grab_input)
	grabber->grab_input(-1,cur_norm);
    if (norm_notify)
	norm_notify();

    if (cap != CAPTURE_OFF)
	set_capture(cap);
}

static void
set_input(int j)
{
    cur_input = j;
    if (grabber->grab_input)
	grabber->grab_input(cur_input,-1);
    if (input_notify)
	input_notify();
}

static void
set_attr(int id, int val)
{
    int i;

    for (i = 0; i < NUM_ATTR; i++)
	if (id == pict_attr[i].id)
	    break;	
    if (i == NUM_ATTR)
	return;

    *(pict_attr[i].val) = val;
    grabber->grab_setattr(id,val);
    if (attr_notify)
	attr_notify(id);
}

static void
set_freqtab(int j)
{
    chantab   = j;
    chanlist  = chanlists[chantab].list;
    chancount = chanlists[chantab].count;

    /* cur_channel might be invalid (>chancount) right now */
    cur_channel = -1;
    /* this is valid for (struct CHANNEL*)->channel too    */
    calc_frequencies();

    if (freqtab_notify)
	freqtab_notify();
}


static void
set_title()
{
    static char  title[256];

    keypad_state = -1;
    if (update_title) {
	if (-1 != cur_sender) {
#if 0
	    sprintf(title,"%d - %s",cur_sender+1,channels[cur_sender]->name);
#endif
	    sprintf(title,"%s",channels[cur_sender]->name);
	} else if (-1 != cur_channel) {
	    sprintf(title,"channel %s",chanlist[cur_channel].name);
	    if (cur_fine != 0)
		sprintf(title+strlen(title)," (%d)",cur_fine);
	    sprintf(title+strlen(title)," (%s/%s)",
		    grabber->norms[cur_norm].str,
		    chanlists[chantab].name);
	} else {
	    sprintf(title,"???");
	}
	update_title(title);
    }
}

static void
set_msg_int(char *name, int val)
{
    static char  title[256];
    
    if (display_message) {
	sprintf(title,"%s: %d%%",name,val*100/65535);
	display_message(title);
    }
}

static void
set_msg_str(char *name, char *val)
{
    static char  title[256];
    
    if (display_message) {
	sprintf(title,"%s: %s",name,val);
	display_message(title);
    }
}

/* ----------------------------------------------------------------------- */

#define STEP (65536/100)

static int update_int(int old, char *new)
{
    int ret = old;
    
    if (0 == strcasecmp(new,"inc"))
        ret += STEP;
    else if (0 == strcasecmp(new,"dec"))
	ret -= STEP;
    else if (new[0] == '+')
	ret += attr_to_int(new+1);
    else if (new[0] == '-')
	ret -= attr_to_int(new+1);
    else if (isdigit(new[0]))
	ret = attr_to_int(new);
    else
	fprintf(stderr,"update_int: can't parse %s\n",new);

    if (ret < 0)     ret = 0;
    if (ret > 65535) ret = 65535;

    return ret;
}

/* ----------------------------------------------------------------------- */

void
attr_init()
{
    if (grabber->grab_hasattr(GRAB_ATTR_COLOR)) {
	cur_color = grabber->grab_getattr(GRAB_ATTR_COLOR);
	if (attr_notify)
	    attr_notify(GRAB_ATTR_COLOR);
    }
    if (grabber->grab_hasattr(GRAB_ATTR_BRIGHT)) {
	cur_bright = grabber->grab_getattr(GRAB_ATTR_BRIGHT);
	if (attr_notify)
	    attr_notify(GRAB_ATTR_BRIGHT);
    }
    if (grabber->grab_hasattr(GRAB_ATTR_HUE)) {
	cur_hue = grabber->grab_getattr(GRAB_ATTR_HUE);
	if (attr_notify)
	    attr_notify(GRAB_ATTR_HUE);
    }
    if (grabber->grab_hasattr(GRAB_ATTR_CONTRAST)) {
	cur_contrast = grabber->grab_getattr(GRAB_ATTR_CONTRAST);
	if (attr_notify)
	    attr_notify(GRAB_ATTR_CONTRAST);
    }
}


void
set_defaults()
{
    /* image parameters */
    set_attr(GRAB_ATTR_COLOR,   defaults.color);
    set_attr(GRAB_ATTR_BRIGHT,  defaults.bright);
    set_attr(GRAB_ATTR_HUE,     defaults.hue);
    set_attr(GRAB_ATTR_CONTRAST,defaults.contrast);
    set_capture(defaults.capture);
    
    /* input source */
    if (cur_input   != defaults.input)
	set_input(defaults.input);
    if (cur_norm    != defaults.norm)
	set_norm(defaults.norm);
    
    /* station */
    cur_channel  = defaults.channel;
    cur_fine     = defaults.fine;
    cur_freq     = defaults.freq;
    if (grabber->grab_tune)
	grabber->grab_tune(defaults.freq,defaults.sat);
}

/* ----------------------------------------------------------------------- */

static int setstation_handler(char *name, int argc, char **argv)
{
    int i;

    if (0 == argc) {
	set_title();
	return 0;
    }
    
    if (count && 0 == strcasecmp(argv[0],"next")) {
	i = (cur_sender+1) % count;
    } else if (count && 0 == strcasecmp(argv[0],"prev")) {
	i = (cur_sender+count-1) % count;
    } else if (count && 0 == strcasecmp(argv[0],"back")) {
	if (-1 == last_sender)
	    return -1;
	i = last_sender;
    } else {
	/* search the configured channels first... */
	for (i = 0; i < count; i++)
	    if (0 == strcasecmp(channels[i]->name,argv[0]))
		break;
	/* ... if it failes, take the argument as index */
	if (i == count)
	    i = atoi(argv[0]);
    }

    /* ok ?? */
    if (i < 0 || i >= count)
	return -1;
    
    /* switch ... */
    if (channel_switch_hook)
	channel_switch_hook();

    last_sender = cur_sender;
    cur_sender = i;

    if (!cur_mute) {
      /*DAG	if (have_mixer)
	    mixer_mute();
	    else*/
	    grabber->grab_setattr(GRAB_ATTR_MUTE,1);
    }

    /* image parameters */
    set_attr(GRAB_ATTR_COLOR,   channels[i]->color);
    set_attr(GRAB_ATTR_BRIGHT,  channels[i]->bright);
    set_attr(GRAB_ATTR_HUE,     channels[i]->hue);
    set_attr(GRAB_ATTR_CONTRAST,channels[i]->contrast);
    set_capture(channels[i]->capture);
    
    /* input source */
    if (cur_input   != channels[i]->input)
	set_input(channels[i]->input);
    if (cur_norm    != channels[i]->norm)
	set_norm(channels[i]->norm);
    
    /* station */
    cur_channel  = channels[i]->channel;
    cur_fine     = channels[i]->fine;
    cur_freq     = channels[i]->freq;
    if (grabber->grab_tune)
	grabber->grab_tune(channels[i]->freq,channels[i]->sat);
    
    set_title();
    if (setstation_notify)
	setstation_notify();

    if (!cur_mute) {
	usleep(2000);
	/*DAG	if (have_mixer)
	    mixer_unmute();
	    else*/
	    grabber->grab_setattr(GRAB_ATTR_MUTE,0);
    }
    return 0;
}

static int setchannel_handler(char *name, int argc, char **argv)
{
    int c,i;
    
    if (0 == argc) {
	set_title();
	return 0;
    }
    
    if (0 == strcasecmp(argv[0],"next")) {
	cur_channel = (cur_channel+1) % chancount;
	cur_fine = defaults.fine;
    } else if (0 == strcasecmp(argv[0],"prev")) {
	cur_channel = (cur_channel+chancount-1) % chancount;
	cur_fine = defaults.fine;
    } else if (0 == strcasecmp(argv[0],"fine_up")) {
	cur_fine++;
    } else if (0 == strcasecmp(argv[0],"fine_down")) {
	cur_fine--;
    } else {
	if (-1 != (c = lookup_channel(argv[0]))) {
	    cur_channel = c;
	    cur_fine = defaults.fine;
	}
    }

    if (0 != strncmp(argv[0],"fine",4)) {
	/* look if there is a known station on that channel */
	for (i = 0; i < count; i++) {
	    if (cur_channel == channels[i]->channel) {
		char *argv[2];
		argv[0] = channels[i]->name;
		argv[1] = NULL;
		return setstation_handler("", argc, argv);
	    }
	}
    }

    
    if (channel_switch_hook)
	channel_switch_hook();

    cur_sender  = -1;
    cur_freq = get_freq(cur_channel)+cur_fine;

    if (!cur_mute) {
      /*DAG	if (have_mixer)
	    mixer_mute();
	    else*/
	    grabber->grab_setattr(GRAB_ATTR_MUTE,1);
    }

    set_capture(defaults.capture);    
    if (grabber->grab_tune)
	grabber->grab_tune(cur_freq,-1);

    set_title();
    if (setstation_notify)
	setstation_notify();

    if (!cur_mute) {
	usleep(2000);
	/*DAG	if (have_mixer)
	    mixer_unmute();
	    else*/
	    grabber->grab_setattr(GRAB_ATTR_MUTE,0);
    }
    return 0;
}

/* ----------------------------------------------------------------------- */

static void
print_choices(char *name, char *value, struct STRTAB *tab)
{
    int i;
    
    fprintf(stderr,"unknown %s: '%s' (available: ",name,value);
    for (i = 0; tab[i].str != NULL; i++)
	fprintf(stderr,"%s'%s'", (0 == i) ? "" : ", ", tab[i].str);
    fprintf(stderr,")\n");
}

static int setfreqtab_handler(char *name, int argc, char **argv)
{
    int i;

    i = str_to_int(argv[0],chanlist_names);
    if (i != -1)
	set_freqtab(i);
    else
	print_choices("freqtab",argv[0],chanlist_names);
    return 0;
}

static int norm_handler(char *name, int argc, char **argv)
{
    int i;

    i = str_to_int(argv[0],grabber->norms);
    if (i != -1)
	set_norm(i);
    else
	print_choices("norm",argv[0],grabber->norms);
    return 0;
}

static int input_handler(char *name, int argc, char **argv)
{
    int i;

    if (0 == strcmp(argv[0],"next")) {
	i = cur_input+1;
	if (grabber->inputs[i].str == NULL)
	    i = 0;
    } else {
	i = str_to_int(argv[0],grabber->inputs);
    }
    if (i != -1)
	set_input(i);
    else
	print_choices("input",argv[0],grabber->inputs);
    return 0;
}

static int capture_handler(char *name, int argc, char **argv)
{
    int i;

    if (0 == strcasecmp(argv[0],"toggle")) {
	i = (cur_capture == CAPTURE_OFF) ? CAPTURE_ON : CAPTURE_OFF;
    } else {
	i = str_to_int(argv[0],captab);
    }
    if (i != -1)
	set_capture(i);
    return 0;
}

/* ----------------------------------------------------------------------- */

static int volume_handler(char *name, int argc, char **argv)
{
    if (0 == argc)
	goto display;
    
    if (0 == strcasecmp(argv[0],"mute")) {
	/* mute on/off/toggle */
	if (argc > 1) {
	    switch (str_to_int(argv[1],booltab)) {
	    case 0:  cur_mute = 0; break;
	    case 1:  cur_mute = 1; break;
	    default: cur_mute = !cur_mute; break;
	    }
	} else {
	    cur_mute = !cur_mute;
	}
    } else {
	/* volume */
	cur_volume = update_int(cur_volume,argv[0]);
    }
    /*DAG    set_volume();*/

 display:
    if (cur_mute)
	set_msg_str("volume","muted");
    else
	set_msg_int("volume",cur_volume);
    return 0;
}

static int attr_handler(char *name, int argc, char **argv)
{
    int i;
    
    for (i = 0; i < NUM_ATTR; i++)
	if (0 == strcasecmp(pict_attr[i].name,name))
	    break;	
    if (i == NUM_ATTR)
	return -1;
    if (argc > 0)
	set_attr(pict_attr[i].id,update_int(*(pict_attr[i].val),argv[0]));
    set_msg_int(name,*(pict_attr[i].val));
    return 0;
}

/* ----------------------------------------------------------------------- */

static int snap_handler(char *hname, int argc, char **argv)
{
    char message[512];
    void *buffer;
    char *filename = NULL;
    char *name;
    int   jpeg = 0;
    int   ret = 0;
    int   width;
    int   height;
    int   linelength;

    if (!grabber->grab_setparams ||
	!grabber->grab_capture) {
	missing_feature(MISSING_CAPTURE);
	fprintf(stderr,"grabbing: not supported\n");
	return -1;
    }

    /* format */
    if (argc > 0) {
	if (0 == strcasecmp(argv[0],"jpeg"))
	    jpeg = 1;
	if (0 == strcasecmp(argv[0],"ppm"))
	    jpeg = 0;
    }

    /* size */
    width  = 2048;
    height = 1572;
    if (argc > 1) {
	if (0 == strcasecmp(argv[1],"full")) {
	    /* nothing */
	} else if (0 == strcasecmp(argv[1],"win")) {
	    width  = grab_width;
	    height = grab_height;
	} else if (2 == sscanf(argv[1],"%dx%d",&width,&height)) {
	    /* nothing */
	} else {
	    return -1;
	}
    }

    /* filename */
    if (argc > 2)
	filename = argv[2];
    
    if (0 != grabber_setparams(VIDEO_RGB24,&width,&height,
			       &linelength,0) ||
	NULL == (buffer = grabber_capture(NULL,0,1,NULL))) {
	if (display_message)
	    display_message("grabbing failed");
	ret = -1;
	goto done;
    }

    if (NULL == filename) {
	if (-1 != cur_sender) {
	    name = channels[cur_sender]->name;
	} else if (-1 != cur_channel) {
	    name = chanlist[cur_channel].name;
	} else {
	    name = "???";
	}
	filename = snap_filename(snapbase, name, jpeg ? "jpeg" : "ppm");
    }

    if (jpeg) {
#ifdef HAVE_LIBJPEG
	if (-1 == write_jpeg(filename,buffer,width,height, jpeg_quality, 0)) {
	    sprintf(message,"open %s: %s\n",filename,strerror(errno));
	} else {
	    sprintf(message,"saved jpeg: %s",filename);
	}
#else
	missing_feature(MISSING_JPEG);
#endif
    } else {
	if (-1 == write_ppm(filename,buffer,width,height)) {
	    sprintf(message,"open %s: %s\n",filename,strerror(errno));
	} else {
	    sprintf(message,"saved ppm: %s",filename);
	}
    }
    if (grabber->grab_cleanup)
	grabber->grab_cleanup();
    if (display_message)
	display_message(message);

    /* set parameters to main window size (for grabdisplay) */
done:
    if (reconfigure_hook)
	reconfigure_hook();
    return ret;
}

static int
fullscreen_handler(char *name, int argc, char **argv)
{
    if (fullscreen_hook)
	fullscreen_hook();
    return 0;
}

static int
msg_handler(char *name, int argc, char **argv)
{
    if (display_message)
	display_message(argv[0]);
    return 0;
}

static int
exit_handler(char *name, int argc, char **argv)
{
    if (exit_hook)
	exit_hook();
    return 0;
}

/* ----------------------------------------------------------------------- */

static int
keypad_handler(char *name, int argc, char **argv)
{
    int n = atoi(argv[0])%10;
    char msg[8],ch[8];

    if (debug)
	fprintf(stderr,"keypad: key %d\n",n);
    if (-1 == keypad_state) {
	if (n > 0 && n <= (keypad_ntsc ? 99 : count)) {
	    if (keypad_ntsc) {
		sprintf(ch,"%d",n);
		do_va_cmd(2,"setchannel",ch,NULL);
	    } else
		do_va_cmd(2,"setstation",channels[n-1]->name,NULL);
	}
	if (n*10 <= (keypad_ntsc ? 99 : count)) {
	    if (debug)
		fprintf(stderr,"keypad: hang: %d\n",n);
	    keypad_state = n;
	    if (display_message) {
		sprintf(msg,"%d_",n);
		display_message(msg);
	    }
	}
    } else {
	n += keypad_state*10;
	keypad_state = -1;
	if (debug)
	    fprintf(stderr,"keypad: ok: %d\n",n);
	if (n > 0 && n <= (keypad_ntsc ? 99 : count)) {
	    if (keypad_ntsc) {
		sprintf(ch,"%d",n);
		do_va_cmd(2,"setchannel",ch,NULL);
	    } else
		do_va_cmd(2,"setstation",channels[n-1]->name,NULL);
	}
    }
    return 0;
}

void
keypad_timeout(void)
{
    if (debug)
	fprintf(stderr,"keypad: timeout\n");
    if (keypad_state == cur_sender+1)
	set_title();
    keypad_state = -1;
}
