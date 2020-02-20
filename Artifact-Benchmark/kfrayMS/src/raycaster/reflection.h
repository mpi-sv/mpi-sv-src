 /*
 ** KF-Ray, raytracer parallele
 **  reflection.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef REFLECTION_H_
#define REFLECTION_H_

/**
 * \file	reflection.h
 * \brief	Gestion des réflexions
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Calcul la couleur d'un pixel au point d'intersection et renvoie un rayon réfléchi selon le type d'objet rencontré
 *
 */

#include "raycaster.h"

/**
 * \fn 		float		*ReflectionSph(t_scene scn, t_castray struct_cast, int sph_current, float *t, float *RGB);
 * \brief 	Modifie la couleur d'un pixel et réfléchit un rayon
 *
 * \param	scn 			Scène d'entrée 3D
 * \param	struct_cast		Structure de données pratiques pour le castray
 * \param	object_current		La sphère courante
 * \param	t 			Le paramètre d'intersection
 * \param	RGB			Le tableau de couleur RVB
 *
 * \return	Un tableau de couleur RGB
 */
float		*ReflectionSph(t_scene scn, t_castray struct_cast, int sph_current, float *t, float *RGB);

/**
 * \fn 		float		*ReflectionPlane(t_scene scn, t_castray struct_cast, int object_current, float *t, float *RGB);
 * \brief 	Modifie la couleur d'un pixel et réfléchit un rayon
 *
 * \param	scn 			Scène d'entrée 3D
 * \param	struct_cast		Structure de données pratiques pour le castray
 * \param	object_current		Le plan courant
 * \param	t 			Le paramètre d'intersection
 * \param	RGB			Le tableau de couleur RVB*
 *
 * \return	Un tableau de couleur RGB
 */
float		*ReflectionPlane(t_scene scn, t_castray struct_cast, int object_current, float *t, float *RGB);


#endif /* REFLECTION_H_ */
