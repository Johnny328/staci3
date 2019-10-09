clear;

pmin = 5;
pdes = 15;
m = 2.5;
dreq = 1;

p = 0:0.01:30;
d = zeros(size(p));
dd = zeros(size(p));

for i=1:length(p)
    if(p(i)<=pmin)
          d(i) = 0;
          dd(i) = 0;
    elseif(p(i)>pmin && p(i)<pdes)
          d(i) =  dreq*((p(i)-pmin)/(pdes-pmin))^(1/m);
          dd(i) = dreq*(1/m)*((p(i)-pmin)/(pdes-pmin))^(1/m-1)/(pdes-pmin);
    else
          d(i) =  dreq;
          dd(i) = 0;
    end
end

figure;
subplot(2,1,1);
plot(p,d,'linewidth',1.6);
subplot(2,1,2);
plot(p,dd,'linewidth',1.6);
