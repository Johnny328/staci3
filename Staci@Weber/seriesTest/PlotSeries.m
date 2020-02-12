clear;
close all;

caseName = 'C-town';
% caseName = 'MUModel';

file = importdata([caseName,'/seriesResults.txt']);

nodeIndex = []; edgeIndex = [];
text = file.textdata;
iNode = 1; iEdge = 1;
for i=2:length(text)
    if(text{i}(end-4:end) == "[mtr]" || text{i}(end-4:end) == "[psi]")
        nodeID{iNode} = text{i}(1:end-5);
        iNode = iNode + 1;
        nodeIndex = [nodeIndex,i];
    elseif(text{i}(end-4:end) == "[lps]" || text{i}(end-4:end) == "[gpm]")
        edgeID{iEdge} = text{i}(1:end-5);
        iEdge = iEdge + 1;
        edgeIndex = [edgeIndex,i];
    end
end

time = file.data(:,1)/3600; % seconds to hours
nodeData = file.data(:,nodeIndex);
edgeData = file.data(:,edgeIndex);

% for nodes
idx = [1:4];
if(size(nodeData,2)>0)
    figure('Position',  [50, 50, 1600, 900]);
    plot(time,nodeData(:,idx),'linewidth',1.5);
    xlim([min(time),max(time)]);
    xlabel('Time [hours]');
    ylabel('Head [mtr]');
    title(['Node data of ',caseName]);
    legend(nodeID(:,idx))
    grid on;
end

% for edges
idx = [1:4];
if(size(edgeData,2)>0)
    figure('Position',  [100, 75, 1600, 900]);
    plot(time,edgeData(:,idx),'linewidth',1.5);
    xlim([min(time),max(time)]);
    xlabel('Time [hours]');
    ylabel('Volume Flow Rate [lps]');
    title(['Edge data of ',caseName]);
    legend(edgeID{idx});
    grid on;
end

