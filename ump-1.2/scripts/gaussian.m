function [Image_r, minloc, xminloc, yminloc, W]=gaussian(Image,agauss,bgauss);
% Calculate gaussian affinity function W.  
% D. Gutz  6/7/00

% Inputs:
% Image   nxm pixel map
% agauss  Gaussian decay distance constant
% bgauss  Gaussian decay intensity difference constant


% Outputs:
% Image_r Row equivalent of Image
% minloc  Index into Image_r of the minimum image
% xminloc xLocation in Image of minloc
% yminloc yLocation in Image of minloc
% W       Weight




% Find useful information for a minimum image
[n,m]=size(Image);
Image_r = [];
xminloc=[]; yminloc=[]; minloc=[];
x=[]; y=[];
for j=1:m,
   for i=1:n,
      Image_r=[Image_r Image(i, j)];
      x=[x i];
      y=[y j];
      if Image(i, j) >0,
         xminloc= [xminloc; i];
         yminloc= [yminloc; j];
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
   for j=1:N,
      indexiijj = minloc(j);
      dij = sqrt( (x(indexij)-x(indexiijj))^2 +...
                  (y(indexij)-y(indexiijj))^2);
      dIij= Image_r(indexij) - Image_r(indexiijj);
      wij = exp(-(dij/agauss)^2)*exp(-(dIij/bgauss)^2);
      W(i,j)=wij;
   end
end
d = sum(W);
D = diag(d);

