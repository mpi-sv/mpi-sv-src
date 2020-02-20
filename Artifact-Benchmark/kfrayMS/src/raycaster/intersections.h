 /*
 ** KF-Ray, raytracer parallele
 **  intersections.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */


#ifndef INTERSECTIONS_H_
#define INTERSECTIONS_H_

/**
 * \file	intersections.h
 * \brief	Intersections rayons/objets
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date 	18 mars 2008
 *
 * Gère les intersections entre rayons et objets.
 * Le paramètre t permet d'établir la position exacte de l'intersection.
 *
 */

#include "../objects/scene.h"

/* Prototypes */

/**
 * \fn 		bool	 	InterPlane(t_ray ray, t_plane plane, float *t);
 * \brief 	Trouve si l'intersection la plus proche est un plan ou renvoie FALSE
 *
 * \param 	ray	un rayon de lumière
 * \param 	plane	un plan
 * \param 	t	paramètre/abscisse du vecteur
 *
 * \return 	TRUE s'il y a intersection plus proche <br>
 *		FALSE sinon
 */
bool	 	InterPlane(t_ray ray, t_plane plane, float *t);

/**
 * \fn 		bool	 	InterSphere(t_ray ray, t_sphere sph, float *t);
 * \brief 	Trouve si l'intersection la plus proche est une sphère ou renvoie FALSE
 *
 * \param 	ray	un rayon de lumière
 * \param 	sph	un cercle
 * \param 	t 	paramètre/abscisse du vecteur
 *
 * \return 	TRUE s'il y a intersection plus proche <br>
 *		FALSE sinon
 */
bool	 	InterSphere(t_ray ray, t_sphere sph, float *t);

/**
 * \fn 		bool	 	InterSphere2(t_ray ray, t_sphere sph, float *t);
 * \brief 	Trouve si l'intersection la plus proche est une sphère ou renvoie FALSE
 *
 * \deprecated	Cette première méthode d'intersection de sphère est fonctionnelle mais moins précise.
 * 		Elle est laissée car plus intuitive.
 *
 * \param 	ray	un rayon de lumière
 * \param 	sph	un cercle
 * \param 	t 	paramètre/abscisse du vecteur
 *
 * \return 	TRUE s'il y a intersection plus proche <br>
 *		FALSE sinon
 */
bool	 	InterSphere2(t_ray ray, t_sphere sph, float *t);


#endif	/*	INTERSECTIONS_H_	*/
