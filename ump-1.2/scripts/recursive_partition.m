function [v2] = recursive_partition(W);

% recursive_partition:  Method of Barnard-Simon to spectrally separate image
% sped up using recursive partition.
% D. Gutz 6/19/00

% Inputs:
% W    Weighted edge graph

% Outputs
% v2   Fiedler vector, second eigenvector, of matrix diag(sum(W))-W 

% Local:
% edge_thresh   Threshold for existance of an edge
% RL   Real weighted Graph (real Laplacian), D-W where D is total weight node. 
% Nc   Coarse cut, vector indeces of members


% Threshold for existance of an edge
edge_thresh=1e-3;

% Return if done
[N,M]=size(W);
N
if N == 0,
   v2=[];
   return
end
v2 = zeros(N,1)-1;
if N < 6, 
  for i=2:N,
     v2(i)=v2(1)+2*(i/N);
  end
  if N > 1,
     norm_factor = (max(v2) - min(v2)) / 2;
     if norm_factor == 0, norm_factor = 1; end
  else
     norm_factor = 1;
  end
  v2 = v2 / norm_factor;
  return 
end


% STEP 1

% Make coarse, greedy cut
Nc=[];
% If node i not adjacent to any node already in Nc then add i to Nc
for i=1:N,
  member = 0;
  if isempty(Nc),
     member = 0;
  else
     if  any( W(i,Nc)>edge_thresh ), member=1; end
  end
  if member == 0, Nc = [Nc i]; end
end


% Build Wc
nc = length(Nc);
Wc=zeros(nc,nc); Dc = zeros( length(Nc), N );
Dc(:,1)= Nc';
% Unmark all edges in W
marked = diag(ones(N,1));

while ~all(all(marked))
   [I,J] = find( ~marked );
   i = I(1); j = J(1);
   [loci, dummy] = find( Dc == i ); [locj, dummy] = find( Dc == j );
   if isempty(loci), loci=0; end
   if isempty(locj), locj=0; end

   % If exactly one of i and j is in some Dk, mark e and add other to Dk
   if xor( loci>0, locj>0 ),
      marked(i, j) = 1;
      if loci>0,
         loc_first_zero = find( Dc(loci,:) == 0 );
         Dc(loci, loc_first_zero(1)) = j;
      else
         loc_first_zero = find( Dc(locj,:) == 0 );
         Dc(locj, loc_first_zero(1)) = i;
      end
   else

      % If i and j are in different Dks, mark e and add edge to Wc
      if ( (loci ~= locj) & (loci>0) & (locj>0) ),
         marked(i,j) = 1;
         Wc(loci,locj) = Wc(loci,locj)+W(i,j);
         Wc(locj,loci) = Wc(loci,locj);
      else

         % If both i & j are in same Dk, mark e
         if ( (loci == locj) & (loci > 0) & (locj > 0)),
            marked(i,j) = 1;
         end
      end
    end
end  %while


% STEP 2
[v2cr] = recursive_partition(Wc);

% STEP 3
% Interpolate
v2 = zeros(N, 1);
for i=1:N,
   [locir, dummy] = find( Nc == i );
   if isempty(locir), locir=0; end
   if locir>0,
      v2(i) = v2cr(locir);
   else  % average using weight
      Wsum = 0;
      for j=1:length(Nc),
         v2(i) = v2(i) + v2cr(j)*W(i,Nc(j));
         Wsum = Wsum + W(i,Nc(j));
      end
      if Wsum ~= 0, v2(i) = v2(i)/Wsum; end
   end
end
if N > 1,
   norm_factor = (max(v2) - min(v2)) / 2;
   if norm_factor == 0, norm_factor = 1; end
else
   norm_factor = 1;
end
v2 = v2 / norm_factor;

% STEP 4
% Rayleigh quotient iteration
%[v2] = rayleigh_it(W, v2);
%[v2] = rayleigh_mit(W, v2);

d = sum(W);
D = diag(d);



% Min cut eigenvalue problem.
A=(D-W)*inv(D);
[eivec, eival]=eig(A);
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
v2 = eivec(:, icut);
v2 = v2/norm(v2,2);


% Return v2, Nc
