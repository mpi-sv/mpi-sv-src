 /*
 ** KF-Ray, raytracer parallèle
 **  parser_yy.y
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

%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../objects/scene.h"

t_scene		scene;
int		*viewport0;
int		nb_mat;
t_material	*list_mat0;
t_plane		*list_plane0;
t_sphere	*list_sph0;
t_light		*list_light0;
t_camera	camera0;
int		count_mat = 0;
int		count_plane = 0;
int		count_sph = 0;
int		count_light = 0;
int		current;


%}

%union	{
	float	nb_float;
	char	*string;
	int	nb_int;
}

%token <string>		STRING
%token <nb_float>	FLOAT
%token <nb_int>		INTEGER

%token		WIDTH
%token		HEIGHT
%token 		MATERIALS
%token		PLANES
%token		SPHERES
%token		LIGHTS
%token		CAMERA
%token		DISTVIEW
%token		MAT
%token		ID
%token		TYPE
%token		NORMAL
%token		TURBULENCE
%token		MARBEL
%token		WOOD
%token		CHECKER
%token		RVB
%token		REFLECTION
%token		SPECULAR
%token		ROUGHNESS
%token		PERLIN
%token		BUMP
%token		REFRACTION
%token		DENSITY
%token		PLANE
%token		NORMALE
%token		DISTANCE
%token		MATERIAL
%token		SPHERE
%token		CENTER
%token		RADIUS
%token		LIGHT
%token		POSITION
%token		INTENSITY
%token		EOL

%%

description_scene: 	lines
			;

lines:		one_line lines
		| one_line
		;

one_line: 	  line_WIDTH EOL
		| line_HEIGHT EOL
		| line_MATERIALS EOL
		| line_PLANES EOL	
		| line_SPHERES EOL
		| line_LIGHTS EOL
		| line_CAMERA EOL
		| line_DISTVIEW EOL
		| line_MAT EOL
		| line_ID EOL
		| line_TYPE EOL
		| line_RVB EOL
		| line_REFLECTION EOL
		| line_SPECULAR EOL
		| line_ROUGHNESS EOL
		| line_PERLIN EOL
		| line_BUMP EOL
		| line_REFRACTION EOL
		| line_DENSITY EOL
		| line_PLANE EOL
		| line_NORMALE EOL
		| line_DISTANCE EOL
		| line_MATERIAL EOL
		| line_SPHERE EOL
		| line_CENTER EOL
		| line_RADIUS EOL
		| line_LIGHT EOL
		| line_POSITION EOL
		| line_INTENSITY EOL
		| EOL
		;


line_WIDTH:		WIDTH INTEGER
			{
				viewport0 = (int*) malloc (2*sizeof(int)); viewport0[0] = $2;
			}
		
line_HEIGHT:		HEIGHT INTEGER
			{
				viewport0[1] = $2;
			}
		
line_MATERIALS:		MATERIALS INTEGER
			{
				nb_mat = $2; list_mat0 = (t_material*) malloc ($2*sizeof(t_material));
			}
		
line_PLANES:		PLANES INTEGER
			{
				scene.n_plane = $2; list_plane0 = (t_plane*) malloc ($2*sizeof(t_plane));
			}
		
line_SPHERES:		SPHERES INTEGER
			{
				scene.n_sph = $2; list_sph0 = (t_sphere*) malloc ($2*sizeof(t_sphere));
			}
		
line_LIGHTS:		LIGHTS INTEGER
			{
				scene.n_light = $2; list_light0 = (t_light*) malloc ($2*sizeof(t_light));
			}
		
line_CAMERA:		CAMERA FLOAT FLOAT FLOAT 
			{
				camera0.point = CreateVector($2, $3, $4);
			}

line_DISTVIEW:		DISTVIEW FLOAT
			{
				camera0.dist_view = $2;
			}
	
line_MAT:		MAT 
			{ 
				count_mat++;
				current = 1;
			}
		
line_ID :		ID INTEGER
			{
				list_mat0[count_mat-1].id = $2;
			}
		
line_TYPE:		TYPE NORMAL		{ list_mat0[count_mat-1].type = 0; }
			| TYPE TURBULENCE	{ list_mat0[count_mat-1].type = 1; } 
			| TYPE MARBEL 		{ list_mat0[count_mat-1].type = 2; }
			| TYPE WOOD 		{ list_mat0[count_mat-1].type = 3; }
			| TYPE CHECKER		{ list_mat0[count_mat-1].type = 4; }		
						
line_RVB: 		RVB FLOAT FLOAT FLOAT
			{
				list_mat0[count_mat-1].color = CopyfArray(CreateArrayColor($2, $3, $4), 3); 
			}
		
line_REFLECTION:	REFLECTION FLOAT 
			{ 
				list_mat0[count_mat-1].reflection = $2; 
			}
			
line_SPECULAR:		SPECULAR FLOAT
			{ 
				list_mat0[count_mat-1].specular = $2;
			}
			
line_ROUGHNESS: 	ROUGHNESS FLOAT
			{
				list_mat0[count_mat-1].power = $2;
			}
			
line_PERLIN: 		PERLIN FLOAT 
			{
				list_mat0[count_mat-1].perlin = $2;
			}
			
line_BUMP: BUMP FLOAT 	{
				list_mat0[count_mat-1].bump = $2;
			}

line_REFRACTION :	REFRACTION FLOAT
			{
				list_mat0[count_mat-1].refraction = $2;
			}

line_DENSITY :		DENSITY FLOAT
			{
				list_mat0[count_mat-1].density = $2;
			}

line_PLANE : 		PLANE
			{	
				count_plane++; 
				current = 2;
			}
			
line_NORMALE :		NORMALE FLOAT FLOAT FLOAT 
			{
				list_plane0[count_plane-1].normal = CreateVector($2,$3,$4);
			}
			
line_DISTANCE: 		DISTANCE FLOAT
			{
				list_plane0[count_plane-1].dist = $2;
			}
			
line_MATERIAL:		MATERIAL INTEGER
			{ 
				if (current == 2)
					list_plane0[count_plane-1].material = CopyMaterial(&list_mat0[$2-1]);   
				if (current == 3)
					list_sph0[count_sph-1].material = CopyMaterial(&list_mat0[$2-1]);
			}
			
line_SPHERE:		SPHERE
			{ 
				count_sph++; 
				current = 3;
			}
			
line_CENTER : 		CENTER FLOAT FLOAT FLOAT
			{
				list_sph0[count_sph-1].center = CreateVector($2,$3,$4);
			}
			
line_RADIUS:		RADIUS FLOAT
			{
				list_sph0[count_sph-1].radius = $2;
			}
			
line_LIGHT:		LIGHT
			{
				count_light++;  current = 4;
			}
			
line_POSITION:		POSITION FLOAT FLOAT FLOAT
			{
				list_light0[count_light-1].position = CreateVector($2,$3,$4);
			}
			
line_INTENSITY:		INTENSITY FLOAT FLOAT FLOAT
			{
				list_light0[count_light-1].intensity =  CopyfArray(CreateArrayIntensity($2, $3, $4), 3);
			}

%%
int	yyerror(char *s)
{
	printf ("Erreur de lecture du fichier de description de scène : %s\n", s);
	exit (0);
	
 	return 0;
}

void	OpenFile(char *scn_name)
{

	FILE *file;

	char *ptr;
	ptr = (char *) malloc(MAX_FILE_LENGTH * sizeof(char));
	sprintf(ptr, "../scenes/%s", scn_name);
printf("opened file is %s\n",ptr);
	file = fopen(ptr, "r");
	if (!file)
	{
		fprintf(stderr,"Erreur : Le fichier de description de scène %s est introuvable\n", ptr);
		exit(1);
	}

	yyrestart (file);
	yyparse ();
	fclose (file);

}


void 	DefaultValues(void)
{
	int i;	// pour les compteurs 
	
	// On met ici les valeurs par défaut avant de prendre les valeurs du fichiers.
	camera0.point = CreateVector(0.0f, 0.0f, -1000.0f);
	camera0.dist_view = 2000.0f;
	
	for (i = 0; i < nb_mat; i++)
	{
		list_mat0[i].specular = 0.8f;
		list_mat0[i].refraction = 0.0f;
		list_mat0[i].perlin = 1.0f;
		list_mat0[i].bump = 0.0f;
	}
}




t_scene LoaderParser(char *scn_name, t_options scn_opt)
{

	
	// On charge les valeurs par défaut
	DefaultValues();
	
	// On ouvre le fichier demandé
	OpenFile(scn_name);

	scene.viewport = CopyiArray(viewport0, 2);
	scene.plane = list_plane0;
	scene.sph = list_sph0;
	scene.light = list_light0;
							
	scene.camera = camera0;

	scene.options.model_brdf = scn_opt.model_brdf;		// 0 sans modele d'éclairage
								// 1 avec Lambert
								// 2 avec Lambert + Blinn
								// 3 avec Lambert + Blinn-Phong

	scene.options.model_texture = scn_opt.model_texture;	// Texture ou pas

	scene.options.opt_aliasing = scn_opt.opt_aliasing;     	// Activation/désactivation anti aliasing

	
	return scene;
}


