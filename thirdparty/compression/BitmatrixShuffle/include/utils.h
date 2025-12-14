#ifndef UTILS_H
#define UTILS_H

#include <cfloat>

#define BMS_NULL_DISTANCE (-DBL_MAX)
#define BMS_ALLOCATE_MATRIX(nrows, ncols) new char[(nrows)*((ncols)/8)]
#define BMS_DELETE_MATRIX(ptr) delete[] (ptr)

    
#endif