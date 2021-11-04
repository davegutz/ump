function ACPW = calibrate(phi, P, T);
% Calculate rotation matrix from camera pixels to world coordinate directions
%
% Try concepts to transform camera frames to world coordinates
% im, jm, km  camera-level coordinate axes (im=ip-mi, jm=mj-jp)
% i, j, k     world coordinates axes
% P           camera world position column vector
% T           camera world direction column vector
% ACLW        camera-level to world conversion
% ACPL        camera-pixel to camera-level conversion
% phi         camera tilt, degrees

% Convert to radians
phir = phi * pi/180;

% Compute camera-level coordinate axes in world
km = -T; km=km/norm(km);

% im-km =0, in plane i,j
im = [-T(2)  T(1)  0]'; im=im/norm(im);

% jm = km X im
jm = [km(2)*im(3)-im(2)*km(3);
      km(3)*im(1)-im(3)*km(1);
      km(1)*im(2)-im(1)*km(2)];

ACLW  = [im jm km];

% Rotation from camera-pixel coordinates to camera-level coordinates
ACPL = [cos(phir) -sin(phir) 0;
        sin(phir)  cos(phir) 0;
        0         0        1;];

ACPW = -ACLW*ACPL;
