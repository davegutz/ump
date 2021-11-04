% stochdisp:  Display results of stochcut.
% D. Gutz 6/8/00


% STAGE 2
[IMG, MAP] = gray2ind(Image);
image(IMG)
p=s/Mcounter;
Nk=zeros(N,N);
DT=[];TS=[];NI=[];
for r=1:N, r, fflush(1);
   for j=1:N,
      for i=1:N,
         if p(r, (j-1)*N +i) < pthresh,
            p(r, (j-1)*N +i) = 0;
         else
            p(r, (j-1)*N +i) = 1;
         end
      end
   end

   % Find clusters
   for elem=1:N,
      for checkelem=1:N,
         if p(r, (elem-1)*N+checkelem) > 0,if elem ~= checkelem,
             if Nk(r, checkelem) > 0,
                Nk(r, elem) = Nk(r, checkelem);
             else
                if Nk(r, elem) == 0,
                   Nkmax = max(Nk(r,:));
                   Nk(r, elem) = Nkmax + 1;
                end
             end
         end,end
      end
      if Nk(r, elem) == 0,
         Nkmax = max(Nk(r,:));
         Nk(r, elem) = Nkmax + 1;
      end
   end
   [Nksort,dum]=sort(Nk(r,:));
   ni=zeros(N,1)';
   place=1;
   ni(place)=Nksort(1);
   for i=2:N,
      if Nksort(i) > Nksort(i-1),
         place=place+1;
      end
      ni(place)=ni(place)+1;
   end
   NI=[NI; ni];

   NiNjSum=0;
   for j=1:N,
     for i=j+1:N,
        NiNjSum=NiNjSum+ni(j)*ni(i);
     end   
   end
   % Report good ones
   T=2/N/(N-1)*NiNjSum;
   if r>1,
      dT=T-Tp;
   else
      dT=T;
   end
   Tp = T;
   if dT > stoch_thresh,
      disp(dT)
      for id=1:max(Nk(r,:)),
	 count=0;loc=[];
	 for k=1:length(Nk(r,:)),
	    if Nk(r,k) == id,
               count=count+1;
               loc(count)=k; 
            end
         end
	 if count > 2,
	   disp('A shape:')
            CutImage=zeros(mI,nI);
            for j = 1:count,
               pixel= Image(x(minloc(loc(j))),y(minloc(loc(j))));
               CutImage(y(minloc(loc(j))),x(minloc(loc(j))))=pixel;
            end
%            disp(CutImage')
            [IMG, MAP] = gray2ind(CutImage');
            image(IMG)
         end
      end
   end
   TS=[TS T];
   DT=[DT dT];
end
