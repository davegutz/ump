function [W,x,y,N]=makeW(Image,agauss,bgauss);
% Make affinity function W.
% D. Gutz  6/28/00

% Inputs:
% Image   nxm pixel map
% agauss  Gaussian decay distance constant
% bgauss  Gaussian decay intensity difference constant

% Outputs:
% W       Affinity map
% x       Row location of selected pixel
% y       Col location of selected pixel
% N       Number selected pixels

% Find useful information for a minimum image
[n,m]=size(Image);
Image_r = [];
x=[]; y=[]; minloc=[];
for j=1:m,
   for i=1:n,
      Image_r=[Image_r Image(i, j)];
      x=[x i];
      y=[y j];
      if Image(i, j) >0,
         minloc = [minloc; (j-1)*n+i];
      end
   end,
end,
nminloc = length(minloc);
N = nminloc;


% Try doing min
d=[];
W=zeros(N,N);
for i=1:N,
   indexij = minloc(i);
   tij=0;
   for j=1:nminloc,
      indexiijj = minloc(j);
      dij = sqrt( (x(indexij)-x(indexiijj))^2 +...
                  (y(indexij)-y(indexiijj))^2);
      dIij= Image_r(indexij) - Image_r(indexiijj);
      wij = exp(-(dij/agauss)^2)*exp(-(dIij/bgauss)^2);
      W(i,j)=wij;
   end
end

