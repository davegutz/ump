function [v1]=fiedler(W);
% function [v1]=fiedler(W);
% Find second lowest eigenvector of W, assuming lowest eigenvalue is 0 with
% eigenvector = sum(W) typical for weighted image cut graph W

% Create system
d = sum(W);
D = diag(d);
A=D^(-1/2)*(D-W)*D^(-1/2);
[nn,mm]=size(A);

% Calculate the a-priori known lowest eigenvector for eigenvalue=0
z0=max(D^(.5))';
z0=z0/z0(nn);
T=diag(ones(nn,1));T(:,nn)=z0;
TAT=inv(T)*A*T;

% Feed the c program
disp('z0=')
     printv((z0/norm(z0,2)))

% Form reduced order system
for i=1:nn-1,
   for j=1:nn-1,
      R(i,j)=A(i,j)-z0(i)*A(nn,j);
   end
end

% Determine lowest eigenvalue and vector of reduced order system
[rvec, rval]=eig(R);max(rval)
[nrval, mrval]=size(rval);
rval_minreal = rval(1,:);
for i=2:nrval,
   for j=1:length(rval_minreal),
      if abs(rval(i, j)) > abs(rval_minreal(j)),
          rval_minreal(j) = rval(i, j);
      end
   end
end
[srval, irval] = sort(rval_minreal);


% Assign selection.
v1=[];


i=1;
e1=max(rval(:,irval(i)));
e1past=e1;
while e1-e1past<1e-4 & i<nn-1,

   % Get the eigenvector of reduced system
   y1 = rvec(:, irval(i));

   % un-permute the vector
   if e1~=0,
      q=dot(A(nn,1:nn-1),y1)/e1;
   else
      q=0;
   end
   z1T=[y1' q];
   z1=T*z1T';


   % y1 is how you convert the z results back to graph coordinates
   y1=D^(-1/2)*z1;
disp('y1=')
printv(y1/norm(y1,2));


   % Normalize and save for return
   v1i=z1;
   v1i=v1i/norm(v1i,2);
   v1=[v1 v1i];

disp('z1=')
printv(v1);
disp('T-1z1=')
printv(inv(T)*v1);

   % Next one?
   i=i+1;
   e1past=e1;
   e1=max(rval(:,irval(i)));
end
