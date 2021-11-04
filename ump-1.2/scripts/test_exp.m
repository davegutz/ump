% Test power series approx to exp
%clear x,diffe,expapp,expex
for i=1:51,
  x=(i-51)*.1;
  X(i)=x;
  exppow(i)=1 + x*(1 + x*(1/2 + x*(1/6 + x*(1/24 + x*...
      (1/120 + x/720)))));
  expapp(i) = (x+5)/50 + ((x+5)/5)^(0.5)*0;
  expex(i)=exp(x);
  diffe(i)=(expapp(i)-expex(i));
end

    
