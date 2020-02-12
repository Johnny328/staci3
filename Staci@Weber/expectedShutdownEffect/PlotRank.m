clear;
close all

addpath('../../Plot');

%blackBody, blackBodyExt, cividis, coolWarmBent, coolWarmSmooth, inferno, jet, kindlmann, kindlmannExt, magma, plasma, viridis
%discrete: lines, prism
colorMapName = 'grayscale'; 

colorMap = importdata(['../../Plot/ColorMaps/',colorMapName,'.txt']);

cases = ["villasor","ferto","sanchegy","buk","lovo","nagycenk","vashegy","varis","becsidomb","tomalom",...
    "szakov","kohegy","harka","pozsonyiut","sopronkovesd","dudlesz","ivan","agyagosszergeny","kofejto","simasag",...
    "acsad","csaford","nagylozs","balf","csapod","und","rojtokmuzsaj","brennberg","pusztacsalad","kutyahegy",...
    "nyarliget","meszlen","fertoujlak","gorbehalom","tozeggyarmajor","ebergoc","csillahegy","jerevan","gloriette",...
    "alomhegy","ohermes","ujhermes"];
%  i |    0 |    1 |    2 |    3 |    4 |    5 |    6 |    7 |    8 |    9 |   10 |   11 |   12 |   13 |
%-----------------------------------------------------------------------------------------------------------
M2=[
  1 ,    0 ,   93 ,  307 ,  239 ,   62 ,   11 ,    2 ,    2 ,    0 ,    0 ,    0 ,    1 ,    0 ,    0 ;
  2 ,    0 ,  100 ,  182 ,  129 ,   38 ,    9 ,    3 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
  3 ,    0 ,   53 ,   75 ,   60 ,   22 ,    8 ,    3 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
  4 ,    0 ,   35 ,   47 ,   36 ,   18 ,    3 ,    2 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    1 ;
  5 ,    0 ,   45 ,   63 ,   50 ,   11 ,    3 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
  6 ,    0 ,   35 ,   57 ,   37 ,    9 ,    3 ,    0 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
  7 ,    0 ,   39 ,   77 ,   49 ,    8 ,    6 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
  8 ,    0 ,   45 ,   94 ,   71 ,   14 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
  9 ,    0 ,   29 ,   81 ,   69 ,   16 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 10 ,    0 ,    9 ,   21 ,   13 ,   13 ,    3 ,    4 ,    1 ,    0 ,    0 ,    1 ,    0 ,    0 ,    0 ;
 11 ,    0 ,   21 ,   37 ,   21 ,    4 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 12 ,    0 ,   18 ,   18 ,   18 ,    1 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 13 ,    0 ,   10 ,   17 ,   11 ,    5 ,    1 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 14 ,    0 ,   24 ,   25 ,   18 ,    5 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 15 ,    0 ,   11 ,   32 ,   16 ,    4 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 16 ,    0 ,   16 ,   16 ,   10 ,    2 ,    0 ,    1 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 17 ,    0 ,   11 ,   17 ,   13 ,    3 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 18 ,    0 ,   10 ,   29 ,   19 ,    4 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 19 ,    0 ,    9 ,   19 ,   15 ,    1 ,    0 ,    0 ,    0 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 20 ,    0 ,   12 ,   14 ,   10 ,    5 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 21 ,    0 ,    9 ,    6 ,   11 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 22 ,    0 ,   13 ,   13 ,    9 ,    3 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 23 ,    0 ,    8 ,    8 ,    6 ,    3 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 24 ,    0 ,    4 ,    4 ,    4 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 25 ,    0 ,    9 ,    7 ,    7 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 26 ,    0 ,    9 ,    8 ,   11 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 27 ,    0 ,    8 ,   12 ,    8 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 28 ,    0 ,   12 ,    7 ,    4 ,    0 ,    1 ,    1 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 29 ,    0 ,    8 ,    5 ,    8 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 30 ,    0 ,    4 ,    2 ,    1 ,    0 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 31 ,    0 ,    1 ,    5 ,    1 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 32 ,    0 ,    3 ,    2 ,    3 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 33 ,    0 ,    7 ,    1 ,    2 ,    0 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 34 ,    0 ,    7 ,    8 ,    6 ,    0 ,    0 ,    0 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 35 ,    0 ,    4 ,    1 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 36 ,    0 ,    4 ,    1 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 37 ,    0 ,    5 ,    5 ,    1 ,    2 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 38 ,    0 ,    3 ,    9 ,    5 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 39 ,    0 ,    4 ,    1 ,    0 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 40 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 41 ,    0 ,    3 ,    2 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;
 42 ,    0 ,    3 ,    1 ,    1 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ,    0 ;];

% SET INDEX

idx = 1:27; %ALL
% idx = [2:9,11:17]; %BEST
% idx = 1:10; %LARGE
% idx = 11:42; % SMALL

nSeg = sum(M2(:,2:end),2);
% idx = idx(nSeg>35);
nSeg = nSeg(idx);

M2 = M2(idx,:);
M2 = M2(:,2:9);

cases = cases(idx);
nCases = length(cases);

rank = M2(:,2:end);

relRank = zeros(size(rank));
for i=1:size(relRank,1)
    relRank(i,:) = rank(i,:)/nSeg(i);
end

for i=1:size(rank,1)
    ev{i} = [];
    for j=1:size(rank,2)
       ev{i} = [ev{i}, (j-1) * ones(1,rank(i,j))];
    end
    averageRank(i) = mean(ev{i});
end

figure(1);cla;
set(gcf,'units','points','position',[100,100,600,400])
hold on; grid on;
for i=1:size(relRank,1)
    r = min(1,max(0,interp1(colorMap(:,1),colorMap(:,2),(i-1)/size(relRank,1),'spline')));
    g = min(1,max(0,interp1(colorMap(:,1),colorMap(:,3),(i-1)/size(relRank,1),'spline')));
    b = min(1,max(0,interp1(colorMap(:,1),colorMap(:,4),(i-1)/size(relRank,1),'spline')));
    plot(1:size(rank,2),relRank(i,:),'x','color',[r,g,b],'linewidth',1.5);
end
ylim([0,1]);
xlabel('Degree [-]','fontsize',12);
ylabel('Relative frequency [-]','fontsize',12);
ColumnLegend(2,num2str(idx(:)));
rectangle('Position',[5.7 0.43 1.25 0.55],'FaceColor',[1 1 1])
saveas(gca,'Plots/RankDistBlack.fig','fig');
saveas(gca,'Plots/RankDistBlack.png','png');

% FITTING
figure(2);cla;
set(gcf,'units','points','position',[100,100,600,400])
hold on; grid on;
plot(1:size(rank,2),relRank(1,:),'kx','linewidth',1.5);
fitRelRank = mean(relRank);
plot(1:size(rank,2),fitRelRank,'s','markersize',10,'linewidth',1.5,'markerfacecolor',[0.7,0.7,0.7],'color',[0.7,0.7,0.7]);
for i=2:size(relRank,1)
    plot(1:size(rank,2),relRank(i,:),'kx','linewidth',1.5);
end
plot(1:size(rank,2),fitRelRank,'s','markersize',10,'linewidth',1.5,'markerfacecolor',[0.7,0.7,0.7],'color',[0.7,0.7,0.7]);
legend('WDNs','Fitted Function');
ylim([0,1]);
xlabel('Degree [-]','fontsize',12);
ylabel('Relative frequency [-]','fontsize',12);
saveas(gca,'Plots/RankDistFitBlack.fig','fig');
saveas(gca,'Plots/RankDistFitBlack.png','png');

% CHI SQUARE TEST 

p = 0.95;
chi2test = zeros(length(cases),1);
for i = 1:length(cases)
    chi2 = (rank(i,:)-nSeg(i)*fitRelRank).^2./(nSeg(i)*fitRelRank);
    chi2 = chi2(~isnan(chi2));
    r = sum(~isnan(chi2));
    chi2obs = sum(chi2);
    chi2crit = chi2inv(p,r-1);
    if(chi2obs < chi2crit)
        chi2test(i) = 1;
    end
end

nOk = sum(chi2test);
disp(['chi2 test: ', num2str(nOk),' of ', num2str(length(cases)), ' (',num2str(nOk/nCases*100),' %)']);

% % FUNCTION FITTING
% idxff = 2; % index of case for fitting
% % ev = zeros(sum(rank(idx,:)),1);
% ev = [];
% for i=1:length(rank(idxff,:))
%    ev = [ev;(i-1)*ones(rank(idxff,i),1)];
% end
% xaxis = 0:length(rank(idxff,:))-1;
% 
% figure(2);cla;
% grid on; hold on;
% plot(xaxis,relRank(idxff,:),'x','linewidth',2);
% par = fitdist(ev,'Poisson');
% plot(xaxis,poisspdf(xaxis,par.lambda),'o','linewidth',1.5);
% par = fitdist(ev,'Gamma');
% plot(xaxis,gampdf(xaxis,par.a,par.b),'d','linewidth',1.5);
% legend('rank','poisson','gamma');
% % gamma folytonos f�ggv�ny, poisson viszont nem illik r� egy�ltal�n



