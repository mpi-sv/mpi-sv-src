 /*
 ** KF-Ray, raytracer parallèle
 **  callbacks.c
 **
 ** Copyright 2009 Karin AIT-SI-AMER & Florian DANG
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous à COPYING pour les infos sur le Copyright
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#include "../arguments.h"
#include "../misc/misc.h"
#include "functions.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"


void
on_cmd_start_clicked			(GtkButton *button,
					gpointer user_data)
{
	gint		samples = 0;
	t_command	tCommand;
	gchar		*command;
	command = (gchar *) g_malloc(3 * MAX_FILE_LENGTH * sizeof (gchar));

	tCommand = look_command(button);
	command = shell_command(tCommand);

	samples = look_generate(button);

	if (samples != 0)
	{
		gint		level_sample = 0;
		generate_samples(command, samples, &level_sample);
	}
	else
	{
		strcat(command, " -d");		//On affiche à la fin
		if(!system(command))
			g_print(error_backend);
		else
			g_print("Rendu terminé...\n");
	}
	g_free(command);
}



void
on_cmd_open_clicked			(GtkButton *button,
					gpointer user_data)
{
	GtkWidget	*win_main = lookup_widget(GTK_WIDGET(button), "win_main");
	GtkWidget	*entry_open = lookup_widget(GTK_WIDGET(button), "entry_open");
	GtkWidget	*textview_scene = lookup_widget(GTK_WIDGET(button), "textview_scene");

	GtkWidget	*pFileChooser;
	GtkTextBuffer	*pTextBuffer;
	gchar		*sFile, *sBuffer;

	gchar *txt_file = (gchar *) g_malloc(MAX_FILE_LENGTH * sizeof (gchar));

	pFileChooser = gtk_file_chooser_dialog_new(
				"Ouvrir un fichier de description 3D",
				GTK_WINDOW(win_main),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL
			);

   	// Fenetre modale
	gtk_window_set_modal(GTK_WINDOW(pFileChooser), TRUE);

	// On change le répertoire courant
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pFileChooser), default_folder);

	if (gtk_dialog_run (GTK_DIALOG (pFileChooser)) == GTK_RESPONSE_ACCEPT)
	{
		sFile = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(pFileChooser));
		const gchar *sFolder = gtk_file_chooser_get_current_folder(
						GTK_FILE_CHOOSER(pFileChooser));

		strcpy(txt_file, sFile + strlen(sFolder) + 1);
		g_print("Fichier %s chargé.\nChemin complet du fichier : %s\n", txt_file, sFile);
		gtk_entry_set_text(GTK_ENTRY(entry_open), txt_file);

		sBuffer = NULL;
		g_file_get_contents(sFile, &sBuffer, NULL, NULL);
		g_free(sFile);

		pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_scene));
		replace_textview(pTextBuffer, sBuffer);
	}

	gtk_widget_destroy (pFileChooser);
}


void
on_open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *cmd = lookup_widget(GTK_WIDGET(menuitem), "cmd_open");
	gtk_button_clicked(GTK_BUTTON(cmd));
}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	const gchar *authors[] =
	{
		"Karin AIT-SI-AMER <aitsiame@polytech.upmc.fr>",
		"Florian DANG <tdang@polytech.upmc.fr>",
		NULL
	};

	GtkWidget *win_about = gtk_about_dialog_new ();
	gtk_about_dialog_set_version	(GTK_ABOUT_DIALOG (win_about), "0.3");
	gtk_about_dialog_set_name	(GTK_ABOUT_DIALOG (win_about), _("KF-Ray GUI"));
	gtk_about_dialog_set_copyright	(GTK_ABOUT_DIALOG (win_about),
					_("Copyright 2009 Karin Aït-Si-Amer & Florian Dang"));
	gtk_about_dialog_set_comments	(GTK_ABOUT_DIALOG (win_about),
					_("Un logiciel de rendu d'image 3D \n utilisant la méthode du raytracing."));
	gtk_about_dialog_set_license	(GTK_ABOUT_DIALOG (win_about),
					_("Programme sous license GNU General Public Licence (GPL) v3\n\
					Reportez-vous à COPYING pour plus d'informations"));
	gtk_about_dialog_set_website	(GTK_ABOUT_DIALOG (win_about),
					"http://kfray.free.fr\nhttp://kf-ray.googlecode.com");
	gtk_about_dialog_set_authors	(GTK_ABOUT_DIALOG (win_about), authors);
	gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (win_about), authors);	// On est partout !

  	gtk_dialog_run(GTK_DIALOG(win_about));
	gtk_widget_destroy(win_about);
}


void
on_log_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget	*textview_scene = lookup_widget(GTK_WIDGET(menuitem), "textview_scene");
	GtkTextBuffer	*pTextBuffer;
	gchar		*sBuffer;

	const gchar	sLogFile[] = "../ChangeLog";

	g_file_get_contents(sLogFile, &sBuffer, NULL, NULL);

	pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_scene));

	replace_textview(pTextBuffer, sBuffer);
}


void
on_copying_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget	*textview_scene = lookup_widget(GTK_WIDGET(menuitem), "textview_scene");
	GtkTextBuffer	*pTextBuffer;
	gchar		*sBuffer;

	const gchar	sGplFile[] = "../COPYING";

	g_file_get_contents(sGplFile, &sBuffer, NULL, NULL);

	pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_scene));

	replace_textview(pTextBuffer, sBuffer);
}



void
on_clean_img_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gchar	sCleanCommand[30];
	strcpy(sCleanCommand, "./kfray -c");

	if(system(sCleanCommand))
	{
		g_print(error_backend);
	}
	else
		g_print("Images PPM nettoyées !\n");

}


void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


/* Non utilisé mais mis pour Glade */


void
on_simple_light_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_lambert_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_phong_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_blinnphong_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_perlin_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_antialiasing_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}



