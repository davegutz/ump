function mincut2(Image,agauss,bgauss,gran_factor,delta_factor);
% Find cut (selection) that minimizes crossings of affinity function W.  
% Call recursively until nothing to cut
% D. Gutz  6/7/00

% Inputs:
% Image   nxm pixel map
% agauss  Gaussian decay distance constant
% bgauss  Gaussian decay intensity difference constant
% gran_factor Multiplier on eigenvector to create a granularity
% delta_factor Threshold for cut eigenvector

% Outputs:
% Image1  One of two cut images
% Image2  Two of two cut images

global num_cut_images


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


d = sum(W);
D = diag(d);



% Min cut eigenvalue problem.
A=D^(-1/2)*(D-W)*D^(-1/2);
%A=(D-W)*inv(D);
[eivec, eival]=eig(A);
if num_cut_images == 0,
   Nrhs=0;rhs=[];guess=[];exact=[];Title='mincut2';Key='key';
   Type='RUA';Ptrfmt=[];Indfmt=[];Valfmt=[];Rhsfmt=[];Rhstype=[];
   write_csc('mincut2.csc',A, Nrhs, rhs, guess, exact, Title, Key,...
	     Type, Ptrfmt, Indfmt, Valfmt, Rhsfmt, Rhstype);
   disp('CSC file written')
  eig(A)
end



% Find next to smallest eigenvalue, which is boundary of min cut
[neival, meival]=size(eival);
eival_minreal = eival(1,:);
for i=2:neival,
   for j=1:length(eival_minreal),
      if abs(eival(i, j)) > abs(eival_minreal(j)),
          eival_minreal(j) = eival(i, j);
      end
   end
end
[sval, ival] = sort(eival_minreal);


% Assign selection.
if neival > 1, icut=ival(2); else, icut=1; end;

% Normalize and granulate.
bound_eivec = eivec(:, icut)/max(abs(eivec(:,icut)));
bound_eivec = floor(bound_eivec*gran_factor);


% Organize cuts
[sbound, ibound] = sort(bound_eivec);
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
   if ratio > pi/4*0.9, if ratio < pi/4*1.1,
      [IMG, MAP] = gray2ind(Image);
      figure
      colormap(gray)
      image(255-IMG)
      title(eval(sprintf('''Image ratio=%f''',ratio)))
   end, end
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


keyboard

% Calculate new cut.
for i=1:numcuts,
   if sum(sum(abs(eval(sprintf('Image%i', i)))))/...
     max(max(abs(eval(sprintf('Image%i',i))))) > 6,
      mincut2(eval(sprintf('Image%i',i)),agauss,bgauss,gran_factor,delta_factor)
   end
end

