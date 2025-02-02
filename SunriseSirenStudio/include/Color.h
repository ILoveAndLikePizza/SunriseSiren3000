#ifndef _color_h
#define _color_h

static void pick_default_color(GtkWidget *widget, gpointer user_data) {
    GtkColorSelectionDialog *dialog = gtk_color_selection_dialog_new("Select default color");
    GtkWidget *selection = gtk_color_selection_dialog_get_color_selection(dialog);
    gtk_color_selection_set_current_rgba(selection, default_color);
    if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
        gtk_color_selection_set_current_rgba(selection, default_color);
        gtk_widget_override_background_color(DefaultColor, GTK_STATE_NORMAL, default_color);
    }

    gtk_widget_destroy(dialog);
    g_object_unref(dialog);
}

static void pick_alarm_color(GtkWidget *widget, gpointer user_data) {
    GtkColorSelectionDialog *dialog = gtk_color_selection_dialog_new("Select alarm color");
    GtkWidget *selection = gtk_color_selection_dialog_get_color_selection(dialog);
    gtk_color_selection_set_current_rgba(selection, alarm_color);
    if (gtk_dialog_run(dialog) == GTK_RESPONSE_OK) {
        gtk_color_selection_set_current_rgba(selection, alarm_color);
        gtk_widget_override_background_color(AlarmColor, GTK_STATE_NORMAL, alarm_color);
    }

    gtk_widget_destroy(dialog);
    g_object_unref(dialog);
}

#endif
