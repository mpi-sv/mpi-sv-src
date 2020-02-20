 /*
 ** KF-Ray, raytracer parallele
 **  main.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef MAIN_H_
#define MAIN_H_

/**
 * \file	main.h
 * \brief	Programme KF-Ray principal en ligne de commande.
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Fichier principal de KF-Ray. Initialise MPI.
 *
 */

#include "raytracer.h"
#include "arguments.h"

// Commenter/Décommenter pour désactiver/activer MPI
#define MY_MPI

/* TAG de communication MPI */
#define TAG_END 0
#define TAG_REQ 1
#define TAG_DATA 2


/* Prototypes */

/**
 * \fn 		int 	main(int argc, char* argv[]);
 * \brief 	Fonction principale initialise MPI.
 *
 * \param	argc	Nombre d'arguments de KF-Ray
 * \param	argv	Liste des argument de KF-Ray
 *
 * \return	0 si pas d'erreur
 */
int 	main(int argc, char* argv[]);


/**
 * \fn 		int 	CallRaytracer(t_scene scn, int opt_display, char *img_name, int n_proc, int rank, int n_lines);
 * \brief 	Appelle le raytracer sur le processus Maître ou sur les Slave si MPI est activé sur plusieurs processus
 *
 * \param	scn		Nom de la scène
 * \param	opt_display	Activation/désactivation de l'affichage de l'image
 * \param	img_name	Nom de l'image finale
 * \param	n_proc		Nombre de processus total
 * \param	rank		Rang du processus actuel
 * \param	n_lines		Nombre de lignes calculés par chaque processus
 *
 * \return	0 si pas d'erreur
 */
int 	CallRaytracer	(t_scene scn, int opt_display,
			char *img_name, int n_proc, int rank, int n_lines);



/**
 * \fn 		int	ForkRaytracer(t_scene scn, char *img_name);
 * \brief 	Fork permettant l'ouverture de l'image PPM
 *
 * \param	scn		La scène 3D
 * \param	img_name	Nom de l'image finale
 *
 * \return	0 si pas d'erreur
 */
int 	ForkRaytracer(t_scene scn, char *img_name);


#endif /* MAIN_H_ */
