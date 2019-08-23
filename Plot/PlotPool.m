 function []=PlotPool(x,y,r,lw,angle,rgb,tag)

angle = angle*pi/180; %from degree to rad

%sizes of the pool
b=0.3*r; %indicator
bh=b*sqrt(3)/2; %height of indicator
a=0.5*r; %width of tank
m=0.75*r; %height of tank
h=0.5*r; %height of water
l=1.0*r; %connectiond link
lx=l*cos(angle);
ly=l*sin(angle);

%color
% R = 0.5;
% G = 0.5;
% B = 0.5;

%connection
plot([x,x+lx],[y,y+ly],'linewidth',lw,'color',rgb,'tag',tag);
%tank
plot([x+lx-a,x+lx+a],[y+ly,y+ly],'linewidth',lw,'color',rgb,'tag',tag);
plot([x+lx-a,x+lx-a],[y+ly,y+ly+m],'linewidth',lw,'color',rgb,'tag',tag);
plot([x+lx+a,x+lx+a],[y+ly,y+ly+m],'linewidth',lw,'color',rgb,'tag',tag);
%water level
plot([x+lx-a,x+lx+a],[y+ly+h,y+ly+h],'linewidth',lw,'color',rgb,'tag',tag);
%indicator
plot([x+lx,x+lx+b/2,x+lx-b/2,x+lx,x+lx],[y+ly+h,y+ly+h+bh,y+ly+h+bh,y+ly+h,y+ly+h-b],'linewidth',0.75*lw,'color',rgb,'tag',tag);

end
