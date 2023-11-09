#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    openlog("writer", LOG_PID, LOG_USER);

    if (argc != 3) {
        syslog(LOG_ERR, "Usage: %s <file_path> <text_to_write>", argv[0]);
        closelog();
        return 1;
    }

    const char *file_path = argv[1];
    const char *text_to_write = argv[2];

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error: Unable to open or create file: %s", file_path);
        closelog();
        return 1;
    }

    fprintf(file, "%s", text_to_write);
    fclose(file);

    syslog(LOG_DEBUG, "Writing \"%s\" to \"%s\"", text_to_write, file_path);

    closelog();

    return 0;
}
