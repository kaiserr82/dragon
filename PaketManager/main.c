#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <errno.h>

#define BUFFER_SIZE 4096

// FTP Download Callback
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// FTP Download Funktion
int download_file(const char *ftp_url, const char *output_path) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Fehler bei curl_init\n");
        return 1;
    }

    fp = fopen(output_path, "wb");
    if (!fp) {
        perror("Datei konnte nicht geöffnet werden");
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, ftp_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Fehler beim Herunterladen: %s\n", curl_easy_strerror(res));
        return 1;
    }

    return 0;
}

// Datei kopieren
int copy_file(const char *src, const char *dst) {
    FILE *src_file = fopen(src, "rb");
    if (!src_file) {
        perror("Quelle konnte nicht geöffnet werden");
        return 1;
    }

    FILE *dst_file = fopen(dst, "wb");
    if (!dst_file) {
        perror("Ziel konnte nicht erstellt werden");
        fclose(src_file);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes;
    while ((bytes = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        fwrite(buffer, 1, bytes, dst_file);
    }

    fclose(src_file);
    fclose(dst_file);
    return 0;
}

// Datei löschen
int delete_file(const char *path) {
    if (remove(path) == 0) {
        return 0;
    } else {
        perror("Fehler beim Löschen");
        return 1;
    }
}

// Datei lesen
void read_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Fehler beim Öffnen der Datei");
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    fclose(file);
}

// Datei schreiben
void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    if (!file) {
        perror("Fehler beim Schreiben der Datei");
        return;
    }

    fprintf(file, "%s", text);
    fclose(file);
}

// tar.gz entpacken
int extract_tar_gz(const char *archive, const char *target_dir) {
    char command[512];
    snprintf(command, sizeof(command), "mkdir -p %s && tar -xzf %s -C %s", target_dir, archive, target_dir);
    return system(command);
}

// Hauptprogramm
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [install|update|remove]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "install") == 0) {
        printf("Installiere Paket...\n");
        download_file("ftp://example.com/path/to/package.tar.gz", "package.tar.gz");
        extract_tar_gz("package.tar.gz", "./install_dir");
        copy_file("./install_dir/file.txt", "/tmp/copied_file.txt");
        write_file("/tmp/info.txt", "Installation abgeschlossen.\n");
    } else if (strcmp(argv[1], "update") == 0) {
        printf("Aktualisiere Paket...\n");
        download_file("ftp://example.com/path/to/package.tar.gz", "package_update.tar.gz");
        extract_tar_gz("package_update.tar.gz", "./update_dir");
        write_file("/tmp/info.txt", "Update durchgeführt.\n");
    } else if (strcmp(argv[1], "remove") == 0) {
        printf("Entferne Paket...\n");
        delete_file("/tmp/copied_file.txt");
        delete_file("/tmp/info.txt");
        write_file("log.txt", "Paket entfernt.\n");
    } else {
        printf("Unbekannter Befehl: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
