 /*
 ** KF-Ray, raytracer parallèle
 **  raytracer.c
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
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include "raytracer.h"
#include "raycaster/raycaster.h"
#include "objects/scene.h"
#include "misc/misc.h"
#include "misc/printinfo.h"
#include "misc/image.h"
#include "models/postprocessing.h"
#include "misc/image.h"


void	Raytracer(t_scene scn, char *img_name)
{

	InfoRaytracer(scn, img_name); // will not modify scn!

	int		x;
	int		y;

	unsigned char	*image;
	image  = 	(unsigned char*) \
			malloc(scn.viewport[0] * scn.viewport[1] * 3 * sizeof (unsigned char));

	printf(">> Balayage :				[En cours...]\n");

	for (y=0; y<scn.viewport[1]; y++)
		for (x=0; x<scn.viewport[0]; x++)
			ComputePixel(scn, image, x, y);

	SaveImage(image, img_name, scn.viewport[0], scn.viewport[1]);

	FreeScene(scn);
	//free(image); WTF Segmentation fault sur shell mais pas sur codeblocks ?!
}


void	*ComputePixel(t_scene scn, unsigned char *image, int x, int y)
{
	int	k;
	float 	coeff_reflection = 1.0f;
	float	coeff_refraction = 1.0f;
	int 	level = 0;

	float	*RGB;
	RGB = (float*) malloc(3 * sizeof (float));

	// On initialise RGB à [0,0,0]
	for (k = 0 ; k < 3; k++)
		RGB[k] = 0.0f;

	if (scn.options.opt_aliasing == 0)
	{
		t_ray ray_cast = CameraRay(scn, x, y);
		t_castray struct_cast = CreateCastray	(&ray_cast, &level,
							&coeff_reflection, &coeff_refraction);

		RGB = CastRay(scn, struct_cast, RGB);
	}
	else
		RGB = AntiAliasing(scn, RGB, x, y);

	// Remise à l'échelle
	for (k = 0 ; k < 3; k++)
		RGB[k] = RGB[k] * 255.0 / MAX_COLOR;

	//RGB = GammaCorrection(RGB);
	SetPixel(image, scn.viewport[0], x, y,
		(int) RGB[0], (int) RGB[1], (int) RGB[2]);// will not modify scn

	// Désallocation mémoire
	free(RGB);
}


t_ray	CameraRay(t_scene scn, int x, int y)
{
	t_vector point_spectator = CreateVector	(x  + scn.camera.point.x,
						scn.viewport[1] - (y + scn.camera.point.y),
						scn.camera.point.z);

	t_vector vect_view = CreateVector	( point_spectator.x - scn.viewport[0]/2,
						point_spectator.y - scn.viewport[1]/2 ,
						scn.camera.dist_view - point_spectator.z);

	vect_view = Normalize(vect_view);

	t_ray	ray_cast = CreateRay(point_spectator, vect_view);

	return	ray_cast;
}



/* Caméra libre pour les versions prochaines il faut bouger le plan de vision etc...*/

#if 0

t_ray	CameraRayFree(t_scene scn, int x, int y)
{
	t_vector vect_view = CreateVector	( -scn.camera.point.x,
						-scn.camera.point.y,
						-scn.camera.point.z );

	vect_view = Normalize(vect_view);


	t_vector point_view = Prod(scn.camera.center_view.z, vect_view);

	/* Truc compliqué prise de tête qui marche pas normal faut rotationner le plan de vue -_-
	/*
	t_vector vect_i = Normalize(CreateVector(1.0f, -(point_view.x / point_view.y), 0.0f));
	t_vector vect_j = Normalize(CreateVector(1.0f, 0.0f, -(point_view.x / point_view.z)));

	t_vector point_spectator = CreateVector	(x * vect_i.x + scn.camera.point.x,
						scn.viewport[1] - (y * vect_j.y + scn.camera.point.y),
						scn.camera.point.z + x * vect_i.z + y * vect_j.z );
	*/


	t_vector point_spectator = CreateVector	(x  + scn.camera.point.x,
						scn.viewport[1] - (y + scn.camera.point.y),
						scn.camera.point.z);

	t_vector vect_vision =	CreateVector	(point_spectator.x - point_view.x - scn.camera.center_view.x,
						point_spectator.y - point_view.y - scn.camera.center_view.y,
						point_view.z - point_spectator.z );

	vect_vision = Normalize(vect_vision);

	t_ray	ray_cast = CreateRay(point_spectator, vect_vision);

	return ray_cast;
}

#endif
