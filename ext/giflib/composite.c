#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

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
    printf("\tRead in %d, rubyString: %p, current was %d\n", count, rubyString, rubyString->current);
    rubyString->current += count;
    printf("\tcurrent after increment: %d\n", rubyString->current);
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

static VALUE initialize(VALUE self, VALUE rubyGifString) {
    struct RubyImage * rubyImage;
    void *data;
    struct RubyString *cGifString;
    int errorCode;
   
    printf("Initializing...\n"); 
    Check_Type(rubyGifString, T_STRING);
    
    Data_Get_Struct(self, struct RubyImage *, rubyImage);
    cGifString = getRubyString(rubyGifString);

    printf("Pre GifOpen current %d\n", cGifString->current);
    if ( (rubyImage->gifFileType = DGifOpen(cGifString, readGifFromMemory, &errorCode)) == NULL ) {
        rb_raise(rb_eException, "Could not read gif, giflib error code %d.", errorCode);
    }
    printf("Pre Slurp current %d\n", cGifString->current);
    if (DGifSlurp(rubyImage->gifFileType) == GIF_ERROR) {
        rb_raise(rb_eException, "Could not decode gif, giflib error code %d", rubyImage->gifFileType->Error);
    }
    printf("Post Slurp SavedImages[0/%d]: %p rubyString: %p current: %d\n", rubyImage->gifFileType->ImageCount, rubyImage->gifFileType->SavedImages[0], cGifString, cGifString->current);
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

int writeToMemory(GifFileType *fileType, const GifByteType *buffer, int count) {
	int i;
	struct RubyString *string;
	string = (struct RubyString *)fileType->UserData;

	if (string == NULL) {
		rb_raise(rb_eException, "String expected.");
	}

	char *newText;
	printf("WriteToMemory count: %d\n", count);
	if (string->text == NULL) {
		printf("\tAllocating memory, first time\n");
		newText = allocateNewIncrement(1);
		string->text = newText;
		string->length = RUBY_STRING_INCREMENT_SIZE;
	} else if (string->length - string->current < count) {
		printf("\tAllocating memory\n");
		int increment;
		increment = string->length / RUBY_STRING_INCREMENT_SIZE;
		newText = allocateNewIncrement(++increment);
		memcpy(newText, string->text, count);
		string->text = newText;
		string->length = increment * RUBY_STRING_INCREMENT_SIZE;
	}
	for(i = 0; i < count; i++) {
		string->text[string->current + i] = buffer[i];
	}
	return count;
}

static VALUE encode(VALUE self) {
	int errorCode;
	struct RubyImage *rubyImage;
	struct RubyString *rubyString;
	GifFileType *gifFileType;
        Data_Get_Struct(self, struct RubyImage *, rubyImage);
	
	rubyString = newRubyString();
	printf("Pre EGifOpen SavedImages[0] %p rubyString %p\n", rubyImage->gifFileType->SavedImages[0], rubyString);
	gifFileType = EGifOpen(rubyString, writeToMemory, &errorCode);
	if (gifFileType == NULL) {
		rb_raise(rb_eException, "Could not open gif to encode, giflib error code %d.", errorCode);
	}
	printf("Pre EGifSpew\n");
	if (EGifSpew(gifFileType) == GIF_ERROR) {
		rb_raise(rb_eException, "Could not spew gif to encode, giflib error code %d.", gifFileType->Error);
	}

	return self;
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
