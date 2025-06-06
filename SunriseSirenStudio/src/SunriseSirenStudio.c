#include "../include/SunriseSirenStudio.h"

#include "../include/UI.h"
#include "../include/Dialog.h"
#include "../include/Requests.h"
#include "../include/Credentials.h"

#include "../include/WindowUtilities.h"

enum TargetWindow {
    WINDOW_MAIN,
    WINDOW_CONNECTION,
    WINDOW_ERROR
};

void* thread_function() {
    while (1) {
        get_sensor_values();
        g_usleep(5e6);
    }
}

static void onActivate(GtkApplication *app, gpointer user_data) {
    curl_global_init(CURL_GLOBAL_ALL);
    credentials = g_settings_new("org.x.sunrise-siren-studio.credentials");

    enum TargetWindow target;

    // step 1: check if the config file exists
    if (credentials_exist() && g_settings_get_boolean(credentials, "connect-immediately")) {
        // step 2: parse the config file
        credentials_read();
        // step 3: fetch /status en /sensors
        gchar* status_url[PATH_MAX];
        sprintf(status_url, "http://%s/status", hostname);
        gchar *status_response = request("GET", status_url, username, password, "");

        if (status_response) {
            if (clock_status = json_tokener_parse(status_response)) {
                // success!
                target = WINDOW_MAIN;
            } else {
                // failed to parse clock data
                target = WINDOW_ERROR;
            }
        } else {
            // failed to fetch clock data
            target = WINDOW_ERROR;
        }

    } else {
        // no configuration made, do that first
        target = WINDOW_CONNECTION;
    }

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, UI, -1, NULL);

    if (target == WINDOW_MAIN) {
        // MainWindow
        MainWindow = gtk_builder_get_object(builder, "MainWindow");
        g_signal_connect(MainWindow, "key-press-event", main_window_key_press, NULL);
        g_signal_connect(MainWindow, "key-release-event", main_window_key_release, NULL);

        MainStack = gtk_builder_get_object(builder, "MainStack");

        // Clock settings tab
        // Colors
        DefaultColor = gtk_builder_get_object(builder, "DefaultColor");

        glong current_default_color = json_object_get_int64(json_object_object_get(json_object_object_get(clock_status, "colors"), "default"));
        GdkRGBA *default_color = g_new(GdkRGBA, 1);
        default_color->red = (gdouble) (current_default_color / (int) pow(256, 2) % 256) / 255;
        default_color->green = (gdouble) (current_default_color / (int) pow(256, 1) % 256) / 255;
        default_color->blue = (gdouble) (current_default_color / (int) pow(256, 0) % 256) / 255;
        default_color->alpha = 1;
        gtk_color_chooser_set_rgba(DefaultColor, default_color);

        HighlightColor = gtk_builder_get_object(builder, "HighlightColor");

        glong current_highlight_color = json_object_get_int64(json_object_object_get(json_object_object_get(clock_status, "colors"), "alarm"));
        GdkRGBA *highlight_color = g_new(GdkRGBA, 1);
        highlight_color->red = (gdouble) (current_highlight_color / (int) pow(256, 2) % 256) / 255;
        highlight_color->green = (gdouble) (current_highlight_color / (int) pow(256, 1) % 256) / 255;
        highlight_color->blue = (gdouble) (current_highlight_color / (int) pow(256, 0) % 256) / 255;
        highlight_color->alpha = 1;
        gtk_color_chooser_set_rgba(HighlightColor, highlight_color);

        // Alarms
        gchar* alarm_times = json_object_get_string(json_object_object_get(clock_status, "alarmTimes"));
        gint alarms_enabled = json_object_get_int(json_object_object_get(clock_status, "alarmsEnabled"));

        for (int i=0; i<7; i++) {
            gchar* enable_id[13];
            gchar* hour_id[11];
            gchar* minute_id[13];

            sprintf(enable_id, "AlarmEnable-%i", i);
            sprintf(hour_id, "AlarmHour-%i", i);
            sprintf(minute_id, "AlarmMinute-%i", i);

            AlarmEnable[i] = gtk_builder_get_object(builder, enable_id);
            AlarmHour[i] = gtk_builder_get_object(builder, hour_id);
            AlarmMinute[i] = gtk_builder_get_object(builder, minute_id);

            gchar hour[3] = {alarm_times[i * 4], alarm_times[i * 4 + 1], '\0'};
            gchar minute[3] = {alarm_times[i * 4 + 2], alarm_times[i * 4 + 3], '\0'};

            gtk_spin_button_set_adjustment(AlarmHour[i], gtk_adjustment_new(atoi(hour), 0, 24, 1, 1, 1));
            gtk_spin_button_set_numeric(AlarmHour[i], TRUE);
            gtk_spin_button_set_wrap(AlarmHour[i], TRUE);

            gtk_spin_button_set_adjustment(AlarmMinute[i], gtk_adjustment_new(atoi(minute), 0, 60, 1, 1, 1));
            gtk_spin_button_set_numeric(AlarmMinute[i], TRUE);
            gtk_spin_button_set_wrap(AlarmMinute[i], TRUE);

            gboolean is_enabled = (alarms_enabled & (gint) pow(2, i)) > 0;

            gtk_switch_set_active(AlarmEnable[i], is_enabled);
            validate_alarm_time_sensitivity(AlarmEnable[i], is_enabled, i);
            g_signal_connect(AlarmEnable[i], "state-set", validate_alarm_time_sensitivity, i);
        }

        ClockUpdate = gtk_builder_get_object(builder, "ClockUpdate");
        g_signal_connect(ClockUpdate, "clicked", apply_clock_settings, NULL);

        // Advanced
        EnableLeadingZero = gtk_builder_get_object(builder, "EnableLeadingZero");
        gtk_switch_set_active(EnableLeadingZero, json_object_get_boolean(json_object_object_get(clock_status, "leadingZero")));

        EnableDST = gtk_builder_get_object(builder, "EnableDST");
        gtk_switch_set_active(EnableDST, json_object_get_boolean(json_object_object_get(clock_status, "enableDST")));

        SnoozeInterval = gtk_builder_get_object(builder, "SnoozeInterval");
        gtk_spin_button_set_value(SnoozeInterval, json_object_get_int(json_object_object_get(clock_status, "snoozeInterval")));

        ClockReturn = gtk_builder_get_object(builder, "ClockReturn");
        gtk_spin_button_set_value(ClockReturn, json_object_get_int(json_object_object_get(clock_status, "clockReturn")));

        LDRMin = gtk_builder_get_object(builder, "LDRMin");
        gtk_spin_button_set_value(LDRMin, json_object_get_int(json_object_object_get(json_object_object_get(clock_status, "ldr"), "min")));

        LDRMax = gtk_builder_get_object(builder, "LDRMax");
        gtk_spin_button_set_value(LDRMax, json_object_get_int(json_object_object_get(json_object_object_get(clock_status, "ldr"), "max")));

        // Custom
        CustomDigits = gtk_builder_get_object(builder, "CustomDigits");

        for (int i=0; i<4; i++) {
            gchar* colorbutton_id[13];
            gchar* combobox_id[13];
            gchar* colorentry_id[18];

            sprintf(colorbutton_id, "CustomColor-%i", i);
            sprintf(combobox_id, "CustomDigit-%i", i);
            sprintf(colorentry_id, "CustomDigitEntry-%i", i);

            CustomColor[i] = gtk_builder_get_object(builder, colorbutton_id);
            CustomDigit[i] = gtk_builder_get_object(builder, combobox_id);
            CustomDigitEntry[i] = gtk_builder_get_object(builder, colorentry_id);

            GdkRGBA *custom_color = g_new(GdkRGBA, 1);
            custom_color->red = default_color->red;
            custom_color->green = default_color->green;
            custom_color->blue = default_color->blue;
            custom_color->alpha = 1;
            gtk_color_chooser_set_rgba(CustomColor[i], custom_color);

            g_signal_connect(CustomDigit[i], "changed", validate_custom_entry_sensitive, CustomDigitEntry[i]);
        }

        CustomColor_Colon = gtk_builder_get_object(builder, "CustomColor-Colon");
        GdkRGBA *custom_colon = g_new(GdkRGBA, 1);
        custom_colon->red = default_color->red;
        custom_colon->green = default_color->green;
        custom_colon->blue = default_color->blue;
        custom_colon->alpha = 1;
        gtk_color_chooser_set_rgba(CustomColor_Colon, custom_colon);

        for (int i=0; i<21; i++) {
            gchar* pixel_id[22];
            sprintf(pixel_id, "CustomSegmentNumber-%02i", i);

            CustomSegmentNumber[i] = gtk_builder_get_object(builder, pixel_id);
        }

        CustomDigitEnableAll = gtk_builder_get_object(builder, "CustomDigitEnableAll");
        g_signal_connect(CustomDigitEnableAll, "clicked", set_all_custom_pixels, TRUE);

        CustomDigitDisableAll = gtk_builder_get_object(builder, "CustomDigitDisableAll");
        g_signal_connect(CustomDigitDisableAll, "clicked", set_all_custom_pixels, FALSE);

        CustomDigitInvertAll = gtk_builder_get_object(builder, "CustomDigitInvertAll");
        g_signal_connect(CustomDigitInvertAll, "clicked", invert_all_custom_pixels, FALSE);

        CustomDigitApplyIndex = gtk_builder_get_object(builder, "CustomDigitApplyIndex");
        CustomDigitApply = gtk_builder_get_object(builder, "CustomDigitApply");
        g_signal_connect(CustomDigitApply, "clicked", set_custom_digit, NULL);

        CustomDigitApplyAll = gtk_builder_get_object(builder, "CustomDigitApplyAll");
        g_signal_connect(CustomDigitApplyAll, "clicked", set_all_custom_digits, NULL);

        CustomUpdate = gtk_builder_get_object(builder, "CustomUpdate");
        g_signal_connect(CustomUpdate, "clicked", apply_custom_settings, NULL);

        // Miscellaneous
        // Countdown
        CountdownValue = gtk_builder_get_object(builder, "CountdownValue");
        CountdownPauseable = gtk_builder_get_object(builder, "CountdownPauseable");

        CountdownStart = gtk_builder_get_object(builder, "CountdownStart");
        g_signal_connect(CountdownStart, "clicked", countdown_start, NULL);

        // Sensor information
        LDRReading = gtk_builder_get_object(builder, "LDRReading");
        SHT21TemperatureReading = gtk_builder_get_object(builder, "SHT21TemperatureReading");
        SHT21HumidityReading = gtk_builder_get_object(builder, "SHT21HumidityReading");

        SensorRefresh = gtk_builder_get_object(builder, "SensorRefresh");
        g_signal_connect(SensorRefresh, "clicked", get_sensor_values, NULL);
        SensorRefreshRow = gtk_builder_get_object(builder, "SensorRefreshRow");

        // Other buttons
        SleepClock = gtk_builder_get_object(builder, "SleepClock");
        g_signal_connect(SleepClock, "clicked", sleep_clock, NULL);

        RebootClock = gtk_builder_get_object(builder, "RebootClock");
        g_signal_connect(RebootClock, "clicked", reboot_clock, NULL);

        AboutProgram = gtk_builder_get_object(builder, "AboutProgram");
        g_signal_connect(AboutProgram, "clicked", show_about_dialog, NULL);

        Reconfigure = gtk_builder_get_object(builder, "Reconfigure");
        g_signal_connect(Reconfigure, "clicked", reconfigure, TRUE);

        gtk_application_add_window(app, MainWindow);
        gtk_widget_show_all(MainWindow);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_function, NULL) == 0) {
            gtk_widget_set_visible(SensorRefreshRow, FALSE);
        } else {
            get_sensor_values();
            show_message_dialog(MainWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Note", "An error occured while starting the sensor values thread.\nYou have to refresh the sensor values manually.");
        }
    } else if (target == WINDOW_CONNECTION) {
        ConnectionWindow = gtk_builder_get_object(builder, "ConnectionWindow");
        g_signal_connect(ConnectionWindow, "key-press-event", connection_window_key_press, NULL);

        ConnectHostname = gtk_builder_get_object(builder, "ConnectHostname");
        ConnectUsername = gtk_builder_get_object(builder, "ConnectUsername");
        ConnectPassword = gtk_builder_get_object(builder, "ConnectPassword");

        if (credentials_exist()) {
            gtk_entry_set_text(ConnectHostname, g_settings_get_string(credentials, "hostname"));
            gtk_entry_set_text(ConnectUsername, g_settings_get_string(credentials, "username"));
            gtk_entry_set_text(ConnectPassword, g_settings_get_string(credentials, "password"));
        }

        ConnectConfirm = gtk_builder_get_object(builder, "ConnectConfirm");
        g_signal_connect(ConnectConfirm, "clicked", create_connection, NULL);

        ConnectVersion = gtk_builder_get_object(builder, "ConnectVersion");
        gtk_label_set_label(ConnectVersion, VERSION);

        gtk_application_add_window(app, ConnectionWindow);
        gtk_widget_show_all(ConnectionWindow);
    } else if (target == WINDOW_ERROR) {
        ErrorWindow = gtk_builder_get_object(builder, "ErrorWindow");

        CloseAfterError = gtk_builder_get_object(builder, "CloseAfterError");
        g_signal_connect(CloseAfterError, "clicked", quit, NULL);

        RetryAfterError = gtk_builder_get_object(builder, "RetryAfterError");
        g_signal_connect(RetryAfterError, "clicked", reboot_program, NULL);

        ReconfigureAfterError = gtk_builder_get_object(builder, "ReconfigureAfterError");
        g_signal_connect(ReconfigureAfterError, "clicked", reconfigure, FALSE);

        gtk_application_add_window(app, ErrorWindow);
        gtk_widget_show_all(ErrorWindow);
    } else exit(1);

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
