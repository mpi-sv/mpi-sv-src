 /*
 ** KF-Ray, raytracer parallele
 **  functions.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

/**
 * \file	functions.h
 * \brief	Frontend GUI en GTK fonctions diverses liées aux évènement GTK.
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Diverses fonctions liées aux évenèments de l'interface graphique de KF Ray en GTK.
 *
 */

#include <gtk/gtk.h>

#include "main_gui.h"

/**
 * \struct s_command
 * \brief Structure des arguments de commande du frontend au backend
 *
 */
typedef struct 	s_command
{

	gchar		*command;
	const gchar	*txt_open;
	const gchar	*txt_img;
	gboolean	bool_simple;
	gboolean	bool_lambert;
	gboolean	bool_phong;
	gboolean	bool_blinn;
	gboolean	bool_perlin;
	gboolean	bool_parallel;
	const gchar	*txt_parallel;
	gboolean	bool_aliasing;

} t_command;


/* Prototypes */

/**
 * \fn 		void replace_textview		(GtkTextBuffer *pTextBuffer, gchar *sBuffer);
 * \brief 	Réaffiche un nouveau texte (buffer) dans un widget TextView
 *
 * \param	pTextBuffer	Le buffer du widget TextView cible
 * \param	sBuffer		Le texte à afficher
 *
 * \return	néant
 */
void
replace_textview		(GtkTextBuffer *pTextBuffer, gchar *sBuffer);

/**
 * \fn 		void	 	gtk_print		(gchar *str);
 * \brief 	Affichage fenêtre d'exécution de l'interface GUI
 *
 * \param	str			Le texte à afficher
 *
 * \return	néant
 */
void
gtk_print			(gchar *str);


/**
 * \fn 		void msg_warning	(const gchar *msg_error, GtkWidget *window);
 * \brief 	Affichage d'un message de warning/erreur
 *
 * \param	msg_error	Le message d'erreur à afficher
 * \param	window		La widget fenêtre parent
 *
 * \return	néant
 */
void
msg_warning			(const gchar *msg_error, GtkWidget *window);

/**
 * \fn 		t_command  look_command			(GtkButton *button);
 * \brief 	Regarde les options passées au GUI pour le traduire en ligne de commandes pour le backend
 *
 * \param	button		Un widget Button pour récupérer les autres widgets
 *
 * \return	une structure des arguments de commande
 */
t_command
look_command			(GtkButton *button);


/**
 * \fn 		gint look_generate (GtkButton *button);
 * \brief 	Regarde les options du générateur d'images
 *
 * \param	button		Un widget Button pour récupérer les autres widgets
 *
 * \return	le nombre d'échantillons
 */
gint
look_generate			(GtkButton *button);


/**
 * \fn 		void generate_samples (gchar *command, gint samples, gint *level_sample);
 * \brief 	Génère des images pour animation
 *
 * \param	command		La commande à lancer pour un échantillon donné
 * \param	samples		Le nombre d'échantillons
 * \param	level_sample	Le rang de l'échantillon produit
 *
 * \return	le nombre d'échantillons
 */
void
generate_samples		(gchar *command, gint samples, gint *level_sample);


/**
 * \fn 		gchar  *shell_command			(t_command tCommand);
 * \brief 	Regarde les options passés au GUI pour le traduire en ligne de commandes pour le backend
 *		<br>2e partie
 *
 * \param	tCommand	Une structure des arguments de commande
 *
 * \return	chaîne de caractère correspondant à la commande complète à lancer sur le backend
 */
gchar
*shell_command			(t_command tCommand);


/**
 * \fn 		void give_args	(t_command *tCommand);
 * \brief 	Regarde les options passés au GUI pour le traduire en ligne de commandes pour le backend
 *		<br>1ere partie
 *
 * \param	tCommand	Pointeur de structure des arguments de commande
 *
 * \return	néant
 */
void
give_args			(t_command *tCommand);


/**
 * \fn 		gint  model_brdf (t_command tCommand);
 * \brief	Regarde le modèle BRDF utilisé
 *
 * \param	tCommand	Une structure des arguments de commande
 *
 * \return	entier correspondant au modèle BRDF utilisé
 */
gint
model_brdf			(t_command tCommand);


/**
 * \fn 		gint  model_texture	(t_command tCommand);
 * \brief	Regarde le modèle de texture utilisé
 *
 * \param	tCommand	Une structure des arguments de commande
 *
 * \return	entier correspondant au model texture utilisé
 */
gint
model_texture			(t_command tCommand);

#endif /* FUNCTIONS_H_ */
