%eigcut2:  Use min cut eigenvalue algorithm to find image min cuts
% D. Gutz 6/7/00

global num_cut_images
num_cut_images=0;


agauss=.5;
bgauss=1000;
gran_factor=50;
delta_factor=1;
cut_screen=1;

% Load image
stoch_image3

%Take cuts recursively
mincut2(Image,agauss,bgauss,gran_factor,delta_factor);


