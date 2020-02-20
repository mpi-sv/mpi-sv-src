 /*
 ** KF-Ray, raytracer parallele
 **  misc.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef MISC_H_
#define MISC_H_

/**
 * \file	misc.h
 * \brief	Diverses définitions et structures
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Définit les couleurs, certaines fonctions usuelles, et les structures d'options communes au backend et frontend.
 *
 */

#define MAX_COLOR		65535.0

#define MAX_DEPTH		10

#define MATERIAL_NORMAL		0
#define MATERIAL_TURBULENCE	1
#define MATERIAL_MARBEL		2
#define MATERIAL_WOOD		3
#define MATERIAL_CHECKER	4

#define MAX_FILE_LENGTH 100

/*!
 *  \def MAX(X,Y)
 *  Renvoie le maximum entre \a X et \a Y
 */
#ifndef MAX
#define MAX(X, Y) ( X > Y ? X : Y)
#endif

/*!
 *  \def MIN(X,Y)
 *  Renvoie le minimum entre \a X et \a Y
 */
#ifndef MIN
#define MIN(X, Y) ( X < Y ? X : Y)
#endif

#ifndef PI
#define PI 3.14159
#endif

#ifndef EPSILON
#define EPSILON 0.02f
#endif

#ifndef INFINITE
#define INFINITE 1.0e34f
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

typedef int bool;


/**
 * \struct	s_options
 * \brief	Options de simulation
 *
 * Structure les paramètres pour choisir les modèles utilisés dont :
 * - le modèle d'éclairement (simple, Lambert, Phong, Blinn-Phong),
 * - l'activation/désactivation des textures.
 */
typedef struct 	s_options
{

	int	model_brdf;
	int	model_texture;
	int	opt_aliasing;

} t_options;


/**
 * \fn 		float	*CopyfArray(float *Tab0, int length);
 * \brief 	Copie un tableau de float
 *
 * \param 	Tab0	Un tableau de nombres flottants
 * \param 	length	Taille du tableau
 *
 * \return 	Nouveau tableau identique à celui passé en paramètre
 */
float	*CopyfArray(float *Tab0, int length);

/**
 * \fn 		float	*CopyiArray(int *Tab0, int length);
 * \brief 	Copie un tableau d'entiers
 *
 * \param 	Tab0	Un tableau d'entiers à copier
 * \param 	length	Taille du tableau
 *
 * \return 	Nouveau tableau identique à celui passé en paramètre
 */
int		*CopyiArray(int *Tab0, int length);


/**
 * \fn 		void	PrintfArray(float *Tab, int length);
 * \brief 	Affiche un tableau de nombres flottants
 *
 * \param 	Tab	Un tableau de nombres flottants
 * \param 	length	Taille du tableau
 *
 * \return 	Néant
 */
void		PrintfArray(float *Tab, int length);

/**
 * \fn 		void	PrintiArray(int *Tab, int length);
 * \brief 	Affiche un tableau d'entiers
 *
 * \param 	Tab	Un tableau d'entiers
 * \param 	length	Taille du tableau
 *
 * \return 	Néant
 */
void		PrintiArray(int *Tab, int length);


/**
 * \fn 		void	PrintcArray(unsigned char *Tab, int length);
 * \brief 	Affiche un tableau de caracères non signés
 *
 * \param 	Tab	Un tableau de caracères non signés
 * \param 	length	Taille du tableau
 *
 * \return 	Néant
 */
void		PrintcArray(unsigned char *Tab, int length);

/**
 * \fn 		float	*CreateArrayColor(float red0, float green0, float blue0);
 * \brief 	Crée un tableau de couleurs RVB
 *
 * \param 	red0	La composante rouge
 * \param 	green0	La composante verte
  * \param 	blue0	La composante bleue
 *
 * \return 	Le tableau des trois composantes RVB
 */

float		*CreateArrayColor(float red0, float green0, float blue0);

float		*CreateArrayIntensity(float red0, float green0, float blue0);

/**
 * \fn 		double my_gettimeofday(void);
 * \brief 	Fonction chronométrage
 *
 * \return	Le temps chronométré
 */
double		my_gettimeofday(void);


/* Définition des couleurs pour les essais avant le parser */

static const float noir[3] = {0.0f, 0.0f, 0.0f};
static const float jaune[3] = {65535.0f, 65535.0f, 0.0f};
static const float vert[3] = {0, 65535.0f,0.0f};
static const float bleu[3] = {0.0f, 0.0f, 65535.0f};
static const float rouge[3] = {65535.0f, 0.0f, 0.0f};
static const float cyan[3] = {0.0f, 65535.0f, 65535.0f};
static const float blanc[3] = {65535.0f, 65535.0f, 65535.0f};
static const float magenta[3] = {65535.0f, 0.0f, 65535.0f};
static const float couleur1[3] = {40000.0f,65535.0f,65535.0f};
static const float couleur2[3] = {0.0f, 23000.0f, 56000.0f};
static const float couleur3[3] = {6930.0f, 12000.0f, 0.0f};

#endif	/* MISC_H_ */
