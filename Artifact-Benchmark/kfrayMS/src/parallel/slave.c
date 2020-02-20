 /*
 ** KF-Ray, raytracer parallèle
 **  slave.c
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
#include <unistd.h>
#include "../main.h"
#include "../raycaster/raycaster.h"
#include "../models/postprocessing.h"
#include "slave.h"

#ifdef MY_MPI

#include <mpi.h>

void			Slave(t_scene scn, int h, int w, int n_lines, int rank)
{
	MPI_Status	status;
	int		x, y, i, j;
	int		step_slave;

	// On alloue un tableau slave différent qui fait la taille de n_lines
	unsigned char	*grid_slave;
	grid_slave = (unsigned char *)malloc(w * n_lines * 3 * sizeof (unsigned char));

	while (1)
	{
		MPI_Recv ( &step_slave, 1, MPI_INT, MASTER_NODE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

		// Si on reçoit un TAG_END on quitte la boucle
		if (status.MPI_TAG == TAG_END)
			break;

		// Calcul du tableau grid_slave
		for (	j = 0, y = step_slave * n_lines;
			j < n_lines;
			j ++, y ++ )
			for (	i = 0, x = 0;
				i < 3*w;
				i = i + 3, x ++ )
                                //YHB: the line below is the source of the stucking call chain, which contains no communication, so I use a nop instruction instead
				RaytracerSlave(scn, x, y, i, j, grid_slave); 
                                //j=j;

		MPI_Ssend( &step_slave, 1, MPI_INT, MASTER_NODE, TAG_REQ, MPI_COMM_WORLD);
		MPI_Ssend( grid_slave, w*n_lines*3, MPI_UNSIGNED_CHAR, MASTER_NODE, TAG_DATA, MPI_COMM_WORLD);
	}
}


void			RaytracerSlave(t_scene scn, int x, int y, int i, int j, unsigned char *img_slave)
{
	int 		level = 0;
	float 		coeff_reflection = 1.0f;
	float		coeff_refraction = 1.0f;
	int		k;

	float	*RGB;
	RGB = (float*) malloc(3 * sizeof (float));

	for (k = 0 ; k < 3 ; k++)
		RGB[k] = 0.0f;

	if (scn.options.opt_aliasing == 0)
	{
		t_ray	ray_cast = CameraRay(scn, x, y);
		t_castray struct_cast = CreateCastray	(&ray_cast, &level,
							&coeff_reflection, &coeff_refraction);
		RGB = CastRay(scn, struct_cast, RGB);
	}
	else
		RGB = AntiAliasing(scn, RGB, x, y);

	// Remise à l'échelle
	for (k = 0 ; k < 3; k++)
		RGB[k] = RGB[k] * 255.0 / MAX_COLOR;

	// Attention au j !
	SetPixel(img_slave, scn.viewport[0], x, j, (int) RGB[0], (int) RGB[1] , (int) RGB[2]);

	free(RGB);
}
#endif
