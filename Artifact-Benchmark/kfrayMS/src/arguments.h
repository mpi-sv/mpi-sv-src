 /*
 ** KF-Ray, raytracer parallele
 **  arguments.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef ARGUMENTS_H_
#define ARGUMENTS_H_

/**
 * \file	arguments.h
 * \brief	Interprète les arguments donnés au programme
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Traite les arguments passés au programme et donne les bonnes options au raytracer.
 *
 */

#define OPT_STOP	0
#define OPT_CONTINUE	1

static const char help[] = "\
Usage :\n\
kfray [-i Scene] [-o Image] [-b Modele] [-t] [-a] [-l Lignes] [-d]\n\
\n\
Options :\n\
 -i Scene	: Nom du fichier de description 3D de la scène à charger \n\
 -o Image	: Nom de sortie de l'image\n\
 -b Modele	: Modele d'éclairement à utiliser :	0 Eclairage simple\n\
							1 Lambert\n\
							2 Phong\n\
							3 Blinn-Phong\n\
 -a		: Activation de l'anti-aliasing (ne prend pas d'argument)\n\
 -t		: Désactivation des textures (ne prend pas d'argument)\n\
 -l Lignes	: Nombre de lignes calculées par chaque processus \n\
			(compris entre 1 et hauteur de l'image)\n\
 -d		: Affiche l'image avec ImageMagick à la fin (ne prend pas d'argument)\n\
\n\
Options spéciales :\n\
 -h		: Affiche cette aide\n\
 -c		: Supprime tous les fichiers ppm\n\
\n\
Exemples d'exécution :\n\
 kfray -i 4 -d\n\
 kfray -i exemple1 -o img1 -b 1 -t\n\
 mpirun -v -c 10 kfray -i example3 -o img3 -b 3 -l 12 -d\n\n\
";	/*!< Texte d'aide à afficher pour l'option -h */

/**
 * \struct	s_arguments
 * \brief	Arguments passés à KF-Ray
 *
 * Définit la structure par :
 * - le nom de la scène 3D,
 * - le nom de l'image PPM finale,
 * - le modèle d'eclairement BRDF à utiliser,
 * - l'activation/désactivation de texture,
 * - le nombre de lignes calculés par chaque processus esclave (dans la version paralléle)
 * - l'activation/désactivation du display avec ImageMagick
 * - l'activation/désactivation de l'anti-aliasing
 * - le déplacement de la caméra par rapport au point de fuite
 */
typedef struct s_arguments
{
	char 	*scn_name;		/*!< Nom de la scène 3D*/
	char 	*img_name;		/*!< Nom de l'image PPM finale */
	int  	*model_brdf;		/*!< Modèle d'eclairement BRDF à utiliser */
	int  	*model_texture;		/*!< Activation/désactivation de texture */
	int  	*n_lines;		/*!< Nombre de lignes calculés par chaque processus esclave*/
	int  	*opt_display;		/*!< Activation/désactivation du display avec ImageMagick */
	int  	*opt_aliasing;		/*!< Activation/désactivation de l'anti-aliasing */
	float	*cam_move;		/*!< Déplacement de la caméra par rapport au point de fuite */

} t_arguments;


/**
 * \fn 		t_arguments CreateArg	(char *scn_name0, char *img_name0,
 *					int *model_brdf0, int *model_texture0,
 *					int *n_lines0, int *opt_display0,
 *					int *opt_aliasing0, float *cam_move0);
 * \brief 	Créé la structure s_arguments
 *
 * \param 	scn_name0	Nom de la scène 3D
 * \param 	img_name0	Nom de l'image PPM finale
 * \param 	model_brdf0	Modèle d'éclairement BRDF
 * \param 	model_texture0	Activation (1)/Désactivation (0) des textures (activé par défaut)
 * \param 	n_lines0	Nombre de lignes calculés par chaque processus (version paralléle seulement)
 * \param 	opt_display0	Affiche ou non l'image à la fin du rendu avec le display de ImageMagick
 *				si égal à 1 (0 par défaut).
 * \param 	opt_aliasing0	Activation (1)/Désactivation (0) de l'anti-aliasing (désactivé par défaut)
 * \param 	cam_move0	Déplacement de la caméra selon l'axe z
 *
 * \return 	Une structure des arguments passés au programme pour être traité ensuite.
 */
t_arguments	CreateArg	(char *scn_name0, char *img_name0,
				int *model_brdf0, int *model_texture0,
				int *n_lines0, int *opt_display0,
				int *opt_aliasing0, float *cam_move0);


/**
 * \fn 		void		PrintArg(t_arguments arg_main);
 * \brief 	Pour débugage. Affiche une structure s_arguments
 *
 * \param 	arg_main	la structure s_arguments à afficher
 *
 * \return	néant
 */
void		PrintArg(t_arguments arg_main);



/**
 * \fn 		int		LookArguments(int argc, char **argv, t_arguments arg_main);
 * \brief 	Envoie une première salve de travail aux esclaves
 *
 * \param	argc		Nombre d'arguments de KF-Ray
 * \param	argv		Liste des argument de KF-Ray
 * \param	arg_main	Structure des arguments à manipuler
 *
 * \return	OPT_CONTINUE si on peut lancer le raytracer
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int 		LookArguments(int argc, char **argv, t_arguments arg_main);


/**
 * \fn 		int		OptScn(char *scn_name, char *img_name, char *optarg);
 * \brief 	Gère l'option -i dont l'argument est le nom du fichier. <br>
 *		La fonction rectifie le nom du fichier d'entrée si besoin
 *
 * \param 	scn_name	Nom de la scène 3D
 * \param 	img_name	Nom de l'image PPM finale
 * \param 	optarg		Argument passé par l'utilisateur
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptScn(char *scn_name, char *img_name, char *optarg);


/**
 * \fn 		int 		OptImg(char *img_name, char *optarg);
 * \brief 	Gère l'option -o dont l'argument est le nom de sortie de l'image.<br>
 *		La fonction rectifie le nom de l'image si besoin
 *
 * \param 	img_name	Nom de l'image PPM finale
 * \param 	optarg		Argument passé par l'utilisateur
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int 		OptImg(char *img_name, char *optarg);


/**
 * \fn 		int		OptHelp(void);
 * \brief 	Gère l'option -h qui ne prend pas d'argument.
 *		Affiche l'aide.
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptHelp(void);

/**
 * \fn 		int		OptClean(void);
 * \brief 	Gère l'option -c qui ne prend pas d'argument.
 *		Efface les fichiers PPM.
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptClean(void);

/**
 * \fn 		int		OptDisplay(int *opt_display);
 * \brief 	Gère l'option -d.
 *		Sa présence signifie qu'il faut afficher l'image à la fin avec ImageMagick (désactivé par défaut).
 *
 * \param 	opt_display	Affiche l'image à la fin du rendu avec le display de ImageMagick
 *				si égal à 1 (0 par défaut)
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptDisplay(int *opt_display);

/**
 * \fn 		int		OptAliasing(int *opt_aliasing);
 * \brief 	Gère l'option -a.
 *		Sa présence signifie qu'il faut activé l'anti-aliasing (désactivé par défaut).
 *
 * \param 	opt_aliasing	Activation (1)/Désactivation (0) de l'anti-aliasing (désactivé par défaut)
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptAliasing(int *opt_aliasing);


/**
 * \fn 		int		OptBrdf(int *model_brdf, char *optarg);
 * \brief 	Gère l'option -b
 *
 * \param 	model_brdf	Modèle de BRDF
 * \param 	optarg		Argument passé par l'utilisateur
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptBrdf(int *model_brdf, char *optarg);


/**
 * \fn 		int		OptTexture(int *model_texture);
 * \brief 	Gère l'option -t.
 *		Sa présence signifie la désactivation des textures.
 *
 * \param 	model_texture	Activation (1)/Désactivation (0) des textures (activé par défaut)
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptTexture(int *model_texture);


/**
 * \fn 		int		OptLines(int *n_lines, char *optarg);
 * \brief 	Gère l'option -l.
 *		Attribue le nombre de lignes à calculer par chaque processus esclave.
 *		Utilisable seulement avec mpirun.
 *
 * \param 	n_lines		Nombre de lignes calculés par chaque processus (version paralléle seulement)
 * \param 	optarg		Argument passé par l'utilisateur
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptLines(int *n_lines, char *optarg);


/**
 * \fn 		int		OptView(float *cam_move, char *optarg);
 * \brief 	Gère l'option -v.
 *		Précise la distance du point de fuite.
 *
 * \param 	cam_move	Déplacement de la caméra selon l'axe z
 * \param	optarg		Argument passé par l'utilisateur
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptView(float *cam_move, char *optarg);


/**
 * \fn 		int		OptUnknown(char *prgm_name);
 * \brief 	Gère les options inconnues
 *
 * \param 	prgm_name	Nom du programme lancé (KF-Ray ici)
 *
 * \return	OPT_CONTINUE si l'on peut commencer le rendu <br>
 *		OPT_STOP sinon (argument prioritaire ou erreur de syntaxe dans le passage des arguments)
 */
int		OptUnknown(char *prgm_name);

#endif /* ARGUMENTS_H_ */
