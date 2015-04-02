#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "./giflib-5.1.1/lib/gif_lib.h"

const int MAX_IMAGES = 4;

void copyGifToWriteHandle(GifFileType* image, GifFileType* output, int frames) {
    output->SWidth = image->SWidth;
    output->SHeight = image->SHeight;
    output->SColorResolution = image->SColorResolution;
    output->SBackGroundColor = image->SBackGroundColor;
    output->SColorMap = GifMakeMapObject(image->SColorMap->ColorCount, image->SColorMap->Colors);
    
    for (int j = 0; j < frames; j++) {
        fprintf(stderr, "Frame %d\n", frames);
        for (int i = 0; i < image->ImageCount; i++) {
            fprintf(stderr, "Copied %d images.\n", image->ImageCount);
            GifMakeSavedImage(output, &image->SavedImages[i]);
        }
    }
}

void printStats(GifFileType *fileType) {
    fprintf(stderr, "\tWidth: %d Height: %d ImageCount: %d SavedImages: %p\n", fileType->SWidth, fileType->SHeight, fileType->ImageCount, fileType->SavedImages);
}

GifFileType *readFileOrExit(char* fname, int *fileHandle, int *errorCode) {
    GifFileType *result;
    int error;
    fprintf(stderr,"Openning file %s\n", fname);
    if ((*fileHandle = open(fname, O_RDONLY)) == -1) {
        fprintf(stderr, "Error occurred while opening file handle %s.\n", fname);
        error = errno;
        perror("The error was");
        exit(-2);
    }
    
    fprintf(stderr,"Openning read handle %s\n", fname);
    if ((result = DGifOpenFileHandle(*fileHandle, errorCode)) == NULL) {
        fprintf(stderr, "Could not read gif in file %s.\n", fname);
        exit(-3);
    }
    
    fprintf(stderr,"Reading %s\n", fname);
    if (DGifSlurp(result) == GIF_ERROR) {
        fprintf(stderr,"Slurp while reading %s\n", fname);
        exit(-4);
    }
    printStats(result);
    return result;
}

GifFileType* openFileForWriteOrExit(char *fname, int *fileHandle, int *errorCode, GifFileType *background, int frames) {
    GifFileType *result;
    int error;
    fprintf(stderr,"Openning file for write %s\n", fname);
    if ((*fileHandle = open(fname, O_WRONLY | O_CREAT | O_TRUNC)) == -1) {
        fprintf(stderr,"An error occured while opening file handle %s.\n", fname);
        error = errno;
        perror("The error was");
        exit(-2);
    }
    
    fprintf(stderr, "Preparing write file handle %s\n", fname);
    result = EGifOpenFileHandle(*fileHandle, errorCode);
    if (result == NULL) {
        fprintf(stderr, "EGifOpenFileHandle for %s errored.\n", fname);
        exit(-3);
    }
    fprintf(stderr, "Copying background to write handle.\n");
    copyGifToWriteHandle(background, result, frames);
    printStats(result);
    
    return result;
}

GifByteType *skipToStart(int x, int y, GifFileType *file) {
    int toSkip = file->SWidth * y + x;
    return (GifByteType *)file->SavedImages->RasterBits + toSkip;
}

void composite(GifFileType *image, int x, int y, GifFileType *background) {
    if (x < 0 || y < 0 || x + image->SWidth > background->SWidth || image->SHeight > background->SHeight) {
        fprintf(stderr, "Composite image is out of range!\n\tImage width: %d height: %d\n\tx: %d y: %d\n\tBackground width: %d height: %d", image->SWidth, image->SHeight, x, y, background->SWidth, background->SHeight);
    }
    
    GifByteType *imageByte, *backgroundByte;
    imageByte = image->SavedImages->RasterBits;
    backgroundByte = skipToStart(x, y, background);
    for (int i = 0; i < image->SHeight; i++) {
        // Copy row
        for (int j = 0; j < image->SWidth; j++) {
            *(backgroundByte++) = *(imageByte++);
        }
        
        backgroundByte += background->SWidth - image->SWidth;
    }
}


char *concatenate(char *a, char *b) {
    unsigned long size = strlen(a) + strlen(b) + 1;
    char *result = calloc(size, sizeof(char));
    memcpy(result, a, strlen(a));
    memcpy(result + strlen(a), b, strlen(b));
    *(result + strlen(a) + strlen(b)) = '\0';
    return result;
}

struct ImageEntry {
    char* filename;
    int handle;
    GifFileType *gRead;
};

struct CompositeFiles {
    int background;
    struct ImageEntry *iBack;
    
    int days;
    struct ImageEntry *iDays;
    
    int hours;
    struct ImageEntry *iHours;
    
    int minutes;
    struct ImageEntry *iMinutes;
    
    int seconds;
    struct ImageEntry *iSeconds;
    
    int result;
};

int openReadFileHandle(char* file) {
    fprintf(stderr, "Openning read handle for %s\n", file);
    int result = open(file, O_RDONLY);
    if (result == -1) {
        perror("An error occured");
        exit(-1);
    }
    return result;
}

GifFileType *gifRead(char *fname, int fHandle) {
    GifFileType *result;
    int errorCode;
    fprintf(stderr,"Openning gif read handle %s\n", fname);
    if ((result = DGifOpenFileHandle(fHandle, &errorCode)) == NULL) {
        fprintf(stderr, "Could not read gif in file %s.\n", fname);
        exit(-3);
    }
    
    fprintf(stderr,"Performing gif read %s\n", fname);
    if (DGifSlurp(result) == GIF_ERROR) {
        fprintf(stderr,"Slurp while reading %s\n", fname);
        exit(-4);
    }
    return result;
}

struct ImageEntry *openFile(char *fname) {
    struct ImageEntry *image = malloc(sizeof(struct ImageEntry));
    image->filename = fname;
    image->handle = openReadFileHandle(fname);
    image->gRead = gifRead(fname, image->handle);
    return image;
}

void openAndReadAllFilesInDirectory(char *dirPath, struct ImageEntry **images, int maxCount) {
    fprintf(stderr, "Openning up to %d files in %s\n", maxCount, dirPath);
    DIR *dir = opendir(dirPath);
    if (dir == NULL) {
        perror("An error occurred while reading directory.\n");
        return;
    }
    int count = 0;
    struct dirent *de;
    while ( (de = readdir(dir)) != 0 && count < maxCount) {
        if (de->d_type & DT_REG) {
            char *filePath = concatenate(dirPath, de->d_name);
            *images = openFile(filePath);
            count++;
            images++;
        }
    }
    closedir(dir);
}

int *newMaxIntArray(int size, int exitStatus) {
    int *result = calloc(size, sizeof(int));
    if (result == NULL) {
        fprintf(stderr, "Insufficient memory\n");
        exit(exitStatus);
    }
    return result;
}

GifFileType **newGifFileTypeArray(int size, int exitStatus) {
    GifFileType **result = calloc(size, sizeof(GifFileType *));
    if (result == NULL) {
        fprintf(stderr, "Insufficient memory\n");
        exit(exitStatus);
    }
    return result;
}

struct ImageEntry *newImageEntryArray(int size, int exitStatus) {
    struct ImageEntry *result = calloc(size, sizeof(struct ImageEntry *));
    if (result == NULL) {
        fprintf(stderr, "Insufficient memory\n");
        exit(exitStatus);
    }
    return result;
}

void displayImageEntry(struct ImageEntry *ie) {
    fprintf(stderr, "\tie: %p", ie);
    fprintf(stderr, "\tfilename: %s handle: %d gif: %p\n", ie->filename, ie->handle, ie->gRead);
}

void displayCompositeFiles(struct CompositeFiles *cp) {
    fprintf(stderr, "Composite file (%p)\n", cp);
    fprintf(stderr, "\tbackground: %d days: %d hours: %d minutes: %d seconds: %d\n", cp->background, cp->days, cp->hours, cp->minutes, cp->seconds);
    struct ImageEntry *work = cp->iBack;
    for (int i = 0; i < cp->background; i++) {
        fprintf(stderr, "\tback[%p] %d\n", i, *work);
        if (work != NULL) {
            displayImageEntry(work);
        }
        work++;
    }
    work = cp->iDays;
    for (int i = 0; i < cp->days; i++) {
        fprintf(stderr, "\tdays[%p] %d\n", i, *work);
        if (work != NULL) {
            displayImageEntry(work);
        }
        work++;
    }
}

struct CompositeFiles *openCompositeFiles(char* root) {
    fprintf(stderr, "Openning composite files\n");
    
    struct CompositeFiles *cp = calloc(1, sizeof(struct CompositeFiles));

    fprintf(stderr, "BACKGROUND\n");
    cp->background = 1;
    cp->iBack = newImageEntryArray(MAX_IMAGES, -1);
    char *backgroundPath = concatenate(root,"/background/");
    openAndReadAllFilesInDirectory(backgroundPath, cp->iBack, cp->background);
    displayCompositeFiles(cp);
    free(backgroundPath);
    
    fprintf(stderr, "DAYS\n");
    cp->days = MAX_IMAGES;
    cp->iDays = newImageEntryArray(MAX_IMAGES, -2);
    char *dayPath = concatenate(root, "/days/");
    openAndReadAllFilesInDirectory(dayPath, cp->iDays, cp->days);
    displayCompositeFiles(cp);
    free(dayPath);
    
    fprintf(stderr, "HOURS\n");
    cp->hours = MAX_IMAGES;
    cp->iHours = newImageEntryArray(MAX_IMAGES, -3);
    char *hourPath = concatenate(root, "/hours/");
    openAndReadAllFilesInDirectory(dayPath, cp->iHours, cp->hours);
    displayCompositeFiles(cp);
    free(hourPath);
    
    fprintf(stderr, "MINUTES\n");
    cp->minutes = MAX_IMAGES;
    cp->iMinutes = newImageEntryArray(MAX_IMAGES, -4);
    char *minutePath = concatenate(root, "/minutes/");
    openAndReadAllFilesInDirectory(dayPath, cp->iMinutes, cp->minutes);
    displayCompositeFiles(cp);
    free(minutePath);
    
    fprintf(stderr, "SECONDS\n");
    cp->seconds = MAX_IMAGES;
    cp->iSeconds = newImageEntryArray(MAX_IMAGES, -5);
    char *secondPath = concatenate(root, "/seconds/");
    openAndReadAllFilesInDirectory(dayPath, cp->iSeconds, cp->seconds);
    displayCompositeFiles(cp);
    free(secondPath);
    
    fprintf(stderr, "Openned composite files %p\n", cp);
    return cp;
}

void closeCompositeFiles(struct CompositeFiles *cp) {
    free(cp);
    fprintf(stderr, "!!! Closed composite files %p\n", cp);
}

int main(int argc, char* argv[]) {
    struct CompositeFiles *cf;
    cf = openCompositeFiles(argv[1]);
    closeCompositeFiles(cf);
    
    // Open read files
    char* backgroundFilename = argv[1], *imageFilename = argv[2], *resultFilename = argv[3];
    int frames = 2;
    fprintf(stderr,"Background file: %s\nImage file: %s\nResult file: %s\n", argv[1], argv[2], argv[3]);
    
    int fbackground, fimage, fresult; // File handle, not file descriptor
    
    // Read gif files to memory
    int errorCode = 0;
    GifFileType *gbackground, *gimage;
    gbackground = readFileOrExit(backgroundFilename, &fbackground, &errorCode);
    gimage = readFileOrExit(imageFilename, &fimage, &errorCode);
    
    // Open gif handle for writing
    GifFileType* gifWriteHandle = openFileForWriteOrExit(resultFilename, &fresult, &errorCode, gbackground, frames);
    
    // DO STUFF HERE
    int x = 41;
    int y = 205;
    composite(gimage, x, y, gifWriteHandle);
    
    // Write output gif to file
    fprintf(stderr, "Writing output contents to disk.\n");
    if (EGifSpew(gifWriteHandle) == GIF_ERROR) {
        fprintf(stderr, "Spew errored: %s\n", GifErrorString(errorCode));
    }
    
    // Close and deallocate memory
    fprintf(stderr, "Closing.\n");
    if (DGifCloseFile(gbackground, &errorCode) == GIF_ERROR) {
        fprintf(stderr, "Error occurred while closing background file.\n\t%s\n", GifErrorString(errorCode));
    }
    fprintf(stderr, "DClose complete\n");
    if (DGifCloseFile(gimage, &errorCode) == GIF_ERROR) {
        fprintf(stderr, "Error occurred while closing image file.\n\t%s\n", GifErrorString(errorCode));
    }
    fprintf(stderr,"DClose complete\n");
    if (EGifCloseFile(gifWriteHandle, &errorCode) == GIF_ERROR) {
        fprintf(stderr, "Error occurred while closing output file.\n\t%s\n", GifErrorString(errorCode));
    }
    fprintf(stderr,"EClose complete");
    
    return 0;
}

// Executed by ruby require
void Init_composite() {
    printf("Initialising composite");
    VALUE mGiflib = rb_define_module("Composite");
    VALUE cGiflibImage = rb_define_class_under(mGiflib, "Image", rb_cObject);
}
