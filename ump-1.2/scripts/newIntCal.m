% newIntCal:   Method to internally calibrate camera with a table input.
% D Gutz 06/24/06
% AbConstant    Aberration constant to estimate aberration (see R/rhoE)		
% AbipimR	Aberration constant to estimate aberration (see R/rhoE)
% AbjpjmR	Aberration constant to estimate aberration (see R/rhoE)
% Abrqw2	Aberration constant to estimate aberration (see R/rhoE)
% abs(rE-R)	= abs(rE - R), magnitude of correction error, pixels
% ARC           Aspect ratio constant used to square up pixel measurements
%               in application.		
% CENTER PT.	The coordinates of the observed grid crossing closest to the
%               image center.		
% D             The distance from camera focal point to the center of the grid
%               board, inches.		
% dr/rhoEB	= -(R/rhoEB - R/rho), residual after second regression
% dr/rhoEIntA	= -(R / rhoEIntA - R / rho), residual after first regression
% imHeight      Height of image, pixels
% imWidth       Width of image, pixels
% ip-imTC       Predicted true curved pixels, pixels
% jm-jpTC       Predicted true curved pixels, pixels
% RTC           Predicted true radius, pixels
% RTC-R         Predicted true minus indicated, pixels
% ipCORR        Pixel correction to be added to raw pixels, pixels
% jpCORR        Pixel correction to be added to raw pixels, pixels
% ipTCE         Corrected predicted true curved indicated pixels
% jpTCE         Corrected predicted true curved indicated pixels
% ip            Pixel coordinate along imHeight, pixels
% ip-im         Coordinate from image center, pixels
% ipi-R         = ip-imR / imWidth, normalized pixel coordinate
% ip-imR	= ip - imR / imHeight, ip-im rotated by rotation, pixels     
% ip-imT        = x*pix/inch, true pixels		
% ip-imTE	= R/rhoE * ip-im, estimated true grid coordinate from image center,
%               pixels (rotation=0)		
% ipTE          = ip-imTE + imHeight/2,  estimated true grid coordinate along
%               imHeight, pixels (rotation=0)		
% jm-jp         Coordinate from image center, pixels		
% jm-jpR	= jm - jpR / imHeight, jp-jm rotated by rotation, pixels	  
% jm-jpT	jm-jpT = y*pix/inch, true pixels		
% jm-jpTE	= R/rhoE * jm-jp, estimated true grid coordinate from image center,
%               pixels (rotation=0)		
% jp            Pixel coordinatealong height, pixels		
% jpjmR         = ip-imR / imWidth, normalized pixel coordinate		
% jpTE          = imHeight / 2 - jm-jpTE,  estimated true grid coordinate along
%               width, pixels (rotation=0)		
% L             Grid size, user chosen units		
% Li            Calculated grid size along width, pixels		
% Lj            Calculated grid size along height, pixels		
% R             = sqrt(ip-imT^2  + jm-jpT^2)		
% R/rho         = R / rho, measured aberration, fraction		
% R/rhoE	= AbConstant + Abrhoqw2*rhoQwidth2 + AbipimR*ipimR + AbjpjmR*jpjmR,
%               estimated aberration		
% R/rhoEB	= AbConstant + Abrhoqw2*rhoQwidth2 + AbipimR*ipimR,
%               estimated aberration after second regression		
% R/rhoEIntA	= AbConstant + Abrhoqw2*rhoQwidth2, estimated aberration after
%               first regression		
% rE            = R/rhoE*rho, estimated true grid location, pixels		
% rE-R          = rE - R, correction error, pixels		
% rho           = sqrt(im-im^2  + jm-jp^2), distance from image center, pixels
% rhoQwidth2	= (rho / imHeight)^2, normalized image locaton		
% rotation	Relative angle of grid in image, radians		
% R-rho         = R - rho, measured aberration, pixels		
% scale         Distortion factor on height		
% Std Err	Standard deviation of the error rE-R, pixels		
% theta         Angle of line between image center and pixel, radians		
% thetaR	Rotated angle of line between image center and pixel, radians
% x,y           Actual board grid locations, inches.  0 assumed mid.		
% xR, yR	Adjusted grid locations projected onto sphere of radius D, inches.

% initialize
clear all;
close all;

% from umpIntCalDrive062106.xls "video1_aberration 160x120"
AbConstant =  1.021912862;
Abrqw2	   = -0.067794808;
AbipimR	   =  0.030570055;
AbjpjmR    = -0.000588694;
ARC        =  1.348171334;
scale      =  0.98899398;

% ip ij r  c
data0=[...
5	8	1	1
31	7	1	2
57	6	1	3
82	5	1	4
107	5	1	5
131	6	1	6
155	6	1	7
5	32	2	1
31	31	2	2
57	30	2	3
83	30	2	4
109	29	2	5
134	29	2	6
157	29	2	7
5	57	3	1
31	56	3	2
58	55	3	3
85	55	3	4
110	53	3	5
135	53	3	6
159	52	3	7
6	83	4	1
32	82	4	2
59	81	4	3
86	80	4	4
111	79	4	5
137	78	4	6
159     77	4	7
5	109	5	1
33	108	5	2
59	107	5	3
87	105	5	4
112	104	5	5
137	103	5	6
160	100	5	7
];
data1=[...
15	16	1	1
38	15	1	2
61	15	1	3
83	14	1	4
105	13	1	5
128	13	1	6
150	13	1	7
15	39	2	1
38	38	2	2
61	38	2	3
83	37	2	4
105	37	2	5
128	36	2	6
150	36	2	7
15	62	3	1
38	61	3	2
61	61	3	3
84	60.5	3	4
105	60	3	5
128	59	3	6
151	59	3	7
15	85	4	1
39	84	4	2
61	84	4	3
83	83	4	4
105	83	4	5
128	83	4	6
151	82	4	7
16	108	5	1
39	108	5	2
61	107	5	3
83	107	5	4
106	107	5	5
128	106	5	6
151	105	5	7
];

dataNull=[...
20  5
40  5
60  5
80  5
100 5
120 5
140 5
20  27
40  27
60  27
80  27
100 27
120 27
140 27
20  49
40  49
60  49
80  49
100 49
120 49
140 49
20  71
40  71
60  71
80  71
100 71
120 71
140 71
20  93
40  93
60  93
80  93
100 93
120 93
140 93];

dataVideo1=[...
24 6
42 4
60 4
79 5
97 4
116 4
133 4
24 22
42 22
60 21
79 21
97 21
115 21
133 21
24 39
42 39
60 39
79 39
97 39
115 39
133 39
24 57
42 57
60 56
79 56
97 56
116 56
134 56
24 74
42 74
60 74
79 74
97 74
116 74
   134 74];







L        = 1;  % units between real grid lines
imWidth  = 160; % image width pixels
imHeight = 120; % image height pixels

%data = data1;
%data = dataNull;
data = dataVideo1;

% Extract
if length(data) == 35,
    nr=5; nc=7;
else
    error('bad size data')
end 
for r=1:nr,
    for c=1:nc,
        index   = (r-1)*nc+c;
        ip(r,c) = data(index, 1);
        jp(r,c) = data(index, 2);
    end
end
midr = ceil(nr/2);
midc = ceil(nc/2);
for r=1:nr,
    for c=1:nc,
        ipmim(r,c) = ip(r,c) - imWidth/2;
        jmmjp(r,c) = imHeight/2 - jp(r,c);
    end
end

Li1 = abs(mean( ipmim(midr-1:midr+1,midc+1) - ipmim(midr-1:midr+1,midc   )));
Li2 = abs(mean( ipmim(midr-1:midr+1,midc)   - ipmim(midr-1:midr+1,midc-1 )));
Li  = mean( [Li1 Li2] ) / L;

Lj1 = abs(mean( jmmjp(midr+1,midc-1:midc+1) - jmmjp(midr,midc-1:midc+1   )));
Lj2 = abs(mean( jmmjp(midr,midc-1:midc+1)   - jmmjp(midr-1,midc-1:midc+1 )));
Lj  = mean( [Lj1 Lj2] ) / L;

SY = Li/Lj;
x(midr,midc)= ipmim(midr,midc)/Li;
y(midr,midc)= jmmjp(midr,midc)/Lj;
for r=1:nr,
    for c=1:nc,
        if r ~= midr | c ~= midc,
            x(r,c) = x(midr,midc) - L*(midc-c);
            y(r,c) = y(midr,midc) + L*(midr-r);
        end
    end
end
xR=x;
yR=y;

% calculate real grid rotation alpha
theta = 0;num = 0;
for c=1:nc,
    if c < midc,
        theta = theta +...
	   atan2(SY*(jmmjp(midr,midc)-jmmjp(midr,c)), ipmim(midr,midc)-ipmim(midr,c));
        num = num + 1;
    end
    if c > midc,
        theta = theta +...
            atan2(SY*(jmmjp(midr,c)-jmmjp(midr,midc)), ipmim(midr,c)-ipmim(midr,midc));
        num = num + 1;
    end
end
theta = theta / num;
phi = 0;num = 0;
for r=1:nr,
    if r < midr,
        phi = phi +...
          atan2(SY*(jmmjp(r,midc)-jmmjp(midr,midc)), ipmim(r,midc)-ipmim(midr,midc));
        num = num + 1;
    end
    if r > midr,
        phi = phi +...
          atan2(SY*(jmmjp(midr,midc)-jmmjp(r,midc)), ipmim(midr,midc)-ipmim(r,midc));
        num = num + 1;
    end
end
phi = phi / num - pi()/2;
alpha = mean([phi theta]);


% Translate to pixels and rotate
for r=1:nr,
    for c=1:nc,
        ipmimT(r,c)  = xR(r,c)*Li;
        jmmjpT(r,c)  = yR(r,c)*Lj*SY;
        R(r,c)       = sqrt(ipmimT(r,c)^2 + jmmjpT(r,c)^2);
        gamma(r,c)   = atan2(jmmjpT(r,c), ipmimT(r,c));
        di(r,c)      = -R(r,c)*alpha*sin(gamma(r,c));
        dj(r,c)      =  R(r,c)*alpha*cos(gamma(r,c));
        dz(r,c)      = sqrt(di(r,c)^2 + dj(r,c)^2);
        ipmimTC(r,c) = ipmimT(r,c) + di(r,c);
        jmmjpTC(r,c) = jmmjpT(r,c) + dj(r,c);
        RTC(r,c)     = sqrt(ipmimTC(r,c)^2 + jmmjpTC(r,c)^2);
        RTCmR(r,c)   = RTC(r,c) - R(r,c);
        ipCORR(r,c)  =   ipmimTC(r,c) - ipmim(r,c);
        jpCORR(r,c)  = -(jmmjpTC(r,c) - jmmjp(r,c));
        ipTCE(r,c)   = ipmimTC(r,c) + imWidth/2;
        jpTCE(r,c)   = imHeight/2   - jmmjpTC(r,c);
    end
end

% construct 2-d table
for c=1:nc,
	iCORR(c) = Li*(c-midc);
end
for r=1:nr,
	jCORR(r) = Lj*(r-midr);
end
for r=1:imHeight,
   jpm = (r-imHeight/2);
   for c=1:imWidth,
      ipm =  c-imWidth /2;
      IPCORR(r,c)=  tab2(ipm, jpm, iCORR, jCORR, ipCORR);
      JPCORR(r,c)=  tab2(ipm, jpm, iCORR, jCORR, jpCORR);
   end
end

% Test it
for r=1:nr,
    for c=1:nc,
	    ipcorr(r,c) = ip(r,c) + IPCORR(floor(jp(r,c)), floor(ip(r,c)));
            jpcorr(r,c) = jp(r,c)*SY + JPCORR(floor(jp(r,c)), floor(ip(r,c)));
    end
end

ipStr=['ipGrid = ['];
jpStr=['jpGrid = ['];
for r=1:nr, 
   for c=1:nc,
	ipStr = [ipStr sprintf("%4d", ip(r,c))];
	jpStr = [jpStr sprintf("%4d", jp(r,c))];
   end
   if r ~= nr,
      ipStr = [ipStr sprintf("\n          ")];
      jpStr = [jpStr sprintf("\n          ")];
   end
end
ipStr = [ipStr sprintf("  ]\n")];
jpStr = [jpStr sprintf("  ]\n")];
disp(ipStr)
disp(jpStr)




% Old aberration calculations for reference
for r=1:nr,
    for c=1:nc,
        rho(r,c) = sqrt(ipmim(r,c)^2 + jmmjp(r,c)^2);
        thetaR(r,c) = atan2(jmmjp(r,c), ipmim(r,c));
        ipmimR(r,c) = rho(r,c)*cos(thetaR(r,c));
        jmmjpR(r,c) = rho(r,c)*sin(thetaR(r,c));
        rmRho(r,c)  = R(r,c) - rho(r,c);
        rqRho(r,c)  = R(r,c)/rho(r,c);
        rhoQwidth2(r,c)  = (rho(r,c)/imWidth)^2;
        rqrhoEIntA(r,c)  = AbConstant + Abrqw2*rhoQwidth2(r,c);
        drqrhoEIntA(r,c) = rqRho(r,c) - rqrhoEIntA(r,c);
        ipimR(r,c)       = ipmimR(r,c)/imWidth;
        rqrhoEB(r,c)     = AbConstant + Abrqw2*rhoQwidth2(r,c) + AbipimR*ipimR(r,c);
        drqrhoEB(r,c)    = rqRho(r,c) - rqrhoEB(r,c);
        jpjmR(r,c)       = jmmjpR(r,c)/imWidth;
        rqrhoE(r,c)      = AbConstant + Abrqw2*rhoQwidth2(r,c) +...
	                   AbipimR*ipimR(r,c) + AbjpjmR*jpjmR(r,c);
        rE(r,c)          = rqrhoE(r,c)*rho(r,c);
        rEmR(r,c)        = rE(r,c) - R(r,c);
        ipmimTE(r,c)     = ipmim(r,c)*rqrhoE(r,c);
        jmmjpTE(r,c)     = jmmjp(r,c)*rqrhoE(r,c);
        ipTE(r,c)        = ipmimTE(r,c) + imWidth/2;
        jpTE(r,c)        = imHeight/2   - jmmjpTE(r,c);
    end
end
StdErr = norm(rEmR);
StdErrWas = norm(rmRho);

