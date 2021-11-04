function [xold,lambda]=rayleigh(A,mu,x,tol,itemax)
%
%
%  *** [xold,lambda]=RAYLEIGH(A,mu,x,tol,itemax)  ***
%
%
% input:     
%   A        matrix
%   mu       complex shift 
%   x        initial vector
%   tol      tolerance for change in eigenvector
%   itemax   maximum number of iterations
%
% output:    
%   x        eigenvector
%   lambda   eigenvalue
%
% 
% Implements Rayleigh quotient iteration to find a single eigenvalue
% and corresponding eigenvector of a (possibly) complex matrix A.
%

% check for 1, 2, 3 or 4 arguments and set mu, x, tol and niter if necessary
if (nargin == 1)
  mu=0
  x=ones(size(A,1),1);
  tol=1.0E-05;
  itemax=100;
elseif (nargin == 2)
  x=ones(size(A,1),1);
  tol=1.0E-05;
  itemax=100;
elseif (nargin == 3)
  tol=1.0E-05;
  itemax=100;
elseif (nargin == 4)
  itemax=100;
  elseif (nargin ~= 5)
  fprintf('\nUsage: [x,lambda]=invpower(A,mu,x,tol,itemax) \n');
  return
end


% Check that the matrix A is square
[m,n]=size(A);
if m ~= n
  fprintf('\nMatrix is not square. \n')
  return 
end

% Check that the initial vector x is a column vector 
% with the correct number of rows.
[m,k]=size(x);
if m ~= n | k ~= 1
  fprintf('\nInitial vector is inappropriate. \n')
  return 
end


% Find the eigenvectors and eigenvalues of A using the MATLAB eig() routine
[V,D]=eig(A);

% Sort the eigenvalues (and eigenvectors) of A in order of decreasing magnitude
% (Note that the MATLAB sort routine sorts in ascending order)
[spec,ID]=sort(abs(diag(D)));
for i=1:n
  DS(i)=D(ID(n+1-i),ID(n+1-i));
  VS(:,i)=V(:,ID(n+1-i));
end
fprintf('\nThe eigenvalues of A in order descending  magnitude care \n')
DS

fprintf('\nThe corresponding eigenvectors of A are \n')
VS

dsratio=abs(DS(2:n)./DS(1:n-1));
fprintf('\nThe ratios of the magnitudes of consecutive eigenvalues are \n')
dsratio

fprintf('\nPress space bar to continue... \n')
pause

% Find the eigenvectors and eigenvalues of (A-mu*I)^(-1)
% Sort in descending order

fprintf('\n\n')
[V,D]=eig( inv(A-mu*eye(n)) );
[tmp,ID]=sort(abs(diag(D)));
for i=1:n
  VS(:,i)=V(:,ID(n-i+1));
  DS(i)=D(ID(n-i+1),ID(n-i+1));
end

fprintf('Complex shift \n')
mu

fprintf('Eigenvalues of (A-mu*I)^(-1) in descending order \n')
DS 

fprintf('Eigenvectors of (A-mu*I)^(-1) in descending order \n')
VS

dsratio=abs(DS(2:n)./DS(1:n-1));
fprintf('\nThe ratios of the magnitudes of consecutive eigenvalues are \n')
dsratio

fprintf('\nPress space bar to continue... \n')
pause


% Find the largest component of the dominant eigenvector
[vdmax,ivdmax] = max( abs( VS(:,1) ) );



ite=0;
xold=x;
errold=1;


%
% Rayleigh quotient iteration
%

while errold > tol
  ite=ite+1;
  if ite > itemax
    fprintf('Maximum number of iterations exceeded \n')
    fprintf('Estimate eigenvalue using the Rayleigh quotient \n')
    x=xold;
    lambda=(x'*A*x) / (x'*x);
    return
  end

  xnew  = (A-mu*eye(n))\xold;;

% The power method converges to a scalar multiple of the dominant eigenvector
% and that scalar may be complex.
% In order to compare xnew and VS(:,1) we need to scale by a complex number.
  scale = xnew(ivdmax)/VS(ivdmax,1);
  xnew  = xnew/scale;

% Collect convergence information
  errnew(ite) = norm(xnew-VS(:,1));
  crate(ite)  = errnew(ite)/errold;
  crate2(ite) = errnew(ite)/errold^2;
  crate3(ite) = errnew(ite)/errold^3;
  fprintf('Iteration number %4.0f \n',ite)
  fprintf('xnew is \n',ite)
  xnew'

% Update xold and errold
  xold=xnew;
  errold=errnew(ite);
  
% Calculate Rayleigh quotient
  fprintf('New best estimate of eigenvalue \n')
  mu = xold'*A*xold  / (xold'*xold)
  pause
  
end

% Find the eigenvalue using the rayleigh quotient
fprintf('Rayleigh quotient iteration converged \n')
fprintf('Estimated eigenvalue of A using the Rayleigh quotient \n')
x=xold;
lambda = mu


fprintf('\n\nConvergence data \n')
errnew
crate
crate2
crate3
