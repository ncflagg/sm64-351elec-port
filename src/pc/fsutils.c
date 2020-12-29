#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(TARGET_RG351) || defined(TARGET_LINUX)
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#endif
#include "fsutils.h"

#ifdef TARGET_RG351

int File_Copy (char FileSource [], char FileDestination [])
{
    int   c;
    FILE *stream_R;
    FILE *stream_W; 

    stream_R = fopen (FileSource, "r");
    if (stream_R == NULL)
        return -1;
    stream_W = fopen (FileDestination, "w");   //create and write to file
    if (stream_W == NULL)
     {
        fclose (stream_R);
        return -2;
     }    
    while ((c = fgetc(stream_R)) != EOF)
        fputc (c, stream_W);
    fclose (stream_R);
    fclose (stream_W);

    return 0;
}

void backupFormerPathSave()
{
    static char fnamepathsrc[2048], fnamepathtarget[2048];
    const char *path = getenv("HOME");

    snprintf(fnamepathsrc, sizeof(fnamepathsrc), "%s/.sm64-port/sm64_save_file.bin", path);
    snprintf(fnamepathtarget, sizeof(fnamepathtarget), "%s/save/sm64_save_file.bin", exePath);

    if(access(fnamepathsrc, F_OK) == 0 && access(fnamepathtarget, F_OK) != 0) {

        File_Copy(fnamepathsrc, fnamepathtarget);

    }

}
#endif

FILE *fopen_home(const char *filename, const char *mode)
{
#if defined(TARGET_RG351) || defined(TARGET_LINUX)
    static char fnamepath[2048];
    const char *path = getenv("HOME");
    struct stat info;

    snprintf(fnamepath, sizeof(fnamepath), "%s/.sm64-port/", path);
    if (stat(fnamepath, &info) != 0) {
        fprintf(stderr, "Creating '%s' for the first time...\n", fnamepath);
        mkdir(fnamepath, 0700);
        if (stat(fnamepath, &info) != 0) {
            fprintf(stderr, "Unable to create '%s': %s\n", fnamepath, strerror(errno));
            abort();
        }
    }

    snprintf(fnamepath, sizeof(fnamepath), "%s/.sm64-port/%s", path, filename);
    
    return fopen(fnamepath, mode);
#else
    return fopen(filename, mode);
#endif
}

FILE *fopen_conf(const char *filename, const char *mode)
{
#if defined(TARGET_RG351) || defined(TARGET_LINUX)
    static char fnamepath[2048];
    struct stat info;

    snprintf(fnamepath, sizeof(fnamepath), "%s/configuration/", exePath);
    if (stat(fnamepath, &info) != 0) {
        fprintf(stderr, "Creating '%s' for the first time...\n", fnamepath);
        mkdir(fnamepath, 0700);
        if (stat(fnamepath, &info) != 0) {
            fprintf(stderr, "Unable to create '%s': %s\n", fnamepath, strerror(errno));
            abort();
        }
    }

    snprintf(fnamepath, sizeof(fnamepath), "%s/configuration/%s", exePath, filename);
    
    return fopen(fnamepath, mode);
#else
    return fopen(filename, mode);
#endif
}

FILE *fopen_save(const char *filename, const char *mode)
{
#if defined(TARGET_RG351) || defined(TARGET_LINUX)
    static char fnamepath[2048];
    struct stat info;

    backupFormerPathSave();

    snprintf(fnamepath, sizeof(fnamepath), "%s/save/", exePath);
    if (stat(fnamepath, &info) != 0) {
        fprintf(stderr, "Creating '%s' for the first time...\n", fnamepath);
        mkdir(fnamepath, 0700);
        if (stat(fnamepath, &info) != 0) {
            fprintf(stderr, "Unable to create '%s': %s\n", fnamepath, strerror(errno));
            abort();
        }
    }

    snprintf(fnamepath, sizeof(fnamepath), "%s/save/%s", exePath, filename);
    
    return fopen(fnamepath, mode);
#else
    return fopen(filename, mode);
#endif
}
