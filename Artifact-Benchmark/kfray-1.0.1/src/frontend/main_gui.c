 /*
 ** KF-Ray GUI, interface graphique
 **  main_gui.c
 **
 ** Copyright 2009 Karin AIT-SI-AMER & Florian DANG
 ** <aitsiame@polytech.upmc.fr>
 ** <tdang@polytech.upmc.fr>
 **
 ** Reportez-vous à COPYING pour les infos sur le Copyright
 */

 /*************************************************************************
 *   This file is part of KF-Ray.
 *
 *   KF-Ray is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   KF-Ray is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with KF-Ray.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include "interface.h"
#include "support.h"
#include "callbacks.h"
#include "main_gui.h"
#include "functions.h"

int main (int argc, char* argv[])
{

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	gtk_set_locale ();
	gtk_init (&argc, &argv);

	//add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");
	add_pixmap_directory("frontend/pixmaps");
	
	win_main = create_win_main();
	gtk_widget_show(win_main);

	// Redirige la sortie g_print vers le GTK Text View
	g_set_print_handler((GPrintFunc) gtk_print);

	gtk_signal_connect	(GTK_OBJECT(win_main), 
				"delete_event",(GtkSignalFunc)destroy_window, (gpointer)win_main);
	gtk_signal_connect 	(GTK_OBJECT(win_main),
				"destroy",(GtkSignalFunc)destroy_window, (gpointer)win_main);

	gtk_main ();

	return 0;
}

void destroy_window (GtkWidget *widget, gpointer client_data)
{
	gtk_main_quit ();
}
