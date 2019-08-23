clear; close all;

% caseName = 'Net1';
% caseName = 'ky1';
% caseName = 'C-town';
caseName = 'hermes';
% caseName = 'linear_3';
% caseName = 'ferto';
% caseName = 'balf';
% caseName = 'becsidomb';
% caseName = 'villasor';
% caseName = 'Anytown';

% plot settings
pumpRadius = 0.01;
presRadius = 0.01;
tankSize = 0.02;
valveSize = 0.0005;
lineWidth = 1.5;
nodeMarkerSize = 8;
backgroundColor = [0.9,0.9,0.9];

% colorbar settings

%blackBody, blackBodyExt, cividis, coolWarmBent, coolWarmSmooth, inferno, jet, kindlmann, kindlmannExt, magma, plasma, viridis
%discrete: lines, prism
colorMapName = 'viridis'; 
colorBarText = 'Head [m]';
margin = 0.03; % margins around plot
colorDelta = 0.15; %space for colorbar
colorPos = "east"; %position of colorbar (east or south)
colorBarFontSize = 12;
colorElement = "Node"; % "Node" or "Pipe" or "All"

caseFolder = '../../Networks/';
addpath('../../Plot');
projectFolder = pwd;

operatingSystem = ispc;
if(operatingSystem)
    slashSign = '\';
else
    slashSign = '/';
end

% loading the INP file
LoadINP;

% loading data for coloring
LoadColors;

fig=figure;
hold on;
% adjusting the size of the plot window
if(colorPos == "east")
    xlim([-margin,max(coordX)+margin+colorDelta]);
    ylim([-margin,max(coordY)+margin]);
elseif(colorPos == "south")
    xlim([-margin,max(coordX)+margin]);
    ylim([-margin-colorDelta,max(coordY)+margin]);
else
    disp('colorPos can be: south | east'); 
end
xPos = 50;
yPos = 50;
xyRatio = diff(xlim)/diff(ylim);
if(xyRatio<16/9)
   ySize = 1000;
   xSize = ySize*xyRatio;
else
   xSize = 1800;
   ySize = xSize/xyRatio;
end
set(gcf, 'Position',  [xPos, yPos, xSize, ySize]);
axis off; 
set(gcf,'InvertHardCopy','off','Color',backgroundColor);
%colorbar
cb = colorbar;
colorMap = importdata(['../../Plot/ColorMaps/',colorMapName,'.txt']);
%adjusting to discrete colormaps
if(colorMapName == "lines" || colorMapName == "prism")
    colorMap = repmat(colorMap,ceil(colorMapNumber/size(colorMap,1)),1);
    colorMap = colorMap(1:colorMapNumber,:);
    colorMap(:,1) = linspace(0,1,size(colorMap,1));
end
    % else
    colormap(colorMap(:,2:4));
    cb.FontSize = colorBarFontSize;
    cb.Label.String=colorBarText;
    for i=1:length(colorBarTicks)
       cb.TickLabels{i} = round(colorBarTicks(i),-round(log(max(colorBarTicks))/log(10))+2); 
    end
    if(colorPos == "south")
        cb.Orientation = 'horizontal';
        set(cb,'position',[0.1,0.08,0.8,0.025]);
    elseif(colorPos == "east")
        set(cb,'position',[0.9,0.1,0.025,0.8]);
    else
        disp('!!! WARNING !!! available colorPos: south | east'); 
    end
% end

if(colorElement == "Node")
   pipeData = zeros(size(pipe));
elseif(colorElement == "Pipe")
   nodeData = zeros(size(node));
end

% start of plotting
for i=1:pipeCounter
    if(colorElement == "Pipe" || colorElement == "All")
        r = min(1,max(0,interp1(colorMap(:,1),colorMap(:,2),pipeColor(i),'spline')));
        g = min(1,max(0,interp1(colorMap(:,1),colorMap(:,3),pipeColor(i),'spline')));
        b = min(1,max(0,interp1(colorMap(:,1),colorMap(:,4),pipeColor(i),'spline')));
    else
        r = 0.2;
        g = 0.2;
        b = 0.2;
    end
    x = [node(pipe(i).nodeFromIdx).coordX,node(pipe(i).nodeToIdx).coordX];
    y = [node(pipe(i).nodeFromIdx).coordY,node(pipe(i).nodeToIdx).coordY];
    plot([x(1),(x(1)+x(2))/2,x(2)],[y(1),(y(1)+y(2))/2,y(2)],'k','linewidth',lineWidth,'color',[r,g,b],'tag',join([strrep(pipe(i).ID,'_','\_'),'  ',pipeData(i)]));
end
for i=1:pumpCounter
    if(colorElement == "All")
        r = min(1,max(0,interp1(colorMap(:,1),colorMap(:,2),pumpColor(i),'spline')));
        g = min(1,max(0,interp1(colorMap(:,1),colorMap(:,3),pumpColor(i),'spline')));
        b = min(1,max(0,interp1(colorMap(:,1),colorMap(:,4),pumpColor(i),'spline')));
    else
        r = 0.5;
        g = 0.5;
        b = 0.5;
    end
    PlotPump([node(pump(i).nodeFromIdx).coordX,node(pump(i).nodeToIdx).coordX],[node(pump(i).nodeFromIdx).coordY,node(pump(i).nodeToIdx).coordY],pumpRadius,1,[r,g,b],join([strrep(pump(i).ID,'_','\_'),'  ',pumpData(i)]));
end
for i=1:presCounter
    if(colorElement == "All")
        r = min(1,max(0,interp1(colorMap(:,1),colorMap(:,2),presColor(i),'spline')));
        g = min(1,max(0,interp1(colorMap(:,1),colorMap(:,3),presColor(i),'spline')));
        b = min(1,max(0,interp1(colorMap(:,1),colorMap(:,4),presColor(i),'spline')));
    else
        r = 0.5;
        g = 0.5;
        b = 0.5;
    end
    PlotPres(pres(i).coordX,pres(i).coordY,presRadius,1,90,[r,g,b],join([strrep(pres(i).ID,'_','\_'),'  ',presData(i)]));
end
for i=1:poolCounter
    if(colorElement == "All")
        r = min(1,max(0,interp1(colorMap(:,1),colorMap(:,2),poolColor(i),'spline')));
        g = min(1,max(0,interp1(colorMap(:,1),colorMap(:,3),poolColor(i),'spline')));
        b = min(1,max(0,interp1(colorMap(:,1),colorMap(:,4),poolColor(i),'spline')));
    else
        r = 0.5;
        g = 0.5;
        b = 0.5;
    end
    PlotPool(pool(i).coordX,pool(i).coordY,tankSize,1,0,[r,g,b],join([strrep(pool(i).ID,'_','\_'),'  ',poolData(i)]));
end
for i=1:valveCounter
    PlotValve([node(valve(i).nodeFromIdx).coordX,node(valve(i).nodeToIdx).coordX],[node(valve(i).nodeFromIdx).coordY,node(valve(i).nodeToIdx).coordY],valveSize,1,join([strrep(valve(i).ID,'_','\_'),'  ']));
end
for i=1:nodeCounter
    if(colorElement == "Node" || colorElement == "All")
        r = min(1,max(0,interp1(colorMap(:,1),colorMap(:,2),nodeColor(i),'spline')));
        g = min(1,max(0,interp1(colorMap(:,1),colorMap(:,3),nodeColor(i),'spline')));
        b = min(1,max(0,interp1(colorMap(:,1),colorMap(:,4),nodeColor(i),'spline')));
    else
        r = 0.2;
        g = 0.2;
        b = 0.2;
    end
    plot(node(i).coordX,node(i).coordY,'ko','linewidth',1,'markersize',nodeMarkerSize,'markerfacecolor',[r,g,b],'color',[0,0,0],'tag',join([strrep(node(i).ID,'_','\_'),'  ',nodeData(i)]));
end

datacursormode on
dcm = datacursormode(gcf);
set(dcm,'UpdateFcn',@ShowTag)

SaveTightFigure(fig,caseName);