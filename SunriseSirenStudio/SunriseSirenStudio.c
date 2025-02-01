#include "SunriseSirenStudio.h"

#include "UI.h"
#include "File.h"
#include "String.h"
#include "Requests.h"

static void onActivate(GtkApplication *app, gpointer user_data) {
    curl_init();

    // g_print(UI);
    // g_print("I am looking for %s", filename_after_cwd("StudioUI.glade"));

    GtkBuilder *builder = gtk_builder_new();
    // gtk_builder_add_from_file(builder, "./StudioUI.glade", NULL);
    gtk_builder_add_from_string(builder, UI, -1, NULL);

    GtkWidget *MainWindow = gtk_builder_get_object(builder, "MainWindow");

    // MainWindow
    // Clock settings tab
    GtkWidget *DefaultColor = gtk_builder_get_object(builder, "DefaultColor");

    gtk_application_add_window(app, MainWindow);
    gtk_widget_show_all(MainWindow);

    g_object_unref(builder);
}

int main(int argc, char* argv[]) {
    GtkApplication *app = gtk_application_new("sunrise.siren.studio", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", onActivate, NULL);

    int status = g_application_run(app, argc, argv);
    g_object_unref(app);
    return status;
}
