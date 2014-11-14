#ifndef UTIL_H
#define UTIL_H

// funzioni varie di utilita'
template<class T>
void clamp(T & r, T low, T hi)
{
	if(r < low) r = low;
	if(r > hi) r = hi;
}


template <class T>
bool inRange(T r, T low, T hi)
{ return r >= low && r <= hi;}
#endif