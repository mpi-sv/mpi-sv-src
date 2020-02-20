 /*
 ** KF-Ray, raytracer parallèle
 **  raycaster.c
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
#include "../misc/misc.h"
#include "reflection.h"
#include "refraction.h"
#include "intersections.h"
#include "raycaster.h"

#define DEBUG 0

t_castray	CreateCastray	(t_ray *ray_cast0, int *level0,
				float *coeff_reflection0, float *coeff_refraction0)
{
	t_castray struct_cast;

	struct_cast.ray_cast = ray_cast0;
	struct_cast.level = level0;
	struct_cast.coeff_reflection = coeff_reflection0;
	struct_cast.coeff_refraction = coeff_refraction0;

	return struct_cast;
}



float		*CastRay(t_scene scn, t_castray struct_cast, float *RGB)
{
	float	t = 20000.0f;	// On met l'absisse assez loin au début
	int	object_current = -1;
	int 	object_type = LookIntersect(scn, struct_cast, &object_current, &t);

	if (*struct_cast.level < MAX_DEPTH)
	{
		LookObjectType(scn, struct_cast, RGB, object_type, object_current, t);

		(*struct_cast.level) ++;
	}

	return	RGB;
}



void		LookObjectType	(t_scene scn, t_castray struct_cast,
				float *RGB, int object_type,
				int object_current, float t)
{
	int 	k;

	switch (object_type)
	{
		case PLANE:
			if (scn.options.model_brdf == 0)	// Sans éclairage
				for ( k = 0; k < 3; k++)
					RGB[k] = scn.plane[object_current].material->color[k];
			else
				RGB = ReflectionPlane(scn, struct_cast, object_current, &t, RGB);

			break;

		case SPHERE:
			if (scn.options.model_brdf == 0)
				for ( k = 0; k < 3; k++)
					RGB[k] = scn.sph[object_current].material->color[k];
			else
			{
				if (scn.sph[object_current].material->reflection > EPSILON)
					RGB = ReflectionSph(scn, struct_cast, object_current, &t, RGB);
				if (scn.sph[object_current].material->refraction > EPSILON)
					RGB = RefractionSph(scn, struct_cast, object_current, &t, RGB);
			}
			break;

		default:
			break;
	}

}




int		LookIntersect	(t_scene scn, t_castray struct_cast,
				int *object_current, float *t)
{
	int	i;
	int	object_type = 0;

	for (i = 0; i < scn.n_plane; i++)
	{
		if ( InterPlane(*struct_cast.ray_cast, scn.plane[i], t) )
		{
			object_type = PLANE;
			*object_current = i;
		}
	}

	for (i = 0; i < scn.n_sph; i++)
	{
		if ( InterSphere(*struct_cast.ray_cast, scn.sph[i], t) )
		{
			object_type = SPHERE;
			*object_current = i;
		}
	}

	#if DEBUG
		printf("Type objet = %d | object_current = %d -", object_type, *object_current);
	#endif

	return	object_type;
}
