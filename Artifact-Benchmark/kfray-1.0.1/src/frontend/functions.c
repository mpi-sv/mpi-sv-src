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


#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../arguments.h"
#include "../misc/misc.h"
#include "functions.h"
#include "interface.h"
#include "support.h"


void
replace_textview		(GtkTextBuffer *pTextBuffer, gchar *sBuffer)
{
	GtkTextIter iStart, iEnd;

	// Suppression des données du buffer
	gtk_text_buffer_get_start_iter(pTextBuffer, &iStart);
	gtk_text_buffer_get_end_iter(pTextBuffer, &iEnd);
	gtk_text_buffer_delete(pTextBuffer, &iStart, &iEnd);

	// Affichage du fichier
	gtk_text_buffer_get_start_iter(pTextBuffer, &iStart);
	gtk_text_buffer_insert(pTextBuffer, &iStart, sBuffer, -1);
	g_free(sBuffer);

}


void
gtk_print			(gchar *str)
{
	// VARIABLE GLOBALE UTILISEE ICI !!!
	GtkWidget	*textview_print = lookup_widget(GTK_WIDGET(win_main), "textview_print");
	GtkTextBuffer	*pPrintBuffer;
	GtkTextIter iStart, iEnd;

	pPrintBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_print));

	//replace_textview(pPrintBuffer, str);  // Marche pas ??!

	// Suppression des données du buffer
	gtk_text_buffer_get_start_iter(pPrintBuffer, &iStart);
	gtk_text_buffer_get_end_iter(pPrintBuffer, &iEnd);
	gtk_text_buffer_delete(pPrintBuffer, &iStart, &iEnd);

	gtk_text_buffer_get_start_iter(pPrintBuffer, &iStart);
	gtk_text_buffer_insert(pPrintBuffer, &iStart, str, -1);
}


void
msg_warning				(const gchar *msg_error, GtkWidget *window)
{
	GtkWidget *error_box;

	error_box = gtk_message_dialog_new (GTK_WINDOW(window),
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_OK,
				"%s", msg_error);

	gtk_dialog_run(GTK_DIALOG(error_box));
	gtk_widget_destroy(error_box);
}



t_command
look_command			(GtkButton *button)
{
	t_command tCommand;

	tCommand.command = (gchar *) g_malloc(3 * MAX_FILE_LENGTH * sizeof (gchar));

	GtkWidget	*entry_open = lookup_widget(GTK_WIDGET(button), "entry_open");
	GtkWidget	*entry_img_name = lookup_widget(GTK_WIDGET(button), "entry_img_name");
	GtkWidget	*entry_parallel = lookup_widget(GTK_WIDGET(button), "entry_parallel");
	GtkWidget	*simple = lookup_widget(GTK_WIDGET(button), "simple");
	GtkWidget	*lambert = lookup_widget(GTK_WIDGET(button), "lambert");
	GtkWidget	*phong = lookup_widget(GTK_WIDGET(button), "phong");
	GtkWidget	*blinn = lookup_widget(GTK_WIDGET(button), "blinn_phong");
	GtkWidget	*perlin = lookup_widget(GTK_WIDGET(button), "perlin");
	GtkWidget	*parallel = lookup_widget(GTK_WIDGET(button), "chk_parallel");
	GtkWidget	*aliasing = lookup_widget(GTK_WIDGET(button), "antialiasing");

	tCommand.bool_simple = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(simple));
	tCommand.bool_lambert = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lambert));
	tCommand.bool_phong = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(phong));
	tCommand.bool_blinn = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(blinn));
	tCommand.bool_perlin = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(perlin));
	tCommand.bool_parallel = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(parallel));
	tCommand.bool_aliasing = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(aliasing));

	tCommand.txt_open = gtk_entry_get_text(GTK_ENTRY(entry_open));
	tCommand.txt_img = gtk_entry_get_text(GTK_ENTRY(entry_img_name));
	tCommand.txt_parallel = gtk_entry_get_text(GTK_ENTRY(entry_parallel));

	return tCommand;
}


gint
look_generate			(GtkButton *button)
{
	gint		samples = 0;
	const gchar	*txt_sample;
	gboolean	bool_generate;
	GtkWidget	*generate = lookup_widget(GTK_WIDGET(button), "chk_generate");
	GtkWidget	*entry_sample = lookup_widget(GTK_WIDGET(button), "entry_sample");

	bool_generate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(generate));
	txt_sample = gtk_entry_get_text(GTK_ENTRY(entry_sample));

	if (bool_generate == FALSE)
		return 0;

	if (strcmp(txt_sample, "") != 0)
		samples = atoi(txt_sample);

	return samples;
}


void
generate_samples		(gchar *command, gint samples, gint *level_sample)
{
	// C'est le pas de l'animation on peut réduire pour avoir une animation plus précise avec plus de fps.
	gfloat	step = 2.0f;
	step *= (*level_sample);

	gchar	*command_tmp;
	command_tmp = (gchar *) g_malloc(4 * MAX_FILE_LENGTH * sizeof (gchar));	// Faut allouer plus d'espace mémoire -_-
	strcpy(command_tmp, command);

	gchar	*opt_view;
	opt_view = (gchar *) g_malloc(MAX_FILE_LENGTH * sizeof (gchar));
	if (*level_sample < 10)	// Pour que ca marche avec l'animate de imagemagick
		sprintf(opt_view, " -v %f -o anim00%d", step, *level_sample);	// un peu moche mais bon
	else if (*level_sample < 100)
		sprintf(opt_view, " -v %f -o anim0%d", step, *level_sample);
	else
		sprintf(opt_view, " -v %f -o anim%d", step, *level_sample);

	strcat(command_tmp, opt_view);
	g_free(opt_view);

	(*level_sample) ++;
	//g_print("%s", command_tmp);

	if(system(command_tmp))
	{
		g_print("!Erreur : KF-Ray backend émet un signal d'erreur. Activation Sample.\n\
			Vérifiez que kfray est dans le même répertoire que kfray_gui.\n\
			Vérifiez également que vous avez bien activé la parallélisation\
			référez-vous au manuel d'utilisation");
	}
	else
	{
		g_free(command_tmp);
		g_print("Rendu %d/%d images terminées avec succès !\n", *level_sample, samples);
		if (*level_sample < samples)
			generate_samples(command, samples, level_sample);
	}
}


gchar
*shell_command			(t_command tCommand)
{
	gint	proc_parallel = 1;
	gchar	*command_tmp;
	command_tmp = (gchar *) g_malloc(3 * MAX_FILE_LENGTH * sizeof (gchar));

	give_args(&tCommand);

	if (tCommand.bool_aliasing == TRUE)
		strcat(tCommand.command, " -a");

	if (tCommand.bool_parallel == TRUE)
	{
		if (strcmp(tCommand.txt_parallel, "") != 0)
			proc_parallel = atoi(tCommand.txt_parallel);
		strcpy(command_tmp, tCommand.command);
		sprintf(tCommand.command, "mpirun -v -c %d %s", proc_parallel, command_tmp);
	}

	g_print(">> KF-RAY GUI - Execution de :\n%s\n", tCommand.command);
	g_free(command_tmp);

	return	tCommand.command;
}



void
give_args			(t_command *tCommand)
{
	gint 	modelbrdf, modeltexture;
	gchar	*file_name, *img_name;

	img_name = (gchar *) g_malloc(MAX_FILE_LENGTH * sizeof (gchar));
	file_name = (gchar *) g_malloc(MAX_FILE_LENGTH * sizeof (gchar));

	if (strcmp(tCommand->txt_open, "") != 0)
		strcpy(file_name, tCommand->txt_open);
	else
		strcpy(file_name, "scene1.kfr");

	if (strcmp(tCommand->txt_img, "") == 0)
	{
		strcpy(img_name, file_name);
		if (strlen(file_name) >= 3)
			if (strcmp(".kfr", file_name + (strlen(file_name)-4)) == 0)
				img_name[strlen(file_name)-4] = '\0'; // On tronque
	}
	else
		strcpy(img_name, tCommand->txt_img);

	modelbrdf = model_brdf(*tCommand);
	modeltexture = model_texture(*tCommand);

	sprintf(tCommand->command, "./kfray -i %s -o %s -b %d", file_name, img_name, modelbrdf);

	if (modeltexture == 0)
		strcat(tCommand->command, " -t");

	g_free(file_name);
	g_free(img_name);
}


gint
model_brdf				(t_command tCommand)
{
	gint modelbrdf;

	if (tCommand.bool_simple == TRUE)
		modelbrdf = 0;
	else if (tCommand.bool_lambert == TRUE)
		modelbrdf = 1;
	else if (tCommand.bool_phong == TRUE)
		modelbrdf = 2;
	else
		modelbrdf = 3;

	return modelbrdf;
}

gint
model_texture			(t_command tCommand)
{
	gint modeltexture;

	if (tCommand.bool_perlin == TRUE)
		modeltexture = 1;
	else
		modeltexture = 0;

	return modeltexture;
}
