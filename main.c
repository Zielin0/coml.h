#include <stdio.h>

#define COML_IMPLEMENTATION
#include "coml.h"

int main(void) {
    char data[] = { 
        "# this is a comment\n"
        "some-string = \"some string\"\n"
        "\n"
        "# also a comment\n"
        "[stuff]\n"
        "str1 = \"qweqwe\"\n"
        "str2 = \"asdasd\"\n"
        "\n"
        "[qwerty]\n"
        "boolean = true\n"
        "number = 213\n"
        "floot = 42.69\n"
        "stringwithspace = \"this string has spaces\"\n"
        "list = [ \"this\", \"is\", \"a\", \"list\" ]\n"
        "nums = [ 123, 321 ]\n"
    };

    Coml* coml = coml_parse(data, false);
    if (coml == NULL) {
        fprintf(stderr, "Something went wrong...\n");
        return 1;
    }
    
    coml_print(coml);

    printf("Before:\n");
    printf("%.2f\n", coml_get_value_float(coml, "qwerty", "floot"));

    bool success = coml_set_float(coml, 69.42f, "qwerty", "floot");
    if (!success) {
        fprintf(stderr, "Setting a KV failed\n");
    }

    printf("After:\n");
    printf("%.2f\n", coml_get_value_float(coml, "qwerty", "floot"));

    bool success_write = coml_write_file(coml, "write_test.toml");
    if (!success_write) {
        fprintf(stderr, "Failed to write\n"); 
    }

    coml_free(coml);
    
    return 0;
}

