/*----------------------------------------------------------------*
 *   Declaration of public types and functions                    *
 *   used by this module                                          *
 *----------------------------------------------------------------*/
#include <stdio.h>
#include <azq.h>
#include <malloc.h>
#include <time.h>


/*----------------------------------------------------------------*
 *   Definition of private data                                   *
 *----------------------------------------------------------------*/
static int cnt[2];


/*----------------------------------------------------------------*
 *   Implementation of private functions                          *
 *----------------------------------------------------------------*/

     /*----------------------------------------------------------*\
    |    adder_no_chn                                              |
    |                                                              |
    |                                                              |
     \*----------------------------------------------------------*/
int adder_no_chn(int *dim)
{
  int              size        = *dim * sizeof(int),
                  *value,
                   acum        = 0;
  int              excpn       = 0;
  int              i,
                   grpSize,
                   gix         = getGroup();
  int              myRank      = getRank();

  GRP_getSize(gix, &grpSize);
  if(myRank == 0 || myRank == 1) {
    int              itr = 0;
    GC_Rqst          rqst;

    /*fprintf(stdout, "\n**************************************************************************************\n");
    fprintf(stdout, "Adder_no_chn Operator(%x), (Gix %x, Rank %x)\n", (unsigned int)(THR_self()), gix, myRank);*/
    //size = *dim * sizeof(int);

    /* Get data memory */
    if(NULL == (value = (int *)malloc(size))) {
      fprintf(stdout, "::::::::::::::::: Bug Operator: Exception %d ::::::::::::::::\n", -5);
      return(-5);
    }
    for(i = 0; i < size/sizeof(int); i++) {
      value[i] = i;
      acum += i;
    }
    cnt[0] = cnt[1] = 0;
//fprintf(stdout, "\t\t\t\tData_Source: GOOD RESULT = %d\n", acum);
  //fprintf(stdout, "Data_Source: antes de CHN_open, ");

    while (1) {
      //fprintf(stdout, "=============== Data_Source(%x)  Rank %d:  Envio ...%d\n", (unsigned int)(THR_self()), myRank, (cnt[myRank]) );
      /*if(0 > (excpn = GC_send((char *)(&value[0]), size, 2, 0, 0))) {
        fprintf(stdout, "::::::::::::::::: Adder_no_chn Operator (Sender): Exception %d ::::::::::::::::\n", excpn);
        free(value);
        return(excpn);
      }*/
      if(0 > (excpn = GC_asend((char *)(&value[0]), size, 2, 0, &rqst, 1))) {
        fprintf(stdout, "::::::::::::::::: Adder_no_chn Operator (Sender): Exception %d ::::::::::::::::\n", excpn);
        free(value);
        return(excpn);
      }
      //fprintf(stdout, "=============== Data_Source(%x):  Rank %d:  Enviado :)  %d\n", (unsigned int)(THR_self()), myRank, (cnt[myRank])++);
      //fprintf(stdout, "=============== Data_Source(%x):  Espero...\n", (unsigned int)(THR_self()) );
        if(0 > (excpn = GC_wait_send(&rqst)))  {
        fprintf(stdout, "::::::::::::::::: Adder_no_chn Operator (Sender): Exception %d ::::::::::::::::\n", excpn);
        free(value);
        return(excpn);
      }

    //if (!(++cnt % 1000)) fprintf(stdout, "(%d) Envio numero %d", getRank(), cnt);
      if (++itr == 32)
        break;
      //clock_nanosleep (CLOCK_REALTIME, TIMER_ABSTIME, &rqtp, NULL);
    }

    if(value)
      free(value);
  }
  else {
    GC_Rqst        rqst;
    GC_Status      status;
    int            itr       = 0;
    int            sum       = 0;
    int           *s_0       = NULL,
                  *s_1       = NULL;

    /*fprintf(stdout, "**************************************************************************************\n");
    fprintf(stdout, "Adder_no_chn Operator(%x). (Gix %x, Rank %x)\n", (unsigned int)(THR_self()), gix, myRank);*/

    if(NULL == (s_0 = (int *)malloc(size)))   {
      fprintf(stdout, "::::::::::::::::: Adder_no_chn Operator: Exception %d ::::::::::::::::\n", -5);
      return(-5);
    }
    if(NULL == (s_1 = (int *)malloc(size)))   {
      fprintf(stdout, "::::::::::::::::: Adder_no_chn Operator: Exception %d ::::::::::::::::\n", -5);
      free(s_0);
      return(-5);
    }
    while(1) {
      //---------- 1. Get Data ---------
      //fprintf(stdout, "\n\n\t\t=============== Adder  %x:  Recibo el 1... (sinc)\n", (unsigned int)(THR_self()) );
      if(0 > (excpn = GC_recv(s_0, size, 0, 0, &status, 1)))    {
        fprintf(stdout, "\n::::::::::::::::: Adder_no_chn Operator(%x): Exception %d ::::::::::::::::\n", THR_self(), excpn);
        if(s_1)
          free(s_1);
        if(s_0)
          free(s_0);
        return(excpn);
      }
     //fprintf(stdout, "\n\t\t=============== Adder  %x:  Recibido el 1\n", (unsigned int)(THR_self()) );
    //fprintf(stdout, "\t\t=============== Adder  %x:  Recibo el 2 (asinc)...\n", (unsigned int)(THR_self()) );
      if(0 > (excpn = GC_arecv (s_1, size, 1, 0, &rqst, 1)))  {
        fprintf(stdout, "\n::::::::::::::::: Adder_no_chn Operator(%x): Exception %d ::::::::::::::::\n", THR_self(), excpn);
        if(s_1)
          free(s_1);
        if(s_0)
          free(s_0);
        return(excpn);
      }
      //fprintf(stdout, "\t\t=============== Adder:  Espero en wait ... \n");
      if(0 > (excpn = GC_wait_recv(&rqst, &status))) {
        fprintf(stdout, "\n::::::::::::::::: Adder_no_chn Operator(%x): Exception %d ::::::::::::::::\n", THR_self(), excpn);
        if(s_1)
          free(s_1);
        if(s_0)
          free(s_0);
        return(excpn);
      }
      //fprintf(stdout, "\t\t=============== Adder:  Recibido el 2 %d\n", cnt++ );
      //sleep(1);

      //---------- 2. Run the ALGORITHM ---------
      sum = 0;
      for(i = 0; i < *dim; i++) {
        //fprintf(stdout, "%d/%d ", *(s_0 + i), *(s_1 + i));
        sum += *(s_0 + i) + *(s_1 + i);
      }
    //fprintf(stdout, "::::::::::::::::: Adder Operator (tsk ).  SUM = %d ::::::::::::::::", sum); fflush(stdout);
    //---------- 3. Print results ---------
      if(sum != 4032) {
        fprintf(stdout, "\n::::::::::::::::: Adder_no_chn Operator (%x). Bad SUM = %d ::::::::::::::::\n", THR_self(), sum);
        excpn = THR_E_EXHAUST;
        fprintf(stdout, "\n::::::::::::::::: Adder_no_chn Operator(%x): Exception %d ::::::::::::::::\n", THR_self(), excpn);
        if(s_1)
          free(s_1);
        if(s_0)
          free(s_0);
        return(excpn);
      }
      /*for(i = 0; i < 10000000; i++) { // Loose time
        sum += 1;
      }*/
      itr++;
    //if (!(itr %  100)) fprintf(stdout, "Rank (%d):Recibo numero %d ", getRank(), itr);fflush(stdout);
      if (itr == 32) {
      //fprintf(stdout, "\n");
        break;
      }
    }
    if(s_1)
      free(s_1);
    if(s_0)
      free(s_0);
  }
  return(0);
}
