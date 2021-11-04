% Make new Image from old and cut information which is either -1,1,or 0 elements.
% D. Gutz 6/7/00
minImage=min(min(Image));
if minImage < 0,  
   Image=abs(Image+CutImage');
else
   Image=abs(Image-CutImage');
end

% Normalize
maxImage=max(max(Image));
if maxImage ~= 0, Image=Image/maxImage; end
