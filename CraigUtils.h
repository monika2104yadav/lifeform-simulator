#ifndef _CraigUtils_h
#define _CraigUtils_h 1
#include <cassert>
#include <cmath>

template <class T>
class Nil {
public:
	operator T* () { return static_cast<T*>(0); }
};
//NOTE: static_cast<T*>(0) is basically yielding NULL pointer to the destination type.
template <class T>
void Delete(T*& x) {
	delete x;
	x = Nil<T>();
}

inline double drand48(void) {
	double result = (double)rand() / (double) 32768.0;
	assert(result >= 0.0 && result <= 1.0);
	return result;
}

#define MAXFLOAT (1.0e127)
#endif /* _CraigUtils_h */

