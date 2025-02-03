#include "../include/SunriseSirenStudio.h"

#include "../include/UI.h"
#include "../include/Dialog.h"
#include "../include/String.h"
#include "../include/Requests.h"
#include "../include/ConfigFile.h"

#include "../include/Color.h"
#include "../include/Connection.h"

enum TargetWindow {
    WINDOW_MAIN,
    WINDOW_CONNECTION,
    WINDOW_ERROR
};

void quit() {
    exit(0);
}

void reboot() {
    execl("/proc/self/exe", argv0, NULL);

    perror("An error occured while rebooting Sunrise Siren Studio.");
    exit(2);
}

void reboot_clock() {

}

void reconfigure() {
    config_file_delete();
    reboot();
}

void get_sensor_values() {
    gchar* sensors_url[PATH_MAX];
    sprintf(sensors_url, "http://%s/sensors", hostname);
    gchar *sensors_response = request(sensors_url, username, password);

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

static void onActivate(GtkApplication *app, gpointer user_data) {
    curl_init();
    sprintf(config_file_path, "%s/.config/SunriseSirenStudio.json", g_getenv("HOME"));

    enum TargetWindow target;

    // step 1: check if the config file exists
    if (config_file_exists()) {
        // step 2: parse the config file
        if (config_file_parse()) {
            // step 3: fetch /status en /sensors
            gchar* status_url[PATH_MAX];
            sprintf(status_url, "http://%s/status", hostname);
            gchar *status_response = request(status_url, username, password);

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
            // error while parsing config file
            target = WINDOW_ERROR;
        }

    } else {
        // no configuration made, do that first
        target = WINDOW_CONNECTION;
    }

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, UI, -1, NULL);

    if (target == WINDOW_MAIN) {
        default_color = g_new(GdkRGBA, 1);
        alarm_color = g_new(GdkRGBA, 1);
        default_color->alpha = 1;
        alarm_color->alpha = 1;

        // MainWindow
        MainWindow = gtk_builder_get_object(builder, "MainWindow");
        // Clock settings tab
        // Colors
        DefaultColor = gtk_builder_get_object(builder, "DefaultColor");
        g_signal_connect(DefaultColor, "clicked", pick_default_color, NULL);

        AlarmColor = gtk_builder_get_object(builder, "AlarmColor");
        g_signal_connect(AlarmColor, "clicked", pick_alarm_color, NULL);

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
            gtk_switch_set_active(AlarmEnable[i], (alarms_enabled & (gint) pow(2, i)) > 0);

            gtk_spin_button_set_adjustment(AlarmHour[i], gtk_adjustment_new(atoi(hour), 0, 24, 1, 1, 1));
            gtk_spin_button_set_numeric(AlarmHour[i], TRUE);
            gtk_spin_button_set_wrap(AlarmHour[i], TRUE);

            gtk_spin_button_set_adjustment(AlarmMinute[i], gtk_adjustment_new(atoi(minute), 0, 60, 1, 1, 1));
            gtk_spin_button_set_numeric(AlarmMinute[i], TRUE);
            gtk_spin_button_set_wrap(AlarmMinute[i], TRUE);
        }

        // Advanced
        EnableLeadingZero = gtk_builder_get_object(builder, "EnableLeadingZero");
        gtk_switch_set_active(EnableLeadingZero, json_object_get_boolean(json_object_object_get(clock_status, "leadingZero")));

        EnableDST = gtk_builder_get_object(builder, "EnableDST");
        gtk_switch_set_active(EnableDST, json_object_get_boolean(json_object_object_get(clock_status, "enableDST")));

        ClockReturn = gtk_builder_get_object(builder, "ClockReturn");
        gtk_spin_button_set_value(ClockReturn, json_object_get_int(json_object_object_get(clock_status, "clockReturn")));

        LDRMin = gtk_builder_get_object(builder, "LDRMin");
        gtk_spin_button_set_value(LDRMin, json_object_get_int(json_object_object_get(json_object_object_get(clock_status, "ldr"), "min")));

        LDRMax = gtk_builder_get_object(builder, "LDRMax");
        gtk_spin_button_set_value(LDRMax, json_object_get_int(json_object_object_get(json_object_object_get(clock_status, "ldr"), "max")));

        // Custom
        // TODO

        // Miscellaneous
        // Sensor information
        LDRReading = gtk_builder_get_object(builder, "LDRReading");
        SHT21TemperatureReading = gtk_builder_get_object(builder, "SHT21TemperatureReading");
        SHT21HumidityReading = gtk_builder_get_object(builder, "SHT21HumidityReading");
        
        SensorRefresh = gtk_builder_get_object(builder, "SensorRefresh");
        g_signal_connect(SensorRefresh, "clicked", get_sensor_values, NULL);
        get_sensor_values();

        AboutProgram = gtk_builder_get_object(builder, "AboutProgram");

        RebootClock = gtk_builder_get_object(builder, "RebootClock");

        Reconfigure = gtk_builder_get_object(builder, "Reconfigure");
        g_signal_connect(Reconfigure, "clicked", reconfigure, NULL);


        gtk_application_add_window(app, MainWindow);
        gtk_widget_show_all(MainWindow);
    } else if (target == WINDOW_CONNECTION) {
        ConnectionWindow = gtk_builder_get_object(builder, "ConnectionWindow");

        ConnectHostname = gtk_builder_get_object(builder, "ConnectHostname");
        ConnectUsername = gtk_builder_get_object(builder, "ConnectUsername");
        ConnectPassword = gtk_builder_get_object(builder, "ConnectPassword");

        ConnectConfirm = gtk_builder_get_object(builder, "ConnectConfirm");
        g_signal_connect(ConnectConfirm, "clicked", create_connection, NULL);

        gtk_application_add_window(app, ConnectionWindow);
        gtk_widget_show_all(ConnectionWindow);
    } else if (target == WINDOW_ERROR) {
        ErrorWindow = gtk_builder_get_object(builder, "ErrorWindow");

        CloseAfterError = gtk_builder_get_object(builder, "CloseAfterError");
        g_signal_connect(CloseAfterError, "clicked", quit, NULL);

        RetryAfterError = gtk_builder_get_object(builder, "RetryAfterError");
        g_signal_connect(RetryAfterError, "clicked", reboot, NULL);

        ReconfigureAfterError = gtk_builder_get_object(builder, "ReconfigureAfterError");
        g_signal_connect(ReconfigureAfterError, "clicked", reconfigure, NULL);

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
