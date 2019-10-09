% loading the data
nodeColorData = importdata([caseName,'/segmentNode.txt']);
pipeColorData = importdata([caseName,'/segmentPipe.txt']);
poolColorData = importdata([caseName,'/segmentPool.txt']);
presColorData = importdata([caseName,'/segmentPres.txt']);
pumpColorData = importdata([caseName,'/segmentPump.txt']);
nodeColorName = nodeColorData.textdata;
nodeColor = nodeColorData.data;
pipeColorName = pipeColorData.textdata;
pipeColor = pipeColorData.data;
if(~isempty(poolColorData))
    poolColorName = poolColorData.textdata;
    poolColor = poolColorData.data;
else
    poolColor = 0;
end
if(~isempty(presColorData))
    presColorName = presColorData.textdata;
    presColor = presColorData.data;
else
    presColor = 0;
end
if(~isempty(pumpColorData))
    pumpColorName = pumpColorData.textdata;
    pumpColor = pumpColorData.data;
else
    pumpColor = 0;
end
allColor = [nodeColor;pipeColor;presColor;poolColor;pumpColor];

% defining the labels for colorbar
if(colorElement == "Pipe")
    colorBarTicks = min(pipeColor):(max(pipeColor)-min(pipeColor))/10:max(pipeColor);
elseif(colorElement == "Node")
    colorBarTicks = min(nodeColor):(max(nodeColor)-min(nodeColor))/10:max(nodeColor);
elseif(colorElement == "All")
    colorBarTicks = min(allColor):(max(allColor)-min(allColor))/10:max(allColor);
else
   disp("!!! WARNING !!! Available colorELement: Pipe, Node, All"); 
end
% reset to [0,1] interval
if(colorElement == "All")
    nodeColor = (nodeColor-min(allColor))/(max(allColor)-min(allColor));
    pipeColor = (pipeColor-min(allColor))/(max(allColor)-min(allColor));
    poolColor = (poolColor-min(allColor))/(max(allColor)-min(allColor));
    presColor = (presColor-min(allColor))/(max(allColor)-min(allColor));
    pumpColor = (pumpColor-min(allColor))/(max(allColor)-min(allColor));
else
    nodeColor = (nodeColor-min(nodeColor))/(max(nodeColor)-min(nodeColor));
    pipeColor = (pipeColor-min(pipeColor))/(max(pipeColor)-min(pipeColor));
    if(max(poolColor)-min(poolColor) ~=0)
        poolColor = (poolColor-min(poolColor))/(max(poolColor)-min(poolColor));
    end
    if(max(presColor)-min(presColor)~=0)
        presColor = (presColor-min(presColor))/(max(presColor)-min(presColor));
    end
    if(max(pumpColor)-min(pumpColor)~=0)
        pumpColor = (pumpColor-min(pumpColor))/(max(pumpColor)-min(pumpColor));
    end
end