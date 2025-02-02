#ifndef _dialog_h
#define _dialog_h

gint show_message_dialog(GtkWindow *parent, GtkMessageType type, GtkButtonsType buttons, gchar* title, gchar* text) {
    GtkMessageDialog *message_dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, type, buttons, text);
    gtk_window_set_title(message_dialog, title);

    gint *response = gtk_dialog_run(message_dialog);

    gtk_widget_destroy(message_dialog);
    return response;
}

void show_about_dialog() {

}

#endif
