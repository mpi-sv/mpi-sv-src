 /*
 ** KF-Ray, raytracer parallele
 **  sphere.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef VECTOR_H_
#define VECTOR_H_

/**
 * \file	vector.h
 * \brief	Manipulation des vecteurs (et des points).
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Définit les structures des objets vecteurs (et points).
 * Déclare les prototypes de fonctions de calculs sur les vecteurs.
 *
 */


/*	--------------------
		Structures
	-------------------- */


/**
 * \struct	s_vector
 * \brief	Structure d'un vecteur
 *
 * Définit l'objet vecteur (et les points) par :
 * - ses coordonnées dans l'espace R^3
 */

typedef struct 	s_vector
{

	float		x;	/*!< Coordonnée x */	float 		y;	/*!< Coordonnée y */
	float 		z;	/*!< Coordonnée z */

} t_vector;


/*	--------------------
		Prototypes
	-------------------- */

/**
 * \fn 		t_vector CreateVector(float x0, float y0, float z0);
 * \brief 	Fonction de création d'un vecteur
 *
 * \param 	x0 Coordonnéés en x du vecteur
 * \param 	y0 Coordonnéés en y du vecteur
 * \param 	z0 Coordonnéés en z du vecteur
 *
 * \return 	Vecteur demandé
 */
t_vector	CreateVector(float x0, float y0, float z0);

/**
 * \fn 		t_vector	*CopyVector(t_vector *vect0);
 * \brief 	Fonction de copie d'un vecteur
 *
 * \param 	vect0	une adresse de vecteur
 *
 * \return 	un pointeur sur le nouveau vecteur copié
 */
t_vector	*CopyVector(t_vector *vect0);


/**
 * \fn 		void		PrintVect(t_vector v);
 * \brief 	Affichage d'un vecteur
 *
 * \param 	v Vecteur
 * \return	Affiche le vecteur demandé.
 */
void		PrintVect(t_vector v);


/**
 * \fn 		float Module(t_vector v);
 * \brief 	Calcule le module d'un vecteur
 *
 * \param 	v Vecteur
 * \return 	Module du vecteur.
 */
float		Module(t_vector v);

/**
 * \fn 		t_vector	Normalize(t_vector v);
 * \brief 	Normalise un vecteur
 *
 * \param 	v Vecteur
 * \return	Vecteur normalisé.
 */
t_vector	Normalize(t_vector v);

/**
 * \fn 		t_vector	Addition(t_vector u, t_vector v);
 * \brief 	Addition entre deux vecteurs u+v
 *
 * \param 	u Vecteur
 * \param	v Vecteur
 * \return 	Retourne un vecteur qui est l'addition des deux.
 */
t_vector	Addition(t_vector u, t_vector v);

/**
 * \fn 		t_vector	Substraction(t_vector v, t_vector u);
 * \brief 	Soustraction entre deux vecteurs v-u
 *
 * \param 	v Vecteur
 * \param	u Vecteur à soustraire
 * \return	Retourne un vecteur qui est la soustraction du vecteur v par u.
 */
t_vector	Substraction(t_vector v, t_vector u);

/**
 * \fn 		float		DotProd(t_vector u, t_vector v);
 * \brief 	Produit scalaire
 *
 * \param 	u Vecteur
 * \param	v Vecteur
 * \return	Retourne le produit scalaire de deux vecteurs.
 */
float		DotProd(t_vector u, t_vector v);

/**
 * \fn 		t_vector	CrossProd(t_vector u, t_vector v);
 * \brief 	Produit vectoriel
 *
 * \param 	u Vecteur
 * \param	v Vecteur
 * \return	Retourne le produit vectoriel de deux vecteurs.
 */
t_vector	CrossProd(t_vector u, t_vector v);

/**
 * \fn 		t_vector	Prod(float lambda,t_vector v);
 * \brief 	Produit externe
 *
 * \param 	lambda Scalaire
 * \param	v Vecteur
 * \return	Retourne la multiplication d'un vecteur par un scalaire.
 */
t_vector	Prod(float lambda,t_vector v);

/**
 * \fn 		Angle(t_vector u, t_vector v);
 * \brief 	Angle entre deux vecteurs
 *
 * \param 	u Vecteur
 * \param	v Vecteur
 * \return	Retourne l'angle entre deux vecteurs.
 */
float		Angle(t_vector u, t_vector v);


#endif	/* VECTOR_H_ */
