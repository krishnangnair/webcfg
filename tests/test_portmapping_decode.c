 /**
  * Copyright 2019 Comcast Cable Communications Management, LLC
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
 */
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <CUnit/Basic.h>
#include "../src/portmappingdoc.h"
#include "../src/webcfg.h"
#include "../src/webcfg_db.h"
#include "../src/webcfg_pack.h"
#include "../src/webcfg_multipart.h"
#include "../src/portmappingdoc.h"

int readFromFile1(char *filename, char **data, int *len)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		printf("Failed to open file %s\n", filename);
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	ch_count = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	*data = (char *) malloc(sizeof(char) * (ch_count + 1));
	
	fread(*data, 1, ch_count,fp);
        //fgets(*data,400,fp);
        printf("........data is %s len%lu\n", *data, strlen(*data));
	*len = ch_count;
	(*data)[ch_count] ='\0';
        printf("character count is %d\n",ch_count);
        printf("data is %s len %lu\n", *data, strlen(*data));
	fclose(fp);
	return 1;
}

int writeToFile(char *filename, char *data)
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
		fwrite(data, strlen(data), 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		printf("WriteToJson failed, Data is NULL\n");
		return 0;
	}
}


void test_portmapping_decode()
{
        int err;
  
	//decode inner blob
	portmappingdoc_t *rpm;
        int len =0;//, i =0;
        char *binfileData = NULL;
        char subdocfile[64] = "/home/comcast/portforwarding_test.bin";
        msgpack_zone mempool;
	msgpack_object deserialized;
	msgpack_unpack_return unpack_ret;

	printf("\n--------------decode blob-------------\n");
        readFromFile1(subdocfile, &binfileData , &len);
	printf("----Start of msgpack decoding1----\n");
	char * base64blobencoder(char *binfileData, size_t len );
        //Start of msgpack decoding just to verify
	printf("----Start of msgpack decoding----\n");
	msgpack_zone_init(&mempool, 2048);
	unpack_ret = msgpack_unpack(binfileData, len, NULL, &mempool, &deserialized);
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
	printf("\n----End of msgpack decoding----\n");
        printf("\n\n");
                rpm = portmappingdoc_convert( binfileData, len) ;
		err = errno;
		printf( "errno: %s\n", portmappingdoc_strerror(err) );
		CU_ASSERT_FATAL( NULL != rpm );
		//CU_ASSERT_FATAL( NULL != rpm->entries );

		for(int i = 0; i < (int)rpm->entries_count ; i++)
		{
			printf("rpm->entries[%d].InternalClient %s\n", i, rpm->entries[i].internal_client);
			printf("rpm->entries[%d].ExternalPortEndRange %s\n" , i, rpm->entries[i].external_port_end_range);
			printf("rpm->entries[%d].Enable %s\n", i, rpm->entries[i].enable?"true":"false");
			printf("rpm->entries[%d].Protocol %s\n", i, rpm->entries[i].protocol);
			printf("rpm->entries[%d].Description %s\n", i, rpm->entries[i].description);
			printf("rpm->entries[%d].external_port %s\n", i, rpm->entries[i].external_port);
		}

		portmappingdoc_destroy( rpm );


}


void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test_portmapping_decode", test_portmapping_decode);
}


/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
    unsigned rv = 1;
    CU_pSuite suite = NULL;
 
    (void ) argc;
    (void ) argv;
    
    if( CUE_SUCCESS == CU_initialize_registry() ) {
        add_suites( &suite );

        if( NULL != suite ) {
            CU_basic_set_mode( CU_BRM_VERBOSE );
            CU_basic_run_tests();
            printf( "\n" );
            CU_basic_show_failures( CU_get_failure_list() );
            printf( "\n\n" );
            rv = CU_get_number_of_tests_failed();
        }

        CU_cleanup_registry();

    }

    return rv;
}
