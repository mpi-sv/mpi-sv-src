 /*
 ** KF-Ray, raytracer parallele
 **  sphere.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef SPHERE_H_
#define SPHERE_H_

/**
 * \file	sphere.h
 * \brief	Gestion de l'objet sphère
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Déclare la structure de la sphère et les prototypes de fonctions liées à cet objet.
 *
 */

#include "vector.h"
#include "../misc/misc.h"

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

	t_vector 	center;		/*!< Centre de la sphère */
	float 		radius;		/*!< Rayon de la sphère */
	t_material	*material;	/*!< Matériau de la sphère */

}		t_sphere;


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
void		PrintSphere(t_sphere sph);

#endif	/* SPHERE_H_ */
