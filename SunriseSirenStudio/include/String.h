#ifndef _string_h
#define _string_h

gchar* str_escape(gchar* input, gchar to_escape) {
    gint len = strlen(input);
    gchar* escaped = (gchar*)g_malloc(2 * len + 1);
    gint j = 0;

    for (gint i=0; i<len; i++) {
        if (input[i] == to_escape) {
            escaped[j++] = '\\';
        }
        escaped[j++] = input[i];
    }

    escaped[j] = '\0';
    return escaped;
}

#endif
