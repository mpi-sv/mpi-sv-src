 /*
 ** KF-Ray, raytracer parallele
 **  printinfo.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef PRINTINFO_H_
#define PRINTINFO_H_

/**
 * \file	printinfo.h
 * \brief	Fonctions d'affichage KF-Ray
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Affiche des infos sur la console.
 *
 */


static const char splash[] = "\
\n **   ** ********     *******                   \
\n/**  ** /**/////     /**////**           **   **\
\n/** **  /**          /**   /**   ****** //** ** \
\n/****   /************/*******   //////** //***  \
\n/**/**  /**///////// /**///**    *******  /**   \
\n/**//** /**          /**  //**  **////**  **    \
\n/** //**/**          /**   //**//**********     \
\n//   // //           //     //  //////////      \
\n\nKF-Ray - Raytracer parallele ecrit en C\
\n	 par Karin Ait-Si-Amer et Florian Dang\
\n		(c) Copyright 2009 reportez-vous a COPYING.\n\n\
";/*!< Texte de présentation */


static const char txt_active[] = "ON";

static const char txt_unactive[] = "OFF";

/**
 * \fn 		void 		InfoRaytracer(t_scene scn, char *img_name);
 * \brief 	Affiche des informations lors d'un raytracing uni-processeur
 *
 * \param 	scn		La scène 3D
 * \param 	img_name	Nom de l'image finale
 *
 * \return 	néant
 */
void 	InfoRaytracer(t_scene scn, char *img_name);


/**
 * \fn 		void	InfoMaster(char *img_name, int w, int h, int n_proc, int n_lines);
 * \brief 	Affiche des informations sur le processus maître
 *
 * \param 	img_name	Nom de l'image finale
 * \param	w		Largeur (Width) de l'image
 * \param	h		Hauteur (Height) de l'image
 * \param	n_proc		Le nombre de processus total
 * \param	n_lines		Nombre de lignes calculés par chaque processus
 *
 * \return 	néant
 */
void	InfoMaster(char *img_name, int w, int h, int n_proc, int n_lines);


#endif	/* PRINTINFO_H_ */
