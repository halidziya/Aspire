function aspire_createBinaryFiles(filename,X,G,Psi,mu0,m,kappa,kappai,alpha,gamma)
    mkdir('data');
    writeMat([ filename '.matrix'],X,'double');
    writeMat([ filename '_labels.matrix'],G,'double');
    writeMat([ filename '_params.matrix'],[size(mu0,2) m kappa kappai alpha gamma],'double');
    writeMat([ filename '_prior.matrix'],[Psi;mu0],'double');

end


function writeMat(filename,mat,prec)
[n m]=size(mat);
file=fopen(filename,'w');
fwrite(file,n,'int');
fwrite(file,m,'int');
fwrite(file,mat',prec);
fclose(file);
end