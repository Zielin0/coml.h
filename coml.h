// coml.h - Simple header-only library for parsing TOML
// License: MIT (more at the bottom)
//
// Before including
// #define COML_IMPLEMENTATION

#ifndef COML_H_
#define COML_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef COMLDEF
#define COMLDEF static inline
#endif

typedef enum {
    ComlType_Double,
    ComlType_String,
    ComlType_Boolean,
    ComlType_ListDouble,
    ComlType_ListString,
} Coml_Type;

typedef struct Coml_KV {
    const char* key;
    void* value;
    Coml_Type type;
    size_t list_length;
    struct Coml_KV* next;
} Coml_KV;

typedef struct Coml_Table {
    const char* name;
    Coml_KV* items;
    struct Coml_Table* next;
} Coml_Table;

typedef struct {
    char* raw_content;
    Coml_Table* tables;
    Coml_KV* items;
    size_t next_table;
} Coml;

COMLDEF Coml* coml_from_file(const char* path); // Returns NULL if failed
COMLDEF bool coml_write_file(Coml* coml, const char* path); // Returns false if failed
COMLDEF void coml_format_kv(FILE* file, Coml_KV* kv);
COMLDEF void coml_format_table(FILE* file, Coml_Table* table);

COMLDEF Coml* coml_parse(char* content, bool from_file); // Returns NULL if failed, set from_file to false
COMLDEF void coml_free(Coml* coml); // Frees the Coml structure

COMLDEF void coml_free_split(char** split);

COMLDEF bool coml_parse_kv(Coml_Table* table, char* input);
COMLDEF bool coml_parse_table(Coml* coml);

COMLDEF bool coml_parse_value(Coml_KV* kv, const char* value);
COMLDEF Coml_KV* coml_new_kv(const char* key, const char* value);
COMLDEF Coml_KV* coml_insert_kv(Coml_KV* kv, const char* key, const char* value);
COMLDEF Coml_Table* coml_new_table(const char* name, Coml_KV* items);
COMLDEF Coml_Table* coml_insert_table(Coml_Table* table, const char* name, Coml_KV* items);

// Get values directly by table name and key
COMLDEF void* coml_get_value_raw(Coml* coml, const char* table_name, const char* key_name);
COMLDEF int coml_get_value_int(Coml* coml, const char* table_name, const char* key_name);
COMLDEF float coml_get_value_float(Coml* coml, const char* table_name, const char* key_name);
COMLDEF char* coml_get_value_string(Coml* coml, const char* table_name, const char* key_name);
COMLDEF bool coml_get_value_bool(Coml* coml, const char* table_name, const char* key_name);
COMLDEF double* coml_get_value_list_double(Coml* coml, const char* table_name, const char* key_name);
COMLDEF char** coml_get_value_list_string(Coml* coml, const char* table_name, const char* key_name);

// Get values without table name (searches everywhere)
COMLDEF void* coml_find_value_raw(Coml* coml, const char* key_name);
COMLDEF int coml_find_value_int(Coml* coml, const char* key_name);
COMLDEF float coml_find_value_float(Coml* coml, const char* key_name);
COMLDEF char* coml_find_value_string(Coml* coml, const char* key_name);
COMLDEF bool coml_find_value_bool(Coml* coml, const char* key_name);
COMLDEF double* coml_find_value_list_double(Coml* coml, const char* key_name);
COMLDEF char** coml_find_value_list_string(Coml* coml, const char* key_name);

// Set the values, set table_name to NULL to search everywhere
COMLDEF Coml_KV* coml_get_kv(Coml* coml, const char* table_name, const char* key_name);
COMLDEF bool coml_set_int(Coml* coml, int value, const char* table_name, const char* key_name);
COMLDEF bool coml_set_float(Coml* coml, float value, const char* table_name, const char* key_name);
COMLDEF bool coml_set_string(Coml* coml, char* value, const char* table_name, const char* key_name);
COMLDEF bool coml_set_bool(Coml* coml, bool value, const char* table_name, const char* key_name);
COMLDEF bool coml_set_list_double(Coml* coml, double* value, size_t length, const char* table_name, const char* key_name);
COMLDEF bool coml_set_list_string(Coml* coml, char** value, size_t length, const char* table_name, const char* key_name);

COMLDEF void coml_print_kv(const Coml_KV* kv, bool indent);
COMLDEF void coml_print_table(const Coml_Table* table);
COMLDEF void coml_print(const Coml* coml); // Prints the Coml structure

COMLDEF char* coml_trim(char* input);
COMLDEF char** coml_split(char* input, char* delim);
COMLDEF size_t coml_split_length(char** split);

#endif // COML_H_

#ifdef COML_IMPLEMENTATION

COMLDEF Coml* coml_from_file(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) return NULL;
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* content = (char*)malloc(file_size+1);
    if (content == NULL) return NULL;

    fread(content, 1, file_size, file); 

    Coml* coml = coml_parse(strdup(content), true);
    if (coml == NULL) {
        free(content);
        return NULL;
    }

    free(content);
    fclose(file);

    return coml;
}

COMLDEF bool coml_write_file(Coml* coml, const char* path) {
    if (coml == NULL || path == NULL || strcmp(path, "") == 0) return false;

    FILE* file = fopen(path, "w");
    if (file == NULL) return false;

    Coml_KV* alone_kv = coml->items;
    coml_format_kv(file, alone_kv);

    fprintf(file, "\n");

    Coml_Table* current_table = coml->tables;
    coml_format_table(file, current_table);

    fclose(file);

    return true;
}

COMLDEF void coml_format_kv(FILE* file, Coml_KV* kv) {
    if (kv == NULL) return;

    coml_format_kv(file, kv->next);

    switch (kv->type) {
        case ComlType_Double:
            if (floor(*(double*)kv->value) == *(double*)kv->value) {
                fprintf(file, "%s = %i\n", kv->key, (int)*(double*)kv->value); 
            } else {
                fprintf(file, "%s = %.5f\n", kv->key, *(double*)kv->value); 
            }
            break;
        case ComlType_String:
            fprintf(file, "%s = \"%s\"\n", kv->key, (char*)kv->value);
            break;
        case ComlType_Boolean:
            fprintf(file, "%s = %s\n", kv->key, *(bool*)kv->value ? "true" : "false");
            break;
        case ComlType_ListDouble: {
            char* line = (char*)malloc(sizeof(char*)+1);
            sprintf(line, "%s = [", kv->key);
            for (size_t i = 0; i < kv->list_length; ++i) {
                if (floor(((double*)kv->value)[i]) == ((double*)kv->value)[i]) {
                    sprintf(line, "%s %i%s", line, (int)((double*)kv->value)[i], i == kv->list_length-1 ? "" : ","); 
                } else {
                    sprintf(line, "%s %.5f%s", line, ((double*)kv->value)[i], i == kv->list_length-1 ? "" : ","); 
                }
            }
            sprintf(line, "%s ]", line); 
            fprintf(file, "%s\n", line);
            free(line);
            break;
        }
        case ComlType_ListString: {
            char* line = (char*)malloc(sizeof(char*)+1);
            sprintf(line, "%s = [", kv->key);
            for (size_t i = 0; i < kv->list_length; ++i) {
                sprintf(line, "%s \"%s\"%s", line, ((char**)kv->value)[i], i == kv->list_length-1 ? "" : ","); 
            }
            sprintf(line, "%s ]", line); 
            fprintf(file, "%s\n", line);
            free(line);
            break;
        }
        default:
            fprintf(file, "%s = \"NULL (default)\"\n", kv->key); 
            break;
    }
}

COMLDEF void coml_format_table(FILE* file, Coml_Table* table) {
    if (table == NULL) return;

    coml_format_table(file, table->next);

    fprintf(file, "[%s]\n", table->name); 

    Coml_KV* current_kv = table->items;
    coml_format_kv(file, current_kv);

    fprintf(file, "\n"); 
}

COMLDEF Coml* coml_parse(char* content, bool from_file) {
    Coml* coml = (Coml*)malloc(sizeof(Coml));
    
    if (content == NULL || strcmp(content, "") == 0) {
        if (from_file) free(content);
        free(coml);
        return NULL;
    }
    
    coml->raw_content = strdup(content);
    coml->tables = NULL;
    coml->items = NULL;
    coml->next_table = 0;
    
    char** lines = coml_split(coml->raw_content, "\n");
    
    for (size_t i = 0; lines[i][0] != '['; ++i) {
        if (lines[i][0] == '#') continue;

        char* trim = coml_trim(lines[i]);
        char** parts = coml_split(trim, "=");
        if (parts == NULL) {
            free(trim);
            break;
        }

        coml->items = coml_insert_kv(coml->items, parts[0], parts[1]);

        free(trim);
        coml_free_split(parts);

        coml->next_table = i+1;
    }

    size_t length = coml_split_length(lines);
    coml_free_split(lines);

    while (coml->next_table != length) {
        bool res = coml_parse_table(coml);
        if (!res) {
            if (from_file) free(content);
            coml_free(coml);
            return NULL;
        }
    }

    if (from_file) free(content);
    
    return coml;
}

COMLDEF void coml_free(Coml* coml) {
    if (coml == NULL) return;
    
    Coml_Table* current_table = coml->tables;
    while (current_table != NULL) {
        Coml_Table* temp_table = current_table;
        current_table = current_table->next;
        
        Coml_KV* current_kv = temp_table->items;
        while (current_kv != NULL) {
            Coml_KV* temp_kv = current_kv;
            current_kv = current_kv->next;
            free((void*)temp_kv->key);
            if (temp_kv->type == ComlType_ListString) {
                for (size_t i = 0; i < temp_kv->list_length; ++i) {
                    free(((char**)temp_kv->value)[i]);
                }

                free(temp_kv->value);
            } else {
                free(temp_kv->value);
            }
            free(temp_kv);
        }
        
        free((void*)temp_table->name);
        free(temp_table);
    }

    Coml_KV* alone_kv = coml->items;
    while (alone_kv != NULL) {
        Coml_KV* temp_kv = alone_kv;
        alone_kv = alone_kv->next;
        free((void*)temp_kv->key);
        if (temp_kv->type == ComlType_ListString) {
            for (size_t i = 0; i < temp_kv->list_length; ++i) {
                free(((char**)temp_kv->value)[i]);
            }

            free(temp_kv->value);
        } else {
            free(temp_kv->value);
        }

        free(temp_kv);
    }
    
    free(coml->raw_content);
    free(coml);
}

COMLDEF void coml_free_split(char** split) {
    for (size_t i = 0; split[i] != NULL; ++i) {
        free(split[i]);
    }
    
    free(split);
}

COMLDEF bool coml_parse_kv(Coml_Table* table, char* input) {
    if (table == NULL) return false;
    
    char* trim = coml_trim(input);
    char** parts = coml_split(trim, "=");
    if (parts == NULL) {
        free(trim);
        return false;
    }
    
    table->items = coml_insert_kv(table->items, parts[0], parts[1]);
    
    free(trim);
    coml_free_split(parts);
    
    return true;
}

COMLDEF bool coml_parse_table(Coml* coml) {
    if (coml == NULL) return false;
    
    char** lines = coml_split(coml->raw_content, "\n");
    if (lines == NULL) return false;
    if (lines[coml->next_table][0] == '#') coml->next_table += 1;
    
    char* table_name = (char*)malloc(strlen(lines[coml->next_table])-1);
    memset(table_name, 0, strlen(lines[coml->next_table])-1);
    if (table_name == NULL) {
        coml_free_split(lines);
        return false;
    }
    
    strncpy(table_name, lines[coml->next_table]+1, strlen(lines[coml->next_table])-2);
    
    Coml_Table* new_table = coml_new_table(table_name, NULL);

    for (size_t i = coml->next_table+1; lines[i] != NULL; ++i) {
        if (lines[i][0] == '#') continue;

        if (lines[i][0] == '[') {
            coml->next_table = i;
            break;
        }
        
        if (lines[i+1] == NULL) coml->next_table = i+1;
        
        bool res = coml_parse_kv(new_table, lines[i]);
        if (!res) {
            coml_free_split(lines);
            free(table_name);
            free((void*)new_table->name);
            free(new_table);
            return false;
        }
    }
    
    coml->tables = coml_insert_table(coml->tables, new_table->name, new_table->items);
    
    coml_free_split(lines);
    free(table_name);
    free((void*)new_table->name);
    free(new_table);
    
    return true;
}

COMLDEF bool coml_parse_value(Coml_KV* kv, const char *input) {
    if (input[0] == '"' || input[0] == '\'') {
        if (input[0] == '"' && input[strlen(input)-1] != '"') return false;
        if (input[0] == '\'' && input[strlen(input)-1] != '\'') return false;

        char* actual_string = (char*)malloc(strlen(input)-1);
        strncpy(actual_string, input+1, strlen(input)-2);

        kv->value = strdup(actual_string);
        kv->type = ComlType_String;
        kv->list_length = 0;
        
        free(actual_string);
        return true;
    }

    if (input[0] == '[') {
        if (input[strlen(input)-1] != ']') return false;

        char* actual_list = (char*)malloc(strlen(input)-1);
        strncpy(actual_list, input+1, strlen(input)-2);

        char** list_elements = coml_split(actual_list, ",");
        size_t list_length = coml_split_length(list_elements);

        if (list_elements[0][0] == '"' || list_elements[0][0] == '\'') {
            if (list_elements[0][0] == '"' && list_elements[0][strlen(list_elements[0])-1] != '"') {
                free(actual_list);
                coml_free_split(list_elements);
                return false;
            }
            if (list_elements[0][0] == '\'' && list_elements[0][strlen(list_elements[0])-1] != '\'') {
                free(actual_list);
                coml_free_split(list_elements);
                return false;
            }

            kv->value = malloc(sizeof(char*)*list_length+list_length);

            for (size_t i = 0; list_elements[i] != NULL; ++i) {
                char* string_value = (char*)malloc(strlen(list_elements[i])-1);
                strncpy(string_value, list_elements[i]+1, strlen(list_elements[i])-2);
                string_value[strlen(list_elements[i])-2] = '\0';
                
                ((char**)kv->value)[i] = strdup(string_value);

                free(string_value);
            }

            kv->type = ComlType_ListString;
        } else {
            kv->value = malloc(sizeof(double)*list_length);

            for (size_t i = 0; list_elements[i] != NULL; ++i) {
                double val = atof(list_elements[i]);
                ((double*)kv->value)[i] = val;
            }

            kv->type = ComlType_ListDouble;
        }
        
        kv->list_length = list_length;

        free(actual_list);
        coml_free_split(list_elements);
        return true;
    }

    if (strcmp(input, "true") == 0 || strcmp(input, "false") == 0) {
        kv->value = malloc(sizeof(bool));
        *((bool*)kv->value) = strcmp(input, "true") == 0;
        kv->type = ComlType_Boolean;
        kv->list_length = 0;

        return true;
    }

    kv->value = malloc(sizeof(double));
    double val = atof(input);
    *((double*)kv->value) = val;
    kv->type = ComlType_Double;
    kv->list_length = 0;
    
    return true;
}

COMLDEF Coml_KV* coml_new_kv(const char* key, const char* value) {
    Coml_KV* kv = (Coml_KV*)malloc(sizeof(Coml_KV));
    if (kv != NULL) {
        kv->key = strdup(key);
        bool res = coml_parse_value(kv, value);
        if (!res) kv->value = NULL;
        kv->next = NULL;
    }
    
    return kv;
}

COMLDEF Coml_KV* coml_insert_kv(Coml_KV* kv, const char* key, const char* value) {
    Coml_KV* new_kv = coml_new_kv(key, value);
    if (new_kv != NULL) {
        new_kv->next = kv;
        kv = new_kv;
    }
    
    return kv;
}

COMLDEF Coml_Table* coml_new_table(const char* name, Coml_KV* items) {
    Coml_Table* table = (Coml_Table*)malloc(sizeof(Coml_Table));
    if (table != NULL) {
        table->name = strdup(name);
        table->items = items;
        table->next = NULL;
    }
    
    return table;
}

COMLDEF Coml_Table* coml_insert_table(Coml_Table* table, const char* name, Coml_KV* items) {
    Coml_Table* new_table = coml_new_table(name, items);
    if (new_table != NULL) {
        new_table->next = table;
        table = new_table;
    }
    
    return table;
}

COMLDEF void* coml_get_value_raw(Coml* coml, const char* table_name, const char* key_name) {
    Coml_Table* current_table = coml->tables;
    while (current_table != NULL) {
        if (strcmp(current_table->name, table_name) == 0) {
            Coml_KV* kv = current_table->items;
            while (kv != NULL) {
                if (strcmp(kv->key, key_name) == 0) {
                    return kv->value;
                }

                kv = kv->next;
            }
        }

        current_table = current_table->next;
    }

    return NULL;
}

COMLDEF int coml_get_value_int(Coml* coml, const char* table_name, const char* key_name) {
    void* value = coml_get_value_raw(coml, table_name, key_name);
    if (value == NULL) return 0;

    return (int)*(double*)value;
}

COMLDEF float coml_get_value_float(Coml* coml, const char* table_name, const char* key_name) {
    void* value = coml_get_value_raw(coml, table_name, key_name);
    if (value == NULL) return 0.f;

    return (float)*(double*)value;
}

COMLDEF char* coml_get_value_string(Coml* coml, const char* table_name, const char* key_name) {
    void* value = coml_get_value_raw(coml, table_name, key_name);
    if (value == NULL) return NULL;

    return (char*)value;
}

COMLDEF bool coml_get_value_bool(Coml* coml, const char* table_name, const char* key_name) {
    void* value = coml_get_value_raw(coml, table_name, key_name);
    if (value == NULL) return false;

    return *(bool*)value;
}

COMLDEF double* coml_get_value_list_double(Coml* coml, const char* table_name, const char* key_name) {
    void* value = coml_get_value_raw(coml, table_name, key_name);
    if (value == NULL) return NULL;

    return (double*)value;
}

COMLDEF char** coml_get_value_list_string(Coml* coml, const char* table_name, const char* key_name) {
    void* value = coml_get_value_raw(coml, table_name, key_name);
    if (value == NULL) return NULL;

    return (char**)value;
}

COMLDEF void* coml_find_value_raw(Coml* coml, const char* key_name) {
    Coml_KV* alone_kv = coml->items;
    while (alone_kv != NULL) {
        if (strcmp(alone_kv->key, key_name) == 0) {
            return alone_kv->value;
        }

        alone_kv = alone_kv->next;
    }

    Coml_Table* current_table = coml->tables;
    while (current_table != NULL) {
        Coml_KV* kv = current_table->items;
        while (kv != NULL) {
            if (strcmp(kv->key, key_name) == 0) {
                return kv->value;
            }

            kv = kv->next;
        }

        current_table = current_table->next;
    }

    return NULL;
}

COMLDEF int coml_find_value_int(Coml* coml, const char* key_name) {
    void* value = coml_find_value_raw(coml, key_name);
    if (value == NULL) return 0;

    return (int)*(double*)value;
}

COMLDEF float coml_find_value_float(Coml* coml, const char* key_name) {
    void* value = coml_find_value_raw(coml, key_name);
    if (value == NULL) return 0.f;

    return (float)*(double*)value;
}

COMLDEF char* coml_find_value_string(Coml* coml, const char* key_name) {
    void* value = coml_find_value_raw(coml, key_name);
    if (value == NULL) return NULL;

    return (char*)value;
}

COMLDEF bool coml_find_value_bool(Coml* coml, const char* key_name) {
    void* value = coml_find_value_raw(coml, key_name);
    if (value == NULL) return false;

    return *(bool*)value;
}

COMLDEF double* coml_find_value_list_double(Coml* coml, const char* key_name) {
    void* value = coml_find_value_raw(coml, key_name);
    if (value == NULL) return NULL;

    return (double*)value;
}

COMLDEF char** coml_find_value_list_string(Coml* coml, const char* key_name) {
    void* value = coml_find_value_raw(coml, key_name);
    if (value == NULL) return NULL;

    return (char**)value;
}

COMLDEF Coml_KV* coml_get_kv(Coml* coml, const char* table_name, const char* key_name) {
    if (table_name == NULL) {
        Coml_KV* alone_kv = coml->items;
        while (alone_kv != NULL) {
            if (strcmp(alone_kv->key, key_name) == 0) {
                return alone_kv;
            }

            alone_kv = alone_kv->next;
        }
    }

    Coml_Table* current_table = coml->tables;
    while (current_table != NULL) {
        if (table_name != NULL && strcmp(current_table->name, table_name) == 0) {
            Coml_KV* kv = current_table->items;
            while (kv != NULL) {
                if (strcmp(kv->key, key_name) == 0) return kv;

                kv = kv->next;
            }
        } else {
            Coml_KV* kv = current_table->items;
            while (kv != NULL) {
                if (strcmp(kv->key, key_name) == 0) return kv;

                kv = kv->next;
            }
        }
    
        current_table = current_table->next;
    }

    return NULL;
}

COMLDEF bool coml_set_int(Coml* coml, int value, const char* table_name, const char* key_name) {
    Coml_KV* kv = coml_get_kv(coml, table_name, key_name);
    if (kv == NULL || kv->type != ComlType_Double) return false;

    kv->value = realloc(kv->value, sizeof(double));
    *((double*)kv->value) = (double)value;

    return true;
}

COMLDEF bool coml_set_float(Coml* coml, float value, const char* table_name, const char* key_name) {
    Coml_KV* kv = coml_get_kv(coml, table_name, key_name);
    if (kv == NULL || kv->type != ComlType_Double) return false;

    kv->value = realloc(kv->value, sizeof(float));
    *((double*)kv->value) = (double)value;

    return true;
}

COMLDEF bool coml_set_string(Coml* coml, char* value, const char* table_name, const char* key_name) {
    Coml_KV* kv = coml_get_kv(coml, table_name, key_name);
    if (kv == NULL || kv->type != ComlType_String) return false;

    free(kv->value);
    char* temp_buf = strdup(value);
    kv->value = strdup(temp_buf);
    free(temp_buf);

    return true;
}

COMLDEF bool coml_set_bool(Coml* coml, bool value, const char* table_name, const char* key_name) {
    Coml_KV* kv = coml_get_kv(coml, table_name, key_name);
    if (kv == NULL || kv->type != ComlType_Boolean) return false;

    kv->value = realloc(kv->value, sizeof(bool));
    *((bool*)kv->value) = value;

    return true;
}

COMLDEF bool coml_set_list_double(Coml* coml, double* value, size_t length, const char* table_name, const char* key_name) {
    Coml_KV* kv = coml_get_kv(coml, table_name, key_name);
    if (kv == NULL || kv->type != ComlType_ListDouble) return false;

    kv->value = realloc(kv->value, sizeof(double)*length);
    for (size_t i = 0; i < length; i++) {
        ((double*)kv->value)[i] = value[i];
    }
    kv->list_length = length;

    return true;
}

COMLDEF bool coml_set_list_string(Coml* coml, char** value, size_t length, const char* table_name, const char* key_name) {
    Coml_KV* kv = coml_get_kv(coml, table_name, key_name);
    if (kv == NULL || kv->type != ComlType_ListString) return false;

    for (size_t i = 0; i < kv->list_length; ++i) {
        free(((char**)kv->value)[i]);
    }
    free(kv->value);

    kv->value = realloc(NULL, sizeof(char*)*length+length);
    for (size_t i = 0; i < length; i++) {
        ((char**)kv->value)[i] = strdup(value[i]);
    }
    kv->list_length = length;

    return true;
}

COMLDEF void coml_print_kv(const Coml_KV* kv, bool indent) {
    if (kv == NULL) return;
    
    coml_print_kv(kv->next, indent);

    const char* indent_str = indent ? "    " : "";
    const char* indent_str2 = indent ? "\t" : "    ";

    switch (kv->type) {
        case ComlType_Double:
            printf("%s%s: %.10lf\n", indent_str, kv->key, *(double*)kv->value);
            break;
        case ComlType_String:
            printf("%s%s: %s\n", indent_str, kv->key, (char*)kv->value);
            break;
        case ComlType_Boolean:
            printf("%s%s: %s\n", indent_str, kv->key, *(bool*)kv->value ? "true" : "false");
            break;
        case ComlType_ListDouble:
            printf("%s%s:\n", indent_str, kv->key);
            for (size_t i = 0; i < kv->list_length; ++i) {
                printf("%s%zu - %.10lf\n", indent_str2, i, ((double*)kv->value)[i]);
            }
            break;
        case ComlType_ListString:
            printf("%s%s:\n", indent_str, kv->key);
            for (size_t i = 0; i < kv->list_length; ++i) {
                printf("%s%zu - %s\n", indent_str2, i, ((char**)kv->value)[i]);
            }
            break;
        default:
            printf("%s%s: NULL (default)\n", indent_str, kv->key);
            break;
    }
}

COMLDEF void coml_print_table(const Coml_Table* table) {
    if (table == NULL) return;
    
    coml_print_table(table->next);
    
    printf("Table: %s\n", table->name);
    
    Coml_KV* current_kv = table->items;
    coml_print_kv(current_kv, true);
}

COMLDEF void coml_print(const Coml* coml) {
    Coml_KV* current_item = coml->items;
    coml_print_kv(current_item, false);

    printf("\n");

    Coml_Table* current_table = coml->tables;
    coml_print_table(current_table);
}

COMLDEF char* coml_trim(char* input) {
    char* iter = input;
    char* out = (char*)malloc(strlen(input)+1);
    bool is_in_string = false;
    
    if (out != NULL) {
        char* out_iter = out;

        while (*iter != '\0') {
            if (*iter == '"' || *iter == '\'') {
                is_in_string = !is_in_string;
            }

            if (is_in_string) {
                *out_iter = *iter;
                ++out_iter;
            } else {
                if (*iter != 0x09 && *iter != 0x20) {
                    *out_iter = *iter;
                    ++out_iter;
                }
            }
            ++iter;
        }

        *out_iter = '\0';
    }
    
    return out;
}

COMLDEF char** coml_split(char* input, char* delim) {
    char* temp_input = strdup(input);
    char** result = NULL;
    size_t count = 0;
    char* token = strtok(temp_input, delim);
    
    while (token) {
        result = (char**)realloc(result, (count+1)*sizeof(char*));
        result[count++] = strdup(token);
        token = strtok(NULL, delim);
    }
    
    free(temp_input);
    result = (char**)realloc(result, (count+1)*sizeof(char*));
    result[count] = NULL;
    
    return result;
}

COMLDEF size_t coml_split_length(char **split) {
    size_t length = 0;
    while (split[length] != NULL) length++;

    return length;
}

#endif // COML_IMPLEMENTATION

// MIT License
//
// Copyright (c) 2024 Zielino
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// TODO: Support multi-line (strings, lists, objects, etc.)

