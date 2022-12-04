/* Copyright (c) 2017 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */
#include <string.h>
#include <stdlib.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"
#include <sys/time.h>
#include <esp_task_wdt.h>

#include "pbtx_client.h"

#include "mbedtls/sha256.h"
#include "pbtx-rpc.pb.h"
#include <pb_encode.h>
#include <pb_decode.h>

static const char* TAG = "main";

#define BUFLEN 4096

static unsigned char buf[BUFLEN];



static void dump_buf( unsigned char *buf, size_t len )
{
    size_t i;
    for( i = 0; i < len; i++ )
        printf("%c%c", "0123456789ABCDEF" [buf[i] / 16],
               "0123456789ABCDEF" [buf[i] % 16] );
    printf( "\n" );
}




void vTaskCode( void * pvParameters )
{
    size_t msgsize;
    if( pbtx_client_rpc_register_account(buf, BUFLEN, &msgsize) != 0 ) {
        esp_system_abort("pbtx_client_rpc_register_account error");
    }

    pbtxrpc_RegisterAccountResponse msg;

    mbedtls_sha256_context md_ctx;
    mbedtls_sha256_init( &md_ctx );
    mbedtls_sha256_starts( &md_ctx, 0 );
    mbedtls_sha256_update( &md_ctx, buf, msgsize );
    mbedtls_sha256_finish( &md_ctx, msg.request_hash.bytes );
    mbedtls_sha256_free( &md_ctx );
    msg.request_hash.size = 32;

    msg.status = 0;
    msg.network_id = 9223372036854775808ULL;
    msg.last_seqnum = 4294900000;
    msg.prev_hash = 9223372036854775808ULL;

    pb_ostream_t respstream = pb_ostream_from_buffer(buf, BUFLEN);
    if( !pb_encode(&respstream, pbtxrpc_RegisterAccountResponse_fields, &msg) ) {
        ESP_LOGE(TAG, "RegisterAccountResponse pb_encode error: %s", PB_GET_ERROR(&respstream));
        esp_system_abort("");
    }

    if( pbtx_client_rpc_register_account_response(buf, respstream.bytes_written) != 0 ) {
        esp_system_abort("pbtx_client_rpc_register_account_response error");
    }

    for( ;; )
    {
        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        int64_t time_start_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;

        size_t olen;
        if( pbtx_client_rpc_transaction(4294900000, buf, 2048, buf, BUFLEN, &olen) != 0 ) {
            esp_system_abort("pbtx_client_rpc_transaction error");
        }

        ESP_LOGI(TAG, "message length: %d", olen);
                
        gettimeofday(&tv_now, NULL);
        int64_t time_stop_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
                
        ESP_LOGI(TAG, "Elapsed %f ms", (float)(time_stop_us - time_start_us)/1e3);
        vTaskDelay(50);
    }
}


void app_main() {
    pbtx_client_init();

    TaskHandle_t xHandle = NULL;
    xTaskCreate( vTaskCode, "NAME", 10000, NULL, 1, &xHandle );
    configASSERT( xHandle );
}
