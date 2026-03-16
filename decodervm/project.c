#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project.h"
#include "vm.h"
#include "engine.h"
#include "player.h"
#include "audio.h"
#include "utils.h"
#include "variables.h"
#include "cv.h"

#define SECTION_INFO         1
#define SECTION_SLOT         2
#define SECTION_WAVE         3
#define SECTION_FUNCTION     4
#define SECTION_CV           5
#define SECTION_FUNCTION_KEY 6

#define PROJECT_FUNCTIONS 96

#define FUNCTION_MAX_IN   4
#define FUNCTION_MAX_OUT  4

static char *function_key_names[VM_FUNCTION_KEYS];

typedef struct Function {
    uint8_t inputs[FUNCTION_MAX_IN];
    uint8_t not_inputs[FUNCTION_MAX_IN];
    uint8_t logic[FUNCTION_MAX_OUT];
    uint8_t slots[FUNCTION_MAX_OUT];
} Function;

static uint8_t project_type;
static char *project_name;

static Function *functions;
static uint8_t function_count;

static bool function_load_array(FILE *f, uint8_t max, uint8_t *arr)
{
    uint8_t len;
    if (!file_read_uint8(f, &len)) {
        return false;
    }
    if (len > max) {
        return false;
    }
    while (len--) {
        if (!file_read_uint8(f, arr)) {
            return false;
        }
        ++arr;
    }
    return true;
}

static bool project_load_function(FILE *f)
{
    void *new_functions = realloc(functions, (function_count + 1) * sizeof(Function));
    if (!new_functions) {
        return false;
    }
    functions = new_functions;
    ++function_count;
    memset(&functions[function_count - 1], 0, sizeof(Function));
    if (!function_load_array(f, FUNCTION_MAX_IN, functions[function_count - 1].inputs)) {
        return false;
    }
    if (!function_load_array(f, FUNCTION_MAX_IN, functions[function_count - 1].not_inputs)) {
        return false;
    }
    if (!function_load_array(f, FUNCTION_MAX_OUT, functions[function_count - 1].logic)) {
        return false;
    }
    if (!function_load_array(f, FUNCTION_MAX_OUT, functions[function_count - 1].slots)) {
        return false;
    }
    return true;
}

static bool project_load_function_key(FILE *f)
{
    uint8_t num;
    if (!file_read_uint8(f, &num)) {
        return false;
    }
    if (num >= VM_FUNCTION_KEYS || function_key_names[num]) {
        return false;
    }
    if (!file_read_string(f, &function_key_names[num])) {
        return false;
    }
    return true;
}

void project_open(void)
{
    FILE *f = fopen(PROJECT_FILENAME, "rb");
    if (!f) {
        return;
    }
    uint32_t magic;
    if (!file_read_uint32(f, &magic)) {
        goto ret;
    }
    if (magic != PROJECT_MAGIC) {
        goto ret;
    }
    uint8_t version;
    if (!file_read_uint8(f, &version)) {
        goto ret;
    }
    if (version != 1) {
        goto ret;
    }
    uint8_t section;
    while (file_read_uint8(f, &section)) {
        printf("reading %d\n", section);
        if (section == SECTION_INFO) {
            if (!file_read_uint8(f, &project_type)) {
                goto ret;
            }
            if (!file_read_string(f, &project_name)) {
                goto ret;
            }
        } else if (section == SECTION_SLOT) {
            if (!vm_load_slot(f)) {
                goto ret;
            }
        } else if (section == SECTION_WAVE) {
            if (!wave_load_info(f)) {
                goto ret;
            }
        } else if (section == SECTION_FUNCTION) {
            if (!project_load_function(f)) {
                goto ret;
            }
        } else if (section == SECTION_CV) {
            if (!cv_load(f)) {
                goto ret;
            }
        } else if (section == SECTION_FUNCTION_KEY) {
            if (!project_load_function_key(f)) {
                goto ret;
            }
        } else {
            goto ret;
        }
    }
    wave_init(PROJECT_FILENAME);
ret:
    fclose(f);
}

void project_close(void)
{
    for (int i = 0 ; i < VM_FUNCTION_KEYS ; ++i) {
        free(function_key_names[i]);
        function_key_names[i] = NULL;
    }
    free(functions);
    functions = NULL;
    function_count = 0;
    engine_stop();
    vm_clear();
    player_clear();
    wave_clear();
    free(project_name);
    project_name = NULL;
}

const char *project_get_name(void)
{
    return project_name;
}

const char *project_get_function_key_name(uint8_t id)
{
    if (id < VM_FUNCTION_KEYS) {
        return function_key_names[id];
    }
    return NULL;
}

void project_stop(void)
{
    engine_stop();
    vm_reset();
    player_clear();
    for (int f = 0 ; f < VM_FUNCTION_KEYS ; ++f) {
        vm_set_function_key(f, false);
    }
}
