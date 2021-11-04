%stochcut.m:  Stochastic min cut per Gdalyahu, Weinshall, Werman

%STAGE-1
%input:  
% graph G(V,E) = Image   with N nodes
% stoch_thresh  Probability changes to report

%output: 3D array p of probabilities


stoch_thresh=.1;
agauss=8;
bgauss=1000;
%agauss=.5;bgauss=0.1;
Mcounter=100;
pthresh=0.5;


% Load input
stoch_image
tempR044

% Cleanup image
[n,m]=size(Image);
for j=1:m, for i=1:n, if Image(i,j) < 20, Image(i,j)=0; end,end,end


%initialize counters
[nI,mI]=size(Image);
Image_r = [];
x=[]; y=[]; minloc=[];
for j=1:mI,
   for i=1:nI,
      Image_r=[Image_r Image(i, j)];
      x=[x i];
      y=[y j];
      if Image(i, j) > 0,
         minloc = [minloc; (j-1)*nI+i];
      end
   end
end
nminloc = length(minloc);
N = nminloc;

% Matlab memory consideration
if N > 200, 
   minloc=minloc(1:16:N);
   nminloc = length(minloc);
   N = nminloc;
end


     

% Calculate edges
W=zeros(N,N);
for i=1:N,
   indexij = minloc(i);
   for j=i+1:N,
      indexiijj = minloc(j);
      dij = sqrt( (x(indexij)-x(indexiijj))^2 +...
                  (y(indexij)-y(indexiijj))^2);
      dIij= Image_r(indexij) - Image_r(indexiijj);
      W(i,j) = exp(-(dij/agauss)^2)*exp(-(dIij/bgauss)^2);
   end
end
disp('Edges calculated')
fflush(1);


% Counters
s=zeros(N,N*N);

% Number of samples
% Main loop
for m=1:Mcounter, m, fflush(1);
   E=W;
   Node = zeros(N,1)';
   for i = 1:N, Node(i)=i; end
   for r=N-1:-1:1, r, fflush(1);
       [Node, E] = contract(Node, E);
       for j=1:N,
	  for i=1:N,
	     indexij = (j-1)*N + i;
	     indexji = (i-1)*N + j;
             if i ~= j, if Node(i) == Node(j),
	        s(r, indexij) = s(r, indexij) + 1;
             end, end
          end
       end
   end
end

stochdisp

