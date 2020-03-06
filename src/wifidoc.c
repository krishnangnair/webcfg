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

#include "wifi_helpers.h"
#include "wifidoc.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
enum {
    PM_OK                       = HELPERS_OK,
    PM_OUT_OF_MEMORY            = HELPERS_OUT_OF_MEMORY,
    PM_INVALID_FIRST_ELEMENT    = HELPERS_INVALID_FIRST_ELEMENT,
    PM_MISSING_PM_ENTRY         = HELPERS_MISSING_WRAPPER,
    PM_INVALID_PORT_RANGE,
    PM_INVALID_PORT_NUMBER,
    PM_INVALID_INTERNAL_IPV4,
    PM_BOTH_IPV4_AND_IPV6_TARGETS_EXIST,
    PM_INVALID_IPV6,
    PM_MISSING_INTERNAL_PORT,
    PM_INVALID_DATATYPE,
    PM_MISSING_TARGET_IP,
    PM_MISSING_PORT_RANGE,
    PM_MISSING_PROTOCOL,
    PM_INVALID_PM_OBJECT,
    PM_INVALID_VERSION,
};

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
int process_wifidocparams( wifidoc_t *e, msgpack_object_map *map );
int process_wifimappingdoc( wifimappingdoc_t *pm, int num, ...); 
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

/* See portmappingdoc.h for details. */
wifidoc_t* wifidoc_convert( const void *buf, size_t len )
{
	return wifi_helper_convert( buf, len, sizeof(wifi_t), "home_ssid_2g", 
                            MSGPACK_OBJECT_ARRAY, true,
                           (process_fn_t) process_wifimappingdoc,
                           (destroy_fn_t) wifimappingdoc_destroy );
}

/* See portmappingdoc.h for details. */
void wifidoc_destroy( wifimappingdoc_t *pm )
{
    if( NULL != pm ) {
        size_t i;
        for( i = 0; i < pm->entries_count; i++ )
        {
            if( NULL != pm->entries[i].internal_client )
            {
                free( pm->entries[i].internal_client );
            }
	    
	    if( NULL != pm->entries[i].protocol )
            {
                free( pm->entries[i].protocol );
            }
	    if( NULL != pm->entries[i].description )
            {
                free( pm->entries[i].description );
            }
        }
        if( NULL != pm->entries )
        {
            free( pm->entries );
        }
        free( pm );
    }
}

/* See webcfgdoc.h for details. */
const char* wifidoc_strerror( int errnum )
{
    struct error_map {
        int v;
        const char *txt;
    } map[] = {
        { .v = PM_OK,                               .txt = "No errors." },
        { .v = PM_OUT_OF_MEMORY,                    .txt = "Out of memory." },
        { .v = PM_INVALID_FIRST_ELEMENT,            .txt = "Invalid first element." },
        { .v = PM_INVALID_VERSION,                 .txt = "Invalid 'version' value." },
        { .v = PM_INVALID_PM_OBJECT,                .txt = "Invalid 'value' array." },
        { .v = 0, .txt = NULL }
    };
    int i = 0;

    while( (map[i].v != errnum) && (NULL != map[i].txt) ) { i++; }

    if( NULL == map[i].txt )
    {
        return "Unknown error.";
    }

    return map[i].txt;
}
/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

/**
 *  Convert the msgpack map into the doc_t structure.
 *
 *  @param e    the entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
int process_wifidocparams( wifidoc_t *e, msgpack_object_map *map )
{
    int left = map->size;
    uint8_t objects_left = 0x06;
    msgpack_object_kv *p;

    p = map->ptr;
    printf("map size:%d\n",left);
    while( (0 < objects_left) && (0 < left--) )
    {
        if( MSGPACK_OBJECT_STR == p->key.type )
        {
              if( MSGPACK_OBJECT_STR == p->val.type )
              {
                if( 0 == strncmp(p->key.via.str.ptr, "ExternalPortEndRange",strlen("ExternalPortEndRange")))
                {
                    e->external_port_end_range = strndup( p->val.via.str.ptr, p->val.via.str.size );
                    objects_left &= ~(1 << 1);
                } 
                else if( 0 == match(p, "ExternalPort") )
                {
                    e->external_port = strndup( p->val.via.str.ptr, p->val.via.str.size );
                    objects_left &= ~(1 << 2);
                }
            
                else if( 0 == match(p, "InternalClient") )
                {
                    e->internal_client = strndup( p->val.via.str.ptr, p->val.via.str.size );
                    objects_left &= ~(1 << 3);
                } 
                else if( 0 == match(p, "Protocol") )
                {
                    e->protocol = strndup( p->val.via.str.ptr, p->val.via.str.size );
                    objects_left &= ~(1 << 4);
                }
                else if( 0 == match(p, "Description") )
                {
                    e->description = strndup( p->val.via.str.ptr, p->val.via.str.size );
                    objects_left &= ~(1 << 5);
                }
    
	        else if( 0 == match(p, "Enable") )
                {
	            e->enable =  strndup( p->val.via.str.ptr, p->val.via.str.size );
	            objects_left &= ~(1 << 6);
	        }
            }
        }
           p++;
    }
        
    
    //printf("objects_left after out %d\n", objects_left);
    if( 1 & objects_left ) {
   /*     errno = PM_MISSING_INTERNAL_PORT;
    } else if( (1 << 1) & objects_left ) {
        errno = PM_MISSING_TARGET_IP;
    } else if( (1 << 2) & objects_left ) {
        errno = PM_MISSING_PORT_RANGE;
    } else if( (1 << 3) & objects_left ) {
        errno = PM_MISSING_PROTOCOL;*/
    } else {
        errno = PM_OK;
    }
   
    return (0 == objects_left) ? 0 : -1;
}

int process_wifimappingdoc( wifimappingdoc_t *pm,int num, ... )
{   
    va_list valist;
    va_start(valist, num);
    
    msgpack_object *obj = va_arg(valist, msgpack_object *);
    msgpack_object_array *array = &obj->via.array;

    if( 0 < array->size )
    {
        size_t i;

        pm->entries_count = array->size;
       
        msgpack_object *obj1 = va_arg(valist, msgpack_object *);
        pm->version = (uint32_t) obj1->via.u64;

        msgpack_object *obj2 = va_arg(valist, msgpack_object *);
        pm->transaction_id = (uint16_t) obj2->via.u64;

        va_end(valist);

        printf("pm->version in blob is %ld\n",(long)pm->version);
        printf("pm->transaction_id in blob is %d\n",pm->transaction_id);

        pm->entries = (portdoc_t *) malloc( sizeof(wifidoc_t) * pm->entries_count );

        if( NULL == pm->entries )
        {
            printf("entries count is null\n");
            pm->entries_count = 0;
            return -1;
        }

        memset( pm->entries, 0, sizeof(wifidoc_t) * pm->entries_count );

        for( i = 0; i < pm->entries_count; i++ )
        {
            if( MSGPACK_OBJECT_MAP != array->ptr[i].type )
            {
                printf("invalid PM_OBJECT \n");
                errno = PM_INVALID_PM_OBJECT;
                return -1;
            }

            if( 0 != process_wifidocparams(&pm->entries[i], &array->ptr[i].via.map) )
            {
		printf("process_wifidocparams failed\n");
                return -1;
            }
           
        }
    }

    return 0;
}