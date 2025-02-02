#ifndef _connection_h
#define _connection_h

#define VALIDATION_STRING "Yes, a Sunrise Siren 3000 is here!"

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

    gchar* req = request(url, username, password);
    
    if (strstr(req, VALIDATION_STRING)) {
        // authentication successful, save and continue
        show_message_dialog(ConnectionWindow, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Yes!", "Connection successful!");
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
