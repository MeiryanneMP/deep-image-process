#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

float* extract_image_features(const char* file_path, int bins) {
    int w, h, ch;
    unsigned char* img = stbi_load(file_path, &w, &h, &ch, 3);
    if (!img) return NULL;

    int total = w * h;
    float* hist = calloc(bins * 3, sizeof(float));
    float bin_size = 256.0f / bins;

    for (int i = 0; i < total; i++) {
        unsigned char r = img[i * 3 + 0];
        unsigned char g = img[i * 3 + 1];
        unsigned char b = img[i * 3 + 2];

        int ir = r / bin_size;
        int ig = g / bin_size;
        int ib = b / bin_size;

        if (ir >= bins) ir = bins - 1;
        if (ig >= bins) ig = bins - 1;
        if (ib >= bins) ib = bins - 1;

        hist[ir]++;
        hist[bins + ig]++;
        hist[bins * 2 + ib]++;
    }

    float sum = 0;
    for (int i = 0; i < bins * 3; i++) sum += hist[i];
    if (sum > 0)
        for (int i = 0; i < bins * 3; i++)
            hist[i] /= sum;

    stbi_image_free(img);
    return hist;
}

int is_image(const char* name) {
    return strstr(name, ".jpg") || strstr(name, ".png") || strstr(name, ".jpeg");
}

int main() {
    const char* dir = getenv("IMAGE_DATA_DIR");
    if (!dir) return 1;

    DIR* d = opendir(dir);
    if (!d) return 1;

    FILE* out = fopen("features.csv", "w");
    if (!out) return 1;

    int bins = 32;

    fprintf(out, "filename");
    for (int i = 0; i < bins * 3; i++) fprintf(out, ",f%d", i);
    fprintf(out, "\n");

    struct dirent* e;

    while ((e = readdir(d)) != NULL) {
        if (!is_image(e->d_name)) continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);

        float* feat = extract_image_features(path, bins);
        if (!feat) continue;

        fprintf(out, "%s", e->d_name);
        for (int i = 0; i < bins * 3; i++)
            fprintf(out, ",%f", feat[i]);
        fprintf(out, "\n");

        free(feat);
    }

    fclose(out);
    closedir(d);
    return 0;
}
