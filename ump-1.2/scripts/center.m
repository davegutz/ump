function Op = center(ip, jp, fp, width, height, SY);
% function Op = center(ip, jp, fp, width, height);
% Calculate camera coordinates centered on ccd at focal distance from ccd
% im, jm, km  camera coordinate axes (im=ip-mi, jm=mj-jp)
% ip, jp      pixel values from image
% fp          focal length, pixels
% width       image width along i, pixels
% height      image height along j, pixels

Op = [ip-(width-1)/2 (height-1)/2*SY-jp fp]';
