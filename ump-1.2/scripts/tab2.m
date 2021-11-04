function z = tab2(x,y,datax,datay,datar)
%
% tab2 - This function is an exact duplicate of John Schwoerer's TAB2
%
[ny,nx] = size(datar);
if nx == 1 & ny == 1,
  z = datar(1,1);
  return
elseif length(datax) ~= nx | length(datay) ~= ny,
  error('X, Y and Z data vectors must compatible sizes')
end

kxhi = nx;
kxlo = 1;

if x >= datax(nx),
  kxlo = nx;
  dx = 0.0;
elseif x <= datax(1),
  kxhi = 1;
  dx = 0.0;
else
  cont = 1;
  while cont,
    kx = floor((kxhi + kxlo)/2);
    if datax(kx) > x,
      kxhi = kx;
    else
      kxlo = kx;
    end
    if (kxhi - kxlo) <= 1,
      dx = (x - datax(kxlo))/(datax(kxhi) - datax(kxlo));
      cont = 0;
    end
  end
end

kyhi = ny;
kylo = 1;

if y >= datay(ny),
  kylo = ny;
  dy = 0.0;
elseif y <= datay(1),
  kyhi = 1;
  dy = 0.0;
else
  cont = 1;
  while cont,
    ky = (kyhi + kylo)/2;
    if datay(ky) > y,
      kyhi = ky;
    else
      kylo = ky;
    end
    if (kyhi - kylo) <= 1,
      dy = (y - datay(kylo))/(datay(kyhi) - datay(kylo));
      cont = 0;
    end
  end
end

r0 = datar(kylo,kxlo) + dx*(datar(kylo,kxhi) - datar(kylo,kxlo));
r1 = datar(kyhi,kxlo) + dx*(datar(kyhi,kxhi) - datar(kyhi,kxlo));

z = r0 + dy*(r1 - r0);
