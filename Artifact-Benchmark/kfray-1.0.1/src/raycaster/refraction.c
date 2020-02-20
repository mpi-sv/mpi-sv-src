 /*
 ** KF-Ray, raytracer parallèle
 **  refraction.c
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
#include <math.h>

#include "../objects/scene.h"
#include "../models/brdf.h"
#include "../models/effects.h"
#include "../misc/misc.h"
#include "refraction.h"
#include "reflection.h"



float		*RefractionSph(t_scene scn, t_castray struct_cast, int object_current, float *t, float *RGB)
{
	// Point d'intersection et vecteur normal
	t_vector	point_intersect = Addition(struct_cast.ray_cast->start, Prod(*t, struct_cast.ray_cast->dir));
	t_vector	vect_normal = Substraction(point_intersect, scn.sph[object_current].center);
	vect_normal = Normalize(vect_normal);

//	if (scn.sph[object_current].material->bump > 0.0f)
//		vect_normal = BumpMapping(vect_normal, point_intersect, scn.sph[object_current].material->bump);

	t_brdf struct_brdf = CreateBrdf(struct_cast.ray_cast, point_intersect, vect_normal,
				scn.sph[object_current].material, *struct_cast.coeff_refraction);
	RGB = Brdf(scn, struct_brdf, t, RGB);

	float index_refractive = scn.sph[object_current].material->refraction;  // Rapport n_i/n_t
	float costheta_i, costheta_t, sintheta_t_square;

	// On est à l'intérieur de la sphère ?
	if (DotProd(vect_normal, struct_cast.ray_cast->dir) > 0.0f)	// in
		vect_normal = Prod(-1.0f, vect_normal);
	else								// out
		index_refractive = 1.0f/index_refractive;

	costheta_i = abs(DotProd(struct_cast.ray_cast->dir, vect_normal));
	sintheta_t_square = index_refractive * index_refractive * (1 - costheta_i * costheta_i);

	//if ( sintheta_t_square > 1.0f )	// réfraction interne totale
	//	return ReflectionSph(scn, struct_cast, object_current, t, RGB);

	costheta_t =  sqrtf(1.0f - sintheta_t_square);
	t_vector vect_temp = Prod(index_refractive * costheta_i + costheta_t, vect_normal);

	// On envoie le rayon transmis
	struct_cast.ray_cast->start = point_intersect;
	struct_cast.ray_cast->dir = Substraction(Prod(index_refractive, struct_cast.ray_cast->dir), vect_temp);
	struct_cast.ray_cast->dir = Normalize(struct_cast.ray_cast->dir);

	*struct_cast.coeff_refraction *= scn.sph[object_current].material->refraction;

	if (*struct_cast.coeff_refraction < EPSILON)
		return RGB;

	return CastRay(scn, struct_cast, RGB);
}


#if 0

float		*RefractionSph(t_scene scn, t_castray struct_cast, int object_current, float *t, float *RGB)
{
	// Point d'intersection et vecteur normal
	t_vector	point_intersect = Addition(struct_cast.ray_cast->start, Prod(*t, struct_cast.ray_cast->dir));
	t_vector	vect_normal = Substraction(point_intersect, scn.sph[object_current].center);
	vect_normal = Normalize(vect_normal);

	if (scn.sph[object_current].material->bump > 0.0f)
		vect_normal = BumpMapping(vect_normal, point_intersect, scn.sph[object_current].material->bump);

	t_brdf struct_brdf = CreateBrdf(struct_cast.ray_cast, point_intersect, vect_normal, scn.sph[object_current].material, *struct_cast.coeff_reflection);
	RGB = Brdf(scn, struct_brdf, t, RGB);


	float index_refractive = scn.sph[object_current].material->refraction;  // Rapport n_i/n_t
	float costheta_i, costheta_t, sintheta_t_square;

	// On est à l'intérieur de la sphère
	if (DotProd(vect_normal, struct_cast.ray_cast->dir) > 0.0f)
		vect_normal = Prod(-1.0f, vect_normal);
	else
		index_refractive /= 1.0f;	// On inverse

	costheta_i = abs(DotProd(struct_cast.ray_cast->dir, vect_normal));
	sintheta_t_square = index_refractive * index_refractive * (1 - costheta_i * costheta_i);

	// Reflection totale interne
	//if (1 - sintheta_t_square <= 0.0f)
		//return ReflectionSph(scn, struct_cast, object_current, t, RGB);

	costheta_t =  sqrtf(1 - sintheta_t_square);
	t_vector vect_temp = Prod(index_refractive * costheta_i + costheta_t, vect_normal);

	// On envoie le rayon transmis
	struct_cast.ray_cast->start = point_intersect;
	struct_cast.ray_cast->dir = Substraction(Prod(index_refractive, struct_cast.ray_cast->dir), vect_temp);
	struct_cast.ray_cast->dir = Normalize(struct_cast.ray_cast->dir);

	*struct_cast.coeff_refraction *= scn.sph[object_current].material->refraction;

	if (*struct_cast.coeff_refraction < EPSILON)
		return RGB;

	return CastRay(scn, struct_cast, RGB);
}

#endif
