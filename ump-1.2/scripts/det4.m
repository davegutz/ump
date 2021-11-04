function detA = det4(A)
% detA = det4(A) by cofactors
% A      square matrix
% detA   determinant of A

[n,m]=size(A);
if n ~= m,
  fprintf(1, 'A is %d X %d\n', n, m);
  error('input array not square')
end
if n == 1,
  detA =  A(1,1);
  return;
end

detA = 0;
i=1;
for j=1:n,
  detA = detA + A(i,j)*cof4(A, i, j);
end

