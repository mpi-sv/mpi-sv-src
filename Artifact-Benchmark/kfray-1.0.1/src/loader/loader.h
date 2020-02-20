 /*
 ** KF-Ray, raytracer parallele
 **  loader.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef LOADER_H_
#define LOADER_H_

/**
 * \file	loader.h
 * \brief	Charge la scène 3D à partir du fichier de description
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Charge le fichier de description 3D
 *
 */

#include "../objects/scene.h"

/**
 * \fn 		t_scene 	Loader		(char *scn_name, int model_brdf,
 *						int model_texture, int opt_aliasing, float cam_move);
 * \brief 	Charge la scène 3D à partir d'un fichier de description ou par défaut
 *
 * \param	scn_name	Le nom de la scène
 * \param	model_brdf	Le modèle d'éclairement utilisé
 * \param	model_texture	Le modèle de texture utilisé
 * \param	opt_aliasing	Activation/désactivation de l'anti-aliasing
 * \param	cam_move	Déplacement de la caméra par rapport au point de fuite
 *
 * \return	Une scène 3D
 */
t_scene 	Loader		(char *scn_name, int model_brdf,
				int model_texture, int opt_aliasing, float cam_move);

/**
 * \fn 		t_scene 	LoadDefaultScn	(int model_brdf, int model_texture,
 *						int opt_aliasing);
 * \brief 	Charge la scène 3D par défaut
 *
 * \param	model_brdf	Le modèle d'éclairement utilisé
 * \param	model_texture	Le modèle de texture utilisé
 * \param	opt_aliasing	Activation/désactivation de l'anti-aliasing
 *
 * \return	La scène 3D par défaut
 */
t_scene 	LoadDefaultScn	(int model_brdf, int model_texture,
				int opt_aliasing);



/**
 * \fn 		t_material	*DefaultMat(void);
 * \brief 	Charge les matériaux de la scène par défaut
 *
 * \return	La liste de sphères de la scène par défaut
 */
t_material	*DefaultMat(void);



/**
 * \fn 		t_sphere	*DefaultSph	(t_material mat_jaune,
 *						t_material mat_magenta,
 *						t_material mat_cyan);
 * \brief 	Charge les  sphères de la scène par défaut
 *
 * \param	mat_jaune	Le matériau jaune de la scène par défaut
 * \param	mat_magenta	Le matériau magenta de la scène par défaut
 * \param	mat_cyan	Le matériau cyan de la scène par défaut
 *
 * \return	La liste de sphères de la scène par défaut
 */
t_sphere	*DefaultSph	(t_material mat_jaune,
				t_material mat_magenta,
				t_material mat_cyan);


/**
 * \fn 		t_sphere	*DefaultLight	(t_material mat_jaune,
 *						t_material mat_magenta,
 *						t_material mat_cyan);
 * \brief 	Charge les sources de lumières de la scène par défaut
 *
 * \param	mat_jaune	Le matériau jaune de la scène par défaut
 * \param	mat_magenta	Le matériau magenta de la scène par défaut
 * \param	mat_cyan	Le matériau cyan de la scène par défaut
 *
 * \return	La liste de sources de lumières de la scène par défaut
 */
t_light		*DefaultLight	(t_material mat_jaune,
				t_material mat_magenta,
				t_material mat_cyan);

#endif	/* LOADER_H_ */
