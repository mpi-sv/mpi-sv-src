 /*
 ** KF-Ray, raytracer parallèle
 **  sphere.c
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

t_sphere	CreateSphere(t_vector center0, float radius0, t_material *mat0)
{
	t_sphere sph;

	sph.center = center0;
	sph.radius = radius0;
	sph.material = CopyMaterial(mat0);

	return	sph;
}

t_sphere	*CopySphere(t_sphere *sph0)
{
	t_sphere *sph;
	sph = (t_sphere*) malloc(sizeof (t_sphere));

	sph->center = sph0->center;
	sph->radius = sph0->radius;
	sph->material = CopyMaterial(sph0->material);

	return	sph;
}

void		PrintSphere(t_sphere sph)
{
	fprintf(stdout, "[Sphere] Radius = %f | Center = ", sph.radius);
	PrintVect(sph.center);
	fprintf(stdout, "\n");
	PrintMaterial(sph.material);
	fprintf(stdout,"***\n");
}
