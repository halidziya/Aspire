#include "Vector.h"

inline 
double& Vector::operator[](const int i){
		return data[i];
}
