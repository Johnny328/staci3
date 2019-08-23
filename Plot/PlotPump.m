function []=PlotPump(x,y,r,lw,rgb,tag)

dist = pdist([x',y']);
center = [mean(x),mean(y)];
ratio = dist/(dist-2*r)*2;
angle = atan2((y(2)-y(1)),(x(2)-x(1)));

% color
% R = 0.5;
% G = 0.5;
% B = 0.5;

% plotting the circle
ang=0:0.001:2*pi;
xp=r*cos(ang);
yp=r*sin(ang);
plot(center(1)+xp,center(2)+yp,'linewidth',lw,'color',rgb,'tag',tag);
% plotting the "arrow"
plot([center(1)+r*cos(2*pi/3+angle),x(2)-(x(2)-x(1))/ratio],[center(2)+r*sin(2*pi/3+angle),y(2)-(y(2)-y(1))/ratio],'linewidth',lw,'color',rgb,'tag',tag);
plot([center(1)+r*cos(4*pi/3+angle),x(2)-(x(2)-x(1))/ratio],[center(2)+r*sin(4*pi/3+angle),y(2)-(y(2)-y(1))/ratio],'linewidth',lw,'color',rgb,'tag',tag);
plot([center(1)+r*cos(2*pi/3+angle),center(1)+r*cos(4*pi/3+angle)],[center(2)+r*sin(2*pi/3+angle),center(2)+r*sin(4*pi/3+angle)],'linewidth',lw,'color',rgb,'tag',tag);
% plotting the links from the circle to the nodes
plot([x(1),x(1)+(x(2)-x(1))/ratio],[y(1),y(1)+(y(2)-y(1))/ratio],'linewidth',lw,'color',rgb,'tag',tag);
plot([x(2),x(2)-(x(2)-x(1))/ratio],[y(2),y(2)-(y(2)-y(1))/ratio],'linewidth',lw,'color',rgb,'tag',tag);

end