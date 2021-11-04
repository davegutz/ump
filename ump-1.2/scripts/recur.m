% recur:  Top-level wrapper for recursive_partition by Barnard-Simon.
% ref:  http://www.cs.berkeley.edu/~demmel/cs267/lecture20/lecture20.html
% D. Gutz  619/00

% Inputs:
% Image   nxm pixel map
% agauss  Gaussian decay distance constant
% bgauss  Gaussian decay intensity difference constant
% gran_factor Multiplier on eigenvector to create a granularity
% delta_factor Threshold for cut eigenvector

% Outputs:
% Imagei  Cut images


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

%stoch_image;
%agauss=1;


[n,m]=size(Image);
for j=1:m, for i=1:n, if Image(i,j) < 20, Image(i,j)=0; end,end,end



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
      if i~=j, W(i,j)=wij; end
   end
end
d = sum(W);
D = diag(d);
RL=(D-W)*inv(D);

% Recursive partition call
[v2] = recursive_partition(W)
[eivec, eival] = eig(RL);




% Organize cuts
[sbound, ibound] = sort(v2*gran_factor);
numcuts=1;
eval(sprintf('bound%i=[];', numcuts));
eval(sprintf('bound%i=[bound%i %i];', numcuts, numcuts, ibound(1)));
reference=sbound(1);
for i=2:length(sbound),
   if sbound(i) > reference+2*delta_factor,
     numcuts = numcuts+1;
     eval(sprintf('bound%i=[];', numcuts));
     reference=sbound(i);
   end
   eval(sprintf('bound%i=[bound%i ibound(%i)];', numcuts, numcuts, i));
end

keyboard





% Construct images
for i = 1:numcuts,
   eval(sprintf('Image%i=zeros(n,m);',i));
   for j=1:length(eval(sprintf('bound%i', i))),
      bound_pos=eval(sprintf('bound%i(j)',i));
      eval(sprintf('Image%i(x(minloc(bound_pos)),y(minloc(bound_pos)))=Image(x(minloc(bound_pos)),y(minloc(bound_pos)));', i));
   end
end


% Display results
num_cut_images=num_cut_images+1;
display_image=ceil(Image1/max(max(abs(abs(Image1)))))*num_cut_images;
for i=2:numcuts,
  num_cut_images=num_cut_images+1;
  eval(sprintf('display_image=display_image+ceil(Image%i/max(max(abs(abs(Image%i)))))*num_cut_images;', i,i));
end
if 0,
  disp('Cut image:')
  disp(display_image)
end

if num_cut_images-numcuts == 0,
   [IMG, MAP] = gray2ind(Image);
   figure
   colormap(gray)
   image(255-IMG)
   title('Base Image')
   [area, avg_width, avg_height, width, height]=discrete_area(Image);
   ratio = 0; full_area = width*height;
   if full_area > 0, ratio = area/full_area; end
   disp(sprintf('Image ratio =%f',ratio))
   if ratio > pi/4*0.9 & ratio < pi/4*1.1,
      [IMG, MAP] = gray2ind(Image);
      figure
      colormap(gray)
      image(255-IMG)
      title(eval(sprintf('''Image ratio=%f''',ratio)))
   end
end
for i=1:numcuts,
   [area, avg_width, avg_height, width, height]=...
                         discrete_area(eval(sprintf('Image%i',i)));
   ratio = 0; full_area = width*height;
   if full_area > 0, ratio = area/full_area; end
   disp(eval(sprintf('''Image%i ratio =%f''',num_cut_images-numcuts+i,ratio)))
   if ratio > pi/4*0.9, if ratio < pi/4*1.1,
     [IMG, MAP] = gray2ind(eval(sprintf('Image%i',i)));
     figure
     colormap(gray)
     image(255-IMG)
     title(eval(sprintf('''Image%i ratio=%f''',num_cut_images-numcuts+i,ratio)))
   end, end
end


