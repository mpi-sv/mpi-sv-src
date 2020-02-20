 /*
 ** KF-Ray, raytracer parallele
 **  callbacks.h
 **
 ** Copyright 2009 Karin Ait-Si-Amer & Florian Dang
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous a COPYING pour les infos sur le Copyright
 */

#ifndef CALLBACKS_H_
#define CALLBACKS_H_

/**
 * \file	callbacks.h
 * \brief	Fonctions callbacks du Frontend GUI en GTK
 * \author	Karin Ait-Si-Amer et Florian Dang
 * \version	1.0.1
 * \date	Fevrier-Mai 2009
 *
 * Fonctions d'appel évenèment de l'interface graphique de KF Ray en GTK.
 *
 */

#include <gtk/gtk.h>


static const gchar default_folder[] = "../scenes";

static const gchar error_backend[] = "Erreur : Impossible de localiser KF-Ray backend.\n\
Vérifiez que kfray est dans le même répertoire que kfray_gui.\n\
Vérifiez également que vous avez bien activé la parallélisation\n\
Référez-vous au manuel d'utilisation si besoin\n";

/**
 * \fn 		on_cmd_start_clicked(GtkButton *button, gpointer user_data);
 * \brief 	Fonction évènement lors du clic bouton start
 *
 * \param	button		Le bouton de commande cliqué
 * \param	user_data	Données complémentaires GTK
 *
 * \return	néant
 */
void
on_cmd_start_clicked			(GtkButton	*button,
					gpointer	user_data);

/**
 * \fn 		on_cmd_open_clicked(GtkButton *button, gpointer user_data);
 * \brief 	Fonction évènement lors du clic bouton open
 *
 * \param	button		Le bouton de commande cliqué
 * \param	user_data	Données complémentaires GTK
 *
 * \return	néant
 */
void
on_cmd_open_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

/**
 * \fn 		on_copying_activate     (GtkMenuItem     *menuitem,
 *                                       gpointer         user_data);
 * \brief 	Fonction évènement du menu About->COPYING. Affiche la licence GNU GPL v3.
 *
 * \param	menuitem	Le menu COPYING
 * \param	user_data	Données complémentaires GTK
 *
 * \return	néant
 */
void
on_copying_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

/**
 * \fn 		on_copying_activate     (GtkMenuItem     *menuitem,
 *                                       gpointer         user_data);
 * \brief 	Fonction évènement du menu About->ChangeLog. Affiche les logs de versions.
 *
 * \param	menuitem	Le menu ChangeLog
 * \param	user_data	Données complémentaires GTK
 *
 * \return	néant
 */
void
on_log_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

/**
 * \fn 		on_clean_img_activate     (GtkMenuItem     *menuitem,
 *                                       gpointer         user_data);
 * \brief 	Fonction évènement du menu File->Nettoyer. Supprime les fichiers images PPM dans scenes/images/
 *
 * \param	menuitem	Le menu Nettoyer
 * \param	user_data	Données complémentaires GTK
 *
 * \return	néant
 */
void
on_clean_img_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


/**
 * \fn 		on_copying_activate     (GtkMenuItem     *menuitem,
 *                                       gpointer         user_data);
 * \brief 	Fonction évènement du menu Fichier->Quitter. Quitte le GUI
 *
 * \param	menuitem	Le menu Quitter
 * \param	user_data	Données complémentaires GTK
 *
 * \return	néant
 */
void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


/* Non utilisé mais mis pour Glade ça sert à rien donc de les documenter... */


void
on_simple_light_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_lambert_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_phong_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_blinnphong_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_perlin_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_antialiasing_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

#endif /* CALLBACKS_H_ */


