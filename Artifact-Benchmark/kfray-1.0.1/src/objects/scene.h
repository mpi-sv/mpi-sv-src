 /*
 ** KF-Ray, raytracer parallele
 **  scene.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef SCENE_H_
#define SCENE_H_

/**
 * \file	scene.h
 * \brief	Gère les objets de la scène
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Définit les structures des différents objets de la scène (rayon, matériau, lumière, sphère...).
 * Déclare les prototypes de fonctions liées à ces objets.
 *
 */

#include "../misc/misc.h"
#include "vector.h"
#include "light.h"
#include "material.h"
#include "plan.h"
#include "sphere.h"

/*	--------------------
		Structures
	--------------------*/

/**
 * \struct	s_ray
 * \brief	Structure de rayon de lumière
 *
 * Définit le rayon de lumière par :
 * - son point d'origine,
 * - un vecteur directeur.
 */

typedef struct	s_ray
{
	t_vector 	start;		/*!< Point d'origine du vecteur rayon de lumière */
	t_vector 	dir;		/*!< Vecteur directeur du rayon de lumière */
}		t_ray;


/**
 * \struct	s_camera
 * \brief	Objet Caméra
 *
 * Définit une caméra par :
 * - son origine
 *
 */
typedef struct	s_camera
{

	t_vector	point;		/*!< Point de vue de la caméra */
	float		dist_view;	/*!< Point d'observation de fuite*/

}		t_camera;


/**
 * \struct	s_scene
 * \brief	Scène 3D
 *
 * Définit la scène 3D par :
 * - la résolution de l'image,
 * - le nombre de plans, de sphères, de lumières,
 * - sa liste de plans, de sphères, de lumières,
 * - les options (modèle d'éclairage, activation/désactivation des textures),
 * - sa camera
 */
typedef struct	s_scene
{

	int		*viewport;		/*!< Résolution de l'image */
	int		n_plane;		/*!< Nombre de plans */
	int		n_sph;			/*!< Nombre de sphères */
	int		n_light;		/*!< Nombre de lumières */
	t_plane		*plane;			/*!< Liste des plans */
	t_sphere 	*sph;			/*!< Liste des sphères */
	t_light		*light;			/*!< Liste des lumières */
	t_options	options;		/*!< Diverses options de rendu */
	t_camera	camera;			/*!< Caméra */

}		t_scene;



/*	--------------------
		Prototypes
	-------------------- */


/**
 * \fn 		t_ray CreateRay(t_vector start0, t_vector dir0);
 * \brief 	Fonction de création d'un rayon
 *		Un rayon est définit tel que \f$\overrightarrow{P}=\overrightarrow{O}+\lambda.t\f$
 *
 * \param 	start0 Point d'origine du vecteur
 * \param 	dir0 Vecteur direction
 *
 * \return	Objet Rayon de lumière
 */
t_ray		CreateRay(t_vector start0, t_vector dir0);


/**
 * \fn 		t_ray		*CopyRay(t_ray *ray0);
 * \brief 	Fonction de copie d'un rayon
 *
 * \param 	ray0	une adresse d'un objet rayon.
 *
 * \return 	un pointeur sur le nouveau rayon copié
 */
t_ray		*CopyRay(t_ray *ray0);


/**
 * \fn 		void		PrintRay(t_ray ray0);
 * \brief 	Fonction d'affichage de rayon
 *
 * \param 	ray			un objet rayon
 *
 * \return 	néant
 */
void		PrintRay(t_ray ray);

/**
 * \fn 		t_camera	CreateCamera(t_vector point0, t_vector center_view0, float dist_view0);
 * \brief 	Fonction de création de caméra
 *
 * \param 	point0		Emplacement du spectateur
 * \param	center_view0	Le milieu de la scène que le spectateur veut voir (fixe)
 * \param	dist_view0	Distance fictive de vision
 *
 * \return 	néant
 */
t_camera	CreateCamera(t_vector point0, float dist_view0);


/**
 * \fn 		void		PrintCamera(t_camera camera);
 * \brief 	Fonction d'affichage de la caméra
 *
 * \param 	camera		La caméra de la scène
 *
 * \return 	néant
 */
void		PrintCamera(t_camera camera);


/**
 * \fn 		 t_scene CreateScene	(int *viewport0, int n_plane0, int n_sph0, int n_light0,
 *					t_plane *list_plane0, t_sphere *list_sph0, t_light *list_light0,
 *					t_options options0, t_camera camera0);
 * \brief 	Fonction de création de la scène 3D
 *
 * \param 	viewport0	Taille du viewport, de la scène 3D
 * \param	n_plane0	Nombre de plans
 * \param 	n_sph0		Nombre de sphères
 * \param	n_light0	Nombre de lumières
 * \param	list_plane0	Liste des plans
 * \param 	list_sph0	Liste des objets sphères
 * \param	list_light0	Liste des sources de lumière
 * \param	options0	Paramètres de raytracing pour la scène
 * \param	camera0		Point de vue de la caméra
 *
 * \return 	Scène 3D
 */
 t_scene	CreateScene	(int *viewport0, int n_plane0, int n_sph0, int n_light0,
				t_plane *list_plane0, t_sphere *list_sph0, t_light *list_light0,
				t_options options0, t_camera camera0);


/**
 * \fn 		void		FreeScene(t_scene scn);
 * \brief 	Libère l'espace mémoire allouée pour une scène
 *
 * \param 	scn		La scène à libérer
 *
 * \return 	néant
 */
void		FreeScene(t_scene scn);



#endif	/* SCENE_H_ */
