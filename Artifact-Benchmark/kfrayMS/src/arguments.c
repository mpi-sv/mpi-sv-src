 /*
 ** KF-Ray, raytracer parallèle
 **  arguments.c
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
#include <string.h>
#include <unistd.h>
#include <string.h>

#include "main.h"
#include "misc/printinfo.h"
#include "arguments.h"

t_arguments	CreateArg	(char *scn_name0, char *img_name0,
				int *model_brdf0, int *model_texture0,
				int *n_lines0, int *opt_display0,
				int *opt_aliasing0, float *cam_move0)
{
	t_arguments arg_main;

	arg_main.scn_name = scn_name0;
	arg_main.img_name = img_name0;
	arg_main.model_brdf = model_brdf0;
	arg_main.model_texture = model_texture0;
	arg_main.n_lines = n_lines0;
	arg_main.opt_display = opt_display0;
	arg_main.opt_aliasing = opt_aliasing0;
	arg_main.cam_move = cam_move0;

	return	arg_main;
}


void		PrintArg(t_arguments arg_main)
{
	printf("scn: %s - img: %s - brdf/texture/n_lines: opt_display/opt_aliasing:",
		arg_main.scn_name, arg_main.img_name);
}


int		LookArguments(int argc, char **argv, t_arguments arg_main)
{
	int	optchar;
	optind = 1; // added by zhenbang to fix the multitheaded problem

	while ((optchar = getopt (argc, argv, "i:o:h c b:t l:v:a d ")) != -1)
	{
		// Normalement on indente pas comme ça mais c'est ridicule de créer une autre fonction pour ça
		// Juste pour respecter les règles.
		switch (optchar)
		{
			case 'i':	OptScn(arg_main.scn_name, arg_main.img_name, optarg); break;

			case 'o':	OptImg(arg_main.img_name, optarg); break;

			case 'h':	return OptHelp();

			case 'c':	return OptClean();

			case 'a':	OptAliasing(arg_main.opt_aliasing); break;

			case 'd':	OptDisplay(arg_main.opt_display); break;

			case 'b':	if(!OptBrdf(arg_main.model_brdf, optarg))
						return OPT_STOP; break;

			case 't':	OptTexture(arg_main.model_texture); break;

			case 'l':	OptLines(arg_main.n_lines, optarg); break;

			case 'v':	OptView(arg_main.cam_move, optarg); break;

			case '?':	return OptUnknown(argv[0]);

			default : 	break;
		}
	}

#ifndef MY_MPI
	printf("%s", splash);
	printf(">> Nom du fichier de scene :		[%s]\n", arg_main.scn_name);
#endif

	return	 OPT_CONTINUE;
}



int		OptScn(char *scn_name, char *img_name, char *optarg)
{
	strcpy(scn_name, optarg);

	// On regarde si le nom finit par .kfr et s'il a un .
	if (strlen(scn_name) >= 3)
		if (
		strcmp(".kfr", scn_name + (strlen(scn_name)-4)) != 0 &&
		strchr(scn_name, '.') == NULL
		)
			strcat(scn_name, ".kfr");

	if (strlen(scn_name) < 3)
		strcat(scn_name, ".kfr");

	// On donne à l'image le nom de la scène par défaut
	strcpy(img_name, optarg);
	if (strlen(optarg) >= 3)
		if (strcmp(".kfr", scn_name + (strlen(scn_name)-4)) == 0)
			img_name[strlen(scn_name)-4] = '\0'; // On tronque

	strcat(img_name, ".ppm");

	return 	OPT_CONTINUE;
}


int		OptImg(char *img_name, char *optarg)
{
	strcpy(img_name, optarg);

	if (strlen(img_name) >= 3)
		if (strcmp(".ppm", img_name + (strlen(img_name)-4)) != 0)
			strcat(img_name, ".ppm");

	if (strlen(img_name) < 3)
		strcat(img_name, ".ppm");

	return 	OPT_CONTINUE;
}


int		OptHelp(void)
{
	printf("%s", help);

	return OPT_STOP;
}


int		OptClean(void)
{
	char	*path;
	path = (char *)malloc(MAX_FILE_LENGTH * sizeof(char));

	sprintf(path, "rm ../scenes/images/*.ppm");

	//execlp("/bin/sh", "/bin/sh", "-c", "rm *.ppm", NULL);
	execlp("/bin/sh", "/bin/sh", "-c", path, NULL);

	free(path);

	return	 OPT_STOP;
}


int		OptDisplay(int *opt_display)
{
	*opt_display = 1;

	return OPT_CONTINUE;
}


int		OptAliasing(int *opt_aliasing)
{
	*opt_aliasing = 1;

	return	OPT_CONTINUE;
}


int		OptBrdf(int *model_brdf, char *optarg)
{
	if (atoi(optarg) >= 0 && atoi(optarg) <= 3)
		*model_brdf = atoi(optarg);
	else
	{
		printf("L'option -b doit avoir un argument compris entre 0 et 3.\n");
		printf ("Utilisation : kfray [-i Scene] [-o Image] ");
		printf("[-b Modele] [-v Distance] [-t] [-a] [-l Lignes] [-d]\n");
		return OPT_STOP;
	}

	return	OPT_CONTINUE;
}


int		OptTexture(int *model_texture)
{
	*model_texture = 0;

	return	OPT_CONTINUE;
}


int		OptLines(int *n_lines, char *optarg)
{
	if (*n_lines > 0)
		*n_lines = atoi(optarg);
	else
	{
		printf("L'option -l doit avoir un argument compris entre 1 et le hauteur de l'image\n");
		printf ("Utilisation : kfray [-i Scene] [-o Image] [-b Modele] [-v Distance] [-t] [-a] [-l Lignes] [-d]\n");

		return OPT_STOP;
	}

	return	OPT_CONTINUE;
}


int		OptView(float *cam_move, char *optarg)
{

	*cam_move = atof(optarg);

	return	OPT_CONTINUE;
}


int		OptUnknown(char *prgm_name)
{
	printf ("KF-Ray - commande inconnue...\n");
	printf ("Utilisation : %s [-i Scene] [-o Image] [-b Modele] [-t] [-a] [-l Lignes] [-d]\n", prgm_name);
	printf ("Pour afficher l'aide : %s -h\n", prgm_name);

	return	OPT_STOP;
}
