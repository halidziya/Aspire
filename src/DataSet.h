#pragma once
#include "Matrix.h"
#include "Global.h"
#include "Stut.h"

class DataSet : public Global
{
public:
	DataSet(char* datafile,char* labelfile,char* priorfile,char* configfile);
	Matrix data;
	Vector labels;
	Matrix prior;
	~DataSet(void);
};

