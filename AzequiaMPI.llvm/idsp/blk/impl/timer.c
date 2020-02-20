/*-
 * Copyright (c) 2009 Universidad de Extremadura
 *
 * All rights reserved.
 *
 *         See COPYRIGHT in top-level directory
 */


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   implemented by this module                                   *
 *----------------------------------------------------------------*/
#include <config.h>
#include <timer.h>


/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#if defined (__OSI)
  #include <osi.h>
#else

#if defined (HAVE_CLOCK_GETTIME)
  #include <time.h>
#elif defined (HAVE_GETTIMEOFDAY)
  #include <sys/time.h>
#elif defined (AZQ_TIMER_USE_MACOSX_TIME)
  #include <mach/mach_time.h>
#else 
#error "Need to define a TIMER. Allowed kinds are: HAVE_CLOCK_GETTIME, HAVE_GETTIMEOFDAY, AZQ_USE_MACOSX_TIME"
#endif

#endif


/*----------------------------------------------------------------*
 *   Implementation of exported interface                         *
 *----------------------------------------------------------------*/

#if defined (HAVE_CLOCK_GETTIME)

    /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
   |    getAbsTime                                                      |
   |                                                                    |
   |   Get the absolute monotonic time                                  |
   |                                                                    |	 
    \____________/  ___________________________________________________/
                / _/
               /_/
			  */
double getAbsTime (void) {
  
  struct timespec tm;
  
  clock_gettime(CLOCK_REALTIME, &tm);
  
  return ((double) tm.tv_sec + ((double) tm.tv_nsec * 1.0e-9));
}


    /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
   |    getResTime                                                      |
   |                                                                    |
   |   Get the resolution of the timer                                  |
   |                                                                    |	 
    \____________/  ___________________________________________________/
                / _/
               /_/
              */
double getResTime (void) {
  
  struct timespec t;
  
  if (0 > clock_getres(CLOCK_REALTIME, &t)) {
    return ((double)1.0e-9);
  }
  
  return (double)(t.tv_sec + 1.0e-9 * t.tv_nsec);  
}



#elif defined (HAVE_GETTIMEOFDAY)


    /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
   |    getAbsTime                                                      |
   |                                                                    |
   |   Get the absolute monotonic time                                  |
   |                                                                    |	 
    \____________/  ___________________________________________________/
                / _/
               /_/
              */
double getAbsTime (void) {
  
  struct timeval tm;
  
  gettimeofday(&tm, NULL);
    
  return ((double) tm.tv_sec + ((double) tm.tv_usec * 1.0e-6));
}


    /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
   |    getResTime                                                      |
   |                                                                    |
   |   Get the resolution of the timer                                  |
   |                                                                    |	 
    \____________/  ___________________________________________________/
                / _/
               /_/
              */
double getResTime (void) {  
  return (double)1.0e-6; 
}


#elif defined (AZQ_TIMER_USE_MACOSX_TIME)


    /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
   |    getAbsTime                                                      |
   |                                                                    |
   |   Get the absolute monotonic time                                  |
   |                                                                    |	 
    \____________/  ___________________________________________________/
                / _/
               /_/
              */
double getAbsTime (void) {
  
  struct timespec  t;
  double           secs;
  uint64_t         start;
  uint64_t         nano;
  static mach_timebase_info_data_t    sTimebaseInfo;
  
  start = mach_absolute_time();
  
  if ( sTimebaseInfo.denom == 0 ) {
	(void) mach_timebase_info(&sTimebaseInfo);
  }
  
  nano = start * sTimebaseInfo.numer / sTimebaseInfo.denom;
  
  secs = nano / 1.0e+9;
  
  return secs;
}


   /*¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\
  |    getResTime                                                      |
  |                                                                    |
  |   Get the resolution of the timer                                  |
  |                                                                    |	 
   \____________/  ___________________________________________________/
               / _/
              /_/
             */
double getResTime (void) {
  
  struct timespec  t;
  double           secs;
  uint64_t         nano;
  static mach_timebase_info_data_t    sTimebaseInfo;
  
  if ( sTimebaseInfo.denom == 0 ) {
	(void) mach_timebase_info(&sTimebaseInfo);
  }
  
  nano = sTimebaseInfo.numer / sTimebaseInfo.denom;
  
  secs = (double)nano / 1.0e9;
  
  return secs;
}


#endif

