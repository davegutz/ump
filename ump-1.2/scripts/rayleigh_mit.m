function [v2] = rayleigh_mit(W, v2);
% Rayleigh quotient iteration
% W   weight edges
% v2  initial second eigenvector estimate, 
% Ref http://www.ime.auc.dk/afd3/odessy/manuals/theory/node40.html#SECTION02635200000000000000

[N, M] = size(W);
d=sum(W);
D = diag(d);

if 0,
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
end %if0


Q=[(d/norm(d,2))'  v2  zeros(length(v2),1) zeros(length(v2),1)];
Q(3,3)= 1;
Q(4,4)= 1;
Q(:,1)=Q(:,1)/norm(Q(:,1),2);
Q(:,2)=Q(:,2)/norm(Q(:,2),2);
Q(:,3)=Q(:,3)/norm(Q(:,3),3);
Q(:,4)=Q(:,4)/norm(Q(:,4),4);
K=D-W;
M=D;
Q
for i = 1:15,
   Kb=Q'*K*Q;
   Mb=Q'*M*Q;
%Rb=Kb/Mb
   Ab=Kb*inv(Mb);
   [A,rval]=eig(Ab);
   Q=Q*A;
   Q(:,1)=Q(:,1)/norm(Q(:,1),2);
   Q(:,2)=Q(:,2)/norm(Q(:,2),2);
Q(:,3)=Q(:,3)/norm(Q(:,3),3);
Q(:,4)=Q(:,4)/norm(Q(:,4),4);

Q(:,2)
   rval
pause

end
v=Q*inv(Mb);
v2=v(:,2)/norm(v(:,2),2);
