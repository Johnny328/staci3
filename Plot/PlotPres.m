 function []=PlotPres(x,y,r,lw,angle,rgb,tag)

angle = angle*pi/180;


ratio = 2; %ratio of the radius and the distance between the centre and node
xp = x-ratio*r*cos(angle); %center of circle
yp = y-ratio*r*sin(angle);

%color
% R = 0.5;
% G = 0.5;
% B = 0.5;

%plotting the circle
ang=0:0.001:2*pi; 
xc=r*cos(ang);
yc=r*sin(ang);
plot(xp+xc,yp+yc,'linewidth',lw,'color',rgb,'tag',tag);
plot([x,(1-1/ratio)*xp+1/ratio*x],[y,(1-1/ratio)*yp+1/ratio*y],'linewidth',lw,'color',rgb,'tag',tag);

% plotting the letter P with lines to make it scalable
plot([xp-0.2*r,xp-0.2*r,xp-0.2*r],[yp-0.5*r,yp,yp+0.5*r],'linewidth',lw,'color',rgb,'tag',tag);
plot([xp-0.2*r,xp],[yp,yp],'linewidth',lw,'color',rgb,'tag',tag);
plot([xp-0.2*r,xp],[yp+0.5*r,yp+0.5*r],'linewidth',lw,'color',rgb,'tag',tag);
ang=-pi/2:0.01:pi/2; 
xpp=0.25*r*cos(ang);
ypp=0.25*r*sin(ang);
plot(xp+xpp,yp+ypp+0.25*r,'linewidth',lw,'color',rgb,'tag',tag);

 end