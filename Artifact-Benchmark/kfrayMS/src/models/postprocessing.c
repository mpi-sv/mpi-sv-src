 /*
 ** KF-Ray, raytracer parallèle
 **  postprocessing.c
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

#include "../misc/misc.h"
#include "../objects/scene.h"
#include "../raytracer.h"
#include "../raycaster/raycaster.h"
#include "postprocessing.h"


float *AntiAliasing(t_scene scn, float *RGB, int x, int y)
{
		int	k;
		float	x_delta, y_delta;
		int	n_samples = 4;	// doit être un carré supersampling
		float	delta = 1.0f / sqrtf(n_samples);

		float	*RGB_samples;
		RGB_samples = (float*) malloc(3 * sizeof (float));

		for (x_delta = (float)x ; x_delta < (float) x + 1.0f; x_delta += delta )
			for (y_delta = (float)y ; y_delta < (float) y + 1.0f; y_delta += delta )
			{
				for (k = 0 ; k < 3; k++)
					RGB_samples[k] = 0.0f;

				int 	level = 0;
				float 	coeff_reflection = 1.0f;
				float	coeff_refraction = 1.0f;

				t_ray	ray_cast = CameraRay(scn, x_delta, y_delta);

				t_castray struct_cast = CreateCastray	(&ray_cast, &level,
									&coeff_reflection,
									&coeff_refraction );

				RGB_samples = CastRay(scn, struct_cast, RGB_samples);

				for (k = 0 ; k < 3; k++)
					RGB[k] += RGB_samples[k]/n_samples;
			}

		for (k = 0 ; k < 3; k++)
			RGB[k] = MIN(RGB[k], MAX_COLOR);

		free(RGB_samples);

		return RGB;
}


float *GammaCorrection(float *RGB)
{
	float invgamma = 1.0/2.2f;

	RGB[0] = MIN(powf(RGB[0] * 255.0f, invgamma), 255.0f);
	RGB[1] = MIN(powf(RGB[1] * 255.0f, invgamma), 255.0f);
	RGB[2] = MIN(powf(RGB[2] * 255.0f, invgamma), 255.0f);

	return RGB;
}
