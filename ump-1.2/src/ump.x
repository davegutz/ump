#!/bin/sh
# test installation of ump application program.  Run in playback mode
# and compare images to stored images.

# Initialize
warmup=0
total=8
auto=0
calibrate=0
interactive=1
realtime=0
help=0
version=0
still=0
redo=0
size="160x120"
sub=1
timeWait=5
repeat=0
ERRSTAT_MEM=0
useLight=1

# getopt  from /usr/lib/getopt/parse.bash
TEMP=`getopt -o A:aC:hnRrSs:t:u:V --long AutoRepeat:,auto,calibrate:,help,noninteractive,redo,realtime,still,size:,total:,sub:,version \
      -n 'ump.x' -- "$@"`
if [ $? != 0 ] ; then
    echo "USAGE($0):  $0 [-A:aC:hnRrSs:t:u:V]" >&2
    echo "Enter to quit" >&2
    read ans
    exit 1
fi
eval set -- "$TEMP"
while true ; do
    case "$1" in
	-A|--AutoRepeat)     auto=1; repeat=1; timeWait="$2"; shift 2 ;;
	-a|--auto)           auto=1;           shift ;;
	-C|--calibrate)      calibrate=1; warmup="$2"; shift 2 ;;
	-h|--help)           help=1;           shift ;;
	-V|--version)        version=1;        shift ;;
	-n|--noninteractive) interactive=0; useLight=0;   shift ;;
	-R|--redo)           redo=1;           shift ;;
	-r|--realtime)       realtime=1 ;      shift ;;
        -S|--still)          still=1;          shift ;;
	-s|--size)           size="$2";      shift 2 ;;
	-t|--total)          total="$2";     shift 2 ;;
	-u|--sub)            sub="$2";       shift 2 ;;
	--) shift ; break ;;
	*) echo "ERROR($0): bad option"; 
	    echo "Enter to quit"
	    read ans;
	    break ;;
    esac
done

if ( test $help -eq "1" )
    then
    echo "Runs ump program and turns on lpt port pins depending upon"
    echo "the results."
    echo "usage"
    echo "  $0 [options]"
    echo "  options:"
    echo " -A <t>,  --AutoRepeat<t>   run automatically in this directory"
    echo "                       specify time between shots [$timeWait]"
    echo " -a,  --auto           run automatically in this directory"
    echo " -C <f>,  --calibrate <f>   run from end-end in Blink"
    echo "                       specify  frames of warmup [$warmup]"
    echo " -h,  --help           this screen and exit success"
    echo " -V,  --version        print version and exit success"
    echo " -n,  --noninteractive turn off the interactive - for self tests"
    echo " -R,  --redo           recalculate only, no new shots"
    echo " -r,  --realtime       run ump in real time mode"
    echo " -S,  --still          still shots with white&black balls alternately"
    echo " -s,  --size           specify raw size [$size] for stills only"
    echo " -t,  --total          specify frames [$total] for calibration only"
    echo " -u,  --sub            specify subpixel sampling integer [$sub]"
    exit 0
fi

if ( test $version -eq "1" )
    then
    echo '$Id: ump.x,v 1.2 2010/01/10 11:43:22 davegutz Exp $'
    echo "Written by Dave Gutz, Copyright (C) 2006"
    exit 0
fi

# Calculations
sizelen=`echo $size | wc -c`
imWidthCalEx=`echo $size|cut -d"x" -f1`
imHeightCalEx=`echo $size|cut -d"x" -f2`
focalCalEx=`echo $imWidthCalEx  | awk '{print $1*179/160}'`
if ( test "$imWidthCalEx" -lt "80" ||
     test "$imWidthCalEx" -gt "640" ||
     test "$sizelen" -lt "4"        )
    then
    echo "ERROR($0): bad size: $size."
    echo "Image width should be between 80 & 640 inclusive"; 
    exit 1;
fi

# Go to directory
if ( test "$interactive" -eq "1" ||
     test "$realtime" -eq "1"    ||
     test "$calibrate" -eq "1"   ||
     test "$auto" -eq "1"         ) 
    then
    DATA=.
else
    DATA=$srcdir/../data/X
fi
cd $DATA
if ( test $? -ne 0 )
    then
    echo "Run from ump package src directory or define 'srcdir'"
    exit 1
fi

# end-end test called from Blink
if ( test $calibrate -eq "1" )
    then
    echo "MESSAGE($0): >temp ump -C -ical0.raw -ical1.raw -w$warmup -t$total 2>&1"
    >temp ump -C -ical0.raw -ical1.raw -w$warmup -t$total $* 2>&1
    grep -q ERROR temp; ERRSTAT=$?
    if ( test "$ERRSTAT" -eq "0" )
	then
	grep ERROR temp
	echo "ERROR($0):  ump failed"
	exit 1;
    fi
    grep "BALL_POSITION" temp  >result
    grep "BALL_POSITION" temp | cut -c29-62 |
    awk 'BEGIN{IFS=" "; xa=0; xb=0; xc=0; n=0;xax=-10000;xan=10000; \
                     xbx=-10000;xbn=10000;xcx=-10000;xcn=10000;       }\
       { xa=xa+$1; xb=xb+$2; xc=xc+$3; n=n+1;                         \
         if ($1>xax) xax=$1; if ($1<xan) xan=$1;                      \
         if ($2>xbx) xbx=$2; if ($2<xbn) xbn=$2;                      \
         if ($3>xcx) xcx=$3; if ($3<xcn) xcn=$3;                      }\
         END{if (n==0) printf "ump.x: NO DETECTIONS\n"; else printf "******************AVERAGE=    %s      %s      %s \nN=%s, RANGE= (%s|%s|%s, %s|%s|%s, %s|%s|%s)\n",  \
   xa/n, xb/n, xc/n, n, xan, xax,xax-xan, xbn, xbx, xbx-xbn, xcn, xcx, xcx-xcn}' >>result
    cat result
    cat result >> temp
    exit 0
fi

# Real time operation
if ( test $realtime -eq "1" )
    then
    ump -R2 $*
    if ( test "$?" -eq "1" )
	then
	echo "ERROR($0):  ump failed"
	echo "Return to close window"
	read ans
	exit 1
    else
	echo "MESSAGE($0): Done."
	echo "Return to close window"
	read ans
	exit 0
    fi
fi

# Check operation with pre-recorded data files.
if ( test $interactive -ne "1" )
    then
    if (   test -s xLas.raw    \
        && test -s xLbs.raw    \
        && test -s xLas.tim    \
        && test -s xLbs.tim    \
        && test -s xLas.set    \
        && test -s xLbs.set    \
        && test -s video0.cal.L\
        && test -s video1.cal.L\
        && test -s xLs_images/tempRC0001.ppm \
        && test -s xLs_images/tempRC1001.ppm )
	then
	echo ""
	echo "$0: running ump on xLas and xLbs..."
	echo ""
	echo ""
	cp -f video0.cal.L video0.cal
	cp -f video1.cal.L video1.cal
    else
	echo ""
	echo ""
	echo "$0: data files not found.  Download and install umpsamp.tar\ to run demos."
	echo ""
	echo ""
	echo "Enter to quit this window"
	read ans
	exit 77
    fi
fi

# Loop
rm -f temp
trap 'killUmp >/dev/null; rm -f temp1 temp2 temp3;lptout 0;xset b' 0
IFS="\t"
ans="d"
while ( test "$ans " != "q " )
  do
  ERRSTAT_MEM=0
  if ( test "$still" = "0" )  # non-stills
  then
      if ( test "$useLight " -eq "1 " )
	  then
	  echo "$0: trying to turn off light on lpt port"
	  lptout 0
	  if ( test $? -ne 0 )
	      then
	      echo "WARNING($0):  light not connected at"
	      echo "lpt port.  Will not try light anymore.  Continuing..."
	      useLight=0
	      sleep 2
	  fi
      fi
      if ( test "$ans " = "d " && test "$interactive" = "0" )
	  then
	  echo "$0:  Running demo...."
	  xset b 100 500 400
	  ump -w0 -ixLas.raw -ixLbs.raw $* | sed '/Final/d' > temp1
	  grep -q ERROR temp1; ERRSTAT=$?
	  if ( test "$ERRSTAT" -eq "0" )
	      then
	      grep ERROR temp1
	      echo "ERROR($0):  ump failed"
	      exit 1;
	  fi
	  if ( test `grep KEY temp1 | wc -l` -gt 0 )
	      then
	      date
	      cat temp1 >> temp
	      tail -n26 temp | tee temp2
	      if ( test `grep "CALLED STRIKE" temp2 | wc -l` -gt 0)
		  then
		  if ( test "$useLight " -eq "1 " )
		      then
		      echo "$0: lighting pin 3"
		      lptout 2
		  fi
		  xterm -sl 1000 -fg Black -bg Red -title "STRIKE!!" -geom 180x60 -e sleep 5 &
		  xset b 100 700 2000; printf "\a";
	      elif ( test `grep "CALLED BALL" temp2 | wc -l` -gt 0)
		  then
		  if ( test "$useLight " -eq "1 " )
		      then
		      echo "$0: lighting pin 2"
		      lptout 1
		  fi
		  xterm -sl 1000 -fg Black -bg Green -title "BALL!!" -geom 180x60 -e sleep 5 &
		  xset b 100 200 1000; printf "\a";
	      else
		  if ( test "$useLight " -eq "1 " )
		      then
		      lptout 0
		  fi
	      fi
	  else
	      date
	      echo "NO CALL"
	      if ( test "$useLight " -eq "1 " )
		  then
		  echo "$0: lighting pin 4"
		  lptout 4
	      fi
	      xterm -sl 1000 -fg Black -bg Yellow -title "NO CALL!!" -geom 180x60 -e sleep 5 &
	  fi
	  echo "q to quit, s to replay pitch, d for demo:"
	  read ans
	  if ( test "$ans " = "q " )
	      then
	      exit 0
	  elif ( test "$ans " = "s " )
	      then
	      showppms
	  fi
	  echo "q to quit, d for demo"
	  read ans
	  if ( test "$ans " = "q " )
	      then
	      exit 0
	  fi
      else  # if ( test "$ans " = "d " && test "$interactive" = "0" )
	  if ( test "$redo " -eq "1 " )  # redo
	      then
	      echo "Running detection..."
	      >temp ump -ixA.raw -ixB.raw -S "$sub" $* 2>&1
	      grep -q ERROR temp; ERRSTAT=$?
	      if ( test "$ERRSTAT" -eq "0" )
		  then
		  ERRSTAT_MEM=1
		  grep ERROR temp
		  echo "ERROR($0):  ump failed"
	      else
		  cat temp
	      fi
	      exit 0
	  else # not redo
	      echo "Acquiring and running...."
	      xset b 100 500 400
	      set -x
	      if ( test "$auto " = "1 " )
		  then
		  ump -oxA.raw -oxB.raw -s"$size" -S"$sub" -t90 $* |
		  sed '/Final/d' | sed '/streamery/d' >> temp1
	      else
		  ump -oxA.raw -oxB.raw -s"$size" -S"$sub" -t90 $* |
		  sed '/Final/d' | sed '/streamery/d' >> temp1
	      fi
	      set +x
	      grep ERROR temp3; ERRSTAT=$?
	      if ( test "$ERRSTAT " -eq "0 " )
		  then
		  ERRSTAT_MEM=1
		  grep ERROR temp1
		  echo "ERROR($0):  ump failed"
	      fi
	      if ( test "$ERRSTAT_MEM " -eq "0 " )
		  then
		  if ( test `grep KEY temp1 | wc -l` -gt 0 )
		      then
		      date
		      cat temp1 >> temp
		      tail -n26 temp | tee temp2
		      if ( test `grep "CALLED STRIKE" temp2 | wc -l` -gt 0)
			  then
			  if ( test "$useLight " -eq "1 " )
			      then
			      echo "$0: lighting pin 3"
			      lptout 2
			  fi
			  xterm -sl 1000 -fg Black -bg Red -title "STRIKE!!" -geom 180x60 -e sleep 5 &
			  xset b 100 700 2000; printf "\a";
		      elif ( test `grep "CALLED BALL" temp2 | wc -l` -gt 0)
			  then
			  if ( test "$useLight " -eq "1 " )
			      then
			      echo "$0: lighting pin 2"
			      lptout 1
			  fi
			  xterm -sl 1000 -fg Black -bg Green -title "BALL!!" \
			      -geom 180x60 -e sleep 5 &
			  xset b 100 200 1000; printf "\a";
		      else
			  if ( test "$useLight " -eq "1 " )
			      then
			      lptout 0
			  fi
		      fi
		  else
		      date
		      echo "NO CALL"
		      if ( test "$useLight " -eq "1 " )
			  then
			  echo "$0: lighting pin 4"
			  lptout 4
		      fi
		      xterm -sl 1000 -fg Black -bg Yellow -title "NO CALL" \
			  -geom 180x60 -e sleep 5 &
		  fi
	      fi  #ERRSTAT_MEM
	      if ( test "$auto" -eq "1" )
		  then
		  saveppms
		  printf "$0:  Sleeping $timeWait seconds"
		  if ( test "$repeat" -eq "1" )
		      then
		      printf " then continuing....\n"
		      sleep $timeWait
		      ans=""
		  else
		      printf " then quitting....\n"
		      sleep $timeWait
		      ans=q
		  fi
	      else  # not auto
		  echo "q to quit, s to replay pitch, enter to reaquire"
		  read ans
		  if ( test "$ans " = "q " )
		  then
		      exit 0
		  elif ( test "$ans " = "s " )
		      then
		      if ( test -s tempRC0001.ppm && test -s tempRC1001.ppm ) 
			  then
		      showppms
		      else
			  echo "$0:  no images."
		      fi
		      echo "q to quit, s to save pitch, enter to reaquire"
		      read ans
		      if ( test "$ans " = "q " )
		      then
			  exit 0
		      elif ( test "$ans " = "s " )
			  then
			  if ( test -s tempRC0001.ppm && test -s tempRC1001.ppm ) 
			      then
			  saveppms
			  else
			      echo "$0:  no images."
			  fi  # test ppm
		      fi  # ans
		  fi   # ans
	      fi  # auto
	  fi  # redo
      fi    # if ( test "$ans " = "d " && test "$interactive" = "0" )
  else  # stills
      if ( test "$redo " = "0 " )
	  then
	  echo "Acquiring and running stills...."
	  printf "\nPut white ball in place. Enter when ready > "
	  read ans
	  printf "Using size= $size\n"
	  >tempBallZero \
	      ump -s"$size" -S"$sub" -c/dev/video0 -oballZero.raw -t90 $* 2>&1
	  grep -q ERROR tempBallZero; ERRSTAT=$?
	  if ( test "$ERRSTAT" -eq "0" )
	      then
	      ERRSTAT_MEM=1
	      grep ERROR tempBallZero
	      echo "ERROR($0):  ump failed"
	  fi
	  >tempBallOne \
	      ump -s"$size" -S"$sub" -c/dev/video1 -oballOne.raw  -t90 $* 2>&1
	  grep -q ERROR tempBallOne; ERRSTAT=$?
	  if ( test "$ERRSTAT" -eq "0" )
	      then
	      ERRSTAT_MEM=1
	      grep ERROR tempBallOne
	      echo "ERROR($0):  ump failed"
	  fi
	  printf "\nPut black ball in place. Enter when ready > "
	  read ans
	  >tempBlankZero \
	      ump -s"$size" -S"$sub" -c/dev/video0 -oblankZero.raw -t90 $* 2>&1
	  grep -q ERROR tempBlankZero; ERRSTAT=$?
	  if ( test "$ERRSTAT" -eq "0" )
	      then
	      ERRSTAT_MEM=1
	      grep ERROR tempBlankZero
	      echo "ERROR($0):  ump failed"
	  fi
	  >tempBlankOne \
	      ump -s"$size" -S"$sub" -c/dev/video1 -oblankOne.raw  -t90 $* 2>&1
	  grep -q ERROR tempBlankOne; ERRSTAT=$?
	  if ( test "$ERRSTAT" -eq "0" )
	      then
	      ERRSTAT_MEM=1
	      grep ERROR tempBlankOne
	      echo "ERROR($0):  ump failed"
	  fi
	  if ( test "$ERRSTAT_MEM" -eq "0" )
	      then
	      echo "Slicing..."
	      sleep 1;
	      sliceraw -iballZero.raw  -oballZeros.raw  -t0.6:2
	      sleep 1;
	      sliceraw -iblankZero.raw -oblankZeros.raw -t0.6:2
	      sleep 1;
	      sliceraw -iballOne.raw   -oballOnes.raw   -t0.6:2
	      sleep 1;
	      sliceraw -iblankOne.raw  -oblankOnes.raw  -t0.6:2
	      # Let last sliceraw add time bias to dev 1 by synchronizing here
	      echo "Synchronizing..."
	      cp ballZeros.tim ballOnes.tim
	      echo "Shuffling..."
	      sleep 1;
	      sliceraw -iballZeros.raw -iblankZeros.raw -oxA.raw 
	      sleep 1;
	      sliceraw -iballOnes.raw  -iblankOnes.raw  -oxB.raw
	  fi  #ERRSTAT_MEM
      fi  #redo
      if ( test "$ERRSTAT_MEM" -eq "0" )
	  then
	  echo "Running detection..."
	  >temp ump -ixA.raw -ixB.raw -w0 -C -U -S "$sub" $* 2>&1
	  grep -q ERROR temp; ERRSTAT=$?
	  if ( test "$ERRSTAT" -eq "0" )
	      then
	      ERRSTAT_MEM=1
	      grep ERROR temp
	      echo "ERROR($0):  ump failed"
	  fi
      fi  #ERRSTAT_MEM
      if ( test "$ERRSTAT_MEM" -eq "0" )
	  then
	  grep "BALL_POSITION" temp  >result
	  grep "BALL_POSITION" temp | cut -c29-62 |
	  awk 'BEGIN{IFS=" "; xa=0; xb=0; xc=0; n=0;xax=-10000;xan=10000; \
                     xbx=-10000;xbn=10000;xcx=-10000;xcn=10000;       }\
       { xa=xa+$1; xb=xb+$2; xc=xc+$3; n=n+1;                         \
         if ($1>xax) xax=$1; if ($1<xan) xan=$1;                      \
         if ($2>xbx) xbx=$2; if ($2<xbn) xbn=$2;                      \
         if ($3>xcx) xcx=$3; if ($3<xcn) xcn=$3;                      }\
         END{printf "AVERAGE= (%s, %s, %s) N=%s, RANGE= (%s|%s|%s, %s|%s|%s, %s|%s|%s)\n",  \
   xa/n, xb/n, xc/n, n, xan, xax,xax-xan, xbn, xbx, xbx-xbn, xcn, xcx, xcx-xcn}' >>result
	  cat result
	  cat result >> temp
      fi  #ERRSTAT_MEM
      if ( test "$redo " = "0 " )
	  then
	  echo "q to quit, s to save pitch, r to replay pitch, enter to reaquire"
	  read ans
	  if ( test "$ans " = "q " )
	      then
	      exit 0
	  elif ( test "$ans " = "s " )
	      then
	      if ( test -s tempRC0001.ppm && test -s tempRC1001.ppm ) 
		  then
		  saveppms
	      else
		  echo "$0:  no images."
		  echo "s to save pitch anyway"
		  read ans
		  if ( test "$ans " = "s " )
		      then
		      saveppms
		  fi
	      fi
	  elif ( test "$ans " = "r " )
	      then
	      if ( test "`ls tempRC0???.ppm|wc -l` " != "0 " \
		  && test "`ls tempRC1???.ppm|wc -l` " != "0 " ) 
		  then
		  showppms
	      else
		  echo "$0:  no images."
	      fi
	      echo "q to quit, s to save pitch, enter to reaquire"
	      read ans
	      if ( test "$ans " = "q " )
		  then
		  exit 0
	      elif ( test "$ans " = "s " )
		  then
		  if ( test -s tempRC0001.ppm && test -s tempRC1001.ppm ) 
		      then
		      saveppms
		  else
		      echo "$0:  no images."
		      echo "s to save pitch anyway"
		      read ans
		      if ( test "$ans " = "s " )
			  then
			  saveppms
		      fi
		  fi
	      fi
	  fi
      else
	  ans="q"
      fi
  fi
done
exit 0

