#ifndef EFFECTS_H_
#define EFFECTS_H_

/**
 * \file	effects.h
 * \brief	Effets supplémentaires (BumpMapping)
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Gère divers effets dont le bump mapping, le brouillard...
 *
 */

 #include "../objects/scene.h"

/**
 * \fn 		BumpMapping(t_vector vect_normal, t_vector point_intersect, float bump);
 * \brief 	Fonction de calcul pour l'effet bump mapping
 * \bug		Il y a apparition de traces peu précise sur les bords lorsque le coefficient est élevé.
 *		Résolvable sûrement en retravaillant la fonction.
 *
 * \param 	vect_normal		Vecteur normal à modifier
 * \param 	point_intersect		Point d'intersection entre le rayon et l'objet
 * \param 	bump			Coefficient de bump mapping du matériau
 *
 * \return 	Vecteur normal modifié
 */
t_vector	BumpMapping(t_vector vect_normal, t_vector point_intersect, float bump);


/**
 * \fn 		float		*Mist(t_scene scn, t_vector point_intersect, float *RGB);
 * \brief 	Fonction de calcul d'effet brouillard exponentiel
 * \bug		Pas encore bien implémenté.
 *
 * \param	scn			Scène 3D
 * \param 	point_intersect		Point d'intersection entre le rayon et l'objet
 * \param 	RGB			Tableau de couleur RGB
 *
 * \return 	Tableau de couleur RGB modifié avec effet brouillard
 */
float		*Mist(t_scene scn, t_vector point_intersect, float *RGB);

 #endif
