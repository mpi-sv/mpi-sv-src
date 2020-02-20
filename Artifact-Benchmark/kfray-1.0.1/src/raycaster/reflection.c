 /*
 ** KF-Ray, raytracer parallèle
 **  reflection.c
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
#include <math.h>

#include "../objects/scene.h"
#include "../models/brdf.h"
#include "../models/effects.h"
#include "../misc/misc.h"
#include "reflection.h"


float		*ReflectionSph	(t_scene scn, t_castray struct_cast,
				int object_current, float *t, float *RGB)
{

	// Point d'intersection et vecteur normal
	t_vector	point_intersect = Addition	(struct_cast.ray_cast->start,
							Prod(*t, struct_cast.ray_cast->dir));
	t_vector	vect_normal = Substraction(point_intersect, scn.sph[object_current].center);

	vect_normal = Normalize(vect_normal);

	if (scn.sph[object_current].material->bump > 0.0f)
		vect_normal = BumpMapping	(vect_normal, point_intersect,
						scn.sph[object_current].material->bump);

	t_brdf struct_brdf = CreateBrdf	(struct_cast.ray_cast, point_intersect,
					vect_normal, scn.sph[object_current].material,
					*struct_cast.coeff_reflection);


	RGB = Brdf(scn, struct_brdf, t, RGB);

	// On relance le rayon de réflexion
	float costheta_i = DotProd(struct_cast.ray_cast->dir, vect_normal);
	struct_cast.ray_cast->start = point_intersect;
	struct_cast.ray_cast->dir = Substraction(struct_cast.ray_cast->dir,
						Prod(2.0f*costheta_i, vect_normal));


	*struct_cast.coeff_reflection *= scn.sph[object_current].material->reflection;

	// Si on pouvait se passer de cette ligne a voir...
	// Mise pour limiter les calculs de la réflectance lambertienne inutiles
	if (*struct_cast.coeff_reflection < EPSILON)
		return RGB;

	return	CastRay(scn, struct_cast, RGB);
}



float		*ReflectionPlane(t_scene scn, t_castray struct_cast,
				int object_current, float *t, float *RGB)
{
	// Point d'intersection et vecteur normal
	t_vector	point_intersect = Addition	(struct_cast.ray_cast->start, Prod(*t,
							struct_cast.ray_cast->dir));
	t_vector	vect_normal = Normalize(scn.plane[object_current].normal);

	// Si la normale n'est pas dans le bon sens
	if (DotProd(vect_normal, struct_cast.ray_cast->dir) > 0.0f)
		vect_normal = Prod(-1.0f, vect_normal);

	if (scn.plane[object_current].material->bump > 0.0f)
		vect_normal = BumpMapping	(vect_normal, point_intersect,
						scn.plane[object_current].material->bump);

	t_brdf struct_brdf = CreateBrdf	(struct_cast.ray_cast, point_intersect,
					vect_normal, scn.plane[object_current].material,
					*struct_cast.coeff_reflection);
	RGB = Brdf(scn, struct_brdf, t, RGB);

	// On relance le rayon de réflexion
	float costheta_i = DotProd(struct_cast.ray_cast->dir, vect_normal);
	struct_cast.ray_cast->start = point_intersect;
	struct_cast.ray_cast->dir = Substraction(struct_cast.ray_cast->dir,
						Prod(2.0f*costheta_i, vect_normal));

	*struct_cast.coeff_reflection *= scn.plane[object_current].material->reflection;

	if (*struct_cast.coeff_reflection < EPSILON)
		return RGB;

	return CastRay(scn, struct_cast, RGB);

}
