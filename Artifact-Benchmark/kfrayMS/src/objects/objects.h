#ifndef OBJECTS_H_
#define OBJECTS_H_

/**
 * \file	objects.h
 * \brief	Gère les objets de la scène
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	0.3
 * \date	Fevrier-Juin 2009
 *
 * Définit les structures des différents objets de la scène (rayon, matériau, lumière, sphère...).
 * Déclare les prototypes de fonctions liées à ces objets.
 *
 */

#include "vector.h"
#include "../misc/misc.h"


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
	t_vector 	start;
	t_vector 	dir;
} t_ray;

/**
 * \struct s_light
 * \brief Structure de source de lumière
 *
 * Définit une source lumineuse par :
 * - le point d'origine de sa source lumineuse,
 * - son intensité lumineuse.
 */
typedef struct 	s_light
{

	t_vector 	position;
	float 		*intensity;

} t_light;

/**
 * \struct	s_material
 * \brief	Structure de matériau
 *
 * Définit un matériau par :
 * - son type (normal ou avec texture procédurale en marbre, bois, turbulence...),
 * - sa couleur RVB,
 * - son coefficient de réflexion (compris entre 0.0 et 1.0),
 * - son coefficient de réflexion spéculaire (compris entre 0.0 et 1.0),
 * - son coefficient de rugosité (compris entre 0.0 et 1000.0),
 * - son coefficient de bruit de Perlin (compris entre 0.0 et 2.0),
 * - son coefficient de bump mapping (compris entre 0.0 et 5.0).
 */
typedef struct	s_material
{
	int 		type;
	float 		*color;
	float 		reflection;
	float		specular;
	float		power;
	float		perlin;
	float		bump;

} t_material;

/**
 * \struct	s_plane
 * \brief	Objet Plan
 *
 * Définit un plan par :
 * - sa distance par rapport au point d'origine (distance du projeté orthognal de l'origine par rapport à la normale du plan),
 * - un vecteur normal,
 * - sa couleur RVB.
 */
typedef struct 	s_plane
{

	float		dist;
	t_vector	normal;
	t_material	*material;

} t_plane;

/**
 * \struct	s_sphere
 * \brief	Objet Sphère
 *
 * Définit une sphère par :
 * - son centre,
 * - son rayon,
 * - son matériau.
 */
typedef struct 	s_sphere
{

	t_vector 	center;
	float 		radius;
	t_material	*material;

} t_sphere;

/**
 * \struct	s_camera
 * \brief	Objet Caméra
 *
 * Définit une caméra par :
 * - son origine
 *
 */
typedef struct s_camera
{

	t_vector	point;			/*!< Point de vue de la caméra */
	t_vector	center_view;	/*!< Point d'observation */
	float		dist_view;		/*!< Distance de l'observation */

} t_camera;


/**
 * \struct	s_scene
 * \brief	Scène 3D
 *
 * Définit la scène 3D par :
 * - la taille de son viewport (de l'image),
 * - le nombre de plans, de sphères, de lumières,
 * - sa liste de plans, de sphères, de lumières,
 * - les options (modèle d'éclairage, activation/désactivation des textures),
 * - sa camera
 */
typedef struct	s_scene
{

	int			*viewport;
	int			n_plane;
	int			n_sph;
	int			n_light;
	t_plane	*plane;
	t_sphere 	*sph;
	t_light		*light;
	t_options	options;
	t_camera	camera;

} t_scene;



/*	--------------------
		Prototypes
	-------------------- */


/**
 * \fn 		t_ray CreateRay(t_vector start0, t_vector dir0);
 * \brief 	Fonction de création d'un rayon
 *			Un rayon est définit tel que \f$\overrightarrow{P}=\overrightarrow{O}+\lambda.t\f$
 *
 * \param 	start0 Point d'origine du vecteur
 * \param 	dir0 Vecteur direction
 *
 * \return  Objet Rayon de lumière
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
 * \param 	ray	un objet rayon
 *
 * \return 	néant
 */
void		PrintRay(t_ray ray);


/**
 * \fn 		t_light CreateLight(t_vector pos0, float *intensity0);
 * \brief 	Fonction de création d'un rayon
 *
 * \param 	pos0		Vecteur lumière
 * \param 	intensity0	Intensité de la lumière RVB
 *
 * \return  Objet Source lumineuse
 */
t_light		CreateLight(t_vector pos0, float *intensity0);


/**
 * \fn 		t_light		*CopyLight(t_light *light0);
 * \brief 	Fonction de copie d'une source de lumière
 *
 * \param 	light0	une adresse de source de lumière
 *
 * \return 	un pointeur sur la nouvelle source copiée
 */
t_light		*CopyLight(t_light *light0);


/**
 * \fn 		void		PrintLight(t_light *light);
 * \brief 	Fonction d'affichage de source de lumière
 *
 * \param 	light	une source de lumière
 *
 * \return 	néant
 */
void		PrintLight(t_light *light);


/**
 * \fn 		t_material CreateMaterial	(int type0, const float *color0, float reflection0,
 *										float specular0, float power0, float perlin0, float bump0);
 * \brief 	Fonction de création d'un objet matériau
 *
 * \param 	type0		Type de matériau
 * \param 	color0		Couleur du matériau
 * \param	reflection0	Coefficient de réflection du matériau
 * \param	specular0	Coefficient de réflection spéculaire
 * \param	power0		Coefficient de rugosité du matériau (de 0 à 1024)
 * \param	perlin0		Coefficient du bruit de Perlin
 * \param	bump0		Coefficient de bump mapping
 *
 * \return 	Objet Matériau
 */
t_material CreateMaterial	(int type0, const float *color0, float reflection0,
							float specular0, float power0, float perlin0, float bump0);


/**
 * \fn 		t_material	*CopyMaterial(t_material *mat0);
 * \brief 	Fonction de copie d'un matériel
 *
 * \param 	mat0	une adresse d'un matériel
 *
 * \return 	un pointeur sur le nouveau matériel copié
 */
t_material	*CopyMaterial(t_material *mat0);


/**
 * \fn 		void        PrintMaterial(t_material *mat);
 * \brief 	Fonction d'affichage de matériel
 *
 * \param 	mat	un matériel
 *
 * \return 	néant
 */
void        PrintMaterial(t_material *mat);


/**
 * \fn 		t_plane CreatePlane(float dist0, t_vector normal0, t_material *mat0);
 * \brief 	Fonction de création d'un objet plan
 *
 * \param 	dist0	Distance du projeté orthogonal de l'origine du raytracer par rapport à la normale
 * \param 	normal0	Un vecteur normal au plan
 * \param	mat0	Matériau de la sphère
 *
 * \return 	Objet Plan
 */
t_plane 	CreatePlane(float dist0, t_vector normal0, t_material *mat0);


/**
 * \fn 		t_plane 	*CopyPlane(t_plane *plane0);
 * \brief 	Fonction de copie d'un matériel
 *
 * \param 	plane0	une adresse d'un plan
 *
 * \return 	un pointeur sur le nouveau plan copié
 */
t_plane 	*CopyPlane(t_plane *plane0);


/**
 * \fn 		t_sphere	CreateSphere(t_vector center0, float radius0, t_material *mat0);
 * \brief 	Fonction de création d'un objet sphère
 *
 * \param 	center0	Centre de la sphère
 * \param 	radius0	Rayon de la sphère
 * \param	mat0	Matériau de la sphère
 *
 * \return 	Objet sphère
 */
t_sphere	CreateSphere(t_vector center0, float radius0, t_material *mat0);

/**
 * \fn 		t_sphere	*CopySphere(t_sphere *sphere0);
 * \brief 	Fonction de copie d'une sphère
 *
 * \param 	sph0	une adresse d'une sphère
 *
 * \return 	un pointeur sur la nouvelle sphère copiée
 */
t_sphere	*CopySphere(t_sphere *sph0);


/**
 * \fn 		void        PrintSphere(t_sphere sph);
 * \brief 	Fonction d'affichage de sphère
 *
 * \param 	sph	une sphère
 *
 * \return 	néant
 */
void        PrintSphere(t_sphere sph);


t_camera	CreateCamera(t_vector point0, t_vector center_view0, float dist_view0);



/**
 * \fn 		 t_scene CreateScene(int *viewport0, int n_plane0, int n_sph0, int n_light0,
 *								t_plane *list_plane0, t_sphere *list_sph0, t_light *list_light0,
 *								t_options options0, t_camera camera0);
 * \brief 	Fonction de création de la scène 3D
 *
 * \param 	viewport0		Taille du viewport, de la scène 3D
 * \param	n_plane0		Nombre de plans
 * \param 	n_sph0			Nombre de sphères
 * \param	n_light0	    Nombre de lumières
 * \param	list_plane0		Liste des plans
 * \param 	list_sph0		Liste des objets sphères
 * \param	list_light0		Liste des sources de lumière
 * \param	options0		Paramètres de raytracing pour la scène
 * \param	camera0			Point de vue de la caméra
 *
 * \return 	Scène 3D
 */
 t_scene CreateScene(int *viewport0, int n_plane0, int n_sph0, int n_light0,
					t_plane *list_plane0, t_sphere *list_sph0, t_light *list_light0,
					t_options options0, t_camera camera0);

#endif	/* OBJECTS_H_ */
