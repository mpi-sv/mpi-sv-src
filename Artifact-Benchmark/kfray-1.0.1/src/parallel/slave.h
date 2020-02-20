 /*
 ** KF-Ray, raytracer parallele
 **  slave.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef SLAVE_H_
#define SLAVE_H_

/**
 * \file	slave.h
 * \brief	Programme KF-Ray principal en ligne de commande.
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Gestion des processus SLAVES de KF-Ray pour la parallélisation
 *
 */

#define MASTER_NODE 0


/**
 * \fn 		void 		Slave(t_scene scn, int h, int w, int n_lines, int rank);
 * \brief 	Fonction d'un processus esclave.
 *		Reçoit un travail à faire, le fait et le renvoie.
 *
 * \param	scn		La scène 3D
 * \param	h		Hauteur (height) de l'image du processus esclave
 * \param	w		Longueur (Width) de l'image du processus esclave
 * \param	n_lines		Nombres de lignes calculés par chaque processus esclave
 * \param	rank		Rang du processus esclave
 *
 * \return	néant
 */
void 		Slave(t_scene scn, int h, int w, int n_lines, int rank);


/**
 * \fn 		void RaytracerSlave(t_scene scn, int x, int y, int i, int j, unsigned char *img_slave);
 * \brief 	Raytracing d'un processus esclave
 *
 * \param 	scn		La scène 3D
 * \param 	x		Coordonnées x par rapport à la scène
 * \param 	y		Coordonnées y par rapport à la scène
 * \param 	i		Coordonnées x par rapport à l'image esclave
 * \param 	j		Coordonnées y par rapport à l'image esclave
 * \param 	img_slave	Image PPM esclave
 *
 * \return 	Néant.
 */
void	RaytracerSlave(t_scene scn, int x, int y, int i, int j, unsigned char *img_slave);

#endif	/* SLAVE_H */
