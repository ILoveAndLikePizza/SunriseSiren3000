#ifndef _file_h
#define _file_h

#ifndef PATH_MAX
    #define PATH_MAX 2048 // just to make VSCode stop complaining
#endif


gchar* config_file_path[PATH_MAX];

gboolean config_file_exists() {
    FILE *f;
    if ((f = fopen(config_file_path, "r"))) {
        fclose(f);
        return TRUE;
    }
    return FALSE;
}

gboolean config_file_parse() {
    json_object *config_file = json_object_from_file(config_file_path);

    json_object *json_hostname = json_object_object_get(config_file, "hostname");
    json_object *json_username = json_object_object_get(config_file, "username");
    json_object *json_password = json_object_object_get(config_file, "password");

    if (!json_hostname || !json_username || !json_password) return FALSE;
    
    sprintf(hostname, json_object_get_string(json_hostname));
    sprintf(username, json_object_get_string(json_username));
    sprintf(password, json_object_get_string(json_password));

    return TRUE;
}

void config_file_write(gchar* hostname, gchar* username, gchar* password) {
    json_object *config_file = json_object_new_object();

    json_object_object_add(config_file, "hostname", json_object_new_string(hostname));
    json_object_object_add(config_file, "username", json_object_new_string(username));
    json_object_object_add(config_file, "password", json_object_new_string(password));

    json_object_to_file_ext(config_file_path, config_file, JSON_C_TO_STRING_PRETTY);
}

gboolean config_file_delete() {
    if (config_file_exists()) return (remove(config_file_path) == 0);

    return TRUE;
}

#endif
