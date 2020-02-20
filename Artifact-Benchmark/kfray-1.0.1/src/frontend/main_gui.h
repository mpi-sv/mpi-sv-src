 /*
 ** KF-Ray, raytracer parallele
 **  main_gui.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef MAIN_GUI_H_
#define MAIN_GUI_H_

/**
 * \file	main_gui.h
 * \brief	Frontend GUI en GTK
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Interface Graphique de KF Ray en GTK.
 *
 */

#include <gtk/gtk.h>

// Variable globale dû à Glade 2
// Sert uniquement pour l'affichage d'un fichier dans le textview
GtkWidget *win_main;


/* Prototypes */

/**
 * \fn 		void destroy_window (GtkWidget *widget, gpointer client_data);
 * \brief 	Quitte la fenêtre
 *
 * \param	widget		La widget fenêtre à fermer
 * \param	client_data	Données complémentaires
 *
 * \return	néant
 */
void		destroy_window(GtkWidget *widget, gpointer client_data);


/**
 * \fn 		int 	main(int argc, char* argv[]);
 * \brief 	Fonction principale initialise GTK
 *
 * \param	argc	Nombre d'arguments de KF-Ray GUI
 * \param	argv	Liste des argument de KF-Ray GUI
 *
 * \return	entier
 */
int		main(int argc, char* argv[]);


#endif /* MAIN_GUI_H_ */
