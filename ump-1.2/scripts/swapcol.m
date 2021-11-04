function [asc]=swapcol(a,i,j);
% function [asc]=swapcol(a,i,j);
% Swap col i with j
[n,m]=size(a);
if i>m | j>m, error ('Bad index larger than array size'),end
asc=a;
asc(:,i)=a(:,j);
asc(:,j)=a(:,i);
