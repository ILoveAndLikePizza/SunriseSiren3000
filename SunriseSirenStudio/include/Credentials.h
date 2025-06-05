#ifndef _credentials_h
#define _credentials_h

gboolean credentials_exist() {
    return (
        g_utf8_strlen(g_settings_get_string(credentials, "hostname"), 32) > 0 &&
        g_utf8_strlen(g_settings_get_string(credentials, "username"), 32) > 0 &&
        g_utf8_strlen(g_settings_get_string(credentials, "password"), 32) > 0
    );
}

void set_connect_immediately(gboolean to) {
    g_settings_set_boolean(credentials, "connect-immediately", to);

    g_settings_apply(credentials);
    g_settings_sync();
}

void credentials_read() {
    sprintf(hostname, g_settings_get_string(credentials, "hostname"));
    sprintf(username, g_settings_get_string(credentials, "username"));
    sprintf(password, g_settings_get_string(credentials, "password"));
}

void credentials_write(gchar* hostname, gchar* username, gchar* password) {
    g_settings_set_string(credentials, "hostname", hostname);
    g_settings_set_string(credentials, "username", username);
    g_settings_set_string(credentials, "password", password);

    g_settings_apply(credentials);
    g_settings_sync();
}

void credentials_reset() {
    g_settings_reset(credentials, "connect-immediately");
    g_settings_reset(credentials, "hostname");
    g_settings_reset(credentials, "username");
    g_settings_reset(credentials, "password");

    g_settings_apply(credentials);
    g_settings_sync();
}

#endif
