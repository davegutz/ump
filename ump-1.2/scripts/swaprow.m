function [asr]=swaprow(a,i,j);
% function [asr]=swaprow(a,i,j);
% Swap row i with j
[n,m]=size(a);
if i>n | j>n, error ('Bad index larger than array size'),end
asr=a;
asr(i,:)=a(j,:);
asr(j,:)=a(i,:);
