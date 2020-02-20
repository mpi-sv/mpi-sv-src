 /*
 ** KF-Ray, raytracer parallèle
 **  procedural.c
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
#include "perlin.h"
#include "../misc/misc.h"
#include "procedural.h"

float		*Turbulence(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert)
{
	int	k;
	float	level_perlin;
	float	coeff_noise = 0.0f;

	for (level_perlin = 1.0f; level_perlin < MAX_LEVEL_PERLIN; level_perlin += 1.0f)
        coeff_noise += 	(1.0f / level_perlin) * fabsf(Pnoise(
						(level_perlin * 0.05 * struct_brdf.point_intersect.x),
						(level_perlin * 0.05 * struct_brdf.point_intersect.y),
						(level_perlin * 0.05 * struct_brdf.point_intersect.z)));


	for (k = 0; k < 3; k ++)
		RGB[k] += 	coeff_lambert * light_current.intensity[k] * \
				struct_brdf.material->color[k] * coeff_noise * struct_brdf.material->perlin;

	return	RGB;
}


float		*Marbel(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert)
{
	int 	k;
	float	level_perlin;
	float	coeff_noise = 0.0f;

	for (level_perlin = 1.0f; level_perlin < MAX_LEVEL_PERLIN; level_perlin += 1.0f)
		coeff_noise += 	(1.0f / level_perlin) * fabsf(Pnoise(
					(level_perlin * 0.05 * struct_brdf.point_intersect.x),
					(level_perlin * 0.05 * struct_brdf.point_intersect.y),
					(level_perlin * 0.05 * struct_brdf.point_intersect.z)));

	coeff_noise = 0.5f * sinf( \
			(struct_brdf.point_intersect.x + struct_brdf.point_intersect.y + \
			struct_brdf.point_intersect.z) * 0.05f + coeff_noise) + 0.5f;

	for (k = 0; k < 3; k ++)
		RGB[k] +=	coeff_lambert * light_current.intensity[k] * \
				struct_brdf.material->color[k] * coeff_noise * struct_brdf.material->perlin;

	return	RGB;
}


float		*Wood(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert)
{
	int 	k;
	float	level_perlin;
	float	coeff_noise = 0.0f;
	float	temp;

	for (level_perlin = 1.0f; level_perlin < 3; level_perlin += 1.0f)
        coeff_noise +=	(1.0f / level_perlin) * fabsf(Pnoise(
				(level_perlin * 0.01 * struct_brdf.point_intersect.x),
				(level_perlin * 0.01 * struct_brdf.point_intersect.y),
				(level_perlin * 0.01 * struct_brdf.point_intersect.z)));


	temp = 20.0f*(coeff_noise - floor(coeff_noise));
	coeff_noise = (1-cosf(PI*temp))*0.05f;

	for (k = 0; k < 3; k ++)
		RGB[k] += 	coeff_lambert * light_current.intensity[k] * struct_brdf.material->color[k] \
				* coeff_noise * struct_brdf.material->perlin;

	return	RGB;
}


float		*Checker(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert)
{
	int	k;
	float	length;
	float	*RGB_tmp;

	float coord[] = { struct_brdf.point_intersect.x,
			struct_brdf.point_intersect.y,
			struct_brdf.point_intersect.z };

	length = 100.0f;	// Largeur d'un côté voir si on l'inclut dans le coeff perlin

	RGB_tmp = CheckerRGB(struct_brdf, coord, length);

	for (k = 0; k < 3; k ++)
		RGB[k] += coeff_lambert * light_current.intensity[k] * RGB_tmp[k];

	return	RGB;
}


float		*CheckerRGB (t_brdf struct_brdf, float *coord, float length)
{
	int	k;
	bool	coord_bool[3];
	float	*RGB_tmp;
	RGB_tmp = (float *) malloc(3 * sizeof(float));

	for (k=0; k<3; k++)
	{
		coord_bool[k] = (( abs((int) coord[k]/length) % 2) == 0);
		if(coord[k] < EPSILON)
			coord_bool[k] = !coord_bool[k];
	}

	if (coord_bool[2])
	{
		if ((coord_bool[0] && coord_bool[1]) || (!coord_bool[0] && !coord_bool[1]))
			for (k = 0; k < 3; k ++)
				RGB_tmp[k] = struct_brdf.material->color[k];
		else
			for (k = 0; k < 3; k++)
				RGB_tmp[k] = MAX_COLOR - 2000.0f;
	}
	else
	{
		if ((coord_bool[0] && coord_bool[1]) || (!coord_bool[0] && !coord_bool[1]))
			for (k = 0; k < 3; k++)
				RGB_tmp[k] = MAX_COLOR - 2000.0f;
		else
			for (k = 0; k < 3; k ++)
				RGB_tmp[k] = struct_brdf.material->color[k];
	}

	return	RGB_tmp;
}
