/*
 * Copyright 2019 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
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
#include <errno.h>
#include <string.h>
#include <msgpack.h>
#include <stdarg.h>
#include "webcfg_log.h"
#include "comp_helpers.h"
#include "xdns_param.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */
/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
enum {
    OK                       = HELPERS_OK,
    OUT_OF_MEMORY            = HELPERS_OUT_OF_MEMORY,
    INVALID_FIRST_ELEMENT    = HELPERS_INVALID_FIRST_ELEMENT,
    MISSING_ENTRY         = HELPERS_MISSING_WRAPPER,
    INVALID_PORT_RANGE,
    INVALID_PORT_NUMBER,
    INVALID_INTERNAL_IPV4,
    BOTH_IPV4_AND_IPV6_TARGETS_EXIST,
    INVALID_IPV6,
    MISSING_INTERNAL_PORT,
    INVALID_DATATYPE,
    MISSING_TARGET_IP,
    MISSING_PORT_RANGE,
    MISSING_PROTOCOL,
    INVALID_OBJECT,
    INVALID_VERSION,
};
/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
int process_xdnsparams( xdnsdoc_t *e, msgpack_object_map *map );
int process_dnsparams( dnsMapping_t *e, msgpack_object_map *map );
int process_xdnsdoc( xdnsdoc_t *xd, int num, ...); 
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
/* See xdnsdoc.h for details. */
xdnsdoc_t* xdnsdoc_convert( const void *buf, size_t len )
{
	return comp_helper_convert( buf, len, sizeof(xdnsdoc_t), "xdns", 
                            MSGPACK_OBJECT_MAP, true,
                           (process_fn_t) process_xdnsdoc,
                           (destroy_fn_t) xdnsdoc_destroy );
}
/* See xdnsdoc.h for details. */
void xdnsdoc_destroy( xdnsdoc_t *xd )
{
	size_t i;

	if( NULL != xd )
	{
		if( NULL != xd->default_ipv4 )
		{
			free(xd->default_ipv4);
		}
		if( NULL != xd->default_ipv6)
		{
			free(xd->default_ipv6);
		}
		if( NULL != xd->default_tag )
		{
			free(xd->default_tag);
		}

		if( NULL != xd->table_param )
		{
			if( NULL != xd->table_param->entries )
			{
				for( i = 0; i < xd->table_param->entries_count; i++ )
				{
					if( NULL != xd->table_param->entries[i].dns_mac )
					{
						free(xd->table_param->entries[i].dns_mac);
					}
					if( NULL != xd->table_param->entries[i].dns_ipv4 )
					{
						free(xd->table_param->entries[i].dns_ipv4);
					}
					if( NULL != xd->table_param->entries[i].dns_ipv6 )
					{
						free(xd->table_param->entries[i].dns_ipv6);
					}
					if( NULL != xd->table_param->entries[i].dns_tag )
					{
						free(xd->table_param->entries[i].dns_tag);
					}
				}
				free(xd->table_param->entries);
			}
			free(xd->table_param);
		}
		if( NULL != xd->subdoc_name )
		{
			free( xd->subdoc_name );
		}
		free( xd );
	}
}
/* See xdnsdoc.h for details. */
const char* xdnsdoc_strerror( int errnum )
{
    struct error_map {
        int v;
        const char *txt;
    } map[] = {
        { .v = OK,                               .txt = "No errors." },
        { .v = OUT_OF_MEMORY,                    .txt = "Out of memory." },
        { .v = INVALID_FIRST_ELEMENT,            .txt = "Invalid first element." },
        { .v = INVALID_VERSION,                 .txt = "Invalid 'version' value." },
        { .v = INVALID_OBJECT,                .txt = "Invalid 'value' array." },
        { .v = 0, .txt = NULL }
    };
    int i = 0;
    while( (map[i].v != errnum) && (NULL != map[i].txt) ) { i++; }
    if( NULL == map[i].txt )
    {
	WebcfgDebug("----xdnsdoc_strerror----\n");
        return "Unknown error.";
    }
    return map[i].txt;
}
/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/**
 *  Convert the msgpack map into the dnsMapping_t structure.
 *
 *  @param e    the entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
int process_dnsparams( dnsMapping_t *e, msgpack_object_map *map )
{
    int left = map->size;
    uint8_t objects_left = 0x04;
    msgpack_object_kv *p;
    p = map->ptr;
    while( (0 < objects_left) && (0 < left--) )
    {
        if( MSGPACK_OBJECT_STR == p->key.type )
        {
              if(MSGPACK_OBJECT_STR == p->val.type)
              {
                 if( 0 == match(p, "DNSMappingMac") )
                 {
                     e->dns_mac = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 0);
                 }
                 if( 0 == match(p, "DNSMappingIPv4") )
                 {
                     e->dns_ipv4 = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 1);
                 }
                 if( 0 == match(p, "DNSMappingIPv6") )
                 {
                     e->dns_ipv6 = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 3);
                 }
                 if( 0 == match(p, "DNSMappingTag") )
                 {
                     e->dns_tag = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 2);
                 }
		
              }
             
        }
           p++;
    }
        
    
    if( 1 & objects_left ) {
    } else {
        errno = OK;
    }
   
    return (0 == objects_left) ? 0 : -1;
}

/**
 *  Convert the msgpack map into the doc_t structure.
 *
 *  @param e    the entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
int process_xdnsparams( xdnsdoc_t *e, msgpack_object_map *map )
{
    int left = map->size;
    size_t i =0;
    uint8_t objects_left = 0x05;
    msgpack_object_kv *p;
    p = map->ptr;
    while( (0 < objects_left) && (0 < left--) )
    {
        if( MSGPACK_OBJECT_STR == p->key.type )
        {
              if( MSGPACK_OBJECT_BOOLEAN == p->val.type )
              {
                 if( 0 == match(p, "EnableXDNS") )
                 {
                     e->enable_xdns = p->val.via.boolean;
                     objects_left &= ~(1 << 0);
                 }
              }
              else if(MSGPACK_OBJECT_STR == p->val.type)
              {
                 if( 0 == match(p, "DefaultDeviceDnsIPv4") )
                 {
                     e->default_ipv4 = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 1);
                 }
                 if( 0 == match(p, "DefaultDeviceDnsIPv6") )
                 {
                     e->default_ipv6 = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 3);
                 }
                 if( 0 == match(p, "DefaultDeviceTag") )
                 {
                     e->default_tag = strndup( p->val.via.str.ptr, p->val.via.str.size );
                     objects_left &= ~(1 << 4);
                 }

              }
              else if( MSGPACK_OBJECT_ARRAY == p->val.type )
              {
                 if( 0 == match(p, "xdnsTable") )
                 {
                      e->table_param = (xdnsTable_t *) malloc( sizeof(xdnsTable_t) );
                      if( NULL == e->table_param )
                      {
	                  WebcfgDebug("table_param malloc failed\n");
                          return -1;
                      }
                      memset( e->table_param, 0, sizeof(xdnsTable_t));

                      e->table_param->entries_count = p->val.via.array.size;

                      e->table_param->entries = (dnsMapping_t *) malloc( sizeof(dnsMapping_t) * e->table_param->entries_count);

                      if( NULL == e->table_param->entries )
                      {
	                  WebcfgDebug("table_param malloc failed\n");
                          e->table_param->entries_count = 0;
                          return -1;
                      }
                      memset( e->table_param->entries, 0, sizeof(dnsMapping_t) * e->table_param->entries_count);

                      for( i = 0; i < e->table_param->entries_count; i++ )
                      {
                          if( MSGPACK_OBJECT_MAP != p->val.via.array.ptr[i].type )
                          {
                              printf("invalid OBJECT \n");
                              errno = INVALID_OBJECT;
                              return -1;
                          }

                          if( 0 != process_dnsparams(&e->table_param->entries[i], &p->val.via.array.ptr[i].via.map) )
                          {
		              printf("process_dnsparams failed\n");
                              return -1;
                          }
           
                      }
			printf("Inside xdnstable\n");
                      objects_left &= ~(1 << 2);
                }
             }
        }
           p++;
    }
        
    
    if( 1 & objects_left ) {
    } else {
        errno = OK;
    }
   
    return (0 == objects_left) ? 0 : -1;
}
int process_xdnsdoc( xdnsdoc_t *xd,int num, ... )
{
//To access the variable arguments use va_list 
	va_list valist;
	va_start(valist, num);//start of variable argument loop

	msgpack_object *obj = va_arg(valist, msgpack_object *);//each usage of va_arg fn argument iterates by one time
	msgpack_object_map *mapobj = &obj->via.map;

	msgpack_object *obj1 = va_arg(valist, msgpack_object *);
	xd->subdoc_name = strndup( obj1->via.str.ptr, obj1->via.str.size );

	msgpack_object *obj2 = va_arg(valist, msgpack_object *);
	xd->version = (uint32_t) obj2->via.u64;

	msgpack_object *obj3 = va_arg(valist, msgpack_object *);
	xd->transaction_id = (uint16_t) obj3->via.u64;

	va_end(valist);//End of variable argument loop

	if( 0 != process_xdnsparams(xd, mapobj) )
	{
		WebcfgDebug("process_xdnsparams failed\n");
		return -1;
	}

    return 0;
}
