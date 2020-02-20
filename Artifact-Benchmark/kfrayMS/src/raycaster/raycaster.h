 /*
 ** KF-Ray, raytracer parallele
 **  raycaster.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef RAYCASTER_H_
#define RAYCASTER_H_

/**
 * \file	raycaster.h
 * \brief	Cheminement d'un rayon
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Simule l'envoie d'un rayon de l'observateur ou d'une intersection vers une source de lumière
 *
 */

#include "../objects/scene.h"

#define PLANE 1
#define SPHERE 2


/**
 * \struct	s_castray
 * \brief	Structure commode pour le castray
 *
 * Comprend des informations utiles pour les fonctions Castray et Reflection
 *
 */
typedef struct	s_castray
{

	t_ray	*ray_cast;		/*!< Rayon de lumière lancé */
	int	*level;			/*!< Profondeur/niveau de réflexion et/ou réfraction */
	float	*coeff_reflection;	/*!< Coefficient de réflexion courant */
	float	*coeff_refraction;	/*!< Coefficient de réfraction courant */

}		t_castray;


/* Prototypes */

/**
 * \fn 		t_castray		CreateCastray	(t_ray *ray_cast0, int *level0, float *coeff_reflection0,
 *							float *coeff_refraction0);
 * \brief 	Créé un rayon
 *
 * \param	ray_cast0		Le rayon lancé simulé
 * \param	level0			Le niveau/profondeur de réflexion
 * \param	coeff_reflection0 	Le coefficient de réflexion
 * \param	coeff_refraction0	Le coefficient de réfraction
 *
 * \return	Objet Rayon de lumière
 */
t_castray	CreateCastray	(t_ray *ray_cast0, int *level0,
				float *coeff_reflection0, float *coeff_refraction0);


/**
 * \fn 		float		*CastRay(t_scene scn, t_castray struct_cast, float *RGB);
 * \brief 	Envoi d'un rayon
 *
 * \param	scn 			Scène d'entrée 3D
 * \param	struct_cast		Structure de données pratiques pour le castray
 * \param	RGB			Le tableau de couleur RVB
 *
 * \return	Un tableau de couleur RGB
 */
float		*CastRay(t_scene scn, t_castray struct_cast, float *RGB);


/**
 * \fn 		void		LookObjectType	(t_scene scn, t_castray struct_cast,
 *						float *RGB, int object_type,
 *						int object_current, float t);
 * \brief 	Regarde le type d'objet rencontré
 *
 * \param	scn 			Scène d'entrée 3D
 * \param	struct_cast		Structure de données pratiques pour le castray
 * \param	RGB			Le tableau de couleur RVB
 * \param	object_type		Le type d'objet intersecté
 * \param	object_current		Le rang de l'objet intersecté
 * \param	t			L'abscisse du rayon lancé
 *
 * \return	Un tableau de couleur RGB
 */
void		LookObjectType	(t_scene scn, t_castray struct_cast,
				float *RGB, int object_type,
				int object_current, float t);


/**
 * \fn 		int		LookIntersect(t_scene scn, t_castray struct_cast,
 *					int *object_current, float *t);
 * \brief 	Regarde l'intersection la plus proche
 *
 * \param	scn 			Scène d'entrée 3D
 * \param	struct_cast		Structure de données pratiques pour le castray
 * \param	object_current		Le rang de l'objet intersecté
 * \param	t			L'abscisse du rayon lancé
 *
 * \return	le type d'objet dont l'intersection est la plus proche
 */
int		LookIntersect	(t_scene scn, t_castray struct_cast,
				int *object_current, float *t);


#endif	/* RAYCASTER_H_ */
