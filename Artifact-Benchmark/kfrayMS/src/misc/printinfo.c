 /*
 ** KF-Ray, raytracer parallele
 **  printinfo.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#include <stdio.h>
#include <string.h>
#include "../objects/scene.h"
#include "printinfo.h"

void	InfoRaytracer(t_scene scn, char *img_name)
{
	char txt_aliasing[10];
	char txt_texture[10];

	if (scn.options.model_texture == 0)
		strcpy(txt_texture, txt_unactive);
	else
		strcpy(txt_texture, txt_active);

	if (scn.options.opt_aliasing == 0)
		strcpy(txt_aliasing, txt_unactive);
	else
		strcpy(txt_aliasing, txt_active);

	printf(">> Chargement de la scene :\
		[OK]\n");

	printf(">> Nom de l'image de sortie :\
		[%s]\n", img_name);

	printf(">> BRDF/Texture/Aliasing :\
		[%d/%s/%s]\n", scn.options.model_brdf, txt_texture, txt_aliasing);

	printf(">> Dimension Viewport :\
			[%dx%d]\n", scn.viewport[0], scn.viewport[1]);

	printf(">> Plans/Spheres/Sources :\
		[%d/%d/%d]\n", scn.n_plane, scn.n_sph, scn.n_light);
}

void	InfoMaster(char *img_name, int w, int h, int n_proc, int n_lines)
{
	printf(">> Chargement de la scene :		[OK]\n");
	printf(">> Nom de l'image de sortie :		[%s]\n", img_name);
	printf(">> Dimension Viewport :			[%dx%d]\n", w, h);
	printf(">> Parallelisme active :		[OK]\n");
	printf(">> Processus/lines :			[%d/%d]\n", n_proc, n_lines);
}
