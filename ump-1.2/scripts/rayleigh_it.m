function [v2] = rayleigh_it(W, v2);
% Rayleigh quotient iteration
% W   weight edges
% v2  initial second eigenvector estimate, 
% Ref http://www.ime.auc.dk/afd3/odessy/manuals/theory/node40.html#SECTION02635200000000000000

[N, M] = size(W);
D = diag(sum(W));
RLc=(D-W)*inv(D);
mu=0;
rho=-10;
rhopast = 0;
i=0;
%for i=1:6,
while (abs((rho-rhopast)/rho) > 1e-10   & i<25),
   i=i+1;
   rhopast = rho;
   v2past  = v2;
   rho = (v2past'*RLc*v2past) / (v2past'*v2past);
   v2  = (RLc - mu*eye(N))\v2past;
   v2  = v2 / norm(v2, 2);
end
