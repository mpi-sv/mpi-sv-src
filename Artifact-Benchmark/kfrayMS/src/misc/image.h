 /*
 ** KF-Ray, raytracer parallele
 **  image.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef IMAGE_H_
#define IMAGE_H_

/**
 * \file	image.h
 * \brief	Gestion des fichiers images
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Fonctions pour gérer les fichiers images PPM
 *
 */


/**
 * \fn 		void SetPixel(unsigned char* img, int w, int x, int y, char r, char g, char b);
 * \brief 	Etablit la couleur du pixel dans l'image
 *
 * \param 	img		Image voulue
 * \param 	w		Largeur (width) de l'image
 * \param 	x		Coordonnées x
 * \param 	y		Coordonnées y
 * \param 	r		Coefficient Rouge
 * \param 	g		Coefficient Vert
 * \param 	b		Coefficient Bleu
 *
 * \return 	Néant.
 */
void	SetPixel(unsigned char* img, int w, int x, int y, char r, char g, char b);


/**
 * \fn 		void SaveImage(unsigned char *image, char *img_name, int w, int h);
 * \brief 	Sauvegarde l'image au format PPM
 *
 * \param 	image		Image voulue
 * \param 	img_name	Nom de l'image PPM
 * \param 	w		Largeur (width) de l'image
 * \param 	h		Hauteur (height) de l'image
 *
 * \return 	Néant.
 */
void	SaveImage(unsigned char *image, char *img_name, int w, int h);


/**
 * \fn 		int	ShowImage(char *img_name);
 * \brief 	Affiche l'image à la fin du rendu avec execlp()
 *
 * \param 	img_name	Nom de l'image PPM
 *
 * \return 	0 si pas d'erreur
 */
int	ShowImage(char *img_name);


/**
 * \fn 		int	ShowImage(char *img_name);
 * \brief 	Affiche l'image à la fin du rendu avec system()
 *
 * \param 	img_name	Nom de l'image PPM
 *
 * \return 	0 si pas d'erreur
 */
int	ShowImageSys(char *img_name);


/**
 * \fn 		void SaveImage2(unsigned char *image, char *img_name, int w, int h);
 * \brief 	Sauvegarde l'image au format PPM
 *
 * \deprecated	Cette fonction utilise un chemin qui n'est pas compréhensible sur tous les systèmes UNIX.
 *		Laissé à titre d'exemple pour le getcwd et fonctionne à Polytech'Paris. Utilisez SaveImage à la place.
 *
 * \param 	image		Image voulue
 * \param 	img_name	Nom de l'image PPM
 * \param 	w		Largeur (width) de l'image
 * \param 	h		Hauteur (height) de l'image
 *
 * \return 	Néant.
 */
void SaveImage2(unsigned char *image, char *img_name, int w, int h);


#endif	/* IMAGE_H_ */
