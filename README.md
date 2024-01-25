# coml.h

> [!WARNING]
> This library doesn't support multi-line values, and types like time, date or object.

Simple header-only library for parsing TOML

## Quick Example

`example.c`:
```c
#include <stdio.h>

#define COML_IMPLEMENTATION
#include "coml.h"

int main(void) {
    char data[] = { 
        "[stuff]\n"
        "str1 = \"qweqwe zxczxc asdasd\"\n"
        "integer = 123\n"
        "floot = 32.69\n"
        "list = [ \"this\", \"is\", \"a\", \"list\" ]\n"
        "nums = [ 123, 69, 420 ]\n"
    };

    Coml* coml = coml_parse(data, false);
    if (coml == NULL) {
            fprintf(stderr, "something went wrong...\n");
            return 1;
    }

    coml_print(coml);

    coml_free(coml);

    return 0;
}
```

This program will output:
```
Table: stuff
    str1: qweqwe zxczxc asdasd
    integer: 123.0000000000
    floot: 32.6900000000
    list:
	    0 - this
	    1 - is
	    2 - a
	    3 - list
    nums:
	    0 - 123.0000000000
	    1 - 69.0000000000
	    2 - 420.0000000000
```

**You can also read from a file**:
```c
Coml* coml = coml_from_file("path/to/file.toml");
```

**Free at the end of the program**:
```c
coml_free(coml);
```

## Getting and Setting a value

```c
// Getting
printf("%.2f\n", coml_get_value_float(coml, "some_table", "some-key"));
// Or
printf("%.2f\n", coml_find_value_float(coml, "some-key"));

// Setting
bool success = coml_set_float(coml, 69.123f, "some_table", "some_key");
```

## Writing to a file

```c
bool success = coml_write_file(coml, "path/to/file.toml");
```

## Building the demo

```shell
$ ./build.sh
$ ./main
```

## License

This project is under the [MIT](./LICENSE) License.

