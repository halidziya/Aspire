#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "DebugUtils.h"
#include "DataSet.h"
#include "util.h"
#include "Dish.h"
#include "Restaurant.h"
#include "Customer.h"
#include <thread>  
#include <iostream>
#include <string>


using namespace std;


int MAX_SWEEP=100;
int BURNIN=10;
int SAMPLE= 10;// Default value is 10 sample + 1 post burnin
string result_dir = "./";

// Variables
int d,n;
double m,kep,eta;


PILL_DEBUG
int main(int argc,char** argv)
{
	debugMode(1);
	char* datafile = argv[1];
	char* labelfile = argv[2];
	char* priorfile = argv[3];
	char* configfile = argv[4];

	srand(time(NULL));
        
	// Default values , 1000 , 100  , out
	if (argc>5)
		MAX_SWEEP = atoi(argv[5]);
	if (argc>6)
		BURNIN = atoi(argv[6]);
	if (argc > 7)
		result_dir = argv[7];
	SAMPLE = (MAX_SWEEP-BURNIN)/10; // Default value
	if (argc>8)
		SAMPLE = atoi(argv[8]);
	
	step();
					 // Computing buffer
        
	string ss(result_dir);
	ofstream nsampleslog(ss.append("nsamples.log"));

	printf("Reading...\n");
	DataSet ds(datafile,labelfile,priorfile,configfile);
	d = ds.d;
	n = ds.n;
	kep = ds.kappa*ds.kappai/(ds.kappa+ds.kappai);
	eta = ds.m - d + 2;

	precomputeGammaLn(2*(n+d)+1);  // With 0.5 increments
	init_buffer(thread::hardware_concurrency(),d);	
	
	Vector priormean(d); 
	
	Matrix priorvariance(d,d);
	
	Global::Psi = ds.prior;
	
	Global::Psi.r = d; // Last row is shadowed
	Global::Psi.n = d*d;
	Global::mu0 =  ds.prior(ds.d).copy();
	
	Global::eta = eta;
	priorvariance = Global::Psi*((kep+1)/((kep)*eta));
	priormean = Global::mu0;
	
	Stut stt(priormean,priorvariance,eta); 
	Vector loglik0;
	
	Dish emptyDish(stt);
	vector<Restaurant> Restaurants;
	vector<Restaurant> beststate;
	Matrix     sampledLabels((MAX_SWEEP-BURNIN)/SAMPLE + 1,ds.n);
	vector<vector<Restaurant>::iterator> Restaurantit; // Hash table
	// vector<Customer> customers;
	list<Dish> franchise;
	list<Dish> bestdishes;
	Table besttable;
	Customer bestcustomer;
	int i,j,k;
	
// INITIALIZATION
	printf("Number of Cores : %d\n",thread::hardware_concurrency());

	step();
	loglik0		= emptyDish.dist.likelihood(ds.data);
	Vector Restaurantids	= ds.labels.unique();
	Restaurantit.resize(Restaurantids.maximum()+1); // Hash table for fast access
	Restaurants.resize(Restaurantids.n);
	// customers.reserve(ds.n);
	step();
	
	// One cluster for each object initially
	franchise.push_back(Dish(ds.d));
	list<Dish>::iterator firstDish = franchise.begin();

	// Create Restaurants
	for (i=0;i<Restaurantids.n;i++)
	{

		Restaurants[i].Restaurantid = Restaurantids(i);
		Restaurantit[Restaurantids(i)] = Restaurants.begin() + i;
		Table t(firstDish); // First dish
		Restaurants[i].addTable(t); 
	}
	
	// Create customers
	vector<Restaurant>::iterator g;
	for(i=0;i<ds.n;i++)
	{
		g = Restaurantit[ds.labels(i)];
                Vector& v= ds.data(i);
		g->tables.begin()->addInitPoint(v);
		g->customers.emplace_back(ds.data(i),loglik0[i],g->tables.begin());
	}

	for (i=0;i<Restaurantids.n;i++)
	{
		Restaurants[i].tables.front().calculateCov();
		firstDish->addCluster(Restaurants[i].tables.front());
	}
        
	step();
	firstDish->calculateDist();
	for (i=0;i<Restaurantids.n;i++)
		Restaurants[i].tables.front().calculateDist();
//END INITIALIZATION

// GIBBS SAMPLER


	// NUMBER OF THREADS
	// =================
	ThreadPool tpool(thread::hardware_concurrency());
	// ==================



	list<Dish>::iterator dit,ditc;
	list<Table>::iterator tit;
	vector<Customer>::iterator cit;
	list<vector<Customer>::iterator> intable;
	list<vector<Customer>::iterator>::iterator points;

	double newdishprob,maxdishprob,sumprob,val,logprob,gibbs_score,best_score;
	int kal=1;
	gibbs_score = -my_infinity();
	best_score = -my_infinity();

	Vector score(MAX_SWEEP+1);
	for (int num_sweep = 0;num_sweep <= MAX_SWEEP ; num_sweep++)
	{
		// Submit Jobs
		// 1st Loop
		for (i=0;i<Restaurants.size();i++)
			tpool.submit(Restaurants[i]);	
		tpool.waitAll(); // Wait for finishing all jobs

		

		for(dit=franchise.begin();dit!=franchise.end();dit++) // For each dish
		{
			dit->reset(); // To start recalculation
		}

		for (i=0;i<Restaurants.size();i++)
		{
			for(tit=Restaurants[i].tables.begin();tit!=Restaurants[i].tables.end();tit++)
			{
				tit->dishp->addCluster(*tit);
			}
		}

		for(dit=franchise.begin();dit!=franchise.end();dit++) // For each dish
		{
			dit->calculateDist();
		}
		

		// 3rd Loop
		for(dit=franchise.begin();dit!=franchise.end();dit++)
		{
			dit->logprob = 0;
			nsampleslog << dit->nsamples << " ";
		}
		nsampleslog << endl;

		for (i=0,kal=1;i<Restaurants.size();i++)
		{
			// Each table
			for(tit=Restaurants[i].tables.begin();tit!=Restaurants[i].tables.end();tit++,kal++)
			{
				tit->dishp->removeCluster(*tit);
				
				if (tit->dishp->ntables==0) // Remove dish 
					franchise.erase(tit->dishp);
				else
					tit->dishp->calculateDist();
				if (intable.size()>0)
				{
					int sss = intable.size();
					intable.clear();
				}
				

				// Create list of customers
				for(cit=Restaurants[i].customers.begin();cit!=Restaurants[i].customers.end();cit++)
				{
					if (cit->table == tit)
						intable.push_back(cit);
				}

				
				newdishprob = tit->npoints * log(Global::gamma);
				for(points=intable.begin();points!=intable.end();points++)
					newdishprob += (*points)->loglik0;

				maxdishprob = newdishprob;
				for(dit=franchise.begin();dit!=franchise.end();dit++) 
				{
					logprob=0;
					for(points=intable.begin();points!=intable.end();points++)
					{
						// Here is computationally time consuming Under 4 for loops matrix division !!!!!!
						logprob+=dit->dist.likelihood((*points)->data);
					}
					dit->logprob = logprob + tit->npoints * log(dit->ntables); //Prior


					if (maxdishprob<dit->logprob)
						maxdishprob = dit->logprob;
				}

				sumprob=0;
				for(dit=franchise.begin();dit!=franchise.end();dit++) 
				{
					dit->logprob = exp(dit->logprob - maxdishprob);
					sumprob += dit->logprob;
                                        
				}

				sumprob += exp(newdishprob - maxdishprob);

				double rrr = urand();
				val = rrr*sumprob;
				for(dit=franchise.begin();dit!=franchise.end();dit++) 
				{
					if ((dit->logprob)>=val)
						break;
					else
						val -= dit->logprob;
				}

				if (dit==franchise.end()) // Create new dish
				{
					franchise.emplace_back(d);
					dit = franchise.end();
					dit--; // Point to actual dish
				}

				tit->dishp = dit;
				dit->addCluster(*tit);
				dit->calculateDist();
			}
		}


		// 4th loop 
		for (i=0;i<Restaurants.size();i++)
		{
			// Each table
			for(tit=Restaurants[i].tables.begin();tit!=Restaurants[i].tables.end();tit++)
			{
				tit->calculateDist();
			}
				
		}
		
		
		// Calculate Gibbs Score
		gibbs_score = 0;
		for (i=0;i<Restaurants.size();i++)
			gibbs_score += Restaurants[i].likelihood;
		score[num_sweep] = gibbs_score;
		if ( best_score < gibbs_score || num_sweep == 0)
		{
			best_score = gibbs_score ;
			bestdishes = franchise;
			for (dit=franchise.begin(),ditc=bestdishes.begin();dit!=franchise.end();dit++,ditc++)
				dit->copy = ditc ;
                        beststate.resize(Restaurants.size());
                        for (i=0;i<Restaurants.size();i++)
                                beststate[i] << Restaurants[i];
		}

		if  (((num_sweep-BURNIN)%SAMPLE)==0 && num_sweep >= BURNIN)
		{
			for (dit=franchise.begin(),i=0;dit!=franchise.end();dit++,i++)
				dit->dishid = i;
			j=0;
			for (i=0;i<Restaurants.size();i++)
				for(cit=Restaurants[i].customers.begin();cit!=Restaurants[i].customers.end();cit++)
					sampledLabels((num_sweep-BURNIN)/SAMPLE)[j++] = cit->table->dishp->dishid;
		}

		printf("Iter %d nDish %d Score %.1f\n",num_sweep,(int)franchise.size(),gibbs_score);
		flush(cout);
		// 2nd Loop
	}
	step();
        

	franchise = bestdishes;
	for (dit=franchise.begin(),ditc=bestdishes.begin();dit!=franchise.end();dit++,ditc++)
				ditc->copy = dit;
        
        Restaurants.resize(beststate.size());
        for (i=0;i<beststate.size();i++)
            Restaurants[i] << beststate[i];
	

string s(result_dir);
ofstream dishfile( s.append("Dish.dish"),ios::out | ios::binary); // Be careful result_dir should include '\'
s.assign(result_dir);
ofstream restfile( s.append("Restaurant.rest"),ios::out | ios::binary);
s.assign(result_dir);
ofstream likefile( s.append("Likelihood.matrix"),ios::out | ios::binary);

s.assign(result_dir);
ofstream labelsout( s.append("Labels.matrix"),ios::out | ios::binary);

	//ofstream dishfile("Dish.dish",ios::out | ios::binary);
	//ofstream restfile("Restaurant.rest",ios::out | ios::binary);
        //======
	int ndish = franchise.size();
	int nrest = Restaurants.size();
	dishfile.write((char*)& ndish,sizeof(int));
	restfile.write((char*)& nrest,sizeof(int));

	for(i=0,dit=franchise.begin();dit!=franchise.end();i++,dit++) 
	{
		dit->dishid = i+1;
		dishfile <<  *dit;
	}
	dishfile.close();


	for (i=0;i<nrest;i++)
	{
		restfile << Restaurants[i];
	}

	restfile.close();

	labelsout << sampledLabels;
	labelsout.close();

	likefile << score;
	likefile.close();

	printf("--%d--\n",sampledLabels.r);
	nsampleslog.close();

        printf("Output is written into files...\n");
}