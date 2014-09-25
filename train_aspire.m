clear;
k0 = 0.1;
ki = 0.5;
load('experiments/simulated/data/aspire_simulated_KDD14.mat')
aspire_createBinaryFiles('data/toy_KDD',X,G,model.Psi,model.mu0,model.m,k0,ki,0.2,0.2)
!aspirem32.exe data/toy_KDD.matrix data/toy_KDD_labels.matrix data/toy_KDD_prior.matrix data/toy_KDD_params.matrix
fprintf(1,'Reading output ...');
[dishes restaurants]=readOutput();
alltables = [restaurants.tables];
allcust=[restaurants.customers];
[Fscore,maxF,AA]=evaluate_aspire(Y,[ allcust.dishid ],ones(length(G),1));
