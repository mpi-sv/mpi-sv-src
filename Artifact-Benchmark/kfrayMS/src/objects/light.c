 /*
 ** KF-Ray, raytracer parallèle
 **  light.c
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
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
#include "scene.h"


t_light		CreateLight(t_vector pos0, float *intensity0)
{
	t_light	light;

	light.position = pos0;
	light.intensity = CopyfArray(intensity0, 3);

	return	light;
}

t_light		*CopyLight(t_light *light0)
{
	t_light	*light;
	light = (t_light*) malloc(sizeof (t_light));

	light->position = light0->position;
	light->intensity = CopyfArray(light0->intensity, 3);

	return	light;
}

void		PrintLight(t_light *light)
{
	fprintf(stdout, "[Light] intensity = ");
	PrintfArray(light->intensity, 3);
	fprintf(stdout, " position =");
	PrintVect(light->position);
	fprintf(stdout, "\n");
}
