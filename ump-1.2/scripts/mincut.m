% Find cut (selection) that minimizes crossings of affinity function W.
% D. Gutz  5/21/00

% Inputs:
% Image   nxm pixel map
% agauss  Gaussian decay distance constant
% bgauss  Gaussian decay intensity difference constant

% Outputs:
% ImageV  nxm boolean map of min cut pixels



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
agauss=1;bgauss=0.1;
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
      wij = exp(-(dij/agauss))*exp(-(dIij/bgauss)^2);
      W(i,j)=wij;
      tij = tij + wij;
   end
   d = [d tij];
end
D = diag(d);



% Min cut eigenvalue problem.
A=(D-W)*inv(D);
[eivec, eival]=eig(A);



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

% Normalize.
bound_eivec = eivec(:, icut)/max(abs(eivec(:,icut)));
boundary=zeros(neival,1);
for i=1:neival,
%  if abs(bound_eivec(i)) > 0.51,
  if bound_eivec(i) > 0,
     boundary(i)=1;
  else
     boundary(i)=-1;
  end
end
CutImage=zeros(m,n);
for j = 1:N,
CutImage(y(minloc(j)),x(minloc(j))) = ...
      Image(x(minloc(j)),y(minloc(j)))*boundary(j);
end

