 /*
 ** KF-Ray, raytracer parallele
 **  light.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef LIGHT_H_
#define LIGHT_H_

/**
 * \file	light.h
 * \brief	Gestion de l'objet source de lumière
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Déclare la structure des sources de lumières et les prototypes de fonctions liées à cet objet.
 *
 */

#include "vector.h"
#include "../misc/misc.h"

/**
 * \struct s_light
 * \brief Structure de source de lumière
 *
 * Définit une source lumineuse par :
 * - le point d'origine de sa source lumineuse,
 * - son intensité lumineuse.
 */
typedef struct	s_light
{

	t_vector	position;		/*!< Point d'origine de la source lumineuse */
	float		*intensity;		/*!< Intensité lumineuse */

}		t_light;


/**
 * \fn 		t_light CreateLight(t_vector pos0, float *intensity0);
 * \brief 	Fonction de création d'un rayon
 *
 * \param 	pos0		Vecteur lumière
 * \param 	intensity0	Intensité de la lumière RVB
 *
 * \return	Objet Source lumineuse
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


#endif	/* LIGHT_H_ */
