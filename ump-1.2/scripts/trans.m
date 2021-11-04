% Try concepts to transform camera frames to world coordinates
% fp          focal length, pixels
% ip, jp      pixel coordinates
% mi, mj      middel pixels
% im, jm, km  camera-level coordinate axes (im=ip-mi, jm=mj-jp)
% i, j, k     world coordinates axes
% Pr          right-hand camera position
% O           object position
% Or          object direction from right-hand camera
% Tr          right-hand camera direction vector
% ACLW        camera-level to world conversion
% ACPL        camera-pixel to camera-level conversion
% phi         camera tilt, radians
% Ox          object positions matrix
% Oxrp        object camera-pixel directions matrix

phir = 10 * pi/180;
phil = 10 * pi/180;
Pr  = [ 27  14  8]';
Pl  = [-27  14  8]';
O1  = [  0  16  1]';
O2  = [  0  16  4]';
O3  = [  0  26  1]';
Ox  = [O1   O2   O3]
Tr  = [-27 -10 -6]';
Tl  = [ 27 -10 -6]';

Or = O1 - Pr; Or=Or/norm(Or);

% Compute camera-level coordinate axes in world
km = -Tr; km=km/norm(km);
% im-km =0, in plane i,j
im = [-Tr(2)  Tr(1)  0]'; im=im/norm(im);
% jm = km X im
jm = [km(2)*im(3)-im(2)*km(3);
      km(3)*im(1)-im(3)*km(1);
      km(1)*im(2)-im(1)*km(2)];

ACLWr = [im jm km];

% Rotation from camera-pixel coordinates to camera-level coordinates
ACPLr = [cos(phir) -sin(phir) 0;
        sin(phir)  cos(phir) 0;
        0         0        1;];

km = -Tl; km=km/norm(km);
% im-km =0, in plane i,j
im = [-Tl(2)  Tl(1)  0]'; im=im/norm(im);
% jm = km X im
jm = [km(2)*im(3)-im(2)*km(3);
      km(3)*im(1)-im(3)*km(1);
      km(1)*im(2)-im(1)*km(2)];

ACLWl = [im jm km];

% Rotation from camera-pixel coordinates to camera-level coordinates
ACPLl = [cos(phil) -sin(phil) 0;
        sin(phil)  cos(phil) 0;
        0         0        1;];

ACPWl = -ACLWl*ACPLl;
ACPWr = -ACLWr*ACPLr;

%test
fp = 167.5;
Orm = -ACLWr'*Or; Orm=Orm/abs(Orm(3))*fp;
Orp = ACPLr'*Orm;
Orm1= ACPLr*Orp;
Or1 = -ACLWr*Orm1; Or1=Or1/norm(Or1);

Orp2 = ACPWr'*Or; 


Orm2 = [47.22 -6.6 167.5]';
Or2 = -ACLWr*Orm2; Or2 = Or2/norm(Or2);

O1r = O1 - Pr; O1r=O1r/norm(O1r);
O2r = O2 - Pr; O2r=O2r/norm(O2r);
O3r = O3 - Pr; O3r=O3r/norm(O3r);
O1rp = ACPWr'*O1r; %O1rp = O1rp / O1rp(3) * fp;
O2rp = ACPWr'*O2r; %O2rp = O2rp / O2rp(3) * fp;
O3rp = ACPWr'*O3r; %O3rp = O3rp / O3rp(3) * fp;
Oxr =[O1r  O2r  O3r];
Oxrp=[O1rp O2rp O3rp];

O1l = O1 - Pl; O1l=O1l/norm(O1l);
O2l = O2 - Pl; O2l=O2l/norm(O2l);
O3l = O3 - Pl; O3l=O3l/norm(O3l);
O1lp = ACPWl'*O1l; %O1lp = O1lp / O1lp(3) * fp;
O2lp = ACPWl'*O2l; %O2lp = O2lp / O2lp(3) * fp;
O3lp = ACPWl'*O3l; %O3lp = O3lp / O3lp(3) * fp;
Oxl =[O1l  O2l  O3l];
Oxlp=[O1lp O2lp O3lp];

Oxrp1=ACPWr'*Oxr;
Oxlp1=ACPWl'*Oxl;

ACPWl1 = (Oxlp*inv(Oxl))'
ACPWr1 = (Oxrp*inv(Oxr))'   % This is how to calculate ACPW from cal data

% Conclusion:  knowing fp, Pr, Oxrp, and Ox the rotation matrix ACPW
% can be calculated

% Since Pr implicit (used in cal and then backwards in use)
% then I think small errors in knowing Pr do not create
% large overall errors.  Could do an error sensitivity study.
% Don't use Pr in calibration or back calc: 

Ple  = Pl;
O1le = O1-Ple; O1le=O1le/norm(O1le);
O2le = O2-Ple; O2le=O2le/norm(O2le);
O3le = O3-Ple; O3le=O3le/norm(O3le);
Oxle =[O1le  O2le  O3le];
ACPWl1e = (Oxlp*inv(Oxle))';
Oxle1 = ACPWl1e*Oxlp;

Pre  = Pr;
O1re = O1-Pre; O1re=O1re/norm(O1re);
O2re = O2-Pre; O2re=O2re/norm(O2re);
O3re = O3-Pre; O3re=O3re/norm(O3re);
Oxre =[O1re  O2re  O3re];
ACPWr1e = (Oxrp*inv(Oxre))';
Oxre1 = ACPWr1e*Oxrp;

% Whoops.  Need accurate Pr and Pl for the reconstruction during triangulation
% Use calibration to predict useful Pru and Plu to be used reconstruction:

% First, a sample reconstruction
% Have ACPWr1 and ACPWl1 and Oxrp and Oxlp.  Know Oxl, Oxr, Pl, Pr, fp
OxrT = ACPWr1*Oxrp;
OxlT = ACPWl1*Oxlp;
ACPWr1;

% vT*RL=dP
% vT   column direction vector to object for device
% RL   [R L] where R is magnitude of Or and L is magnitude of Ol
% dP   Pl - Pr
vT12=[OxrT(1:2,1) -OxlT(1:2,1)];  % Use Object 1
dP12 = (Pl(1:2)-Pr(1:2));
RL12 = inv(vT12)*dP12;
vT12*RL12    % should be = dP12

vT13=[[OxrT(1,1) OxrT(3,1)]' -[OxlT(1,1) OxlT(3,1)]'];  % Use Object 1
dP13 = [(Pl(1)-Pr(1)); (Pl(3)-Pr(3))];
RL13 = inv(vT13)*dP13;
vT13*RL13    % should be = dP13

vT23=[OxrT(2:3,1) -OxlT(2:3,1)];  % Use Object 1
dP23 = (Pl(2:3)-Pr(2:3));
RL23 = inv(vT12)*dP23;
vT23*RL23    % should be = dP23

Oer12 = Pr + RL12(1)*OxrT(:,1)
Oel12 = Pl + RL12(2)*OxlT(:,1)
Oer13 = Pr + RL13(1)*OxrT(:,1)
Oel13 = Pl + RL13(2)*OxlT(:,1)
Oer23 = Pr + RL23(1)*OxrT(:,1)
Oel23 = Pl + RL23(2)*OxlT(:,1)

% real life example

Pr = [ 23.0625, 7.9375,  15.375 ]';
Pl = [-23.8125, 7.5,     16.0   ]';
O1 = [  0,  1.5,  2.625 ]';
O2 = [  0,  0.5,  7.125 ]';
O3 = [  0, 13.5,  7.875 ]';
O1lp  = [  138.0-80, 56-67.4, 167.5]';
O2lp  = [  147.9-80, 56-43.4, 167.5]';
O3lp  = [   61.5-80, 56-37.7, 167.5]';
O1rp  = [   20.0-80, 56-63.4, 167.5]';
O2rp  = [    8.8-80, 56-37.6, 167.5]';
O3rp  = [   99.5-80, 56-30.7, 167.5]';
O1lp=O1lp/norm(O1lp);
O2lp=O2lp/norm(O2lp);
O3lp=O3lp/norm(O3lp);
O1rp=O1rp/norm(O1rp);
O2rp=O2rp/norm(O2rp);
O3rp=O3rp/norm(O3rp);


O1l = O1 - Pl; O1l=O1l/norm(O1l);
O2l = O2 - Pl; O2l=O2l/norm(O2l);
O3l = O3 - Pl; O3l=O3l/norm(O3l);
O1r = O1 - Pr; O1r=O1r/norm(O1r);
O2r = O2 - Pr; O2r=O2r/norm(O2r);
O3r = O3 - Pr; O3r=O3r/norm(O3r);

Oxl =[O1l  O2l  O3l];
Oxlp=[O1lp O2lp O3lp];
Oxr =[O1r  O2r  O3r];
Oxrp=[O1rp O2rp O3rp];

ACPWl = (Oxl*inv(Oxlp));
ACPWr = (Oxr*inv(Oxrp));

OxlT = ACPWl*Oxlp;
OxrT = ACPWr*Oxrp;

% vT*RL=dP
% vT   column direction vector to object for device
% RL   [R L] where R is magnitude of Or and L is magnitude of Ol
% dP   Pl - Pr
vT=[OxrT(1:2,1) -OxlT(1:2,1)];  % Use Object 1
dP = (Pl(1:2)-Pr(1:2));
RL = inv(vT)*dP;
vT*RL    % should be = dP12
Oer = Pr + RL(1)*OxrT(:,1)
Oel = Pl + RL(2)*OxlT(:,1)

