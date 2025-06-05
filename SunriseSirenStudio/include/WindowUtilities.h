#ifndef _window_utilities_h
#define _window_utilities_h

#define VALIDATION_STRING "Yes, a Sunrise Siren 3000 is here!"

// general
void quit() {
    exit(0);
}

void reboot_program() {
    execl("/proc/self/exe", argv0, NULL);

    perror("An error occurred while rebooting Sunrise Siren Studio.");
    exit(2);
}

void sleep_clock() {
    gchar *sleep_url[PATH_MAX];
    sprintf(sleep_url, "http://%s/sleep", hostname);

    gchar *zzz = request("PATCH", sleep_url, username, password, "");

    if (zzz && request_last_status_code == 200) {
        // sleep mode performed successful
        show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Success!", "Your Sunrise Siren 3000 falls asleep now... zzz");
    } else if (zzz && request_last_status_code == 418) {
        // device is already in sleep mode
        show_message_dialog(MainWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "zzz", "Your Sunrise Siren 3000 is already sleeping.\nTo wake it up again, push the button on top of the device.");
    } else if (zzz && request_last_status_code == 400) {
        // device cannot sleep in current state
        show_message_dialog(MainWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Sleep mode failed", "Your Sunrise Siren 3000 cannot go to sleep mode in its current state.");
    } else {
        // error while trying to perform sleep mode
        show_message_dialog(MainWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Sleep mode failed", "An error occurred while setting your Sunrise Siren 3000 in sleep mode. Please try again later.");
    }
}

void reboot_clock() {
    gint prompt = show_message_dialog(MainWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "Reboot?", "Are you sure you want to reboot your Sunrise Siren 3000?");

    if (prompt == GTK_RESPONSE_YES) {
        gchar *reboot_url[PATH_MAX];
        sprintf(reboot_url, "http://%s/reboot", hostname);

        gchar *reboot = request("PATCH", reboot_url, username, password, "");

        if (reboot && strstr(reboot, "Initiating reboot.")) {
            // reboot performed successful
            show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Success!", "Your Sunrise Siren 3000 will reboot any moment now.");
        } else {
            // error while trying to perform reboot
            show_message_dialog(MainWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Failed to reboot", "An error occurred while rebooting your Sunrise Siren 3000. Please try again later.");
        }
    }
}

static void reconfigure(GtkWidget *widget, gpointer do_reset) {
    if (do_reset) {
        gint prompt = show_message_dialog(MainWindow, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "Reset credentials?", "Note that you will have to sign in again!");
        if (prompt == GTK_RESPONSE_OK) credentials_reset();
    } else {
        set_connect_immediately(FALSE);
    }
    reboot_program();
}

// MainWindow
// alarm settings
static void validate_alarm_time_sensitivity(GtkWidget *widget, gboolean state, gpointer user_data) {
    gint index = user_data;

    gtk_widget_set_sensitive(AlarmHour[index], state);
    gtk_widget_set_sensitive(AlarmMinute[index], state);
}

// custom mode
gint custom_combo_box_get_corresponding_number(GtkComboBox *combo_box) {
    GtkTreeIter iter;
    gint corresponding_number = 0;

    if (gtk_combo_box_get_active_iter(combo_box, &iter)) {
        GtkTreeModel *tree = gtk_combo_box_get_model(combo_box);
        gtk_tree_model_get(tree, &iter, 1, &corresponding_number, -1);
    } else exit(3);

    return corresponding_number;
}

static void validate_custom_entry_sensitive(GtkWidget *widget, gpointer user_data) {
    gint corresponding_number = custom_combo_box_get_corresponding_number(widget);
    gtk_widget_set_sensitive(user_data, (corresponding_number == 0));
}

static void set_all_custom_pixels(GtkWidget *widget, gpointer user_data) {
    for (int i=0; i<21; i++)
        gtk_toggle_button_set_active(CustomSegmentNumber[i], user_data);
}

static void invert_all_custom_pixels(GtkWidget *widget, gpointer user_data) {
    for (int i=0; i<21; i++)
        gtk_toggle_button_set_active(CustomSegmentNumber[i], !gtk_toggle_button_get_active(CustomSegmentNumber[i]));
}

static void set_custom_digit(GtkWidget *widget, gpointer user_data) {
    gint target_index = user_data ? user_data - 1 : gtk_spin_button_get_value_as_int(CustomDigitApplyIndex) - 1;
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

static void set_all_custom_digits(GtkWidget *widget, gpointer user_data) {
    for (int i=0; i<4; i++) set_custom_digit(widget, i + 1);
}

// countdown
static void countdown_start(GtkWidget *widget, gpointer user_data) {
    gint countdown_total = gtk_spin_button_get_value_as_int(CountdownValue);
    gint countdown_pauseable = gtk_switch_get_active(CountdownPauseable);

    if (countdown_total < 5) {
        show_message_dialog(MainWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Cannot start countdown", "The countdown must be at least 5 seconds!");
        return;
    }

    gchar *post_url[PATH_MAX];
    gchar *post_string[64];

    sprintf(post_url, "http://%s/countdown", hostname);
    sprintf(post_string, "t=%i&pauseable=%i", countdown_total, countdown_pauseable);

    gchar *result = request("POST", post_url, username, password, post_string);
    if (result && strstr(result, "Done!")) {
        // countdown successfully started
        show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Success!", "Countdown is running now!");
    } else {
        // error while trying to start countdown
        show_message_dialog(MainWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Failed to start countdown", "An error occurred while starting the countdown. Please try again later.");
    }
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

// save buttons
static void apply_clock_settings(GtkWidget *widget, gpointer user_data) {
    // step 1: collect all settings
    GdkRGBA *default_c = g_new(GdkRGBA, 1);
    GdkRGBA *alarm_c = g_new(GdkRGBA, 1);
    gtk_color_chooser_get_rgba(DefaultColor, &*default_c);
    gtk_color_chooser_get_rgba(AlarmColor, &*alarm_c);

    glong default_c_number = (int) (default_c->red * 255) * (int) pow(256, 2)
        + (int) (default_c->green * 255) * (int) pow(256, 1)
        + (int) (default_c->blue * 255) * (int) pow(256, 0);
    glong alarm_c_number = (int) (alarm_c->red * 255) * (int) pow(256, 2)
        + (int) (alarm_c->green * 255) * (int) pow(256, 1)
        + (int) (alarm_c->blue * 255) * (int) pow(256, 0);

    gint alarms_enabled = 0;
    gint alarm_hours[7];
    gint alarm_minutes[7];

    for (int i=0; i<7; i++) {
        if (gtk_switch_get_active(AlarmEnable[i])) alarms_enabled += (int) pow(2, i);

        alarm_hours[i] = gtk_spin_button_get_value_as_int(AlarmHour[i]);
        alarm_minutes[i] = gtk_spin_button_get_value_as_int(AlarmMinute[i]);
    }

    gchar* alarm_times[28];
    sprintf(alarm_times,
        "%02i%02i%02i%02i%02i%02i%02i%02i%02i%02i%02i%02i%02i%02i",
        alarm_hours[0], alarm_minutes[0],
        alarm_hours[1], alarm_minutes[1],
        alarm_hours[2], alarm_minutes[2],
        alarm_hours[3], alarm_minutes[3],
        alarm_hours[4], alarm_minutes[4],
        alarm_hours[5], alarm_minutes[5],
        alarm_hours[6], alarm_minutes[6]
    );

    gint leading_zero = gtk_switch_get_active(EnableLeadingZero);
    gint dst = gtk_switch_get_active(EnableDST);
    gint snooze_t = gtk_spin_button_get_value_as_int(SnoozeInterval) * 1000;
    gint return_after = gtk_spin_button_get_value_as_int(ClockReturn) * 1000;
    gint ldr_min = gtk_spin_button_get_value_as_int(LDRMin);
    gint ldr_max = gtk_spin_button_get_value_as_int(LDRMax);

    // step 2: make a request string
    gchar *post_url[PATH_MAX];
    gchar *post_string[512];

    sprintf(post_url, "http://%s/update", hostname);
    sprintf(
        post_string,
        "default-c=%ld&alarm-c=%ld&alarms-enabled=%i&alarm-times=%s&leading-zero=%i&enable-dst=%i&snooze-t=%i&clock-return=%i&ldr-min=%i&ldr-max=%i",
        default_c_number, alarm_c_number, alarms_enabled, alarm_times, leading_zero, dst, snooze_t, return_after, ldr_min, ldr_max
    );

    // step 3: yeet a request
    gchar *result = request("POST", post_url, username, password, post_string);
    if (result && strstr(result, "Done!")) {
        // settings successfully applied
        show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Success!", "Changes have been saved and applied!");
    } else {
        // error while trying to apply settings
        show_message_dialog(MainWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Failed to save settings", "An error occurred while trying to save and apply the settings. Please try again later.");
    }
}

static void apply_custom_settings(GtkWidget *widget, gpointer user_data) {
    // step 1: collect all settings
    GdkRGBA *custom_colors[5]; // 0-3 are digits, 4 is colon
    glong color_numbers[4];
    gint custom_digits[4];

    for (int i=0; i<4; i++) {
        custom_colors[i] = g_new(GdkRGBA, 1);
        gtk_color_chooser_get_rgba(CustomColor[i], &*custom_colors[i]);

        color_numbers[i] = (int) (custom_colors[i]->red * 255) * (int) pow(256, 2)
            + (int) (custom_colors[i]->green * 255) * (int) pow(256, 1)
            + (int) (custom_colors[i]->blue * 255) * (int) pow(256, 0);

        gint combo_box_number = custom_combo_box_get_corresponding_number(CustomDigit[i]);
        custom_digits[i] = (combo_box_number == 0) ? atoi(gtk_entry_get_text(CustomDigitEntry[i])) : combo_box_number;
    }

    custom_colors[4] = g_new(GdkRGBA, 1);
    gtk_color_chooser_get_rgba(CustomColor_Colon, &*custom_colors[4]);
    glong color_colon = (int) (custom_colors[4]->red * 255) * (int) pow(256, 2)
        + (int) (custom_colors[4]->green * 255) * (int) pow(256, 1)
        + (int) (custom_colors[4]->blue * 255) * (int) pow(256, 0);

    // step 2: make a request string
    gchar *segment_strings[4][64];
    gchar *post_url[PATH_MAX];
    gchar *post_string[512];

    sprintf(post_url, "http://%s/custom", hostname);
    for (int i=0; i<4; i++) sprintf(
        segment_strings[i],
        "segment-%i=%i&color-%i=%ld",
        i, custom_digits[i], i, color_numbers[i]
    );
    sprintf(
        post_string,
        "%s&%s&%s&%s&color-colon=%ld",
        segment_strings[0],
        segment_strings[1],
        segment_strings[2],
        segment_strings[3],
        color_colon
    );

    // step 3: yeet a request
    gchar *result = request("POST", post_url, username, password, post_string);
    if (result && strstr(result, "Done!")) {
        // custom config successfully set
        show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Success!", "Custom configuration has been saved and is visible now!");
    } else {
        // failed to set custom config
        show_message_dialog(MainWindow, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Failed to show custom mode", "An error occurred while trying to save and show the custom mode. Please try again later.");
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

    if (req && strstr(req, VALIDATION_STRING)) {
        // authentication successful, save and continue

        credentials_write(hostname, username, password);
        set_connect_immediately(TRUE);
        reboot_program();
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

// key event handlers
gboolean ctrl_pressed = FALSE;

static void main_window_key_press(GtkWidget *widget, GdkEventKey *e, gpointer data) {
    if (e->keyval == 65507) ctrl_pressed = TRUE;
    else if (e->keyval == 115) {
        gchar* visible_tab = gtk_stack_get_visible_child_name(MainStack);

        if (strstr(visible_tab, "ClockSettings")) apply_clock_settings(widget, data);
        else if (strstr(visible_tab, "CustomMode")) apply_custom_settings(widget, data);
    } else do_absolutely_nothing(); // idk WHY I have to do this, but I have to
}

static void main_window_key_release(GtkWidget *widget, GdkEventKey *e, gpointer data) {
    if (e->keyval == 65507) ctrl_pressed = FALSE;
    else do_absolutely_nothing();
}

static void connection_window_key_press(GtkWidget *widget, GdkEventKey *e, gpointer data) {
    if (e->keyval == 65293) create_connection(widget, data);
    else do_absolutely_nothing();
}

#endif
