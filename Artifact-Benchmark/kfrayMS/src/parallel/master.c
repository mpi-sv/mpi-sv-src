 /*
 ** KF-Ray, raytracer parallèle
 **  master.c
 **
 ** Copyright 2009 Karin AIT-SI-AMER & Florian DANG
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 */

 /*************************************************************************
 *   This file is part of KF-Ray.
 *
 *   KF-Ray is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   KF-Ray is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KF-Ray.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "../main.h"
#include "../misc/image.h"
#include "../misc/printinfo.h"
#include "master.h"

#ifdef MY_MPI

#include <mpi.h>


void			Master		(int n_proc, int h, int w, int n_lines,
					char *img_name, int opt_display)
{
	MPI_Status	status;
        
        printf("h is equal to %d",h);
        printf("n_lines is equal to %d",n_lines);

	int		source, step, step_slave;
	int		n_step = h/n_lines;
	int		n_stepmax = MIN(n_step+1, n_proc);	// Pas totaux nécessaires
        //int n_stepmax=6;
	step = 0;

	// Allocation tableau image final pour processus maître
	unsigned char	*grid;
	grid = (unsigned char *) malloc(w*h*3*sizeof (unsigned char));

	// Affichage informations
	printf("%s", splash);
	InfoMaster(img_name, w, h, n_proc, n_lines);

	// On envoie une première salve de travail
	FirstWork(n_proc, n_stepmax, &step);

	// On reçoit et envoie un autre travail à un slave
	while ( step < n_step )
	{
		MPI_Recv(&step_slave, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REQ, MPI_COMM_WORLD, &status);
		MPI_Recv(grid + w * n_lines * step_slave * 3, w * n_lines * 3,
			MPI_UNSIGNED_CHAR, status.MPI_SOURCE, TAG_DATA, MPI_COMM_WORLD, &status);
		MPI_Send( &step, 1, MPI_INT, status.MPI_SOURCE, TAG_REQ, MPI_COMM_WORLD);
		//printf("MASTER_NODE envoie un travail à slave n°%d : step=%d\n", status.MPI_SOURCE, step);

		step ++;
	}

	// Le travail est fini on envoie un TAG END
	for (source = 1; source < n_stepmax; source ++)
	{
		// On reçoit le travail d'un slave step et tableau sur des TAG différents !
		MPI_Recv(&step_slave, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REQ, MPI_COMM_WORLD, &status);
		MPI_Recv(grid + w * n_lines * step_slave * 3, w * n_lines * 3,
			MPI_UNSIGNED_CHAR, status.MPI_SOURCE, TAG_DATA, MPI_COMM_WORLD, &status);

		// On dit de terminer le travail au slave
		MPI_Send( 0, 0, MPI_INT, source, TAG_END, MPI_COMM_WORLD);
	}
        printf("nprc is %d;step is %d;n_step is %d;",n_proc,step,n_step);
       /* int i1=1;
        while(step<n_step){
         MPI_Send( &step, 1, MPI_INT, i1, TAG_REQ, MPI_COMM_WORLD);
         i1=(i1+1)%n_proc;
         if(i1==0)
            i1=1;
         step++;
         if(step==n_step)
           {
             int j=1;
          for (j=1; j < n_proc; j ++)
	     {
              MPI_Send( 0, 0, MPI_INT, j, TAG_END, MPI_COMM_WORLD);
             }
             
           }
        }
        int i2=0;       
        for(i2=0;i2<n_step;i2++)
        {
                MPI_Recv(&step_slave, 1, MPI_INT, MPI_ANY_SOURCE, TAG_REQ, MPI_COMM_WORLD, &status);
		//MPI_Recv(grid + w * n_lines * step_slave * 3, w * n_lines * 3, MPI_UNSIGNED_CHAR, status.MPI_SOURCE, TAG_DATA, MPI_COMM_WORLD, &status);
                MPI_Recv(grid + w * n_lines * step_slave * 3, w * n_lines * 3, MPI_UNSIGNED_CHAR, MPI_ANY_SOURCE, TAG_DATA, MPI_COMM_WORLD, &status);
        } */


	SaveImage(grid, img_name, w, h);

	if(opt_display)
		ShowImageSys(img_name);
}





void			FirstWork	(int n_proc, int n_stepmax, int *step)
{
	MPI_Status	status;
	int 		source;

	for (source = 1; source < n_proc; source ++)
	{
		if (source < n_stepmax) 		// Si jamais n_proc > n_step
		{
			//printf("MASTER_NODE envoie le 1er travail à slave n°%d : step=%d\n", source, *step);
			MPI_Send( step, 1, MPI_INT, source, TAG_REQ, MPI_COMM_WORLD);
			(*step) ++;
		}
		else	// On dit de ne rien faire aux processus qui ne vont pas servir...
		{
			printf("! Warning : Processus %d non utilisé (inutile) !\n", source);
			MPI_Send( 0, 0, MPI_INT, source, TAG_END, MPI_COMM_WORLD);
		}
	}
}


#endif
