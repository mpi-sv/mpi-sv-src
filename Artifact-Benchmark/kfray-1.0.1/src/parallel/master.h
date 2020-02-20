 /*
 ** KF-Ray, raytracer parallele
 **  master.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef MASTER_H_
#define MASTER_H_

/**
 * \file	master.h
 * \brief	Programme KF-Ray principal en ligne de commande.
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Gestion du processus Master de KF-Ray pour la parallélisation
 *
 */


/**
 * \fn 		void 		Master(int n_proc, int h, int w, int n_lines, char *img_name, int opt_display);
 * \brief 	Fonction du processus maître du programme (par défaut 0 possibilité de changer si besoin mailez nous)
 *		S'occupe des communications pour la distributions des travaux.
 *		Est capable de travailler et de donner l'image s'il est seul.
 *
 * \param	n_proc		Nombre total de processus
 * \param	h		Hauteur (height) de l'image finale
 * \param	w		Longueur (Width) de l'image finale
 * \param	n_lines		Nombre de lignes calculés par chaque processus esclave
 * \param	img_name	Nom de l'image finale
 * \param	opt_display	Activation/désactivation de l'affichage de l'image
 *
 * \return	un entier
 */
void		Master	(int n_proc, int h, int w, int n_lines,
			char *img_name, int opt_display);


/**
 * \fn 		void		FirstWork(int n_proc, int n_stepmax, int *step);
 * \brief 	Envoie une première salve de travail aux esclaves
 *
 * \param	n_proc		Nombre total de processus
 * \param	n_stepmax	Nombre de processus utiles
 * \param	step		Pas de travail
 *
 * \return	néant
 */
void		FirstWork(int n_proc, int n_stepmax, int *step);


#endif /* MASTER_H_ */
