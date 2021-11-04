%Rcut:  Use min cut eigenvalue algorithm to find image min cuts
% D. Gutz 6/7/00

global num_cut_images
num_cut_images=0;


% cut algorithm inputs
agauss=8;
bgauss=1000;
gran_factor=100;
delta_factor=2;


% Load image
tempR044
Image(29,66)=0;
Image(21,80)=0;
Image(30,80)=0;
Image(31,80)=0;


%tempR074



[n,m]=size(Image);
for j=1:m, for i=1:n, if Image(i,j) < 20, Image(i,j)=0; end,end,end

%[Image_r, minloc, xminloc, yminloc, W] = gaussian(Image, agauss, bgauss);

%Take cuts recursively
mincut2(Image,agauss,bgauss,gran_factor,delta_factor);


