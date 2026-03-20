/* Async Request Handlers HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <esp_http_server.h>
#include "esp_ota_ops.h"
#include "cJSON.h"

#include "engine.h"
#include "project.h"
#include "cv.h"

static const char *TAG = "http";

#define MAX_FILE_SIZE   (16*1024*1024)
#define SCRATCH_BUFSIZE 4096
static char scratch[SCRATCH_BUFSIZE];

static esp_err_t web_receive_json_content(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, scratch + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive the request");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    scratch[total_len] = '\0';
    return ESP_OK;
}

static esp_err_t web_index_handler(httpd_req_t *req)
{
    extern const unsigned char index_start[] asm("_binary_index_html_start");
    extern const unsigned char index_end[]   asm("_binary_index_html_end");
    const size_t index_size = (index_end - index_start);

    httpd_resp_send_chunk(req, (const char *)index_start, index_size);
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

static esp_err_t web_control_handler(httpd_req_t *req)
{
    esp_err_t res = web_receive_json_content(req);
    if (res != ESP_OK) {
        return res;
    }

    cJSON *root = cJSON_Parse(scratch);
    cJSON *act = cJSON_GetObjectItem(root, "action");
    cJSON *val = cJSON_GetObjectItem(root, "value");
    cJSON *index = cJSON_GetObjectItem(root, "index");
    if (act && act->valuestring) {
        if (val) {
            if (!strcmp(act->valuestring, "set_throttle")) {
                engine_set_throttle(val->valueint);
            } else if (!strcmp(act->valuestring, "function") && index) {
                vm_set_function_key(index->valueint, val->valueint);
            } else if (!strcmp(act->valuestring, "set_direction")) {
                engine_set_direction(val->type == cJSON_True);
            }
        } else if (!strcmp(act->valuestring, "stop")) {
            project_stop();
        } else if (!strcmp(act->valuestring, "brake")) {
            engine_brake();
        }
    }
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t web_status_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "speed", engine_get_speed_step());
    bool dir = engine_get_direction();
    cJSON_AddStringToObject(root, "direction", dir ? "forward" : "backward");
    cJSON *arr = cJSON_AddArrayToObject(root, "functions");
    for (int i = 0 ; i < VM_FUNCTION_KEYS ; ++i) {
        cJSON *item = cJSON_CreateBool(vm_get_function_key(i));
        cJSON_AddItemToArray(arr, item);
    }
    bool ok = cJSON_PrintPreallocated(root, scratch, SCRATCH_BUFSIZE - 10, false);
    cJSON_Delete(root);
    if (!ok) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON formatting error");
        return ESP_FAIL;
    }
    httpd_resp_sendstr(req, scratch);
    return ESP_OK;
}

static esp_err_t web_info_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", project_get_name());
    cJSON *arr = cJSON_AddArrayToObject(root, "functions");
    for (int i = 0 ; i < VM_FUNCTION_KEYS ; ++i) {
        const char *name = project_get_function_key_name(i);
        if (name) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "function", i);
            cJSON_AddStringToObject(item, "name", name);
            cJSON_AddItemToArray(arr, item);
        }
    }
    bool ok = cJSON_PrintPreallocated(root, scratch, SCRATCH_BUFSIZE - 10, false);
    cJSON_Delete(root);
    if (!ok) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON formatting error");
        return ESP_FAIL;
    }
    httpd_resp_sendstr(req, scratch);
    return ESP_OK;
}

static esp_err_t web_cv_defs_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateArray();
    for (int i = 0 ; i < CV_MAX ; ++i) {
        const char *name = cv_name(i);
        if (name) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddNumberToObject(item, "cv", i);
            cJSON_AddStringToObject(item, "name", name);
            cJSON_AddStringToObject(item, "desc", cv_description(i));
            cJSON_AddItemToArray(root, item);
        }
    }
    bool ok = cJSON_PrintPreallocated(root, scratch, SCRATCH_BUFSIZE - 10, false);
    cJSON_Delete(root);
    if (!ok) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON formatting error");
        return ESP_FAIL;
    }
    httpd_resp_sendstr(req, scratch);
    return ESP_OK;
}

static esp_err_t web_cv_read_handler(httpd_req_t *req)
{
    esp_err_t res = web_receive_json_content(req);
    if (res != ESP_OK) {
        return res;
    }
    cJSON *root = cJSON_Parse(scratch);
    cJSON *j_cv = cJSON_GetObjectItem(root, "cv");
    uint16_t cv = j_cv ? j_cv->valueint : 0;
    cJSON_Delete(root);
    if (!cv || !cv_name(cv)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Can't find CV id");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "value", cv_read(cv));
    bool ok = cJSON_PrintPreallocated(root, scratch, SCRATCH_BUFSIZE - 10, false);
    cJSON_Delete(root);
    if (!ok) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON formatting error");
        return ESP_FAIL;
    }
    httpd_resp_sendstr(req, scratch);
    return ESP_OK;
}

static esp_err_t web_cv_write_handler(httpd_req_t *req)
{
    esp_err_t res = web_receive_json_content(req);
    if (res != ESP_OK) {
        return res;
    }
    cJSON *root = cJSON_Parse(scratch);
    cJSON *j_cv = cJSON_GetObjectItem(root, "cv");
    cJSON *val = cJSON_GetObjectItem(root, "value");
    cv_addr_t cv = j_cv ? j_cv->valueint : 0;
    if (!cv || !cv_name(cv)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Can't find CV id");
        return ESP_FAIL;
    }
    if (val) {
        cv_write(cv, val->valueint);
    }
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

static esp_err_t web_project_upload_handler(httpd_req_t *req)
{
    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File too large");
        return ESP_FAIL;
    }

    int received;
    while ((received = httpd_req_recv(req, scratch, 4)) <= 0) {
        if (received == HTTPD_SOCK_ERR_TIMEOUT) {
            /* Retry if timeout occurred */
            continue;
        }
        ESP_LOGE(TAG, "File reception failed!");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
        return ESP_FAIL;
    }

    /* Check signature */
    if (received != 4 || *(uint32_t*)scratch != PROJECT_MAGIC) {
        ESP_LOGE(TAG, "Signature check failed!");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to check signature");
        return ESP_FAIL;
    }

    project_close();

    FILE *f = fopen(PROJECT_FILENAME, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to create file : %s", PROJECT_FILENAME);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }
    if (fwrite(scratch, 1, 4, f) != 4) {
        goto error;
    }

    int remaining = req->content_len - received;
    while (remaining > 0) {
        if ((received = httpd_req_recv(req, scratch, MIN(remaining, SCRATCH_BUFSIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }
            goto error;
        }
        if (received && (received != fwrite(scratch, 1, received, f))) {
            goto error;
        }
        remaining -= received;
        printf("remaining %d bytes\n", remaining);
    }
//ok:
    fclose(f);
    project_open();
    /* Redirect onto root */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
error:
    /* In case of unrecoverable error, close and delete the unfinished file*/
    fclose(f);
    unlink(PROJECT_FILENAME);
    ESP_LOGE(TAG, "File reception failed!");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
    return ESP_FAIL;
}

static esp_err_t web_firmware_upload_handler(httpd_req_t *req)
{
    /* File cannot be larger than a limit */
    if (req->content_len > 2 * 1024 * 1024) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File too large");
        return ESP_FAIL;
    }

    esp_ota_handle_t ota_handle = 0;
    const esp_partition_t *partition = esp_ota_get_next_update_partition(NULL);
    if (!partition) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Can't get update partition");
        return ESP_FAIL;
    }
    if (esp_ota_begin(partition, req->content_len, &ota_handle) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Can't start OTA");
        return ESP_FAIL;
    }

    int remaining = req->content_len;
    while (remaining > 0) {
        int received;
        if ((received = httpd_req_recv(req, scratch, MIN(remaining, SCRATCH_BUFSIZE))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry if timeout occurred */
                continue;
            }
            goto error;
        }
        if (received && esp_ota_write(ota_handle, scratch, received) != ESP_OK) {
            goto error;
        }
        remaining -= received;
    }
    if (esp_ota_end(ota_handle) != ESP_OK) {
        goto error;
    }
    if (esp_ota_set_boot_partition(partition) != ESP_OK) {
        goto error;
    }
    /* Redirect onto root */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_sendstr(req, "File uploaded successfully");
    esp_restart();
    return ESP_OK;
error:
    if (ota_handle) {
        esp_ota_abort(ota_handle);
    }
    /* In case of unrecoverable error, close and delete the unfinished file*/
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
    return ESP_FAIL;
}

void web_init(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.max_uri_handlers = 10;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGI(TAG, "Error starting server!");
        return;
    }

    ESP_LOGI(TAG, "Registering URI handlers");
    const httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = web_index_handler,
    };
    httpd_register_uri_handler(server, &index_uri);

    httpd_uri_t control_post_uri = {
        .uri = "/api/control",
        .method = HTTP_POST,
        .handler = web_control_handler,
    };
    httpd_register_uri_handler(server, &control_post_uri);

    const httpd_uri_t status_uri = {
        .uri       = "/api/status",
        .method    = HTTP_GET,
        .handler   = web_status_handler,
    };
    httpd_register_uri_handler(server, &status_uri);

    const httpd_uri_t info_uri = {
        .uri       = "/api/info",
        .method    = HTTP_GET,
        .handler   = web_info_handler,
    };
    httpd_register_uri_handler(server, &info_uri);

    const httpd_uri_t cv_defs_uri = {
        .uri       = "/api/cv/defs",
        .method    = HTTP_GET,
        .handler   = web_cv_defs_handler,
    };
    httpd_register_uri_handler(server, &cv_defs_uri);

    httpd_uri_t cv_get_uri = {
        .uri = "/api/cv/read",
        .method = HTTP_POST,
        .handler = web_cv_read_handler,
    };
    httpd_register_uri_handler(server, &cv_get_uri);

    httpd_uri_t cv_post_uri = {
        .uri = "/api/cv/write",
        .method = HTTP_POST,
        .handler = web_cv_write_handler,
    };
    httpd_register_uri_handler(server, &cv_post_uri);

    httpd_uri_t project_upload_post_uri = {
        .uri = "/api/project/upload",
        .method = HTTP_POST,
        .handler = web_project_upload_handler,
    };
    httpd_register_uri_handler(server, &project_upload_post_uri);

    httpd_uri_t firmware_upload_post_uri = {
        .uri = "/api/firmware/upload",
        .method = HTTP_POST,
        .handler = web_firmware_upload_handler,
    };
    httpd_register_uri_handler(server, &firmware_upload_post_uri);
}
