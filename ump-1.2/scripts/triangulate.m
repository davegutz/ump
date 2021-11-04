function [location, uncert] = triangulate(O0, O1, P0, P1);
% function [location, uncert] = triangulate(O0, O1, P0, P1);
% Triangulate ball position based on centered, rotated pixel measurement
% O0   Camera zero (left) direction measurement (ACPW*center(ip,jp)) column
% O1   Camera one (right) direction measurement (ACPW*center(ip,jp)) column
% P0   Camera zero (left) world position column
% P1   Camera one (right) world position column
% location   Estimated position, units of P0 and P1 column
% uncert     Estimated error in position column

% Normalize
O0 = O0/norm(O0);
O1 = O1/norm(O1);

combos = [ 1 2;
           1 3;
	   2 3];
weight   = [0 0 0]';  % weight to apply to each combo
location = [0 0 0]';
uncert   = [0 0 0]';

% Loop through all three combinations and average
for i=1:3,
  I = combos(i,1);
  J = combos(i,2);

  dP = [P0(I)-P1(I)  P0(J)-P1(J)];
  P  = [P0(I)        P0(J)];  % use P0 arbitrarily.  Get same result with P1.
  O  = [O0(I)        O0(J)];

  % Construct
  vT(1,1) =  O1(I);
  vT(1,2) = -O0(I);
  vT(2,1) =  O1(J);
  vT(2,2) = -O0(J);
 dP
 vT
  detvT = abs(vT(1,1)*vT(2,2) - vT(1,2)*vT(2,1));

  % Solve only well behaved conditions (non-parallel direction projections)
  if 0.005 <= detvT,
    % Solve
    RL = inv(vT)*[dP(1) dP(2)]';
 RL
    Oe = P' + RL(2)*O';

    % Predict location and add to weighted average subtotal
    weight(I) = weight(I) + 1;
    weight(J) = weight(J) + 1;
    locI = Oe(1);
    locJ = Oe(2);
    location(I) = location(I)+locI;
    location(J) = location(J)+locJ;
    if 0 == uncert(I),
      uncert(I)  = locI;
    else
      uncert(I) = uncert(I)-locI;
    end
    if 0 == uncert(J),
      uncert(J)  = locJ;
    else
      uncert(J) = uncert(J)-locJ;
    end
  end % detvT

end %for

% Weighted average
for i = 1:3
   if 0 < weight(i),
     location(i) = location(i)/weight(i);
   else 
    error('No triangulation');
   end
   if 1 == weight(i), 
     uncert(i) = 0;
   else
     uncert(i) = uncert(i)/weight(i);
   end %if
end %for




