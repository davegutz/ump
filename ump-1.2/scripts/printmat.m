function printmat(A);
[nn,mm]=size(A);
for i=1:nn,
for j=1:mm,
   if(rem(j,5)==0),fprintf(1,'\n');end
   fprintf(1,' %e,', A(i,j));
end
fprintf(1,'\n');
end
