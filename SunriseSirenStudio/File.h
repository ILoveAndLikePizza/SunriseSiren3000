#ifndef _file_h
#define _file_h

gchar* config_file_path[PATH_MAX];

gboolean file_exists(const gchar *fname) {
    FILE *f;
    if ((f = fopen(fname, "r"))) {
        fclose(f);
        return TRUE;
    }
    return FALSE;
}

void file_write(const gchar *fname, const gchar* contents) {
    if (file_exists(fname)) {
        FILE *f = fopen(fname, "w");
        fprintf(f, contents);
        fclose(f);
    } else {
        g_print("Error while writing file: it does not exist.");
    }
}

#endif
