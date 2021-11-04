function [v]=image_map(x, n, m);
% Map row vector into known image space

v=zeros(m,n);
for i=1:length(x),
  v(floor((i-1)/n)+1, i-floor((i-1)/n)*n) = x(i);
end
