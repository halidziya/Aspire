#pragma once
#include "ThreadPool.h"
#include "Customer.h"

class CustDish : public Task
{
public:
  typedef list<vector<Customer>::iterator> table_list;
  CustDish(list<vector<Customer>::iterator>* intable,list<Dish>::iterator d):intable(intable),dish(d)
  {}
  CustDish():intable(NULL)
  {}
  ~CustDish(void){};
  table_list* intable;
  list<Dish>::iterator dish;
  double likelihood;
  double onelikelihood;
  int npoints;
  void run(int id)
  {

    matbuffer.threadid = id;
    buffer.threadid = id;
    absbuffer.threadid = id;

    // printf("Thread %d ond ish: %d  buffer:%d\n",id,dish->dishid,buffer.threadid);
    likelihood=0;
    npoints=0;
    onelikelihood = dish->dist.likelihood ((intable->front ())->data);
    for(table_list::iterator points=intable->begin();points!=intable->end();points++)
    {	
      likelihood+=dish->dist.likelihood((*points)->data);
      npoints++;
    }

    likelihood += intable->size() * log(dish->ntables); // Likelihood
  }
};

