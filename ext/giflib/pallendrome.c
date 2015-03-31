#include <ruby.h>
#include <stdio.h>
#include <stdlib.h>

// Create C pallendrome
char* pallendrome(char* input) {
	int pallendrome_length;
	int length = 0;
	int i;
	char *result;
	char *work;

	printf("C pallendrome called with: %s\n", input);;
        for (i = 0; input[i] != '\0'; i++)
                length++;

	pallendrome_length = length * 2 - 1;

        result = calloc(pallendrome_length + 1, sizeof(char)); // One more character (null terminator)
        work = result;
        for (i = 0; i < pallendrome_length; i++) {
                *work = *input;
                work++;
                if (i < length-1) {
                        input++;
                } else {
                        input--;
                }
        }
	printf("C pallendrome returning: %s\n", result);
        return result;
}

// Handle ruby types
static VALUE ruby_pallendrome(VALUE rb_self, VALUE rb_input) {
	char *c_string;
	char *c_pallendrome;
	printf("ruby_pallendrome called.\n");
	VALUE result = Qnil;
	Check_Type(rb_input, T_STRING);
	
	c_string = RSTRING_PTR(rb_input);
	c_pallendrome = pallendrome(c_string);
	result = rb_str_new2(c_pallendrome);
	free(c_pallendrome);
	
	return result;
}

// Executed by ruby require:
void Init_pallendrome() {
	printf("Init_pallendrome called.\n");
	rb_define_method(rb_cObject, "pallendrome", ruby_pallendrome, 1);
}

int main(int argc, char *argv[]) {
	int i;
	for (i = 1; i < argc; i++) {
		printf("%s\n",pallendrome(argv[i]));
	}
}

