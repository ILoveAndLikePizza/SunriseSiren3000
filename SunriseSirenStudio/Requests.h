#ifndef _requests_h
#define _requests_h

#include <curl/curl.h>

CURL *curl;

void curl_init() {
    curl_global_init(CURL_GLOBAL_ALL);
}

gchar* request(gchar* url, gchar* username, gchar* password) {
    curl = curl_easy_init();
    if (!curl) return "Error while initializing CURL";

    gchar *userpwd[strlen(username) + strlen(password) + 1];
    sprintf(userpwd, "%s:%s", username, password);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERPWD, str_escape(userpwd, ':'));
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 8L);
}

#endif
