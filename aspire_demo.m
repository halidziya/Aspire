clear all;
clc;

currentDir=pwd;
addpath([currentDir,'\experiments\simulated\data']);
results_parentdir=[currentDir,'\experiments\simulated\results\'];

load('aspire_simulated_KDD14.mat');

done=0;
run_no=1;
while done==0
[success,message,messageid] = mkdir(results_parentdir,['run',num2str(run_no)]);
if ~isempty(message)
    run_no=run_no+1;
else
    results_dir=[results_parentdir,'run',num2str(run_no),'\'];
    done=1;
end
end

%renumber uG
uG=unique(G); % unique group ids
G_=zeros(length(G),1);
for i=1:length(uG)
    G_(G==uG(i))=i;
end
G=G_;
[sortedG sortedInd]=sort(G,'ascend');
X=X(sortedInd,:);
G=sortedG;
Y=Y(sortedInd,:);



%tune s and kappa0
d=size(X,2);
k0=model.kappa0;
ki=model.kappa1;
m=model.m;
mu0=model.mu0;
Psi=model.Psi;
alp=(1-model.mt)*model.pt; gam=(1-model.md)*model.pd;


%% ASPIRE
fprintf(1,'Writing files...\n');
aspire_createBinaryFiles(results_dir,X,G,Psi,mu0,m,k0,ki,alp,gam);
data=[results_dir,'.matrix'];
labels=[results_dir,'_labels.matrix'];
prior=[results_dir,'_prior.matrix'];
params=[results_dir,'_params.matrix'];

num_sweeps='1500';
burn_in='1000';
step='50';
fprintf(1,'ASPIRE running...\n');
cmd = ['aspirem32.exe ',data, ' ',labels,' ',prior,' ',params,' ',num_sweeps,' ', burn_in,' ',results_dir,' ',step];
tic;
system(cmd);
elapsed_time=toc;
fprintf(1,'Reading output');
[dishes rests likelihood labels]=aspire_readOutput_ls(results_dir);

Y_pred = align_labels(labels);
[micF1,macF1,maxF,AA]=evaluate_aspire(Y,Y_pred,ones(length(Y),1));


%% Plot Results

f1=figure;
set(gca,'Color',[1 1 1]);
set(gca,'xtick',[],'ytick',[]);
hold;
uY=unique(Y);
cc=hsv(max(uY));

for i=1:length(uY)
    plot_gaussian_ellipsoid(mu(uY(i),:), Sig(:,:,uY(i)),'-',cc(uY(i),:),4,2);
end


for g=1:20
        tg=Y(G==g);
        utg=unique(tg);
    for k=1:length(utg)
        uyk=unique(Y(G==g & Y==utg(k)));
        plot(X(G==g & Y==uyk,1),X(G==g & Y==uyk,2),'k.','Marker','.','MarkerSize',2,'MarkerEdgeColor',[0 0 0]);
        for s=1:size(mui{uyk,g},1)
        plot_gaussian_ellipsoid(mui{uyk,g}(s,:), Sig(:,:,uyk),':',cc(uyk,:),4,1); 
        end
    end
end


dists = [ dishes.dist ];
for i=1:numel(dists)
    sm(i,:)=dists(i).mu;
end
acov=reshape([ dishes.scatter ],size(sm,2),size(sm,2),length(dishes));
ncnt=[dishes.nsamples];
for i=1:length(dishes)
    plot_gaussian_ellipsoid(sm(i,:), acov(:,:,i)/(ncnt(i)-1),'-',[0 0 0],4,2);
end



