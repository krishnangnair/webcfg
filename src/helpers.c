/*
 * Copyright 2020 Comcast Cable Communications Management, LLC
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

#include "helpers.h"
#include "webcfgdoc.h"
#include "portmappingdoc.h"
/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
msgpack_object* __finder( const char *name, 
                          msgpack_object_type expect_type,
                          msgpack_object_map *map );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
void* helper_convert( const void *buf, size_t len,
                      size_t struct_size, const char *wrapper,
                      msgpack_object_type expect_type, bool optional,
                      process_fn_t process, 
                      destroy_fn_t destroy )
{
    void *p = malloc( struct_size );

    if( NULL == p ) {
        errno = HELPERS_OUT_OF_MEMORY;
    } else {
        memset( p, 0, struct_size );

        if( NULL != buf && 0 < len ) {
            size_t offset = 0;
            msgpack_unpacked msg;
            msgpack_unpack_return mp_rv;

            msgpack_unpacked_init( &msg );

            /* The outermost wrapper MUST be a map. */
            mp_rv = msgpack_unpack_next( &msg, (const char*) buf, len, &offset );
	    printf("mp_rv is %d MSGPACK_UNPACK_SUCCESS %d offset %lu\n", mp_rv, MSGPACK_UNPACK_SUCCESS, offset);
	    msgpack_object obj = msg.data;
	    msgpack_object_print(stdout, obj);
            printf("\nMSGPACK_OBJECT_MAP is %d  msg.data.type %d\n", MSGPACK_OBJECT_MAP, msg.data.type);

            if( (MSGPACK_UNPACK_SUCCESS == mp_rv) && (0 != offset) &&
                (MSGPACK_OBJECT_MAP == msg.data.type) )
            {
                msgpack_object *inner;
                msgpack_object *root_version;
                msgpack_object *blob_version;
                msgpack_object *transaction_id;

                inner = &msg.data;
                root_version = &msg.data;
                blob_version = &msg.data;
                transaction_id = &msg.data;
                
                
                if( NULL != wrapper && 0 == strncmp(wrapper,"parameters",strlen("parameters"))) 
                {
                    inner = __finder( wrapper, expect_type, &msg.data.via.map );
                    root_version =  __finder( "version", expect_type, &msg.data.via.map );
                    
                    printf("root_version is %ld\n", (size_t)root_version);

                    if( ((NULL != inner) && (0 == (process)(p, 2, inner, root_version))) || 
                              ((true == optional) && (NULL == inner)) )
                    {
                         msgpack_unpacked_destroy( &msg );
                         errno = HELPERS_OK;

                         return p;
                    }
                    else 
                    {
                         errno = HELPERS_INVALID_FIRST_ELEMENT;
                    }
                }
                else if( NULL != wrapper && 0 != strcmp(wrapper,"parameters")) 
                {
                    inner = __finder( wrapper, expect_type, &msg.data.via.map );
                    blob_version =  __finder( "version", expect_type, &msg.data.via.map );
                    transaction_id =  __finder( "transaction_id", expect_type, &msg.data.via.map );

                    printf("blob_version is %ld\n", (size_t)blob_version);
                    printf("transaction_id is %ld\n", (size_t)transaction_id);
                    
                    if( ((NULL != inner) && (0 == (process)(p,3, inner, blob_version, transaction_id))) || 
                              ((true == optional) && (NULL == inner)) )
                    {
                         msgpack_unpacked_destroy( &msg );
                         errno = HELPERS_OK;

                         return p;
                    }
                    else 
                    {
                         errno = HELPERS_INVALID_FIRST_ELEMENT;
                    }
                } 

              }
            msgpack_unpacked_destroy( &msg );
            if(NULL!=p)
            {
               (destroy)( p );
                p = NULL;
            }
            
        }
    }
    return p;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

msgpack_object* __finder( const char *name, 
                          msgpack_object_type expect_type,
                          msgpack_object_map *map )
{
    uint32_t i;
    
   // printf("The Map_size is %d\n",map->size);
    for( i = 0; i < map->size; i++ ) 
    {
        if( MSGPACK_OBJECT_STR == map->ptr[i].key.type ) 
        {
            //printf("The val.type is : %d\n",map->ptr[i].val.type);
            //printf("expect_type :%d\n",expect_type);
            if( expect_type == map->ptr[i].val.type ) 
            {
                if( 0 == match(&(map->ptr[i]), name) ) 
                {
                    return &map->ptr[i].val;
                }
            }
            else if(MSGPACK_OBJECT_STR == map->ptr[i].val.type)
            {   
                if(0 == strncmp(map->ptr[i].key.via.str.ptr, name, strlen(name)))
                {   
                    return &map->ptr[i].val;
                }
                
             }
            }
        }
     errno = HELPERS_MISSING_WRAPPER;
    return NULL;
    }

   
