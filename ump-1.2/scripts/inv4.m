function invA = inv4(A)
% invA = inv4(A) by cofactors
% A      square matrix
% invA   inverse of A

[n,m]=size(A);
if n ~= m,
  fprintf(1, 'A is %d X %d\n', n, m);
  error('input array not square')
end

invA = zeros(n, n);
detA = det4(A);

for i=1:n, for j=1:n,
  invA(i,j) = cof4(A, j, i) / detA;
end,end





