/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#ifdef __OSI
#include <osi.h>
#endif
#include <stdio.h>
#include <azq.h>
#include <malloc.h>
#include <time.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/
unsigned long long COM_now2(void) {

  struct timespec      now;
  unsigned long long   milliseconds;
  if(clock_gettime(CLOCK_REALTIME, &now))                                       goto exception;

  milliseconds = ((unsigned long long)now.tv_sec) * 1000 +
                 ((unsigned long long)now.tv_nsec) / 1000000;
//fprintf(stdout, "NOW: %ld milliseconds\n", milliseconds);
  return(milliseconds);

exception:
  fprintf(stdout, "COM_now2: error\n");
  return(-1);
}

     /*----------------------------------------------------------*\
    |    timed_radiator                                            |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/

#define ITERA   (100000)
#define PRINT_PERIOD 1
//#define ITERA   (0xEFFFFFFF)
#define SEND_TIMEOUT     1
#define RECV_TIMEOUT     6
//#define TIMEOUT   GC_FOREVER
int timed_operator(int *param) {

  int              senders,
                   receivers,
                   bufSize        = param[0] * sizeof(int),
                  *value,
                   acum        = 0;
  int              excpn       = 0;
  int              i, j,
                   grpSize,
                   gix         = getGroup();
  int              myRank      = getRank();
  Thr_t            me          = THR_self();
//int              itr         = 0;
  int              tag = 32;
  struct timespec      resolution;

  GRP_getSize(gix, &grpSize);
  senders   = param[1];  
  receivers = param[2]; 
  
  fprintf(stdout, "***** Timed Operator (%x), (Gix %x, Rank %d) pth %x\n", (unsigned int)me, gix, myRank, (int)me->Self);

  if(myRank == 0) {  
    
    GC_Rqst_t    rqst;
    int          itr    = 0;
    int          i;

    /* Get the memory for the data to be sent */
    if(NULL == (value = (int *)malloc(bufSize))) {
      fprintf(stdout, "::::::::::::::::: Bug Operator: Malloc Exception %d ::::::::::::::::\n", -5);
      return(-5);
    }
    for(i = 0; i < bufSize/sizeof(int); i++) {
      value[i] = i;
      acum += i;
    }
    
    if (clock_getres(CLOCK_REALTIME, &resolution)) exit(1);
    fprintf(stdout, "CLOCK RESOLUTION: Sec: %ld Nanosec: %ld\n", resolution.tv_sec, resolution.tv_nsec);
//fprintf(stdout, "NOW: %ld milliseconds\n", milliseconds);

    while (1) {
      
      //if(itr % PRINT_PERIOD == 0) fprintf(stdout, "\t\t= S %d ============== itr [%d]...\n", myRank, itr );
      
      //for(i = 0; i < bufSize/sizeof(int); i++)
      //  value[i] = itr;

      //usleep(20001);

      if(0 > (excpn = GC_send((char *)(&value[0]), bufSize, 1, tag))) {
      //if(0 > (excpn = GC_timed_send((char *)(&value[0]), bufSize, 1, tag, SEND_TIMEOUT))) {
        fprintf(stdout, "\n:: S :::::: Radiator(%x). (Gix %x, Rank %d): Exception %d :::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
        free(value);
        return(excpn);
      }
      
      //if (itr == 40) sleep(1);
 
      /*if(0 > (excpn = GC_asend((char *)(&value[0]), bufSize, senders + receivers, 0, &rqst))) {
        fprintf(stdout, "\n:: S :::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d :::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
        free(value);
        return(excpn);
      }*/

      /*if(0 > (excpn = GC_wait(rqst, NULL)))  {
      //  if(0 > (excpn = GC_timed_wait(rqst, NULL, SEND_TIMEOUT)))  { 
         fprintf(stdout, "\n:: S ::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d :::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            free_request(rqst);
            free(value);
            return(excpn);
          }
          break;
      }*/

      if (++itr == ITERA)  break;
    }
    if(value)
      free(value);
  }


  else  { 
    GC_Rqst_t        rqst;
    GC_Status        status;
    int              rango;
    int              itr = 0, i;
    long long        now_1;
    long long        now_0;
    long long        t_start, t_end;
    struct timespec delay = {0, 2000000000};
    
    if(NULL == (value = (int *)malloc(bufSize))) {
      fprintf(stdout, "::::::::::::::::: Bug Operator: Malloc Exception %d ::::::::::::::::\n", -5);
      return(-5);
    }


    now_0 = COM_now2();
    while (1) {
      //if(itr % PRINT_PERIOD == 0)    fprintf(stdout, "\t\t= R %d ============== itr [%d]...\n", myRank, itr );

      t_start = COM_now2();
      if(0 > (excpn = GC_timed_recv((char *)(&value[0]), bufSize, 0, tag, &status, RECV_TIMEOUT))) {
        t_end = COM_now2();
        fprintf(stdout, "------>>>>>>>>> Time:  %ld\n", t_end - t_start);
        fprintf(stdout, "\n:: R ::::::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
        return(excpn);
      }
        t_end = COM_now2();
        if ((t_end - t_start) > RECV_TIMEOUT) {
          fprintf(stdout, "------>>>>>>>>> Iter:  %d ", itr);
          fprintf(stdout, "------>>>>>>>>> Time:  %ld \n", t_end - t_start);
        }


      /*if (0 > (excpn = GC_recv ((char *)(&value[0]), bufSize, 0, tag, &status ))) {
            fprintf(stdout, "\n:: R ::::::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            return(excpn);
          }
        */
          
          //if (itr > 40) nanosleep(&delay, NULL);
          //if (itr == 40) sleep(1);
        
        /*if(0 > (excpn = GC_arecv((char *)(&value[0]), bufSize, 0, tag, &rqst))) {
            fprintf(stdout, "\n:: R ::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d in GC_arecv ::::::::::::::::\n", (int)(THR_self()), gix, myRank, excpn);
            return(excpn);
          }
        
        t_start = COM_now2();  
       if(0 > (excpn = GC_timed_wait(rqst, &status, RECV_TIMEOUT)))  {
        t_end = COM_now2();
        fprintf(stdout, "------>>>>>>>>> Time:  %ld\n", t_end - t_start);          
            fprintf(stdout, "\n:: R ::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d  in GC_wait_recv ::::::::::::::::\n", (int)(THR_self()), gix, myRank, excpn);
            free_request(rqst);
            return(excpn);
          }        
        t_end = COM_now2();
        if ((t_end - t_start) > 1) {
          fprintf(stdout, "------>>>>>>>>> Iter:  %d ", itr);
          fprintf(stdout, "------>>>>>>>>> Time:  %ld \n", t_end - t_start);
        }
        */
     
      if (++itr == ITERA)   break;
    }
    
    now_1 = COM_now2();
    fprintf(stdout, "\nmilliseconds %ld\n", (now_1 - now_0));
    fprintf(stdout, "%f us/msg \n", 1000*(((float)(now_1 - now_0)) / (float)ITERA)  );
  }

  fprintf(stdout, "e :::: Timed Operrator %x %x, Rank %d): ADIOS !!!!\n", (unsigned int)me, gix, myRank);
  return(0);
}
