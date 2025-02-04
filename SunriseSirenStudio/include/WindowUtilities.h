#ifndef _window_utilities_h
#define _window_utilities_h

#define VALIDATION_STRING "Yes, a Sunrise Siren 3000 is here!"

// general
void quit() {
    exit(0);
}

void reboot() {
    execl("/proc/self/exe", argv0, NULL);

    perror("An error occured while rebooting Sunrise Siren Studio.");
    exit(2);
}

void reboot_clock() {
    gint prompt = show_message_dialog(MainWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "Reboot?", "Are you sure you want to reboot your Sunrise Siren 3000?");

    if (prompt == GTK_RESPONSE_YES) {
        gchar *url[PATH_MAX];
        sprintf(url, "http://%s/reboot", hostname);

        request("PATCH", url, username, password, "");

        show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Rebooted!", "Your Sunrise Siren 3000 has been rebooted.");
    }
}

void reconfigure() {
    gint prompt = show_message_dialog(MainWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "Reset credentials?", "Note that you will have to sign in again!");

    if (prompt == GTK_RESPONSE_OK) {
        credentials_reset();
        reboot();
    }
}

// MainWindow
// color selection
static void pick_color(GtkWidget *widget, gpointer user_data) {
    GtkColorSelectionDialog *dialog = gtk_color_selection_dialog_new("Select a color");
    GtkColorSelection *selection = gtk_color_selection_dialog_get_color_selection(dialog);

    gtk_color_selection_set_current_rgba(selection, user_data);

    if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
        gtk_color_selection_get_current_rgba(selection, user_data);
        gtk_widget_override_background_color(widget, GTK_STATE_NORMAL, user_data);
    }

    gtk_widget_destroy(dialog);
}

// custom mode
static void validate_custom_entry_sensitive(GtkWidget *widget, gpointer user_data) {
    GtkTreeIter iter;
    gint corresponding_number;

    if (gtk_combo_box_get_active_iter(widget, &iter)) {
        GtkTreeModel *tree = gtk_combo_box_get_model(widget);
        gtk_tree_model_get(tree, &iter, 1, &corresponding_number, -1);

        gtk_widget_set_sensitive(user_data, (corresponding_number == 0));
    } else exit(3);
}

static void set_all_custom_pixels(GtkWidget *widget, gpointer user_data) {
    for (int i=0; i<21; i++)
        gtk_toggle_button_set_active(CustomSegmentNumber[i], user_data);
}

static void set_custom_digit(GtkWidget *widget, gpointer user_data) {
    gint target_index = gtk_spin_button_get_value_as_int(CustomDigitApplyIndex) - 1;
    gint custom_number = 0;
    gchar *entry_text[12];

    for (int i=0; i<21; i++) {
        if (gtk_toggle_button_get_active(CustomSegmentNumber[i])) custom_number += (int) pow(2, i);
    }

    sprintf(entry_text, "%i", custom_number);
    gtk_entry_set_text(CustomDigitEntry[target_index], entry_text);
    
    gint combo_box_item_count = gtk_tree_model_iter_n_children(gtk_combo_box_get_model(CustomDigit[target_index]), NULL);
    gtk_combo_box_set_active(CustomDigit[target_index], combo_box_item_count - 1);
}

// sensor information
void get_sensor_values() {
    gchar* sensors_url[PATH_MAX];
    sprintf(sensors_url, "http://%s/sensors", hostname);
    gchar *sensors_response = request("GET", sensors_url, username, password, "");

    if (sensors_response) {
        if (clock_sensors = json_tokener_parse(sensors_response)) {
            gchar *ldr_label[4];
            gchar *temperature_label[20];
            gchar *humidity_label[20];

            sprintf(ldr_label, "%d", json_object_get_int(json_object_object_get(clock_sensors, "ldr")));
            sprintf(
                temperature_label,
                "%i → %.2f °C",
                json_object_get_int(json_object_object_get(json_object_object_get(clock_sensors, "temperature"), "raw")),
                json_object_get_double(json_object_object_get(json_object_object_get(clock_sensors, "temperature"), "translated"))
            );
            sprintf(
                humidity_label,
                "%i → %.2f%%",
                json_object_get_int(json_object_object_get(json_object_object_get(clock_sensors, "humidity"), "raw")),
                json_object_get_double(json_object_object_get(json_object_object_get(clock_sensors, "humidity"), "translated"))
            );

            gtk_label_set_label(LDRReading, ldr_label);
            gtk_label_set_label(SHT21TemperatureReading, temperature_label);
            gtk_label_set_label(SHT21HumidityReading, humidity_label);
        }
    }
}

// ConnectionWindow
static void create_connection(GtkWidget *widget, gpointer user_data) {
    gchar* hostname = gtk_entry_get_text(ConnectHostname);
    gchar* username = gtk_entry_get_text(ConnectUsername);
    gchar* password = gtk_entry_get_text(ConnectPassword);

    if (!strlen(hostname) || !strlen(username) || !strlen(password)) {
        show_message_dialog(ConnectionWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Unable to connect", "Please enter all the fields!");
        return;
    }

    gchar* url[PATH_MAX];
    sprintf(url, "http://%s/connect", hostname);

    gchar* req = request("GET", url, username, password, "");
    
    if (strstr(req, VALIDATION_STRING)) {
        // authentication successful, save and continue

        credentials_write(hostname, username, password);
        reboot();
    } else if (req && request_last_status_code == 401) {
        // incorrect username or password
        show_message_dialog(ConnectionWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Unable to connect", "Incorrect username or password!");
    } else if (req) {
        // failed to validate
        show_message_dialog(ConnectionWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Unable to connect", "Failed to validate the connection.");
    } else {
        // request failed, nothing found at all
        show_message_dialog(ConnectionWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Unable to connect", "Failed to connect to your Sunrise Siren 3000.");
    }
}

#endif
