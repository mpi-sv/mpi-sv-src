 /*
 ** KF-Ray, raytracer parallele
 **  raytracer.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef RAYTRACER_H_
#define RAYTRACER_H_

/**
 * \file	raytracer.h
 * \brief	Technique de raytracing
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Balaye l'écran et affiche pixel par pixel l'image généré par la technique de raytracing
 *
 */

#include "objects/scene.h"


/* Prototypes */

/**
 * \fn 		void Raytracer(t_scene scn, char *img_name);
 * \brief 	Fonction principale du raytracing
 *
 * \param 	scn		La scène 3D
 * \param 	img_name	Nom de l'image PPM finale
 *
 * \return 	Néant.
 */
void	Raytracer(t_scene scn, char *img_name);


void	*ComputePixel(t_scene scn, unsigned char *image, int x, int y);


/**
 * \fn 		t_ray	CameraRay(t_scene scn, int x, int y);
 * \brief 	Détermine le cheminement initial du rayon d'un pixel de l'écran
 *
 * \param 	scn		La scène 3D
 * \param 	x		Coordonnées x par rapport à la scène
 * \param 	y		Coordonnées y par rapport à la scène
 *
 * \return 	un rayon pour le lancer de rayon
 */
t_ray	CameraRay(t_scene scn, int x, int y);


#endif	/*	RAYRACER_H_	*/
