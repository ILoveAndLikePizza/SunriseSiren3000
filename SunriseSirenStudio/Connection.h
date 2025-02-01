#ifndef _connection_h
#define _connection_h

#define VALIDATION_STRING "Yes, a Sunrise Siren 3000 is here!"

static void create_connection(GtkWidget *widget, gpointer user_data) {
    gchar* hostname = gtk_entry_get_text(ConnectHostname);
    gchar* username = gtk_entry_get_text(ConnectUsername);
    gchar* password = gtk_entry_get_text(ConnectPassword);

    gchar* url[PATH_MAX];
    sprintf(url, "http://%s/connect", hostname);

    gchar* req = request(url, username, password);
    g_print("Result is %s", req);
}

#endif
