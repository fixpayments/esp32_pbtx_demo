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


#include "pbtx_client.h"
#include "pbtx_signature_provider.h"


static const char* TAG = "main";

#define BUFLEN 1024

static unsigned char buf[BUFLEN];



static void dump_buf( unsigned char *buf, size_t len )
{
    size_t i;
    for( i = 0; i < len; i++ )
        printf("%c%c", "0123456789ABCDEF" [buf[i] / 16],
               "0123456789ABCDEF" [buf[i] % 16] );
    printf( "\n" );
}


void app_main() {
    pbtx_client_init();
    size_t olen;
    int err = pbtx_client_get_public_key(buf, BUFLEN, &olen);
    if( err != 0 ) {
        ESP_LOGE(TAG, "pbtx_client_get_public_key returned %d", err);
    }
    ESP_LOGI(TAG, "key len: %d", olen);
    dump_buf(buf, olen);

    char* msg = "message";
    err = pbtx_sigp_sign((unsigned char*) msg, strlen(msg), buf, BUFLEN,  &olen);
    if( err != 0 ) {
        ESP_LOGE(TAG, "pbtx_sigp_sign returned %x", -err);
    }    
    else {
        ESP_LOGI(TAG, "sig len: %d", olen);
        dump_buf(buf, olen);
    }
}



