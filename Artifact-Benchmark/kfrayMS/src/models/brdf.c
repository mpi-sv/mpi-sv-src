 /*
 ** KF-Ray, raytracer parallele
 **  brdf.c
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "brdf.h"
#include "../textures/procedural.h"
#include "../misc/misc.h"

#if OMBRAGE
#include "../raycaster/intersections.h"
#endif

#include "../textures/perlin.h"


t_brdf		CreateBrdf	(t_ray *ray_cast0, t_vector point_intersect0,
				t_vector vect_normal0, t_material *mat0, float coeff_reflection0)
{
	t_brdf	struct_brdf;

	struct_brdf.ray_cast = CopyRay(ray_cast0);
	struct_brdf.point_intersect = point_intersect0;
	struct_brdf.vect_normal = vect_normal0;
	struct_brdf.material = CopyMaterial(mat0);
	struct_brdf.coeff_reflection = coeff_reflection0;

	return	struct_brdf;
}


float		*Brdf(t_scene scn, t_brdf struct_brdf, float *t, float *RGB)
{
	int 	i,k;

	// Calcul de la valeur d'éclairement au point pour différentes sources de lumière
	for (i = 0; i < scn.n_light; i++)
	{
		t_light		light_current = scn.light[i];
		t_vector 	vect_light = Substraction	(light_current.position,
								struct_brdf.point_intersect);

		// Lumières du côté non visible supprimées (rayon d'ombre)
		if (DotProd(struct_brdf.vect_normal,vect_light) <= 0.0f)
			continue;

		t_ray ray_light;
		ray_light.start = struct_brdf.point_intersect;
		ray_light.dir = Normalize(vect_light);

		//Teste l'ombrage
		if ( Shadow(scn, ray_light, vect_light) == FALSE )
		{
			RGB = Lambert(scn, struct_brdf, ray_light, light_current, RGB);
			if (scn.options.model_brdf == 2)
				RGB = Phong(scn, struct_brdf, ray_light, light_current, RGB);
			if (scn.options.model_brdf == 3)
				RGB = BlinnPhong(scn, struct_brdf, ray_light, light_current, RGB);
		}
	}

	for (k = 0; k < 3; k++)
		RGB[k] = MIN(RGB[k], MAX_COLOR);

	#if DEBUG_BRDF
	if ( RGB[0] < 0.0 || RGB[1] < 0.0 || RGB[2] < 0.0 )
		printf("!Warning RGB = %f | %f | %f \n", RGB[0], RGB[1], RGB[2]);
	#endif

	return	RGB;
}


bool		Shadow(t_scene scn, t_ray ray_light, t_vector vect_light)
{
	#if OMBRAGE
	int i;
	float	dist_light = Module(vect_light);

	for ( i = 0; i < scn.n_sph; i++)
		if (InterSphere(ray_light, scn.sph[i], &dist_light))
			if (scn.sph[i].material->density <= 1.0f)	// Ne pas oublier le cas si l'objet est transparent !
				return TRUE;
	#endif

	return 	FALSE;
}


float		*Lambert(t_scene scn, t_brdf struct_brdf,
			t_ray ray_light, t_light light_current, float *RGB)
{
	int 	k;
	float 	coeff_lambert = struct_brdf.coeff_reflection*
				DotProd(ray_light.dir,struct_brdf.vect_normal);

  	#if DEBUG_BRDF
	if (coeff_lambert <= 0.0f)
	{
		printf("!Warning : Lambert = %f\n", coeff_lambert);
		PrintVect(struct_brdf.point_intersect); printf("\n");
		PrintMaterial(struct_brdf.material); printf("\n");
	}
	#endif

	for (k = 0; k < 3; k++)
		RGB[k] += coeff_lambert * light_current.intensity[k] * struct_brdf.material->color[k];

	if (scn.options.model_texture == 1)
		switch(struct_brdf.material->type)
		{
			case MATERIAL_TURBULENCE:
				RGB = Turbulence(struct_brdf, light_current, RGB, coeff_lambert);
				break;
			case MATERIAL_MARBEL:
				RGB = Marbel(struct_brdf, light_current, RGB, coeff_lambert);
				break;
			case MATERIAL_WOOD:
				RGB = Wood(struct_brdf, light_current, RGB, coeff_lambert);
				break;
			case MATERIAL_CHECKER:
				RGB = Checker(struct_brdf, light_current, RGB, coeff_lambert);
				break;
			default:
				break;
		}

    return 	RGB;

}


float		*Phong	(t_scene scn, t_brdf struct_brdf,
			t_ray ray_light, t_light light_current, float *RGB)
{
	int 	k;
	float 	coeff_reflet = 2.0f * DotProd(ray_light.dir, struct_brdf.vect_normal);
	t_vector vect_phong = Substraction	(ray_light.dir,
						Prod(coeff_reflet, struct_brdf.vect_normal));
	float 	coeff_phong = MAX(DotProd(vect_phong, struct_brdf.ray_cast->dir), 0.0f);

	coeff_phong =	struct_brdf.material->specular *
			powf(coeff_phong, struct_brdf.material->power) * struct_brdf.coeff_reflection;

  	#if DEBUG_BRDF
	if (coeff_phong < 0.0f)
		printf("\nError : Phong = %f", coeff_phong);
	#endif

	for (k = 0; k < 3; k++)
		RGB[k] += coeff_phong * light_current.intensity[k] * struct_brdf.material->color[k];

	return	RGB;
}


float		*BlinnPhong	(t_scene scn, t_brdf struct_brdf,
				t_ray ray_light, t_light light_current, float *RGB)
{
	int	k;
	t_vector vect_blinn = Substraction(ray_light.dir, struct_brdf.ray_cast->dir);

	if (Module(vect_blinn) != 0.0f)
	{
		vect_blinn = Normalize(vect_blinn);
		float coeff_blinn = MAX(DotProd(vect_blinn, struct_brdf.vect_normal), 0.0f );
		coeff_blinn = 	struct_brdf.material->specular *
				powf(coeff_blinn, struct_brdf.material->power) * struct_brdf.coeff_reflection;

		#if DEBUG_BRDF
		if (coeff_blinn < 0.0f)
			printf("\nError : Blinn = %f", coeff_blinn);
		#endif

		for (k = 0; k < 3; k++)
			RGB[k] += coeff_blinn * light_current.intensity[k] * struct_brdf.material->color[k];
	}

	return	RGB;
}
