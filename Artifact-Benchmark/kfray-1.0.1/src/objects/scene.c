 /*
 ** KF-Ray, raytracer parallèle
 **  loader.c
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

t_ray			CreateRay(t_vector start0, t_vector dir0)
{
	t_ray ray;

	ray.start = start0;
	ray.dir = dir0;

	return	ray;
}

t_ray			*CopyRay(t_ray *ray0)
{
	t_ray *ray;
	ray = (t_ray *) malloc(sizeof (t_ray));

	ray->start = ray0->start;
	ray->dir = ray0->dir;

	return	ray;
}

void			PrintRay(t_ray ray)
{
	fprintf(stdout, "[Ray] ");
	PrintVect(ray.start);
	PrintVect(ray.dir);
	fprintf(stdout, "\n");
}

t_camera		CreateCamera(t_vector point0, float dist_view0)
{
	t_camera camera;

	camera.point = point0;
	camera.dist_view = dist_view0;

	return	camera;
}


void			PrintCamera(t_camera camera)
{
	fprintf(stdout, "[Camera] ");
	PrintVect(camera.point);
	fprintf(stdout, " dist = %f", camera.dist_view);
	fprintf(stdout, "\n");
}


t_scene			CreateScene	(int *viewport0, int n_plane0, int n_sph0, int n_light0,
					t_plane *list_plane0, t_sphere *list_sph0, t_light *list_light0,
					t_options options0, t_camera camera0)
{
	int		i;
	t_scene		scn;
	t_plane		*list_plane;
	t_sphere	*list_sph;
	t_light		*list_light;

	list_plane = (t_plane *) malloc(n_plane0 * sizeof (t_plane));
	list_sph = (t_sphere *) malloc(n_sph0 * sizeof (t_sphere));
	list_light = (t_light *) malloc(n_light0 * sizeof (t_sphere));

	scn.viewport = CopyiArray(viewport0, 2);
	scn.n_plane = n_plane0;
	scn.n_sph = n_sph0;
	scn.n_light = n_light0;

	for (i=0; i < n_plane0; i++)
		list_plane[i] = *CopyPlane(&list_plane0[i]);
	for (i=0; i < n_sph0; i++)
		list_sph[i] = *CopySphere(&list_sph0[i]);
	for (i=0; i < n_light0; i++)
		list_light[i] = *CopyLight(&list_light0[i]);

	scn.plane = list_plane;
	scn.sph = list_sph;
	scn.light = list_light;

	scn.options = options0;

	scn.camera = camera0;

	return		scn;
}


void			FreeScene(t_scene scn)
{
	/*

	int i;

	for (i=0; i < scn.n_plane; i++)
		free(&scn.plane[i]);
	for (i=0; i < scn.n_sph; i++)
		free(&scn.sph[i]);
	for (i=0; i < scn.n_light; i++)
		free(&scn.light[i]);
	*/

	free(scn.plane);
	free(scn.sph);
	free(scn.light);
}
