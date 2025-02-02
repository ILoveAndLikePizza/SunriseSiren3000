#include "../include/SunriseSirenStudio.h"

#include "../include/UI.h"
#include "../include/Dialog.h"
#include "../include/String.h"
#include "../include/Requests.h"
#include "../include/ConfigFile.h"

#include "../include/Color.h"
#include "../include/Connection.h"


void reboot() {
    execl("/proc/self/exe", argv0, NULL);

    perror("An error occured while rebooting Sunrise Siren Studio.");
    exit(2);
}

static void onActivate(GtkApplication *app, gpointer user_data) {
    curl_init();
    sprintf(config_file_path, "%s/.config/SunriseSirenStudio.json", g_getenv("HOME"));

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, UI, -1, NULL);

    // step 1: check if the config file exists
    if (config_file_exists()) {
        // step 2: parse the config file
        if (config_file_parse()) {
            // step 3 (todo): fetch /status en /sensors
            // step 4 (todo): apply to MainWindow
            // step 5 (move here): show MainWindow
            g_print("Connecting %s:%s to %s", username, password, hostname);

        } else {
            g_print("Big Oof");
            exit(12);
        }

        // configuration already made, TODO: test clock connection
        default_color = g_new(GdkRGBA, 1);
        alarm_color = g_new(GdkRGBA, 1);
        default_color->alpha = 1;
        alarm_color->alpha = 1;

        // MainWindow
        MainWindow = gtk_builder_get_object(builder, "MainWindow");
        // Clock settings tab
        DefaultColor = gtk_builder_get_object(builder, "DefaultColor");
        g_signal_connect(DefaultColor, "clicked", pick_default_color, NULL);

        AlarmColor = gtk_builder_get_object(builder, "AlarmColor");
        g_signal_connect(AlarmColor, "clicked", pick_alarm_color, NULL);

        gtk_application_add_window(app, MainWindow);
        gtk_widget_show_all(MainWindow);
    } else {
        // no configuration made, do that first
        ConnectionWindow = gtk_builder_get_object(builder, "ConnectionWindow");

        ConnectHostname = gtk_builder_get_object(builder, "ConnectHostname");
        ConnectUsername = gtk_builder_get_object(builder, "ConnectUsername");
        ConnectPassword = gtk_builder_get_object(builder, "ConnectPassword");

        ConnectConfirm = gtk_builder_get_object(builder, "ConnectConfirm");
        g_signal_connect(ConnectConfirm, "clicked", create_connection, NULL);

        gtk_application_add_window(app, ConnectionWindow);
        gtk_widget_show_all(ConnectionWindow);
    }

    g_object_unref(builder);
}

int main(int argc, char* argv[]) {
    argv0 = argv[0];

    GtkApplication *app = gtk_application_new("sunrise.siren.studio", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", onActivate, NULL);

    int status = g_application_run(app, argc, argv);
    g_object_unref(app);
    return status;
}
