#ifndef _requests_h
#define _requests_h

gint request_last_status_code = 0;

size_t write_memory_callback(void *ptr, size_t size, size_t nmemb, void *data) {
    memcpy(data, ptr, size * nmemb);
    return size * nmemb;
}

gchar* request(gchar* method, gchar* url, gchar* username, gchar* password,  gchar* post_data) {
    CURL *curl = curl_easy_init();
    if (!curl) return "Error while initializing CURL";

    gchar *userpwd[strlen(username) + strlen(password) + 32];
    sprintf(userpwd, "%s:%s", str_escape(username, ':'), str_escape(password, ':'));

    gchar *response_buffer = g_malloc(2048 * sizeof(gchar));
    if (!response_buffer) return NULL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);
    CURLcode response = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &request_last_status_code);
    if (response == CURLE_OK) return response_buffer;

    curl_global_cleanup();
    g_free(response_buffer);
    return NULL;
}

#endif
