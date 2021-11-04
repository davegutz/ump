function [area, avg_width, avg_height, width, height]=discrete_area(Image);
% Calculate object area and average width assuming all non-zero pixels are
% part of object
% Inputs:
%  Image     Gray map 
% Outputs:
%  area      Integrated area of non-zero pixels
%  avg_width Average width of area of non-zero pixels
%  width     Envelope width of box drawn around all non-zero pixels
%  height    Envelope height of box drawn around all non-zero pixels

[n,m] = size(Image);
num_slice = 0;
area = 0;
avg_width = 0;
width = 0;
height = 0;
avg_width = 0;
avg_height = 0;
imin=0; imax=0; jabsmin=0; jabsmax=0;
for i=1:n,
  jmin=0; jmax=0;
  for j=1:m,
     pixel = Image(i,j);
     if pixel > 0,
        if jmin == 0,
           jmin   = jmax;
        end
        jmax = j;
     end
  end
  if jmin > 0,

    % Keep track of area covered
    if jabsmin == 0, jabsmin = jmin; end
    if jabsmax == 0, jabsmax = jmax; end
    jabsmin = min(jabsmin, jmin);
    jabsmax = max(jabsmax, jmax);

    if imin == 0,
      imin = imax;
    end
    imax = i;

    slice = jmax - jmin;
    num_slice = num_slice+1;
    area = area + slice;
    avg_width = (avg_width*(num_slice-1) + slice)/num_slice;
  end
end
if avg_width>0, avg_height = area/avg_width; end
width = jabsmax-jabsmin;
height = imax - imin;
