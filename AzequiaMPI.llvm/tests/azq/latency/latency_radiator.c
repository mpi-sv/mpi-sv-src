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
unsigned long long COM_now2(void)
{
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
#define PRINT_PERIOD 100
//#define ITERA   (0xEFFFFFFF)
#define SEND_TIMEOUT    1
#define TIMEOUT         1
//#define TIMEOUT   GC_FOREVER
int timed_radiator(int *param)
{
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

  GRP_getSize(gix, &grpSize);
  senders   = param[1];  
  receivers = param[2]; 
  if((senders + receivers + 1) != grpSize) {
      fprintf(stdout, "radiator(%x), (Gix %x, Rank %d) pth %x, Bad Parameters\n", (unsigned int)me, gix, myRank, (int)me->Self);
      return(-1);
  }

  fprintf(stdout, "***** Radiator(%x), (Gix %x, Rank %d) pth %x\n", (unsigned int)me, gix, myRank, (int)me->Self);
  if(myRank < senders) {  /* S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S S */
    GC_Rqst_t    rqst;
    int          itr    = 0;
    int          i;

    //fprintf(stdout, "** S ************************************************************************************\n");
    //fprintf(stdout, "Radiator(%x). (Gix %x, Rank %d)\n", (unsigned int)(THR_self()), gix, myRank);

    /* Get the memory for the data to be sent */
    if(NULL == (value = (int *)malloc(bufSize))) {
      fprintf(stdout, "::::::::::::::::: Bug Operator: Malloc Exception %d ::::::::::::::::\n", -5);
      return(-5);
    }
    for(i = 0; i < bufSize/sizeof(int); i++) {
      value[i] = i;
      acum += i;
    }
  //fprintf(stdout, "\t\t\t\tData_Source: GOOD RESULT = %d\n", acum);
  //fprintf(stdout, "Data_Source: antes de CHN_open, ");
    while (1) {
      //if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= S %d ============== itr [%d]...\n", myRank, itr );
      
      //for(i = 0; i < bufSize/sizeof(int); i++)
      //  value[i] = itr;

      //usleep(20001);
      switch(itr % 2) {
        case 0:
          if(0 > (excpn = GC_send((char *)(&value[0]), bufSize, senders + receivers, 0))) {
          //if(0 > (excpn = GC_timed_send((char *)(&value[0]), bufSize, senders + receivers, 0, SEND_TIMEOUT))) {
            fprintf(stdout, "\n:: S ::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            free(value);
            return(excpn);
          }
          break;
        case 1:
          //fprintf(stdout, "= S ============== Radiator-S(%x)  Rank %d:  Send (async) [%d]\n", (unsigned int)(THR_self()), myRank, itr);
          //for(i = 0; i < 40000; i++);
          if(0 > (excpn = GC_asend((char *)(&value[0]), bufSize, senders + receivers, 0, &rqst))) {
            fprintf(stdout, "\n:: S ::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            free(value);
            return(excpn);
          }

          //fprintf(stdout, "= S ============== Radiator-S(%x):  wait...\n", (unsigned int)(THR_self()) );
          //for(i = 0; i < 40000; i++);
        //if(0 > (excpn = GC_wait(rqst, NULL)))  {
          if(0 > (excpn = GC_timed_wait(rqst, NULL, SEND_TIMEOUT)))  { 
            fprintf(stdout, "\n:: S ::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            free_request(rqst);
            free(value);
            return(excpn);
          }
          break;
      }
      if (++itr == ITERA) 
        break;
    }
    if(value)
      free(value);
  }

  else if(myRank == (senders + receivers)) {  /* CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR CR */
    GC_Rqst_t      rqst  [senders + receivers];
    GC_Status      status[senders + receivers];
    GC_Status      statusR; 
    int            index[senders + receivers];
    int            sum = 0, 
                   itr = 0, 
                   k, i, j,
                   outCnt, doneCnt,
                  *s[senders], rango[receivers];

    
    //fprintf(stdout, "** CR ************************************************************************************\n");
    //fprintf(stdout, "Radiator(%x). (Gix %x, Rank %d)\n", (unsigned int)(THR_self()), gix, myRank);

    for(j = 0; j < senders; j++) {
      if(NULL == (s[j] = (int *)malloc(bufSize)))   {
        fprintf(stdout, "::::::::::::::::: Radiator(%x) (R): Malloc Exception %d :::::::::::::::: free ... ", (int)THR_self(), -5);
        for(i = 0; i < j; i++)
          free(s[i]);
        fprintf(stdout, "Tras free (%x)\n", (int)THR_self());
        return(-5);
      }
      s[j][0] = 2;
    }

    while(1) {
      int cual;

      //if(itr % PRINT_PERIOD == 0)     fprintf(stdout, "\t\t= CR ============== itr [%d]...\n", itr );
      for(j = 0; j < senders; j++) {
        if(0 > (excpn = GC_arecv ((char *)s[j], bufSize, j, 0, &rqst[j])))  {
          fprintf(stdout, "\n::::::::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
          for(i = 0; i < j; i++) 
            free_request(rqst[i]);
          for(i = 0; i < senders; i++)
            free(s[i]);
          return(excpn);
        }
      }

      for(j = 0; j < receivers; j++) {
        rango[j] = senders + j;
        if(0 > (excpn = GC_asend ((char *)&rango[j], sizeof(int), senders + j, 9, &rqst[senders + j])))  {
          fprintf(stdout, "\n::::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
          for(i = 0; i < senders + j; i++) 
            free_request(rqst[i]);
          for(i = 0; i < senders; i++)
            free(s[i]);
          return(excpn);
        }
      }

      switch(itr % 4) {
        case 0:
          /* WAITANY test */
          for(k = 0; k < senders + receivers; k++) {
            if(0 > (excpn = GC_timed_waitany(rqst, senders + receivers, &cual, &statusR, TIMEOUT))) {
            //if(0 > (excpn = GC_waitany(rqst, senders + receivers, &cual, &statusR))) {
                fprintf(stdout, "\n:: CR ::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
                for(j = 0; j < senders + receivers; j++) 
                  free_request(rqst[j]);
                for(i = 0; i < senders; i++)
                  free(s[i]);
                return(excpn);
            }
          }
          break;
        case 1:
          if(0 > (excpn = GC_timed_waitany(rqst, senders + receivers, &cual, &statusR, TIMEOUT))) {
        //if(0 > (excpn = GC_waitany      (rqst, senders + receivers, &cual, &statusR         ))) {
              fprintf(stdout, "\n:: CR ::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
              for(j = 0; j < senders + receivers; j++) 
                  free_request(rqst[j]);
              for(i = 0; i < senders; i++)
                  free(s[i]);
              return(excpn);
          }
          /* WAIT test */
          for(j = 0; j < senders + receivers; j++) {
            if(rqst[j] == (Rqst_t)AZQ_RQST_NULL) continue;
            if(0 > (excpn = GC_wait(rqst[j], &statusR))) {
              fprintf(stdout, "\n:: CR ::::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
              for(i = j; i < senders + receivers; i++) {
                if(i == cual) continue;
                free_request(rqst[i]);
              }
              for(i = 0; i < senders; i++)
                free(s[i]);
              return(excpn);
            }
          }
          break;
        case 2:
          /* WAITALL test */
          if(0 > (excpn = GC_waitall(rqst, senders + receivers, status))) {
            fprintf(stdout, "\n:: CR :::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);   
            for(i = 0; i < senders + receivers; i++) {
              fprintf(stdout, ":: CR(%x). Status[%d] = %d\n", (unsigned int)(THR_self()), i, status[i].Error);
              if(status[i].Error == AZQ_WAIT_ERR_PENDING)
                free_request(rqst[i]);
            }            
            for(i = 0; i < senders; i++)
              free(s[i]);
            return(excpn);
          }
          break;
        case 3:
          /* WAITSOME test */
          doneCnt = 0;
          do {
            if(0 > (excpn = GC_waitsome(rqst, senders + receivers, index, status, &outCnt))) {
              fprintf(stdout, "\n:: CR :::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);   
              for(i = 0; i < senders + receivers; i++) {
                fprintf(stdout, ":: CR(%x). Status[%d] = %d\n", (unsigned int)(THR_self()), i, status[i].Error);
                if(status[i].Error == AZQ_WAIT_ERR_PENDING)
                  free_request(rqst[i]);
              }            
              for(i = 0; i < senders; i++)
                free(s[i]);
              return(excpn);
            }
            //fprintf(stdout, ":: CR(%x). GC_waitsome: outCnt = %d\n", (unsigned int)(THR_self()), outCnt);
            if(senders + receivers == (doneCnt += outCnt))
              break;
          } while(outCnt != AZQ_UNDEFINED);
          break;
      }

      /*---------- 2. Run the ALGORITHM ---------
      for(j = 0; j < senders; j++)
        for(i = 0; i < param[0]; i++) {
          if(s[j][i] != itr) {
            fprintf(stdout, "\n::::::::::::::::: Radiator (%x). USR Fail!!. Itr = %d/Value = %d ::::::::::::::::\n", (int)THR_self(), itr, s[j][i]);
            exit(0); 
            return(-1);
          }
        }
      */
      
      /*if(senders) {
        int k;
        for (k = 0; k< 160; k++) {
          sum = 0;
          for(i = 0; i < param[0]; i++) {
            for(j = 0; j < senders; j++)
              sum += (s[j][i]);
          }
        }
      //fprintf(stdout, "::::::::::::::::: Adder Operator (tsk ).  SUM = %d ::::::::::::::::", sum); fflush(stdout);
      //---------- 3. Print results ---------
        if(sum != 2016 * senders) {
          fprintf(stdout, "\n::::::::::::::::: Radiator (%x). Bad SUM = %d ::::::::::::::::\n", (int)THR_self(), sum);
          excpn = THR_E_EXHAUST;
          fprintf(stdout, "\n::::::::::::::::: Radiator(%x): Exception %d ::::::::::::::::\n", (int)THR_self(), excpn);
          for(j = 0; j < senders; j++)
            free(s[j]);
          return(excpn);
        }
      }*/
      //fprintf(stdout, "\t\t= CR ============== Radiator [%d]. End =====================\n\n\n", itr );
      if (++itr == ITERA) {
        break;
      }

    }
    for(j = 0; j < senders; j++) {
      if(s[j])
        free(s[j]);
    }
  }


  else /*if(myRank > senders)*/ { /* R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R R */
    GC_Rqst_t        rqst;
    GC_Status        status;
    int              rango;
    int              itr = 0, i;
    long long        now_1;
    long long        now_0;

    //fprintf(stdout, "** R ************************************************************************************\n");
    //fprintf(stdout, "Radiator(%x). (Gix %x, Rank %d)\n", (unsigned int)(THR_self()), gix, myRank);
    now_0 = COM_now2();
    while (1) {
      //if(itr % PRINT_PERIOD == 0)  fprintf(stdout, "\t\t= R %d ============== itr [%d]...\n", myRank, itr );

      //fprintf(stdout, "= R ============= Radiator-R(%x)  Rank %d:  Receive (async)\n", (unsigned int)(THR_self()), myRank);
      //for(i = 0; i < 40000; i++);
      switch(itr % 3) {
        case 0:
          if(0 > (excpn = GC_timed_recv((char *)(&rango), sizeof(int), senders + receivers, 9, &status, TIMEOUT))) {
            fprintf(stdout, "\n:: R ::::::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            return(excpn);
          }
          break;
        case 1:
          if(0 > (excpn = GC_recv      ((char *)(&rango), sizeof(int), senders + receivers, 9, &status         ))) {
            fprintf(stdout, "\n:: R ::::::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d ::::::::::::::::\n", (unsigned int)(THR_self()), gix, myRank, excpn);
            return(excpn);
          }
          break;
        case 2:
          if(0 > (excpn = GC_arecv((char *)(&rango), sizeof(int), senders + receivers, 9, &rqst))) {
            fprintf(stdout, "\n:: R ::::::::::: Radiator-R(%x). (Gix %x, Rank %d): Exception %d in GC_arecv ::::::::::::::::\n", (int)(THR_self()), gix, myRank, excpn);
            return(excpn);
          }
          if(0 > (excpn = GC_timed_wait(rqst, &status, TIMEOUT)))  {
            fprintf(stdout, "\n:: R ::::::::::::: Radiator(%x). (Gix %x, Rank %d): Exception %d  in GC_wait_recv ::::::::::::::::\n", (int)(THR_self()), gix, myRank, excpn);
            free_request(rqst);
            return(excpn);
          }         
          break;
      }
     
      if(myRank != rango) {
        fprintf(stdout, "= R ============== Radiator-R(%x):  Bad range %d\n", (unsigned int)(THR_self()), rango );
        return(-1);
      }
      //if(itr % PRINT_PERIOD == 0)
      //  fprintf(stdout, "\t\t= R %d ============== itr [%d]\n", myRank, itr );
      if (++itr == ITERA) 
        break;
    }
    now_1 = COM_now2();
    fprintf(stdout, "\nmilliseconds %lld\n", (now_1 - now_0));
    fprintf(stdout, "%f us/msg \n", 1000*(((float)(now_1 - now_0)) / (float)ITERA)  );
  }

  fprintf(stdout, "e :::: Radiator(%x). (Gix %x, Rank %d): ADIOS !!!!\n", (unsigned int)me, gix, myRank);
  return(0);
}
