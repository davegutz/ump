% Generate triangulation sensitivity to pixel error
% normalize with straight-on sensitivity of single camera

octave = 1;  % Use 1 for octave, 0 for Matlab
hcopy  = 1;  % Use 1 for hcopy, 0 for screen

% Arbitrary camera placements
fp     = 167.5;
width  = 160;
height = 120;

R = 20;   % both cameras 20 in from dual cam focus point O
w = 3.63; % CCD width, mm  3-Com 1/4"CCD
g = 3.80; % mm focal length, mm
M = R/g;  % magnification
W = w/g*R;% FOV, in
d = 0.5;  % Ball dia, in
sens = W/width; % sensitiviy, in/pixel

% Arbitrary dual cam focus location
O      = [0 R 0];

deltaBarx=[];
deltaBary=[];
deltaBarz=[];
Hs=[];
thetas=[];
phis=[];

i=0;
for phi = 1:1:70,   % height of cameras above focus location
%for phi = 45:10:45,
phir = phi*pi/180;
H = R*sin(phir);
i=i+1;j=0;
for theta = 10:10:80, % angle cameras to focal point
%for theta = 45:10:45,
j=j+1;
   % projections and conversions
   thetar = theta*pi/180;
   r      = sqrt(R^2 - H^2);

   % Camera positions
   Pl = O + [-r*cos(thetar) -r*sin(thetar) H];
   Pr = O + [ r*cos(thetar) -r*sin(thetar) H];

   % Camera direction vectors, forcing cameras to point to an intersecting
   Tl = O-Pl;
   Tr = O-Pr;

   % Rotation matrix, phil/r degree arbitrary mis-orientations
   ACPWl = calibrate(0,Pl',Tl');
   ACPWr = calibrate(0,Pr',Tr');

   % Back-calculate
   [il jl]  = untriangulate(O, ACPWl, Pl, fp, width, height);
   [ir jr]  = untriangulate(O, ACPWr, Pr, fp, width, height);

   % Triangulate
   Olp = center(il, jl, fp, width, height);
   Olr = ACPWl*Olp;
   Orp = center(ir, jr, fp, width, height);
   Orr = ACPWr*Orp;
   [Oe Ue] = triangulate(Olr, Orr, Pl', Pr');

   % Delta 1
   Olp = center(il+.01, jl, fp, width, height);
   Olr = ACPWl*Olp;
   [OeD1 Ue] = triangulate(Olr, Orr, Pl', Pr');
   
   % Delta 2
   Olp = center(il, jl+.01, fp, width, height);
   Olr = ACPWl*Olp;
   [OeD2 Ue] = triangulate(Olr, Orr, Pl', Pr');

   delta = [OeD1-Oe OeD2-Oe]*100;

   % Save results
   Hs(i,j) = H;
   thetas(i,j) = theta;
   phis(i,j) = phi;
   deltaBarx(i,j) = norm(delta(1,:));
   deltaBary(i,j) = norm(delta(2,:));
   deltaBarz(i,j) = norm(delta(3,:));

   deltaBarxi(i,j) = delta(1,1);
   deltaBaryi(i,j) = delta(2,1);
   deltaBarzi(i,j) = delta(3,1);
   deltaBarxj(i,j) = delta(1,2);
   deltaBaryj(i,j) = delta(2,2);
   deltaBarzj(i,j) = delta(3,2);
end
end


figure(1);
if octave && hcopy,
   gset output 'xerror.ps'
   gset terminal postscript
end
if octave,
   ylabel('Obliqueness Error One Camera, fraction')
   xlabel('Camera Angle with Horizontal - phi, deg')
   title('X-direction Effect of Image Error for Various Sweep Back theta')
   grid
   axis([0 90 0 2]);
   gplot deltaBarx(:,1)/sens title  sprintf("%3.0f deg",thetas(1,1)), \
   deltaBarx(:,2)/sens title  sprintf("%3.0f deg",thetas(1,2)), \
   deltaBarx(:,3)/sens title  sprintf("%3.0f deg",thetas(1,3)), \
   deltaBarx(:,4)/sens title  sprintf("%3.0f deg",thetas(1,4)), \
   deltaBarx(:,5)/sens title  sprintf("%3.0f deg",thetas(1,5)), \
   deltaBarx(:,6)/sens title  sprintf("%3.0f deg",thetas(1,6)), \
   deltaBarx(:,7)/sens title  sprintf("%3.0f deg",thetas(1,7)), \
   deltaBarx(:,8)/sens title  sprintf("%3.0f deg",thetas(1,8))
else,
   plot(deltaBarx/sens);
   ylabel('Obliqueness Error One Camera, fraction')
   xlabel('Camera Angle with Horizontal - phi, deg')
   title('X-direction Effect of Image Error for Various Sweep Back theta')
   grid
   axis([0 90 0 2]);
end

figure(2);
if octave && hcopy,
   gset output 'yerror.ps'
   gset terminal postscript
end
if octave,
   axis([0 90 0 2]);
   ylabel('Obliqueness Error One Camera, fraction')
   xlabel('Camera Angle with Horizontal - phi, deg')
   title('Y-direction Effect of Image Error for Various Sweep Back theta')
   grid
   gplot deltaBary(:,1)/sens title  sprintf("%3.0f deg",thetas(1,1)), \
   deltaBary(:,2)/sens title  sprintf("%3.0f deg",thetas(1,2)), \
   deltaBary(:,3)/sens title  sprintf("%3.0f deg",thetas(1,3)), \
   deltaBary(:,4)/sens title  sprintf("%3.0f deg",thetas(1,4)), \
   deltaBary(:,5)/sens title  sprintf("%3.0f deg",thetas(1,5)), \
   deltaBary(:,6)/sens title  sprintf("%3.0f deg",thetas(1,6)), \
   deltaBary(:,7)/sens title  sprintf("%3.0f deg",thetas(1,7)), \
   deltaBary(:,8)/sens title  sprintf("%3.0f deg",thetas(1,8))
else,
   plot(deltaBary/sens);
   axis([0 90 0 2]);
   ylabel('Obliqueness Error One Camera, fraction')
   xlabel('Camera Angle with Horizontal - phi, deg')
   title('Y-direction Effect of Image Error for Various Sweep Back theta')
   grid
end

figure(3);
if octave && hcopy,
   gset output 'zerror.ps'
   gset terminal postscript
end
if octave,
   axis([0 90 0 2]);
   ylabel('Obliqueness Error One Camera, fraction')
   xlabel('Camera Angle with Horizontal - phi, deg')
   title('Z-direction Effect of Image Error for Various Sweep Back theta')
   grid
   gplot deltaBarz(:,1)/sens title  sprintf("%3.0f deg",thetas(1,1)), \
   deltaBarz(:,2)/sens title  sprintf("%3.0f deg",thetas(1,2)), \
   deltaBarz(:,3)/sens title  sprintf("%3.0f deg",thetas(1,3)), \
   deltaBarz(:,4)/sens title  sprintf("%3.0f deg",thetas(1,4)), \
   deltaBarz(:,5)/sens title  sprintf("%3.0f deg",thetas(1,5)), \
   deltaBarz(:,6)/sens title  sprintf("%3.0f deg",thetas(1,6)), \
   deltaBarz(:,7)/sens title  sprintf("%3.0f deg",thetas(1,7)), \
   deltaBarz(:,8)/sens title  sprintf("%3.0f deg",thetas(1,8))
else,
   plot(deltaBarz/sens);
   axis([0 90 0 2]);
   ylabel('Obliqueness Error One Camera, fraction')
   xlabel('Camera Angle with Horizontal - phi, deg')
   title('Z-direction Effect of Image Error for Various Sweep Back theta')
   grid
end

nfigs=3;



if octave == 0 && hcopy,
   hardfigure(1);!cp metatemp.ps xerror.ps
   hardfigure(2);!cp metatemp.ps yerror.ps
   hardfigure(3);!cp metatemp.ps zerror.ps
end


