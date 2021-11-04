A=[0 80.9375 63.75];
B=[-51.125 169.3125 64.875];
C=[51.0625 169.3125 65.375];
focalCalA=450;
focalCalB=833;
nCal=320;
mCal=240;
Pl=[-106.67 20.39 125.525];
Pr=[260 -90.53 89.725];
al=[298.1-nCal/2  mCal/2-155.2 focalCalA];
bl=[  5.9-nCal/2  mCal/2-146.9 focalCalA];
cl=[190.1-nCal/2  mCal/2-76.6  focalCalA];
ar=[ 57.2-nCal/2  mCal/2-103.6 focalCalB];
br=[144.2-nCal/2  mCal/2-82.9  focalCalB];
cr=[292.3-nCal/2  mCal/2-96.7 focalCalB];
T0A=A-Pl;
T0B=B-Pl;
T0C=C-Pl;
T1A=A-Pr;
T1B=B-Pr;
T1C=C-Pr;
al=al/norm(al);
bl=bl/norm(bl);
cl=cl/norm(cl);
ar=ar/norm(ar);
br=br/norm(br);
cr=cr/norm(cr);
T0A=T0A/norm(T0A);
T0B=T0B/norm(T0B);
T0C=T0C/norm(T0C);
T1A=T1A/norm(T1A);
T1B=T1B/norm(T1B);
T1C=T1C/norm(T1C);
Vl=[al;bl;cl];
Vr=[ar;br;cr];
Tl=[T0A;T0B;T0C];
Tr=[T1A;T1B;T1C];
ATl=-inv(Vl)*Tl;
ACPWl=-ATl'
ATr=-inv(Vr)*Tr;
ACPWr=-ATr'
(-ATl'*Vl')'-Tl
(-ATr'*Vr')'-Tr
