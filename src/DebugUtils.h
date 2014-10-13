#pragma once

#ifdef _WIN32
  #include <Windows.h>
  static LARGE_INTEGER toc;
#elif defined(__linux)
  #include <sys/time.h>
  typedef long long __int64;
	static __int64 toc=0;
#endif

#include <stdio.h>
#include <vector>

static bool debugOn;

inline
__int64 reset(void)
{
  __int64 t;
  #ifdef _WIN32
    LARGE_INTEGER end_time, frequency;
    QueryPerformanceCounter( &end_time );
    QueryPerformanceFrequency( &frequency );
    t = ( end_time.QuadPart - toc.QuadPart )*1000000/ frequency.QuadPart;
    toc = end_time;
  #elif defined(__linux)
    timeval a;
    gettimeofday(&a, 0);
    __int64 tic=a.tv_sec*1000 + a.tv_usec/1000.0 +0.5;
    t = tic-toc;
    toc=tic;
  #endif
  return t;
}
inline
void step(void){
	if(debugOn)    
	{
		printf("Elapsed: %d\n",(int)reset()); 
	} 
}
#define pri(_ii)  if(debugOn) { printf("%d\n",_ii); }
#define prf(_dd)  if(debugOn) { printf("%f\n",(double)_dd); } 
#define prs(_ss)  if(debugOn) { printf(_ss); } 
#ifdef _WIN32
  #define PILL_DEBUG extern LARGE_INTEGER toc; extern bool debugOn;
#elif defined(__linux)
  #define PILL_DEBUG extern __int64 toc; extern bool debugOn;
#endif
#define debugMode(__open)  debugOn=__open;