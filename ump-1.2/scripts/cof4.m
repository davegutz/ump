function cof = cof4(A, I, J)
% cof = cof4(A, I, J)
% subMat cofactor matrix
% A      square matrix
% I      index, 0 = first
% J      index, 0 = first

[n,m]=size(A);
if n ~= m,
  fprintf(1, 'A is %d X %d\n', n, m);
  error('input array not square')
end
if n == 1,
  fprintf(1, 'A is %d X %d\n', n, m);
  error('input array too small')
end

subMat = zeros(n-1, n-1);

ii=0;
for i=1:n,
   if i ~= I,
      ii=ii+1;
      jj=0;
      for j=1:n,
         if j ~= J,
            jj=jj+1;
            subMat(ii,jj) = A(i,j);
         end
      end
    end
end
cof = det4(subMat);

if rem(I+J,2) > 0,
   cof = cof*(-1);
end


