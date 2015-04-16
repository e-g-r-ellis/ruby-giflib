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
                GifMakeSavedImage(out, &(gifFileType)->SavedImages[i]);
        }
}

static VALUE initialize(VALUE self, VALUE rubyGifString) {
    struct RubyImage * rubyImage;
    void *data;
    struct RubyString *cGifString;
    int errorCode, fd, spew, i;
    GifFileType *out;
   
    Check_Type(rubyGifString, T_STRING);
    
    Data_Get_Struct(self, struct RubyImage *, rubyImage);
    cGifString = getRubyString(rubyGifString);

    // Read gif from Ruby
    if ( (rubyImage->gifFileType = DGifOpen(cGifString, readGifFromMemory, &errorCode)) == NULL ) {
        rb_raise(rb_eException, "Could not read gif, giflib error code %d.", errorCode);
    }
    if (DGifSlurp(rubyImage->gifFileType) == GIF_ERROR) {
        rb_raise(rb_eException, "Could not decode gif, giflib error code %d", rubyImage->gifFileType->Error);
    }

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

        gifFileType = EGifOpen(string, giflibWriteToMemory, &errorCode);
        if (gifFileType == NULL) {
                rb_raise(rb_eException, "Could not open gif to encode, giflib error code %d.", errorCode);
        }

	copyGifImage(gifFileType, image);

        if (EGifSpew(gifFileType) == GIF_ERROR) {
                rb_raise(rb_eException, "Could not spew gif to encode, giflib error code %d.", gifFileType->Error);
        }
}

static VALUE encode(VALUE self) {
	int errorCode;
	struct RubyImage *rubyImage;
	struct RubyString *rubyString;
	GifFileType *gifFileType;
        Data_Get_Struct(self, struct RubyImage *, rubyImage);

	rubyString = newRubyString();
	writeToMemory(rubyImage->gifFileType, rubyString);

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

GifByteType *skipToStart(int x, int y, GifFileType *file) {
	int toSkip = file->SWidth * y + x;
	return (GifByteType *)file->SavedImages->RasterBits + toSkip;
}

void giflibCompose(GifFileType *current, GifFileType *compose, int x, int y) {
	GifByteType *currentByte, *composeByte;
	composeByte = compose->SavedImages->RasterBits;
	currentByte = skipToStart(x,y,current);
	int i, j;
	for (i = 0; i < compose->SHeight; i++) {
		for (j = 0; j < compose->SWidth; j++) {
			*(currentByte++) = *(composeByte++);
		}
		currentByte += current->SWidth - compose->SWidth;
	}
}

static VALUE compose(VALUE self, VALUE image, VALUE x, VALUE y) {
	struct RubyImage *current;
	struct RubyImage *compose;
	GifFileType *gifCurrent;
	GifFileType *gifCompose;
	int gifX;
	int gifY;

	Check_Type(x, T_FIXNUM);
	Check_Type(y, T_FIXNUM);

	Data_Get_Struct(self, struct RubyImage *, current);
	Data_Get_Struct(image, struct RubyImage *, compose);
	gifCurrent = current->gifFileType;
	gifCompose = compose->gifFileType;
	gifX = FIX2INT(x);
	gifY = FIX2INT(y);

	if (gifX < 0) {
		rb_raise(rb_eException, "Compose x value must be >= 0 (not %d).", gifX);
	} else if (gifY < 0) {
		rb_raise(rb_eException, "Compose y value must be >= 0 (not %d).", gifY);
	} else if (gifX + gifCompose->SWidth > gifCurrent->SWidth) {
		rb_raise(rb_eException, "Composite would extend over the end of the current image (current image width: %d, x: %d, composite image width %d)", gifCurrent->SWidth, gifX, gifCompose->SWidth);
	} else if (gifY + gifCompose->SHeight > gifCurrent->SHeight) {
		rb_raise(rb_eException, "Composite would extend over the end of the current image (current image height: %d, y: %d, composite image height %d)", gifCurrent->SHeight, gifY, gifCompose->SHeight);
	}
	
	giflibCompose(gifCurrent, gifCompose, gifX, gifY);
}

// Executed by ruby require
void Init_composite() {
    VALUE mGiflib = rb_define_module("Composite");
    VALUE cGiflibImage = rb_define_class_under(mGiflib, "Image", rb_cObject);
    rb_define_alloc_func(cGiflibImage, allocate);
    rb_define_method(cGiflibImage, "initialize", initialize, 1);
    rb_define_method(cGiflibImage, "getWidth", getWidth, 0);
    rb_define_method(cGiflibImage, "getHeight", getHeight, 0);
    rb_define_method(cGiflibImage, "getImageCount", getImageCount, 0);
    rb_define_method(cGiflibImage, "encode", encode, 0);
    rb_define_method(cGiflibImage, "compose", compose, 3);
}
