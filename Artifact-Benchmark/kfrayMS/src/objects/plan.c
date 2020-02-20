 /*
 ** KF-Ray, raytracer parallèle
 **  plan.c
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


t_plane		CreatePlane(float dist0, t_vector normal0, t_material *mat0)
{
	t_plane plane;

	plane.dist = dist0;
	plane.normal = normal0;
	plane.material = CopyMaterial(mat0);

	return	plane;
}

t_plane		*CopyPlane(t_plane *plane0)
{
	t_plane	*plane;
	plane = (t_plane *) malloc(sizeof (t_plane));

	plane->dist = plane0->dist;
	plane->normal = plane0->normal;
	plane->material = CopyMaterial(plane0->material);

	return	plane;
}

void		PrintPlane(t_plane plane)
{
	fprintf(stdout, "[Plane] Dist = %f - Normal = ", plane.dist);
	PrintVect(plane.normal);
	PrintMaterial(plane.material);
	fprintf(stdout, "\n***\n");
}
