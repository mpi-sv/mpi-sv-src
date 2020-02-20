 /*
 ** KF-Ray, raytracer parallèle
 **  loader.c
 **
 ** Copyright 2009 Karin AIT-SI-AMER & Florian DANG
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous à COPYING pour les infos sur le Copyright
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "objects.h"
#include "vector.h"
#include "../misc/misc.h"


t_ray CreateRay(t_vector start0, t_vector dir0)
{
	t_ray ray;

	ray.start = start0;
	ray.dir = dir0;

	return ray;
}

t_ray *CopyRay(t_ray *ray0)
{
	t_ray *ray;
	ray = (t_ray *) malloc(sizeof (t_ray));

	ray->start = ray0->start;
	ray->dir = ray0->dir;

	return ray;
}

void	PrintRay(t_ray ray)
{
	printf("[Ray] ");
	PrintVect(ray.start);
	PrintVect(ray.dir);
	printf("\n");
}

t_plane CreatePlane(float dist0, t_vector normal0, t_material *mat0)
{
	t_plane plane;

	plane.dist = dist0;
	plane.normal = normal0;
	plane.material = CopyMaterial(mat0);

	return plane;
}

t_plane *CopyPlane(t_plane *plane0)
{
	t_plane	*plane;
	plane = (t_plane *) malloc(sizeof (t_plane));

	plane->dist = plane0->dist;
	plane->normal = plane0->normal;
	plane->material = CopyMaterial(plane0->material);

	return plane;
}

t_sphere CreateSphere(t_vector center0, float radius0, t_material *mat0)
{
	t_sphere sph;

	sph.center = center0;
	sph.radius = radius0;
	sph.material = CopyMaterial(mat0);

	return sph;
}

t_sphere *CopySphere(t_sphere *sph0)
{
	t_sphere *sph;
	sph = (t_sphere*) malloc(sizeof (t_sphere));

	sph->center = sph0->center;
	sph->radius = sph0->radius;
	sph->material = CopyMaterial(sph0->material);

	return sph;
}

void    PrintSphere(t_sphere sph)
{
    printf("[Sphere] Radius = %f | Center = ", sph.radius);
    PrintVect(sph.center);
    printf("\n");
}

t_light CreateLight(t_vector pos0, float *intensity0)
{
	t_light light;

	light.position = pos0;
	light.intensity = CopyfArray(intensity0, 3);

	return light;
}

t_light *CopyLight(t_light *light0)
{
	t_light *light;
	light = (t_light*) malloc(sizeof (t_light));

	light->position = light0->position;
	light->intensity = CopyfArray(light0->intensity, 3);

	return light;
}

void PrintLight(t_light *light)
{
	printf("[Light] intensity = ");
	PrintfArray(light->intensity, 3);
	printf(" position =");
	PrintVect(light->position);
}

t_camera CreateCamera(t_vector point0, t_vector center_view0, float dist_view0)
{
	t_camera camera;

	camera.point = point0;
	camera.center_view = center_view0;
	camera.dist_view = dist_view0;

	return camera;
}


t_material CreateMaterial	(int type0, const float *color0, float reflection0,
							float specular0, float power0, float perlin0, float bump0)
{
	t_material mat;

	mat.type = type0;
	mat.color = color0;
	mat.specular = specular0;
	mat.reflection = reflection0;
	mat.power = power0;
	mat.perlin = perlin0;
	mat.bump = bump0;

	return mat;
}

t_material *CopyMaterial(t_material *mat0)
{
	t_material *mat;
	mat = (t_material *) malloc(sizeof (t_material));

	mat->type = mat0->type;
	mat->color = CopyfArray(mat0->color, 3);
	mat->specular = mat0->specular;
	mat->reflection = mat0->reflection;
	mat->power = mat0->power;
	mat->perlin = mat0->perlin;
	mat->bump = mat0->bump;

	return mat;
}

void PrintMaterial(t_material *mat)
{
	printf("[Material] color = ");
	PrintfArray(mat->color, 3);
	printf(" spec = %f | refl = %f | rough = %f\n", mat->reflection, mat->specular, mat->power);
}


t_scene CreateScene	(int *viewport0, int n_plane0, int n_sph0, int n_light0,
					t_plane *list_plane0, t_sphere *list_sph0, t_light *list_light0,
					t_options options0, t_camera camera0)
{
	t_scene scn;
	int i;

    scn.viewport = CopyiArray(viewport0, 2);
    scn.n_plane = n_plane0;
	scn.n_sph = n_sph0;
	scn.n_light = n_light0;

	t_plane *list_plane;
	list_plane = (t_plane*) malloc(n_plane0 * sizeof (t_plane));

	t_sphere *list_sph;
	list_sph = (t_sphere*) malloc(n_sph0 * sizeof (t_sphere));

	t_light *list_light;
	list_light = (t_light*) malloc(n_light0 * sizeof (t_sphere));

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

	return scn;
}
