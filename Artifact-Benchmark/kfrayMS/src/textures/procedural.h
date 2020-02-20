 /*
 ** KF-Ray, raytracer parallele
 **  procedural.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef PROCEDURAL_H_
#define PROCEDURAL_H_

/**
 * \file	procedural.h
 * \brief	Fonctions de générations de textures procédurales
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Modèles de textures procédurales avec utilisation du bruit de Perlin.
 * - Effet Turbulence
 * - Effet Marbré
 * - Effet Bois
 * - Effet Damier
 *
 * Checker (damier) procédural
 */

#include "../models/brdf.h"
#include "../objects/scene.h"

#define MAX_LEVEL_PERLIN 10.0

/**
 * \fn 		float	*Turbulence(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert);
 * \brief	Modifie les couleurs pour donner une texture/effet de turbulence.
 *
 * \param	struct_brdf	La structure de données commode pour les modèles BRDF
 * \param 	light_current	La source lumineuse courante
 * \param 	RGB		Le tableau de couleur RGB courant
 * \param 	coeff_lambert	Le coefficient de Lambert associé au pixel
 *
 * \return 	Tableau de couleur RVB modifié pour la texture turbulence
 */
float		*Turbulence	(t_brdf struct_brdf, t_light light_current,
				float *RGB, float coeff_lambert);


/**
 * \fn 		float *Marbel(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert);
 * \brief	Modifie les couleurs pour donner une texture/effet de marbre.
 *
 * \param	struct_brdf	La structure de données commode pour les modèles BRDF
 * \param 	light_current	La source lumineuse courante
 * \param 	RGB		Le tableau de couleur RGB courant
 * \param 	coeff_lambert	Le coefficient de Lambert associé au pixel
 *
 * \return 	Tableau de couleur RVB modifié pour la texture marbrée
 */
float		*Marbel	(t_brdf struct_brdf, t_light light_current,
			float *RGB, float coeff_lambert);


/**
 * \fn 		float *Wood(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert);
 * \brief	Modifie les couleurs pour donner une texture/effet de bois.
 *
 * \param	struct_brdf	La structure de données commode pour les modèles BRDF
 * \param 	light_current	La source lumineuse courante
 * \param 	RGB		Le tableau de couleur RGB courant
 * \param 	coeff_lambert	Le coefficient de Lambert associé au pixel
 *
 * \return 	Tableau de couleur RVB modifié pour la texture boisée
 */
float		*Wood	(t_brdf struct_brdf, t_light light_current,
			float *RGB, float coeff_lambert);


/**
 * \fn 		float *Checker(t_brdf struct_brdf, t_light light_current, float *RGB, float coeff_lambert);
 * \brief	Modifie les couleurs du matériel pour donner une texture de damier.
 *
 * \param	struct_brdf	La structure de données commode pour les modèles BRDF
 * \param 	light_current	La source lumineuse courante
 * \param 	RGB		Le tableau de couleur RGB courant
 * \param 	coeff_lambert	Le coefficient de Lambert associé au pixel
 *
 * \return 	Tableau de couleur RVB modifié pour la texture damier
 */
float		*Checker(t_brdf struct_brdf, t_light light_current,
			float *RGB, float coeff_lambert);


/**
 * \fn 		float		*CheckerRGB (t_brdf struct_brdf, float *coord, float length);
 * \brief	Calcule un RVB temporaire pour donner la texture damier
 *
 * \param	struct_brdf	La structure de données commode pour les modèles BRDF
 * \param 	coord		Coordonnées du point d'intersection
 * \param 	length		Longueur d'un côté d'une case du damier
 *
 * \return 	Tableau de couleur RVB temporaire modifié pour la texture damier
 */
float		*CheckerRGB (t_brdf struct_brdf, float *coord, float length);


#endif	/* PROCEDURAL_H_ */
