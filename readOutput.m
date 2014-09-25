function [ dishes rests]=aspire_readOutput(results_dir)
if (~exist('results_dir','var')) %Default value
    results_dir = ''; 
end


file=fopen([results_dir 'Dish.dish']);
restfile = fopen([results_dir 'Restaurant.rest']);
dishes = [];
rests = [];
ndish=fread(file,1,'int');
nrest=fread(restfile,1,'int');


for i=1:ndish
    dish=readDish(file);
    dishes = [dishes dish];
end
for i=1:nrest
    rest = readRest(restfile);
    rests = [rests rest];
end

fclose(file);
fclose(restfile);
end

function dish=readDish(file)
    dish.id=fread(file,1,'int');
    dish.ntables = fread(file,1,'int');
    dish.nsamples = fread(file,1,'int');
    dish.kap=fread(file,1,'double');
    dish.kapi=fread(file,1,'double');
    dish.logprob=fread(file,1,'double');
    dish.scatter=readMat(file);
    dish.mean=readMat(file);
    dish.d = length(dish.mean);
    dish.dist = readStut(file);
end

function mat=readMat(file)
    r = fread(file,1,'int');
    d = fread(file,1,'int');
    mat=fread(file,r*d,'double');
    mat = reshape(mat,r,d);
    % triangle = fread(file,1,'int');
end

function stut=readStut(file)
    stut.eta = fread(file,1,'double');
    stut.normalizer = fread(file,1,'double');
    stut.coef1 = fread(file,1,'double');
    stut.mu = readMat(file);
    stut.cholSigma = readMat(file);
end


function rest=readRest(file)
    rest.id = fread(file,1,'int');
    rest.likelihood = fread(file,1,'double');
    rest.ntables   =  fread(file,1,'int');
    rest.tables = readTables(file,rest.ntables);
    rest.ncustomers = fread(file,1,'int');
    rest.customers = readCust(file,rest.ncustomers);
    for i=1:length(rest.customers)
        rest.customers(i).dishid = rest.tables(rest.customers(i).tableid).dishid;
    end
end

function custs=readCust(file,n)
    custs = [];
    for i=1:n
        cust.likelihood0 = fread(file,1,'double');
        cust.tableid = fread(file,1,'int');
        cust.data = readMat(file);
        custs = [custs cust];
    end
end

function tables=readTables(file,n)
    tables = [];
    for i=1:n
        table.tableid = fread(file,1,'int');
        table.dishid = fread(file,1,'int');
        table.npoints = fread(file,1,'int');
        table.likelihood = fread(file,1,'double');
        table.scatter = readMat(file);
        table.mean = readMat(file);
        table.dist = readStut(file);
        tables = [ tables table];
    end
end
