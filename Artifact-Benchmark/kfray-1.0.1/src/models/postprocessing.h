 /*
 ** KF-Ray, raytracer parallele
 **  postprocessing.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef POSTPROCESSING_H_
#define POSTPROCESSING_H_

/**
 * \file	postprocessing.h
 * \brief	Technique de raytracing
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Fonctions supplémentaires pour KF-Ray.
 *
 */


/**
 * \fn 		float	*AntiAliasing(t_scene scn, float *RGB, int x, int y);
 * \brief 	Fonction d'anti-aliasing (antialiassage) par méthode de supersampling.
 *
 * \bug		Méthode par supersamling ne fonctionne pas pour n_samples > 4. A regarder.
 *
 * \param 	scn	Scène 3D
 * \param 	RGB	Couleur RVB
 * \param 	x	Coordonnées x
 * \param 	y	Coordonnées y
 *
 * \return 	Néant.
 */
float	*AntiAliasing(t_scene scn, float *RGB, int x, int y);

/**
 * \fn 		float	*GammaCorrection(float *RGB);
 * \brief 	Correction Gamma pour restitution sRGB.
 *
 * \bug		Cette fonction n'est pas encore correctement implémentée.
 *
 * \param 	RGB	Couleur RVB
 *
 * \return 	Néant.
 */
float	*GammaCorrection(float *RGB);

#endif /* POSTPROCESSING_H */
