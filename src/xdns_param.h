/*
 * Copyright 2019 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.xdns
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __XDNS_PARAM_H__
#define __XDNS_PARAM_H__
#include <stdint.h>
#include <stdlib.h>
#include <msgpack.h>
typedef struct
{
    char * dns_mac;
    char * dns_ipv4;
    char * dns_ipv6;
    char * dns_tag;      
} dnsMapping_t;

typedef struct
{
    dnsMapping_t * entries;
    size_t    entries_count;	   
} xdnsTable_t;


typedef struct {
    bool          enable_xdns;
    char *        default_ipv4;
    char *        default_ipv6;
    char *        default_tag;
    char *        subdoc_name;
    uint32_t      version;
    uint16_t      transaction_id;
    xdnsTable_t * table_param;
} xdnsdoc_t;
/**
 *  This function converts a msgpack buffer into an xdnsdoc_t structure
 *  if possible.
 *
 *  @param buf the buffer to convert
 *  @param len the length of the buffer in bytes
 *
 *  @return NULL on error, success otherwise
 */
xdnsdoc_t* xdnsdoc_convert( const void *buf, size_t len );
/**
 *  This function destroys an xdnsdoc_t object.
 *
 *  @param e the xdnsdoc to destroy
 */
void xdnsdoc_destroy( xdnsdoc_t *d );
/**
 *  This function returns a general reason why the conversion failed.
 *
 *  @param errnum the errno value to inspect
 *
 *  @return the constant string (do not alter or free) describing the error
 */
const char* xdnsdoc_strerror( int errnum );
#endif
