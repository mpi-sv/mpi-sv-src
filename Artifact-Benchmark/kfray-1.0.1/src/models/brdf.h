 /*
 ** KF-Ray, raytracer parallele
 **  brdf.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef BRDF_H_
#define BRDF_H_

/**
 * \file	brdf.h
 * \brief	Bidirectional Reflectance Distribution Functions (BRDF)
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Algorithmes de calcul de BRDF (réflectance)
 * Lambert, Phong, Blinn-Phong, Cook-Torrance...
 *
 */

#include "../objects/scene.h"

// On peut mettre DEBUG à 0 pour augmenter la rapidité des calculs
#define DEBUG_BRDF 1

// L'ombrage on peut le désactiver à 0 pour la réfraction
#define OMBRAGE 1


/**
 * \struct	s_brdf
 * \brief	Groupement de structures pour BRDF
 *
 * Contient les vecteurs et variables utiles lors d'un appel de BRDF dont :
 * - le rayon simulé,
 * - le point d'intersection entre le rayon et un objet,
 * - le vecteur normal au point d'intersection,
 * - le materiau courant,
 * - le coefficient de reflexion de l'objet.
 */
typedef struct	s_brdf
{
	t_ray		*ray_cast;		/*!< Rayon de lumière lancé */
	t_vector	point_intersect;	/*!< Point d'intersection courant rayon/objet */
	t_vector	vect_normal;		/*!< Vecteur normal à l'objet au point d'intersection */
	t_material	*material;		/*!< Matériau courant*/
	float		coeff_reflection;	/*!< Coefficient de réflexion (ou réfraction) courant */

} t_brdf;


/**
 * \struct	s_lighting
 * \brief	Groupement de structures pour les modèles d'éclairement
 *
 * Contient les vecteurs utiles lors de l'utilisation d'un modèle d'éclairement dont :
 * - la structure commode struct_brdf,
 * - le rayon de lumière courant,
 * - la source de lumière courante.
 */
typedef struct	s_lighting
{

	t_brdf		struct_brdf;
	t_ray		ray_light;
	t_light		light_current;

} t_lighting;



/* Prototypes */

/**
 * \fn 		t_brdf		CreateBrdf(t_ray *ray_cast0, t_vector point_intersect0, t_vector vect_normal0, t_material *mat0, float coeff_reflection0);
 * \brief 	Créé la structure s_brdf
 *
 * \param	ray_cast0		Le rayon lancé simulé
 * \param	point_intersect0	Le point d'intersection rayon/objet
 * \param	vect_normal0		Le vecteur normal
 * \param	mat0			Le matériau courant
 * \param	coeff_reflection0	Le coefficient de réflexion du matériau
 *
 * \return	Une struture s_brdf
 */
t_brdf		CreateBrdf	(t_ray *ray_cast0, t_vector point_intersect0,
				t_vector vect_normal0, t_material *mat0, float coeff_reflection0);


/**
 * \fn 		float		*Brdf(t_scene scn, t_brdf struct_brdf, float *t, float *RGB);
 * \brief 	Fonction générale d'éclairement. Commun pour Lambert, Blinn, Blinn-Phong...
 *
 * \param	scn			La scène 3D
 * \param	struct_brdf		La structure s_brdf
 * \param	t			Le paramètre/abscisse variable
 * \param	RGB			Un tableau de couleur RGB
 *
 * \return	Un tableau de couleur RVB
 */
float		*Brdf(t_scene scn, t_brdf struct_brdf, float *t, float *RGB);


/**
 * \fn 		bool Shadow(t_scene scn, t_ray ray_light, t_vector vect_light);
 * \brief 	Fonction de détermination d'ombrage
 *
 * \param	scn			La scène 3D
 * \param	ray_light		Le rayon de lumière lancé
 * \param	vect_light		Le vecteur source de lumière - point intersection
 *
 * \return	Un tableau de couleur RVB
 */
bool		Shadow(t_scene scn, t_ray ray_light, t_vector vect_light);


/**
 * \fn 		float		*Lambert(t_scene scn, t_brdf struct_brdf, t_ray ray_light, t_light light_current, float *RGB);
 * \brief 	Modèle de Lambert pour des surfaces mats. Eclairement ambiant et diffus.
 *
 * \param	scn			La scène 3D
 * \param	struct_brdf		La structure s_brdf
 * \param	ray_light		Le rayon de lumiere venant de la source
 * \param	light_current		La source lumineuse courante
 * \param	RGB			Le tableau de couleur RVB courant
 *
 * \return	Un tableau de couleur RVB
 */
float		*Lambert(t_scene scn, t_brdf struct_brdf,
			t_ray ray_light, t_light light_current, float *RGB);

/**
 * \fn 		float		*Phong(t_scene scn, t_brdf struct_brdf, t_ray ray_light, t_light light_current, float *RGB);
 * \brief 	Modèle de Phong pour des surfaces spéculaires. Eclairement spéculaire.
 *
 * \param	scn			La scène 3D
 * \param	struct_brdf		La structure s_brdf
 * \param	ray_light		Le rayon de lumiere venant de la source
 * \param	light_current		La source lumineuse courante
 * \param	RGB			Le tableau de couleur RVB courant
 *
 * \return	Un tableau de couleur RVB
 */
float		*Phong	(t_scene scn, t_brdf struct_brdf,
			t_ray ray_light, t_light light_current, float *RGB);

/**
 * \fn 		float		*BlinnPhong(t_scene scn, t_brdf struct_brdf, t_ray ray_light, t_light light_current, float *RGB);
 * \brief 	Modèle de Blinn-Phong. Amélioration du modèle de Phong.
 *
 * \param	scn			La scène 3D
 * \param	struct_brdf		La structure s_brdf
 * \param	ray_light		Le rayon de lumiere venant de la source
 * \param	light_current		La source lumineuse courante
 * \param	RGB			Le tableau de couleur RVB courant
 *
 * \return	Un tableau de couleur RVB
 */
float		*BlinnPhong	(t_scene scn, t_brdf struct_brdf,
				t_ray ray_light, t_light light_current, float *RGB);


#endif	/*	BRDF_H_	*/
