 /*
 ** KF-Ray, raytracer parallèle
 **  loader.c
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
#include <string.h>
#include "parser_yy.c"
#include "../objects/scene.h"
#include "loader.h"

t_scene			Loader		(char *scn_name, int model_brdf,
					int model_texture, int opt_aliasing,
					float cam_move)
{
	t_scene		scn;
	t_options	options;

	options.model_brdf = model_brdf;		// 0 sans modele d'éclairage
							// 1 avec Lambert
							// 2 avec Lambert + Blinn
							// 3 avec Lambert + Blinn-Phong
	options.model_texture = model_texture;		// Texture ou pas
	options.opt_aliasing = opt_aliasing;		// Activation/désactivation anti aliasing
    printf("heiheihei4!\n");

	if (strcmp(scn_name,"") == 0)
{
 printf("heiheihei44!\n");
		scn = LoadDefaultScn(model_brdf, model_texture, opt_aliasing); //set the sensitive!
}
	else
{
printf("heiheihei444!\n");
printf(scn_name);
printf("heiheihei4444!\n");

		scn = LoaderParser(scn_name, options);
}
    printf("heiheihei5!\n");

	// Pour l'animation par défaut
	scn.camera.point.z += cam_move;	//On va vers l'avant

	/* Animation pour la scène 4 qui roxxe */
	/*
	scn.camera.point.x += cam_move;		//On bouge à gauche
	scn.sph[0].center.x += 2.0*cam_move;
	scn.sph[0].center.y += 0.6*cam_move;
	scn.sph[1].center.x += 3.0*cam_move;
	scn.sph[1].center.y -= 0.4*cam_move;
	scn.sph[2].center.x += cam_move;
	scn.sph[2].center.x -= 0.5*cam_move;
	scn.sph[2].center.y += cam_move;
	scn.sph[3].center.x -= 1.5*cam_move;
	scn.sph[3].center.y += 3.0*cam_move;
	scn.sph[5].center.y -= cam_move;
	*/
	
	return 		scn;
}


/* Scène par défaut mis dans le code en dur */

t_scene			LoadDefaultScn	(int model_brdf, int model_texture,
					int opt_aliasing)
{
	int		viewport[] = {640, 480}; // set the viewport!
	t_scene		scn_default;
	t_camera	camera;
	t_material	*list_mat;
	t_light		*list_light;
	t_sphere	*list_sphere;
	t_plane 	*list_plane;
	t_options	options;
printf("heiheihei6!\n");
	list_mat = DefaultMat();
	list_plane = NULL;
	list_sphere = DefaultSph(list_mat[0], list_mat[1], list_mat[2]);
	list_light = DefaultLight(list_mat[0], list_mat[1], list_mat[2]);

	options.model_brdf = model_brdf;
	options.model_texture = model_texture;
	options.opt_aliasing = opt_aliasing;

	camera = CreateCamera	(CreateVector(0.0f, 0.0f, -210.0f), 2000.0f);

	scn_default = CreateScene	(viewport, 0, 3, 2,
					list_plane, list_sphere,list_light, options, camera);

	free(list_light);
	free(list_sphere);
	free(list_mat);

	return 		scn_default;
}


t_material		*DefaultMat(void)
{
	t_material	mat_jaune;
	t_material	mat_magenta;
	t_material	mat_cyan;

	t_material	*list_mat;
	list_mat= (t_material *) malloc(3 * sizeof (t_material));

	float		rgb_yellow[3] = {MAX_COLOR, MAX_COLOR, 0.0f};
	float		rgb_cyan[3] = {0.0f, MAX_COLOR, MAX_COLOR};
	float		rgb_magenta[3] = {MAX_COLOR, 0.0f, MAX_COLOR};

	mat_jaune = CreateMaterial	(MATERIAL_TURBULENCE, rgb_yellow,
					0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	mat_magenta = CreateMaterial	(MATERIAL_MARBEL, rgb_magenta,
					0.5f, 1.0f, 60.0f, 1.0f ,0.0f, 1.0f, 0.0f);
	mat_cyan = CreateMaterial	(MATERIAL_WOOD, rgb_cyan,
					0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 0.0f);

	list_mat[0] = mat_jaune;
	list_mat[1] = mat_magenta;
	list_mat[2] = mat_cyan;

	return 		list_mat;
}



t_sphere		*DefaultSph	(t_material mat_jaune,
					t_material mat_magenta,
					t_material mat_cyan)
{
	t_sphere	sph_jaune;
	t_sphere	sph_magenta;
	t_sphere	sph_cyan;
	t_sphere	*list_sphere;
	list_sphere = (t_sphere *) malloc(3 * sizeof (t_sphere));

	sph_jaune = CreateSphere(CreateVector(233.0, 290.0, 0.0), 100.0 , &mat_jaune);
	sph_cyan = CreateSphere(CreateVector(407.0, 290.0, 0.0), 100.0, &mat_cyan);
	sph_magenta = CreateSphere(CreateVector(320.0, 140.0, 0.0), 100.0, &mat_magenta);

	list_sphere[0] = sph_jaune;
	list_sphere[1] = sph_cyan;
	list_sphere[2] = sph_magenta;

	return		list_sphere;
}


t_light			*DefaultLight	(t_material mat_jaune,
					t_material mat_magenta,
					t_material mat_cyan)
{
	t_light		light_1;
	t_light		light_2;
	float		intensity_1[3] = {0.2f, 0.2f, 0.2f};
	float		intensity_2[3] = {0.5f, 0.5f, 0.5f};
	t_light		*list_light;
	list_light = (t_light *) malloc(2 * sizeof (t_light));

	light_1 = CreateLight( CreateVector(0.0f, 240.0f,-100.0f),  intensity_1 );
	light_2 = CreateLight( CreateVector(640.0f,240.0f,-10000.0f), intensity_2 );

	list_light[0] = light_1;
	list_light[1] = light_2;

	return		list_light;
}



/* A titre d'exemples des scènes "à la main" non regardés par le compilateur */
/* Ne pas prendre en compte le code ci-dessous. */

#if 0

t_scene Loader(char *scn_name, int model_brdf, int model_texture, int opt_aliasing, float cam_move)
{
	/* On fait des spheres et des lumières car c'est rigolo */

	t_plane		plane1, plane2, plane3;
	t_sphere	sph1, sph2, sph3, sph4, sph5, sph6;
	t_light		light1, light2;
	t_material	mat_jaune, mat_bleu, mat_rouge, mat_cyan, mat_vert, mat_magenta, mat_couleur1, mat_couleur2, mat_couleur3, mat_rouge1, mat_vert1, mat_bleu1;
	t_camera	camera1, camera2;

	mat_jaune = CreateMaterial(MATERIAL_TURBULENCE, jaune, 0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	mat_magenta = CreateMaterial(MATERIAL_MARBEL, magenta, 0.5f, 1.0f, 60.0f, 1.0f ,0.0f, 1.0f, 1.0f);
	mat_cyan = CreateMaterial(MATERIAL_WOOD, cyan, 0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	mat_bleu = CreateMaterial(MATERIAL_WOOD, bleu, 0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	mat_vert = CreateMaterial(MATERIAL_WOOD, vert, 0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	mat_rouge = CreateMaterial(MATERIAL_MARBEL, rouge, 0.5f, 1.0f, 60.0f, 1.0f, 0.0f, 1.0f, 1.0f);

	mat_couleur1 = CreateMaterial(MATERIAL_NORMAL, couleur1, 0.2f, 0.4f, 5.0f, 1.0f, 0.1f, 1.0f, 1.0f);
	mat_couleur2 = CreateMaterial(MATERIAL_NORMAL, couleur2, 0.4f, 0.8f, 60.0f, 1.0f, 0.1f, 1.0f, 1.0f);
	mat_couleur3 = CreateMaterial(MATERIAL_NORMAL, couleur3, 0.4f, 1.0f, 60.0f, 1.0f, 0.1f, 1.0f, 1.0f);

	mat_rouge1 = CreateMaterial(MATERIAL_TURBULENCE, rouge, 0.2f, 0.8f, 1000.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	mat_bleu1 = CreateMaterial(MATERIAL_WOOD, bleu, 0.6f, 0.8f, 1000.0f, 1.25f, 0.1f, 1.0f, 1.0f);
	mat_vert1 = CreateMaterial(MATERIAL_MARBEL, vert, 0.4f, 0.8f, 1000.0f, 1.25f, 0.0f, 1.0f, 1.0f);

	float		intensity1[3] = {0.6f, 0.6f, 0.6f};
	float		intensity2[3] = {0.4f, 0.4f, 0.4f};
	light1 = CreateLight( CreateVector(0.0f, 240.0f,-100.0f),  intensity1 );

	//light2 = CreateLight( CreateVector(640.0f,140.0f,-500.0f), intensity2 );
	light2 = CreateLight( CreateVector(640.0f,240.0f,-10000.0f), intensity2 );

	t_light liste_lumieres[2] = { light2, light1 };

	sph1 = CreateSphere(CreateVector(250.0f, 100.0f, -30.0f), 60.0, &mat_bleu);
	sph2 = CreateSphere(CreateVector(400.0f, 180.0f, 0.0f), 100.0, &mat_vert);
	sph3 = CreateSphere(CreateVector(500.0f, 240.0f , 40.0f), 50.0, &mat_rouge);
	t_sphere liste1_spheres[3] = { sph1, sph2, sph3 };


	sph1 = CreateSphere(CreateVector(233.0, 290.0, 0.0), 100.0 , &mat_jaune);
	sph2 = CreateSphere(CreateVector(407.0, 290.0, 0.0), 100.0, &mat_cyan);
	sph3 = CreateSphere(CreateVector(320.0, 140.0, 0.0), 100.0, &mat_magenta);
	t_sphere liste2_spheres[3] = { sph1, sph2, sph3 };

/*
	sph1 = CreateSphere(CreateVector(100.0, 400.0, 0.0), 250.0f , &mat_jaune);
	sph2 = CreateSphere(CreateVector(500.0, 400.0, 0.0), 250.0f, &mat_cyan);
	sph3 = CreateSphere(CreateVector(300.0, 100.0, 0.0), 250.0f, &mat_magenta);
	t_sphere liste2_spheres[3] = { sph1, sph2, sph3 };
*/

	sph1 = CreateSphere(CreateVector(200.0f, 250.0f, 260.0f), 40.0f , &mat_rouge1);
	sph2 = CreateSphere(CreateVector(300.0f, 150.0f, 400.0f), 160.0f, &mat_bleu1);
	sph3 = CreateSphere(CreateVector(400.0f, 200.0f, 200.0f), 50.0f, &mat_vert1);
	t_sphere liste3_spheres[3] = { sph1, sph2, sph3 };

	plane1 = CreatePlane( 100.0f, CreateVector(0.0f,-1.0f, 0.05f), &mat_couleur1);
	plane2 = CreatePlane( -350.0f, CreateVector(0.0f,-1.0f, -0.1f), &mat_couleur2);

	t_plane liste_plane[1] = {plane1};
	t_plane liste_plane2[1] = {plane2};

	int viewport[] = {640, 480};
	int viewport2[] = {800, 600};

	camera1 = CreateCamera(CreateVector(0.0f, 0.0f, -110.0f + cam_move), CreateVector(viewport[0]/2, viewport[1]/2, 800.0f));
	camera2 = CreateCamera(CreateVector(0.0f, 0.0f, -300.0f + cam_move), CreateVector(viewport[0]/2, viewport[1]/2, 400.0f));

	// Les options !
	t_options options;

	options.model_brdf = model_brdf;	// 0 sans modele d'éclairage
						// 1 avec Lambert
						// 2 avec Lambert + Blinn
						// 3 avec Lambert + Blinn-Phong
						// 4 avec Pikachu

	options.model_texture = model_texture;		// Texture ou pas

	options.opt_aliasing = opt_aliasing;		// Activation/désactivation anti aliasing

	t_scene scn1 = CreateScene(viewport, 1, 3, 1, liste_plane, liste1_spheres, liste_lumieres, options, camera2);
	t_scene scn2 = CreateScene(viewport, 0, 3, 2, liste_plane, liste2_spheres, liste_lumieres, options, camera1);
	t_scene scn3 = CreateScene(viewport2, 1, 3, 2, liste_plane2, liste3_spheres, liste_lumieres, options, camera1);

	// Scène 4

	int viewport4[] = {1024, 768};

	t_camera camera4 = CreateCamera(CreateVector(0.0f, 0.0f, -1600.0f + cam_move), CreateVector(viewport4[0]/2, viewport4[1]/2, 0.0f), 600.0f);

	mat_bleu1 = CreateMaterial(MATERIAL_CHECKER, bleu, 0.8f, 0.6f, 60.0f, 1.25f, 0.0f, 1.0f, 1.0f);
	mat_vert1 = CreateMaterial(MATERIAL_CHECKER, vert, 0.6f, 0.4f, 100.0f, 1.00f, 0.0f, 1.0f, 1.0f);

	//plane1 = CreatePlane( 100.0f, CreateVector(0.0f, 1.0f, 0.1f), &mat_bleu1);
	plane1 = CreatePlane (20.0f, CreateVector(0.0f, -1.0f, 0.05f), &mat_bleu1);
	plane2 = CreatePlane( 1000.0f, CreateVector(0.0f, 0.0f, -1.0f), &mat_vert1);
	plane3 = CreatePlane( 10.0f, CreateVector(1.0f, 0.0f, 0.05f), &mat_vert1);
	t_plane liste4_plane[3] = {plane1, plane2, plane3};

	float intensity3[3] = {0.4f, 0.4f, 0.4f};
	float intensity4[3] = {0.6f, 0.6f, 0.6f};

	light1 = CreateLight( CreateVector(800.0f, 300.0f, 600.0f), intensity3);
	light2 = CreateLight( CreateVector(100.0f, 400.0f, -1200.0f), intensity4);

	//light1 = CreateLight( CreateVector(800.0f, 300.0f,-2000.0f), intensity3);
	//light2 = CreateLight( CreateVector(200.0f, 100.0f, -2000.0f), intensity4);

//	t_light light3 = CreateLight( CreateVector(0.0f, 240.0f,-100.0f), intensity4);
	t_light liste4_lumieres[2] = { light2, light1 };

	mat_jaune = CreateMaterial(MATERIAL_NORMAL, jaune, 0.4f, 0.1f, 600.0f, 1.25f, 0.3f, 1.0f, 1.0f);
	mat_bleu = CreateMaterial(MATERIAL_NORMAL, bleu, 1.0f, 1.0f, 1000.0f, 1.25f, 0.0f, 1.0f, 1.0f);
	mat_vert = CreateMaterial(MATERIAL_WOOD, vert, 0.2f, 0.2f, 100.0f, 1.25f, 0.0f, 1.0f, 1.0f);
	mat_magenta = CreateMaterial(MATERIAL_CHECKER, magenta, 0.8f, 0.8f, 1000.0f, 1.0f ,0.0f, 1.4f, 1.0f);
	mat_cyan = CreateMaterial(MATERIAL_TURBULENCE, cyan, 0.5f, 0.2f, 60.0f, 1.2f, 0.0f, 1.0f, 1.0f);
	mat_rouge = CreateMaterial(MATERIAL_MARBEL, rouge, 0.1f, 1.0f, 60.0f, 1.25f, 0.0f, 1.0f, 1.0f);

	sph1 = CreateSphere(CreateVector(60.0f, 180.0f, 60.0f), 100.0f , &mat_jaune);
	//sph1 = CreateSphere(CreateVector(600.0f, 320.0f, 0.0f), 280.0f , &mat_jaune);
	sph2 = CreateSphere(CreateVector(220.0f, 350.0f, 160.0f), 180.0f, &mat_bleu);
	sph3 = CreateSphere(CreateVector(480.0f, 480.0f, 80.0f), 100.0f, &mat_vert);
	sph4 = CreateSphere(CreateVector(1000.0f, 350.0f, 90.0f), 80.0f, &mat_rouge);
	sph5 = CreateSphere(CreateVector(850.0f, 700.0f, 1000.0f), 180.0f, &mat_cyan);
	sph6 = CreateSphere(CreateVector(800.0f, 300.0f, 400.0f), 180.0f, &mat_magenta);

	t_sphere liste4_spheres[6] = { sph1, sph2, sph3, sph4, sph5, sph6 };

	t_scene scn4 = CreateScene(viewport4, 1, 6, 2, liste4_plane, liste4_spheres, liste4_lumieres, options, camera4);

	//printf(">> Fichier Scène = [%s]\n", scn_name);
	if (strcmp(scn_name, "scene1.kfd") == 0 || strcmp(scn_name, "1.kfd") == 0)
		return scn1;
	if (strcmp(scn_name, "scene2.kfd") == 0 || strcmp(scn_name, "2.kfd") == 0)
		return scn2;
	if (strcmp(scn_name, "scene3.kfd") == 0 || strcmp(scn_name, "3.kfd") == 0)
		return scn3;
	if (strcmp(scn_name, "scene4.kfd") == 0 || strcmp(scn_name, "4.kfd") == 0)
		return scn4;

	printf("!Warning : Erreur lors du chargement de la scène. Scène par défaut chargée.\n");

	return LoadDefaultScn(model_brdf, model_texture, opt_aliasing, cam_move);
}

#endif
