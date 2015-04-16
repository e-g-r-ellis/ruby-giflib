#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "./giflib-5.1.1/lib/gif_lib.h"

/*
 Ruby binding
 */
struct RubyString {
    char *text;
    int length;
    int current;
};

int readGifFromMemory(GifFileType *fileType, GifByteType *buffer, int count) {
    int i, remainingSpace;
    struct RubyString *rubyString;
    rubyString = (struct RubyString *)fileType->UserData;    // Set by DGifOpen()

    // No data then stop!
    remainingSpace = rubyString->length - rubyString->current;
    if (count > remainingSpace) {
    	count = remainingSpace;
    }
    
    for (i = 0; i < count; i++) {
        buffer[i] = rubyString->text[rubyString->current + i];
    }
    rubyString->current += count;
    return count;
}

struct RubyImage {
    GifFileType *gifFileType;
};

static struct RubyString *getRubyString(VALUE rString) {
    struct RubyString *result;

    Check_Type(rString, T_STRING);
    result = calloc(1, sizeof(struct RubyString));

    if (result != NULL) {
	result->length = RSTRING_LEN(rString);
	char *newText = calloc(result->length, sizeof(char));
	if (newText == NULL) {
		rb_raise(rb_eException, "Insufficient memory\n");
	}
	memcpy(newText, RSTRING_PTR(rString), result->length);
    	//result->text = RSTRING_PTR(rString);
	result->text = newText;
	printf("\tGot string from ruby, length: %d\n", result->length);
    	result->current = 0;
    } else {
	rb_raise(rb_eException, "Insufficient memory\n");
    }

    return result;
}

static void deallocate(void * rubyImage) {
    free(rubyImage);
}

static VALUE allocate(VALUE klass) {
    struct RubyImage * rubyImage = calloc(1, sizeof(struct RubyImage));
    printf("Allocating...\n");
    return Data_Wrap_Struct(klass, NULL, deallocate, rubyImage);
}

static void copyGifImage(GifFileType *out, GifFileType *gifFileType) {
	int i;
	out->SWidth = gifFileType->SWidth;
        out->SHeight = gifFileType->SHeight;
        out->SColorResolution = gifFileType->SColorResolution;
        out->SBackGroundColor = gifFileType->SBackGroundColor;
        out->SColorMap = GifMakeMapObject(gifFileType->SColorMap->ColorCount, gifFileType->SColorMap->Colors);
        for (i = 0; i < gifFileType->ImageCount; i++) {
                fprintf(stderr, "Copied %d images.\n", gifFileType->ImageCount);
                GifMakeSavedImage(out, &(gifFileType)->SavedImages[i]);
        }
}

static void writeToFile(GifFileType *gifFileType, char *fname) {
	GifFileType *out;
	int fd, i, errorCode, spew;
	// Open write gif
	fd = open(fname, O_WRONLY | O_APPEND | O_CREAT);
	if (fd < 0) {
		rb_raise(rb_eException, "Could not open '%s' file descriptor.", fname);
	}
	printf("Opened '%s' for writing.\n", fname);
	out = EGifOpenFileHandle(fd, &errorCode);
	if (out == NULL) {
		rb_raise(rb_eException, "Could not write header '%s' for writing, error code %d.", fname, errorCode);
	}

        // Copy image
        copyGifImage(out, gifFileType);

        // Write header
        spew = EGifSpew(out);
        if (spew == GIF_ERROR) {
		rb_raise(rb_eException, "Could not encode body of 'open.gif' for writing, error code %d.", out->Error);
	}
}

static VALUE initialize(VALUE self, VALUE rubyGifString) {
    struct RubyImage * rubyImage;
    void *data;
    struct RubyString *cGifString;
    int errorCode, fd, spew, i;
    GifFileType *out;
   
    printf("Initializing...\n"); 
    Check_Type(rubyGifString, T_STRING);
    
    Data_Get_Struct(self, struct RubyImage *, rubyImage);
    cGifString = getRubyString(rubyGifString);

    // Read gif from Ruby
    printf("Pre GifOpen string: %p current: %d\n", cGifString, cGifString->current);
    if ( (rubyImage->gifFileType = DGifOpen(cGifString, readGifFromMemory, &errorCode)) == NULL ) {
        rb_raise(rb_eException, "Could not read gif, giflib error code %d.", errorCode);
    }
    printf("Pre Slurp string: %p current %d\n", cGifString, cGifString->current);
    if (DGifSlurp(rubyImage->gifFileType) == GIF_ERROR) {
        rb_raise(rb_eException, "Could not decode gif, giflib error code %d", rubyImage->gifFileType->Error);
    }
    printf("Post Slurp string: %p current: %d\n", cGifString, cGifString->current);

    writeToFile(rubyImage->gifFileType,"./open.gif");

    return self;
}

int RUBY_STRING_INCREMENT_SIZE = 4096;

struct RubyString *newRubyString() {
	struct RubyString *string;
	string = calloc(1, sizeof(struct RubyString));
	if (string == NULL) {
		rb_raise(rb_eException, "Insufficient memory.");
	}
	return string;
}

char *allocateNewIncrement(int increment) {
	char *result;
	result = calloc(increment * RUBY_STRING_INCREMENT_SIZE, sizeof(char));
	if (result == NULL) {
		rb_raise(rb_eException, "Insufficient memory.");
	}
	return result;
}

int giflibWriteToMemory(GifFileType *fileType, const GifByteType *buffer, int count) {
	int i;
	struct RubyString *string;
	string = (struct RubyString *)fileType->UserData;

	if (string == NULL) {
		rb_raise(rb_eException, "String expected.");
	}

	char *newText;
	int increment;
	if (string->text == NULL) {
		increment = 1;
		newText = allocateNewIncrement(1);
		string->text = newText;
		string->length = RUBY_STRING_INCREMENT_SIZE;
	} else if (string->length - string->current < count) {
		increment = string->length / RUBY_STRING_INCREMENT_SIZE;
		newText = allocateNewIncrement(++increment);
		memcpy(newText, string->text, string->current);
		string->text = newText;
		string->length = increment * RUBY_STRING_INCREMENT_SIZE;
	}
	for(i = 0; i < count; i++) {
		string->text[string->current + i] = buffer[i];
	}
	string->current += count;
	return count;
}

void writeToMemory(GifFileType *image, struct RubyString *string) {
	GifFileType *gifFileType;
	int errorCode;

	printf("Write to memory (string %p, current %d, length %d).\n", string, string->current, string->length);
        gifFileType = EGifOpen(string, giflibWriteToMemory, &errorCode);
        if (gifFileType == NULL) {
                rb_raise(rb_eException, "Could not open gif to encode, giflib error code %d.", errorCode);
        }

	copyGifImage(gifFileType, image);

        if (EGifSpew(gifFileType) == GIF_ERROR) {
                rb_raise(rb_eException, "Could not spew gif to encode, giflib error code %d.", gifFileType->Error);
        }
	printf("Finished write to memory (string %p, current %d, length %d).\n", string, string->current, string->length);
}

static VALUE encode(VALUE self) {
	int errorCode;
	struct RubyImage *rubyImage;
	struct RubyString *rubyString;
	GifFileType *gifFileType;
        Data_Get_Struct(self, struct RubyImage *, rubyImage);

	writeToFile(rubyImage->gifFileType,"./encode.gif");
	rubyString = newRubyString();
	writeToMemory(rubyImage->gifFileType, rubyString);
	printf("Final current: %d\n", rubyString->current);

	return rb_str_new(rubyString->text, rubyString->current);
}

static VALUE getWidth(VALUE self) {
	struct RubyImage * rubyImage;
	Data_Get_Struct(self, struct RubyImage *, rubyImage);
	return INT2FIX(rubyImage->gifFileType->SWidth);
}

static VALUE getHeight(VALUE self) {
	struct RubyImage * rubyImage;
	Data_Get_Struct(self, struct RubyImage *, rubyImage);
	return INT2FIX(rubyImage->gifFileType->SHeight);
}

static VALUE getImageCount(VALUE self) {
	struct RubyImage * rubyImage;
	Data_Get_Struct(self, struct RubyImage *, rubyImage);
	return INT2FIX(rubyImage->gifFileType->ImageCount);
}

// Executed by ruby require
void Init_composite() {
    printf("Initialising composite.\n");
    VALUE mGiflib = rb_define_module("Composite");
    VALUE cGiflibImage = rb_define_class_under(mGiflib, "Image", rb_cObject);
    rb_define_alloc_func(cGiflibImage, allocate);
    rb_define_method(cGiflibImage, "initialize", initialize, 1);
    rb_define_method(cGiflibImage, "getWidth", getWidth, 0);
    rb_define_method(cGiflibImage, "getHeight", getHeight, 0);
    rb_define_method(cGiflibImage, "getImageCount", getImageCount, 0);
    rb_define_method(cGiflibImage, "encode", encode, 0);
}
