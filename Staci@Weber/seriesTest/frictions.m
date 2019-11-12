clear;

L = 100; % m
q = 2.5/1000; % m3/s
rho = 1000; % kg/m3
g = 9.81; % m/s2
D = 120/1000; % m
C = 130; % -
v = q / (D^2*pi/4); % m/s

% HAZEM - WILLIAMS
dh_STACI1 = 7.88 * L * abs(q)^0.852 * q * rho *g  / ...
    (C^1.852 * D^4.87 * 0.85^1.85);
f_STACI = abs( dh_STACI1 / (L / D * rho / 2 * v * abs(v)));
dh_STACI2 = f_STACI * L/D * 1/(2*g) / (D^2*pi/4)^2 * q * abs(q); 

dh_WDSH = 10.654 * (q/C)^(1/0.54) * L/(D)^4.87;

% DARCY - WEISBACH
nu = 1e-6; % Pa*s
E = [0.00012;0.00024;0.0006;0.0012;0.0024;0.006;0.012;0.024;0.06;...
    0.12;0.24;0.6;1.2]; % mm
k1 = 2000;
k2 = 4000;

v2 = 0.001:0.001:10;
Re = v2*D / nu;

idx1 = find(Re<k1);
f1 = 64./Re(idx1);
dh_DW1 = f1 * L/D /2/g .* v2(idx1) .* abs(v2(idx1));

tmp1 = Re<k2;
tmp2 = Re>k1;
idx2 = find(tmp1.*tmp2);
% idx2 = find(tmp2);
eps = 1;
f2 = 0.02;
while(eps>1e-6)
    f22 = 1 ./ (2*log10( E./1000/D/3.7*ones(size(idx2)) + 2.51./Re(idx2)./sqrt(f2) )).^2;
    eps = (f22-f2)./f2;
    f2 = f22;
end
dh_DW2 = f2 * L/D /2/g .* v2(idx2) .* abs(v2(idx2));

idx3 = find(Re>k2);
f3 = 0.25 ./ (log10( E/1000/D/3.7 + 5.74./Re(idx3).^0.9)).^2;
dh_DW3 = f3 .* L/D /2/g .* v2(idx3) .* abs(v2(idx3));

figure;
set(gca,'XScale','log','YScale','log')
loglog(Re(idx1),f1,'k');
hold on; grid on;
loglog(Re(idx2),f2,'b');
loglog(Re(idx3),f3,'r');

figure;
set(gca,'XScale','log','YScale','log')
loglog(Re(idx1),dh_DW1,'k');
hold on; grid on;
loglog(Re(idx2),dh_DW2,'b');
loglog(Re(idx3),dh_DW3,'r');




