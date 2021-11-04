function printv(v);
[nn]=length(v);
for j=1:nn,
   if(rem(j,5)==0),fprintf(1,'\n'),end
   fprintf(1,' %e,', v(j))
end
fprintf(1,'\n');
end
