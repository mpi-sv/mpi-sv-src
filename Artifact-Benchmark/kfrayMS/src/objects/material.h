 /*
 ** KF-Ray, raytracer parallele
 **  material.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef MATERIAL_H_
#define MATERIAL_H_

/**
 * \file	material.h
 * \brief	Gestion de l'objet matériel
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Déclare la structure du matériel et les prototypes de fonctions liées à cet objet.
 *
 */

#include "vector.h"
#include "../misc/misc.h"

/**
 * \struct	s_material
 * \brief	Structure de matériau
 *
 * Définit un matériau par :
 *
 * son numéro d'identification,
 * son type (normal ou avec texture procédurale en marbre, bois, turbulence...), <br>
 * sa couleur en format RVB (chaque flottant est compris entre 0 et 255.0), <br>
 * son coefficient de réflexion (compris entre 0.0 et 1.0 exclu), <br>
 * son coefficient de réflexion spéculaire (compris entre 0.0 et 2.0), <br>
 * son coefficient de rugosité (compris entre 0.0 et 1000.0), <br>
 * son coefficient de bruit de Perlin (compris entre 0.0 et 2.0 : 1.0 par défaut), <br>
 * son coefficient de bump mapping (compris entre 0.0 et 5.0 : 0.0 par défaut), <br>
 * sa densité (compris entre 1.0 et 2.0 : 1.0 par défaut), <br>
 * son coefficient réfraction (compris entre 0.0 et 1.0 : 0.0 par défaut).
 *
 */
typedef struct	s_material
{

	int		id;		/*!< Numéro du matériau pour le parser */
	int 		type;		/*!< Type du matériau
					(NORMAL, MARBRE, BOIS, TURBULENCE, CHECKER) */
	float		*color;		/*!< Couleur RVB */
	float 		reflection;	/*!< Coefficient de réflexion (compris entre 0.0 et 1.0) */
	float		specular;	/*!< Coefficient de réflexion spéculaire (compris entre 0.0 et 1.0) */
	float		power;		/*!< Coefficient de rugosité (compris entre 0.0 et 1000.0) */
	float		perlin;		/*!< Coefficient de bruit de Perlin (compris entre 0.0 et 2.0) */
	float		bump;		/*!< Coefficient de Bump Mapping (compris entre 0.0 et 0.5) */
	float		density;	/*!< Indice de réfraction (compris entre 1.0 et 2.0) */
	float		refraction;	/*!< Coefficient de réfraction (compris entre 0.0 et 1.0) */

}		t_material;



/**
 * \fn 		t_material CreateMaterial	(int type0, float *color0, float reflection0,
 *						float specular0, float power0, float perlin0,
 *						float bump0, float density0, float refraction0);
 * \brief 	Fonction de création d'un objet matériau
 *
 * \param 	type0		Type de matériau
 * \param 	color0		Couleur du matériau
 * \param	reflection0	Coefficient de réflection du matériau
 * \param	specular0	Coefficient de réflection spéculaire
 * \param	power0		Coefficient de rugosité du matériau (de 0 à 1024)
 * \param	perlin0		Coefficient du bruit de Perlin
 * \param	bump0		Coefficient de bump mapping
 * \param	density0	L'indice de réfraction du matériau
 * \param	refraction0	Le coefficient de réfraction du matériau
 *
 * \return 	Objet Matériau
 */
t_material CreateMaterial	(int type0, float *color0, float reflection0,
				float specular0, float power0, float perlin0,
				float bump0, float density0, float refraction0);


/**
 * \fn 		t_material	*CopyMaterial(t_material *mat0);
 * \brief 	Fonction de copie d'un matériel
 *
 * \param 	mat0	une adresse d'un matériel
 *
 * \return 	un pointeur sur le nouveau matériel copié
 */
t_material	*CopyMaterial(t_material *mat0);


/**
 * \fn 		void        PrintMaterial(t_material *mat);
 * \brief 	Fonction d'affichage de matériel
 *
 * \param 	mat	un matériel
 *
 * \return 	néant
 */
void        	PrintMaterial(t_material *mat);

#endif	/* MATERIAL_H_ */
