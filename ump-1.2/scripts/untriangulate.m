function [ip jp] = untriangulate(O, ACPW, P, fp, width, height);
% function [ip jp] = untriangulate(O, ACPW, P, fp, width, height);
% Un-Triangulate ball position to pixel measurement
% O     Actual object position, units of P column
% ACPW  Rotation matrix
% P     Camera world position column
% fp    Camera focal length, pixels
% width Camera image width, pixels
% height Camera image height, pixels
% RvT   Object-camera direction in world
% Op    Direction in camera pixel coordinates
% ip,jp Camera pixels, Rx

% Determine direction
RvT = O-P;
RvT = RvT/norm(RvT);

% Rotate direction to camera pixel coordinates
Op  = inv(ACPW)*RvT';

% Scale to camera pixels
Op  = Op*fp/Op(3);

% un-center
ip  = Op(1)+(width-1)/2;
jp  = (height-1)/2-Op(2);
