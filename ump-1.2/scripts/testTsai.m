Ncx= 4.8000000000e+02
Nfx= 4.8000000000e+02
dx= 7.1000000000e-03
dy= 7.1000000000e-03
dpx= 7.1000000000e-03
dpy= 7.1000000000e-03
Cx= 2.4616151750e+02
Cy= 1.8951985256e+02
Sx= 1.0299188640e+00
f= 3.5397039949e+00
Kappa1= 3.5413033553e-02
Tx= 1.3757926024e+02
Ty= 2.5170148884e+02
Tz= 4.6560423876e+02
Rx= 2.3058228925e+00
Ry=-5.4866286786e-01
Rz=-4.9614531500e-01
p1= 0.0000000000e+00
p2= 0.0000000000e+00

ncal_=480; mcal_=360;
%Tsai algorithm assumes data taken with same image size as calibration
if ncal_ != Nfx,
  error('Tsai input does not have same size image as expected by cal');
end

n_=480
m_=360
Xf=71
Yf=163

Sn=ncal_/n_;
Sm=mcal_/m_;

Xfd=Xf*Sn;
Yfd=Yf*Sm;

Xd=dpx*(Xfd-Cx)/Sx;
Yd=dpy*(Yfd-Cy);
Xfu=Xd* (1.0+Kappa1*(Xd^2+Yd^2)) *Sx/dpx+Cx;
Yfu=Yd* (1.0+Kappa1*(Xd^2+Yd^2))    /dpy+Cy;

Xu=Xfu/Sn
Yu=Yfu/Sm

sa=sin(Rx); ca=cos(Rx);
sb=sin(Ry); cb=cos(Ry);
sg=sin(Rz); cg=cos(Rz);
R=...
[ cb*cg   cg*sa*sb-ca*sg  sa*sg+ca*cg*sb;
  cb*sg   sa*sb*sg+ca*cg  ca*sb*sg-cg*sa;
 -sb      cb*sa           ca*cb          ];

Rinv=...
[ cb*cg            cb*sg          -sb;
  cg*sa*sb-ca*sg   sa*sb*sg+ca*cg  cb*sa;
  sa*sg+ca*cg*sb   ca*sb*sg-cg*sa  ca*cb ]/...
((cb*cg)*(sa*sb*sg+ca*cg)*(ca*cb) + ...
 (cg*sa*sb-ca*sg)*(ca*sb*sg-cg*sa)*(-sb) + ...
 (cb*sg)*(cb*sa)*(sa*sg+ca*cg*sb) - ...
 (sa*sg+ca*cg*sb)*(sa*sb*sg+ca*cg)*(-sb) - ...
 (cg*sa*sb-ca*sg)*(cb*sg)*(ca*cb) - ...
 (ca*sb*sg-cg*sa)*(cb*sa)*(cb*cg) );

T=[Tx Ty Tz]';
P=-inv(R)*T/25.4;
