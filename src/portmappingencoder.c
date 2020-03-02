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
#include <base64.h>
#include <msgpack.h>

#include "helpers.h"
#include "portmappingencoder.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
struct portmapping_token {
    const char *name;
    size_t length;
};
/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
static const struct portmapping_token PORTMAPPING_PARAMETERS   = { .name = "parameters", .length = sizeof( "parameters" ) - 1 };
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );
static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct portmapping_token *token,
                                       const char *val );

static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n )
{
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, string, n );
}

static void __msgpack_pack_string_nvp( msgpack_packer *pk,
                                       const struct portmapping_token *token,
                                       const char *val )
{
    if( ( NULL != token ) && ( NULL != val ) )
    {
        __msgpack_pack_string( pk, token->name, token->length );
        __msgpack_pack_string( pk, val, strlen( val ) );
    }
}

void b64_encoder(const void *buf,size_t len);
int writeToFile_b64(char *filename, char *data, int len);

ssize_t portmap_pack_appenddoc(const appenddoc_t *appenddocData,void **data)
{
    size_t rv = -1;

    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
    msgpack_zone mempool;
    msgpack_object deserialized;

    if( appenddocData != NULL )
    {
        
        struct portmapping_token APPENDDOC_MAP_VERSION;
             
        APPENDDOC_MAP_VERSION.name = "version";
        APPENDDOC_MAP_VERSION.length = strlen( "version" );
        __msgpack_pack_string( &pk, APPENDDOC_MAP_VERSION.name, APPENDDOC_MAP_VERSION.length );
        msgpack_pack_int(&pk,appenddocData->version);

        struct portmapping_token APPENDDOC_MAP_TRANSACTION_ID;
             
        APPENDDOC_MAP_TRANSACTION_ID.name = "transaction_id";
        APPENDDOC_MAP_TRANSACTION_ID.length = strlen( "transaction_id" );
        __msgpack_pack_string( &pk, APPENDDOC_MAP_TRANSACTION_ID.name, APPENDDOC_MAP_TRANSACTION_ID.length );
        msgpack_pack_int(&pk, appenddocData->transaction_id);
    }
    else 
    {    
        printf("parameters is NULL\n" );
        return rv;
    } 

    if( sbuf.data ) 
    {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) 
        {
            memcpy( *data, sbuf.data, sbuf.size );
	    printf("sbuf.data of appenddoc is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_zone_init(&mempool, 2048);

    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);
    msgpack_object_print(stdout, deserialized);

    msgpack_zone_destroy(&mempool);

    msgpack_sbuffer_destroy( &sbuf );
    return rv;   
}

ssize_t portmap_pack_subdoc(const subdoc_t *subdocData,void **data)
{
    size_t rv = -1;
    int i = 0;

    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
    msgpack_zone mempool;
    msgpack_object deserialized;

    if( subdocData != NULL && subdocData->count != 0)
    {
        int count = subdocData->count;
        
        msgpack_pack_map( &pk, 1);

        struct portmapping_token SUBDOC_MAP_PORTMAPPING;
     
        SUBDOC_MAP_PORTMAPPING.name = "portforwarding";
        SUBDOC_MAP_PORTMAPPING.length = strlen( "portforwarding" );
        __msgpack_pack_string( &pk, SUBDOC_MAP_PORTMAPPING.name, SUBDOC_MAP_PORTMAPPING.length );
        
        msgpack_pack_array( &pk, count );
                
        for( i = 0; i < count; i++ )
        {
             msgpack_pack_map( &pk, 6);

             struct portmapping_token SUBDOC_MAP_INTERNALCLIENT;
             
             SUBDOC_MAP_INTERNALCLIENT.name = "InternalClient";
             SUBDOC_MAP_INTERNALCLIENT.length = strlen( "InternalClient" );
             __msgpack_pack_string_nvp( &pk, &SUBDOC_MAP_INTERNALCLIENT, subdocData->subdoc_items[i].internal_client );
             
             struct portmapping_token SUBDOC_MAP_EXTERNALPORTENDRANGE;
             
             SUBDOC_MAP_EXTERNALPORTENDRANGE.name = "ExternalPortEndRange";
             SUBDOC_MAP_EXTERNALPORTENDRANGE.length = strlen( "ExternalPortEndRange" );
             __msgpack_pack_string_nvp( &pk, &SUBDOC_MAP_EXTERNALPORTENDRANGE, subdocData->subdoc_items[i].external_port_end_range );

             struct portmapping_token SUBDOC_MAP_ENABLE;
             
             SUBDOC_MAP_ENABLE.name = "Enable";
             SUBDOC_MAP_ENABLE.length = strlen( "Enable" );
             __msgpack_pack_string( &pk, SUBDOC_MAP_ENABLE.name, SUBDOC_MAP_ENABLE.length );
             if( subdocData->subdoc_items[i].enable )
                msgpack_pack_true(&pk);
             else
                msgpack_pack_false(&pk);

             struct portmapping_token SUBDOC_MAP_PROTOCOL;
             
             SUBDOC_MAP_PROTOCOL.name = "Protocol";
             SUBDOC_MAP_PROTOCOL.length = strlen( "Protocol" );
             __msgpack_pack_string_nvp( &pk, &SUBDOC_MAP_PROTOCOL, subdocData->subdoc_items[i].protocol );

             struct portmapping_token SUBDOC_MAP_DESCRIPTION;
             
             SUBDOC_MAP_DESCRIPTION.name = "Description";
             SUBDOC_MAP_DESCRIPTION.length = strlen( "Description" );
             __msgpack_pack_string_nvp( &pk, &SUBDOC_MAP_DESCRIPTION, subdocData->subdoc_items[i].description );

             struct portmapping_token SUBDOC_MAP_EXTERNALPORT;
             
             SUBDOC_MAP_EXTERNALPORT.name = "ExternalPort";
             SUBDOC_MAP_EXTERNALPORT.length = strlen( "ExternalPort" );
             __msgpack_pack_string_nvp( &pk, &SUBDOC_MAP_EXTERNALPORT, subdocData->subdoc_items[i].external_port );

       }
         
    }
    else 
    {
        printf("parameters is NULL\n" );
        return rv;
    }

    if( sbuf.data ) 
    {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) 
        {
            memcpy( *data, sbuf.data, sbuf.size );
	    printf("sbuf.data of subdoc is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_zone_init(&mempool, 2048);

    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);
    msgpack_object_print(stdout, deserialized);

    msgpack_zone_destroy(&mempool);

    msgpack_sbuffer_destroy( &sbuf );
    return rv;
    
}

ssize_t portmap_pack_rootdoc( char *blob, const data_t *packData, void **data )
{
    size_t rv = -1;
    msgpack_sbuffer sbuf;
    msgpack_packer pk;
    msgpack_sbuffer_init( &sbuf );
    msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );

    msgpack_zone mempool;
    msgpack_object deserialized;
    
    int i =0;

    if( packData != NULL && packData->count != 0 ) {
	int count = packData->count;

  	msgpack_pack_map( &pk, 2);
        __msgpack_pack_string( &pk, PORTMAPPING_PARAMETERS.name, PORTMAPPING_PARAMETERS.length );
	msgpack_pack_array( &pk, count );
        
	msgpack_pack_map( &pk, 3); //name, value, type

	for( i = 0; i < count; i++ ) //1 element
	{
	    struct portmapping_token PORTMAPPING_MAP_NAME;

            PORTMAPPING_MAP_NAME.name = "name";
            PORTMAPPING_MAP_NAME.length = strlen( "name" );
            __msgpack_pack_string_nvp( &pk, &PORTMAPPING_MAP_NAME, "Device.NAT.PortMapping." );

	    struct portmapping_token PORTMAPPING_MAP_VALUE;

            PORTMAPPING_MAP_VALUE.name = "value";
            PORTMAPPING_MAP_VALUE.length = strlen( "value" );
	    __msgpack_pack_string_nvp( &pk, &PORTMAPPING_MAP_VALUE, blob );

	    struct portmapping_token PORTMAPPING_MAP_TYPE;

            PORTMAPPING_MAP_TYPE.name = "dataType";
            PORTMAPPING_MAP_TYPE.length = strlen( "dataType" );
             __msgpack_pack_string( &pk, PORTMAPPING_MAP_TYPE.name, PORTMAPPING_MAP_TYPE.length );
	    msgpack_pack_int(&pk, 2 );
	}

	struct portmapping_token PORTMAPPING_MAP_VERSION;

        PORTMAPPING_MAP_VERSION.name = "version";
        PORTMAPPING_MAP_VERSION.length = strlen( "version" );
	__msgpack_pack_string( &pk, PORTMAPPING_MAP_VERSION.name, PORTMAPPING_MAP_VERSION.length );
        msgpack_pack_int(&pk, 54563 );
       
        

    } else {
        printf("parameters is NULL\n" );
        return rv;
    }

    if( sbuf.data ) {
        *data = ( char * ) malloc( sizeof( char ) * sbuf.size );

        if( NULL != *data ) {
            memcpy( *data, sbuf.data, sbuf.size );
	    printf("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
            rv = sbuf.size;
        }
    }

    msgpack_zone_init(&mempool, 2048);
    
    printf("the value is :");
    msgpack_unpack(sbuf.data, sbuf.size, NULL, &mempool, &deserialized);
    msgpack_object_print(stdout, deserialized);

    msgpack_zone_destroy(&mempool);
    msgpack_sbuffer_destroy( &sbuf );
    return rv;
}

void b64_encoder(const void *buf,size_t len)
{
        char* b64buffer =  NULL;
	size_t encodeSize = 0;
        size_t size =0;
	char * decodeMsg =NULL;
	size_t decodeMsgSize =0;

       //uncomment to verify the decodeMsg
       
        msgpack_zone mempool;
	msgpack_object deserialized;
	msgpack_unpack_return unpack_ret;

	printf("-----------Start of Base64 Encode ------------\n");
	encodeSize = b64_get_encoded_buffer_size( len );
	printf("encodeSize is %ld\n", encodeSize);
	b64buffer = malloc(encodeSize+1);
	b64_encode((const uint8_t *)buf, len, (uint8_t *)b64buffer);
	b64buffer[encodeSize] = '\0' ;
	printf("---------- End of Base64 Encode -------------\n");

	//printf("Final Encoded data: %s\n",b64buffer);
	printf("Final Encoded data length: %ld\n",strlen(b64buffer));
	/*********** base64 encode *****************/

	//Start of b64 decoding
	printf("----Start of b64 decoding----\n");
	decodeMsgSize = b64_get_decoded_buffer_size(strlen(b64buffer));
	printf("expected b64 decoded msg size : %ld bytes\n",decodeMsgSize);

	decodeMsg = (char *) malloc(sizeof(char) * decodeMsgSize);

	size = b64_decode( (const uint8_t *)b64buffer, strlen(b64buffer), (uint8_t *)decodeMsg );
	printf("base64 decoded data containing %ld bytes is :%s\n",size, decodeMsg);

	printf("----End of b64 decoding----\n");

        writeToFile_b64("decodeMsg.bin", decodeMsg, (int)size);
	//End of b64 decoding

	//Start of msgpack decoding just to verify
	printf("----Start of msgpack decoding----\n");
	msgpack_zone_init(&mempool, 2048);
	unpack_ret = msgpack_unpack(decodeMsg, size, NULL, &mempool, &deserialized);
	printf("unpack_ret is %d\n",unpack_ret);
	switch(unpack_ret)
	{
		case MSGPACK_UNPACK_SUCCESS:
			printf("MSGPACK_UNPACK_SUCCESS :%d\n",unpack_ret);
			printf("\nmsgpack decoded data is:");
			msgpack_object_print(stdout, deserialized);
		break;
		case MSGPACK_UNPACK_EXTRA_BYTES:
			printf("MSGPACK_UNPACK_EXTRA_BYTES :%d\n",unpack_ret);
		break;
		case MSGPACK_UNPACK_CONTINUE:
			printf("MSGPACK_UNPACK_CONTINUE :%d\n",unpack_ret);
		break;
		case MSGPACK_UNPACK_PARSE_ERROR:
			printf("MSGPACK_UNPACK_PARSE_ERROR :%d\n",unpack_ret);
		break;
		case MSGPACK_UNPACK_NOMEM_ERROR:
			printf("MSGPACK_UNPACK_NOMEM_ERROR :%d\n",unpack_ret);
		break;
		default:
			printf("Message Pack decode failed with error: %d\n", unpack_ret);
	}

	msgpack_zone_destroy(&mempool);
	printf("----End of msgpack decoding----\n");

}

int writeToFile_b64(char *filename, char *data, int len)
{
	FILE *fp;
	fp = fopen(filename , "w+");
	if (fp == NULL)
	{
		printf("Failed to open file %s\n", filename );
		return 0;
	}
	if(data !=NULL)
	{
		fwrite(data, len, 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		printf("WriteToFile failed, Data is NULL\n");
		return 0;
	}
}





 
    
