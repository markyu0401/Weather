#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

// Structure to hold the response data
struct string {
    char *ptr;
    size_t len;
};

// Function to initialize the string structure
void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

// Callback function for writing response data
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

int main(void) {
    CURL *curl;
    CURLcode res;
    struct string s;
    init_string(&s);  // Initialize the string structure
    char city[100];  // Buffer to hold the city name

    // Ask the user for the city
    printf("Enter the city: ");
    scanf("%99s", city);  // Read the city name into the buffer

    // Construct the URL with the user's city
    char url[256];  // Buffer for the URL
    snprintf(url, sizeof(url), "http://api.weatherstack.com/current?access_key=1b4a4b9200a67466d2f3dae2bbec327e&query=%s", city);

    curl = curl_easy_init(); // Initialize the curl session
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            // Parse JSON and output the weather data
            struct json_object *parsed_json;
            struct json_object *current;
            struct json_object *temperature;

            parsed_json = json_tokener_parse(s.ptr);
            json_object_object_get_ex(parsed_json, "current", &current);
            json_object_object_get_ex(current, "temperature", &temperature);

            printf("Current Temperature: %d°C\n", json_object_get_int(temperature));

            json_object_put(parsed_json); // Free json object
        }

        curl_easy_cleanup(curl); // Clean up the curl session
    }

    free(s.ptr); // Free the memory allocated for the response data
    return 0; // Return success
}
