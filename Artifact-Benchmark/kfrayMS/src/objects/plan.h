 /*
 ** KF-Ray, raytracer parallele
 **  plan.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef PLAN_H_
#define PLAN_H_

/**
 * \file	plan.h
 * \brief	Gestion de l'objet plan
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Déclare la structure du plan et les prototypes de fonctions liées à cet objet.
 *
 */

#include "vector.h"
#include "../misc/misc.h"

/**
 * \struct	s_plane
 * \brief	Objet Plan
 *
 * Définit un plan par :
 * - sa distance par rapport au point d'origine
 * (distance du projeté orthogonal de l'origine par rapport à la normale du plan),
 * - un vecteur normal,
 * - son matériau
 */
typedef struct 	s_plane
{

	float		dist;		/*!< Distance du plan à l'origine
					(projeté orthogonal de l'origine par rapport à la normale du plan) */
	t_vector	normal;		/*!< Un vecteur normal du plan */
	t_material	*material;	/*!< Matériau du plan */

}		t_plane;


/**
 * \fn 		t_plane CreatePlane(float dist0, t_vector normal0, t_material *mat0);
 * \brief 	Fonction de création d'un objet plan
 *
 * \param 	dist0	Distance du projeté orthogonal de l'origine du raytracer par rapport à la normale
 * \param 	normal0	Un vecteur normal au plan
 * \param	mat0	Matériau de la sphère
 *
 * \bug		Mineur : Si l'utilisateur met une normale opposée à la source lumineuse
 *		on n'a pas de réflexion (réglage en cours).
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
 * \fn 		void    	PrintPlane(t_plane plane);
 * \brief 	Fonction d'affichage de plan
 *
 * \param 	plane un plan
 *
 * \return 	néant
 */
void    	PrintPlane(t_plane plane);

#endif	/* PLAN_H_ */
