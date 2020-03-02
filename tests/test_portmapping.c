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
#include "../src/portmappingencoder.h"
#include "../src/webcfgparam.h"
#include "../src/multipart.h"

void portMappingPackUnpack();
/*
	{
	  "parameters": [
	    {
	      "name": "Device.NAT.PortMapping.",
	      "value": "<blob>",
	      "datatype": 0
	    }
	  ]
	}
============
	[{'InternalClient': '10.0.0.196', 'ExternalPortEndRange': '7777', 'Enable': 'true', 'Protocol': 'BOTH', 'Description': 'SI-7777-7777-10.0.0.196', 'ExternalPort': '7777'}, {'InternalClient': '10.0.0.196', 'ExternalPortEndRange': '8389', 'Enable': 'true', 'Protocol': 'BOTH', 'Description': 'SI-8389-8389-10.0.0.196', 'ExternalPort': '8389'}]
*/

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

int writeToFile1(char *filename, char *data)
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


void test_portmappingpack_unpack()
{
	subdoc_t *packSubdocData = NULL;
        size_t subdocPackSize = -1;
        void *data = NULL;
        
        packSubdocData = ( subdoc_t * ) malloc( sizeof( subdoc_t ) );
        
        if( packSubdocData != NULL )
        {
            memset(packSubdocData, 0, sizeof(subdoc_t));

            packSubdocData->count = 2;
            packSubdocData->subdoc_items = (struct subdoc *) malloc( sizeof(struct subdoc) * packSubdocData->count );
            memset( packSubdocData->subdoc_items, 0, sizeof(struct subdoc) * packSubdocData->count);

            packSubdocData->subdoc_items[0].internal_client ="10.0.0.196";
            packSubdocData->subdoc_items[0].enable = "true";
            packSubdocData->subdoc_items[0].external_port_end_range = "7777";
            packSubdocData->subdoc_items[0].protocol = "BOTH";        
            packSubdocData->subdoc_items[0].description = "SI-7777-7777-10.0.0.196";        
            packSubdocData->subdoc_items[0].external_port = "7777"; 

            packSubdocData->subdoc_items[1].internal_client = "10.0.0.196";
            packSubdocData->subdoc_items[1].enable = "true";
            packSubdocData->subdoc_items[1].external_port_end_range = "8389";
            packSubdocData->subdoc_items[1].protocol = "BOTH";        
            packSubdocData->subdoc_items[1].description = "SI-8389-8389-10.0.0.196";        
            packSubdocData->subdoc_items[1].external_port = "8389";                

            
        }

        printf("webcfg_pack_subdoc\n");
	subdocPackSize = portmap_pack_subdoc( packSubdocData, &data );
	printf("subdocPackSize is %ld\n", subdocPackSize);
	printf("data packed is %s\n", (char*)data);
        
        portMappingPackUnpack(data);
/*

	int len=0;
	char subdocfile[64] = "../../tests/portmapping-server-response.bin";

	char *binfileData = NULL;
	int status = -1;
	char* blobbuff = NULL; 

	status = readFromFile(subdocfile , &binfileData , &len);

	printf("read status %d\n", status);
	if(status == 1 )
	{
		blobbuff = ( char*)binfileData;
		printf("blobbuff %s blob len %lu\n", blobbuff, strlen(blobbuff));

		portMappingPackUnpack(blobbuff);
	} */
   //portMappingPackUnpack();

}

void portMappingPackUnpack(char *blob)
//void portMappingPackUnpack()
{  
   	data_t *packRootData = NULL;
	size_t rootPackSize=-1;
	void *data =NULL;
	int err;
	
	packRootData = ( data_t * ) malloc( sizeof( data_t ) );
	if(packRootData != NULL)
	{
		memset(packRootData, 0, sizeof(data_t));

		packRootData->count = 1;
                //packRootData->version = strdup("70984884608473595535252890235591967179");
		packRootData->data_items = (struct rootdata *) malloc( sizeof(struct rootdata) * packRootData->count );
		memset( packRootData->data_items, 0, sizeof(struct rootdata) * packRootData->count );

		packRootData->data_items[0].name = strdup("Device.NAT.PortMapping.");
		packRootData->data_items[0].value = strdup(blob);
		packRootData->data_items[0].type = 0;
	}
	printf("webcfg_pack_rootdoc\n");
	rootPackSize = portmap_pack_rootdoc( blob, packRootData, &data );
	printf("rootPackSize is %ld\n", rootPackSize);
	printf("data packed is %s\n", (char*)data); 
        
	int status = writeToFile1("buff.bin", (char*)data);
        //int status = writeToFile("buff.bin", (char*)blob);
	if(status)
	{
		webcfgparam_t *pm;
		int len =0;//, i =0;
		void* rootbuff;
		char *binfileData = NULL;

		int status = readFromFile1("buff.bin", &binfileData , &len);
	
		if(status)
		{
			rootbuff = ( void*)binfileData;

			//decode root doc
			printf("--------------decode root doc-------------\n");
			pm = webcfgparam_convert( rootbuff, len+1 );
			err = errno;
			printf( "errno: %s\n", webcfgparam_strerror(err) );
			//CU_ASSERT_FATAL( NULL != pm );
			//CU_ASSERT_FATAL( NULL != pm->entries );
			CU_ASSERT_FATAL( 1 == pm->entries_count );
                        //CU_ASSERT_STRING_EQUAL("154363892090392891829182011",pm->version);
			CU_ASSERT_STRING_EQUAL( "Device.NAT.PortMapping.", pm->entries[0].name );
			//CU_ASSERT_STRING_EQUAL( blob, pm->entries[0].value );
			CU_ASSERT_FATAL( 2 == pm->entries[0].type );
			//for(i = 0; i < (int)pm->entries_count ; i++)
			//{
				printf("pm->entries[0].name %s\n",  pm->entries[0].name);
				printf("pm->entries[0].value %s\n" , pm->entries[0].value);
				printf("pm->entries[0].type %d\n", pm->entries[0].type);
                               // printf("pm->version %d\n",  pm->version);
			//}
                        //Appending docs
                        printf("\n-----------------------------Appenddocs---------------------------\n");
                        appenddoc_t *appenddata = NULL;
                        size_t appenddocPackSize = -1;
                        size_t embeddeddocPackSize = -1;
                        void *appenddocdata = NULL;
                        void *embeddeddocdata = NULL;
 
                        msgpack_zone mempool;
			msgpack_object deserialized;
			msgpack_unpack_return unpack_ret;

                        appenddata = (appenddoc_t *) malloc(sizeof(appenddoc_t ));
                        if(appenddata != NULL)
                        {   
                            memset(appenddata, 0, sizeof(appenddoc_t));
 
                            appenddata->version = 2689711003;
                            appenddata->transaction_id = 1001;
                        }

                        printf("Append Doc \n");
                        appenddocPackSize = portmap_pack_appenddoc(appenddata, &appenddocdata);
                        printf("appenddocPackSize is %ld\n", appenddocPackSize);
	                printf("data packed is %s\n", (char*)appenddocdata);
                        

                        printf("---------------------------------------------------------------\n");
                        embeddeddocPackSize = appendEncodedData(&embeddeddocdata, (void *)pm->entries[0].value, (size_t)pm->entries[0].value_size, appenddocdata, appenddocPackSize);
                         printf("embeddeddocPackSize is %ld\n", embeddeddocPackSize);
	                printf("data packed is %s\n", (char*)embeddeddocdata);

                        b64_encoder(embeddeddocdata,embeddeddocPackSize);
 
                        printf("\n-----------------------------End of Appenddocs---------------------------\n");

			//decode inner blob
	/*		portmappingdoc_t *rpm;
                        int len =0;//, i =0;
		        char *binfileData = NULL;
                        char subdocfile[64] = "decodeMsg.bin";
                        msgpack_zone mempool;
			msgpack_object deserialized;
			msgpack_unpack_return unpack_ret;

			printf("--------------decode blob-------------\n");
                        readFromFile1(subdocfile, &binfileData , &len);*/
                //Start of msgpack decoding just to verify
		printf("----Start of msgpack decoding----\n");
		msgpack_zone_init(&mempool, 2048);
		unpack_ret = msgpack_unpack(embeddeddocdata, embeddeddocPackSize, NULL, &mempool, &deserialized);
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
			//rpm = portmappingdoc_convert( pm->entries[0].value, strlen(pm->entries[0].value) );
                  //      rpm = portmappingdoc_convert( binfileData, len) ;
			//printf("blob len %lu\n", strlen(pm->entries[0].value));
			//err = errno;
			//printf( "errno: %s\n", portmappingdoc_strerror(err) );
		//	CU_ASSERT_FATAL( NULL != rpm );
		//	CU_ASSERT_FATAL( NULL != rpm->entries );
			//CU_ASSERT_FATAL( 4 == rpm->entries_count );
			
/*
			//first blob
			CU_ASSERT_STRING_EQUAL( "blob1", rpm->entries[0].name );
			CU_ASSERT_STRING_EQUAL( "http://example.com/mac:112233445566/blob1", rpm->entries[0].url );
			CU_ASSERT_FATAL( 1234 == rpm->entries[0].version );

			//second blob
			CU_ASSERT_STRING_EQUAL( "blob2", rpm->entries[1].name );
			CU_ASSERT_STRING_EQUAL( "http:/example.com/<mac>/blob2", rpm->entries[1].url );
			CU_ASSERT_FATAL( 3366 == rpm->entries[1].version );

			//third blob
			CU_ASSERT_STRING_EQUAL( "blob3", rpm->entries[2].name );
			CU_ASSERT_STRING_EQUAL( "$host/example.com/<mac>/blob3", rpm->entries[2].url );
			CU_ASSERT_FATAL( 1357 == rpm->entries[2].version );

			//fourth blob
			CU_ASSERT_STRING_EQUAL( "blob4", rpm->entries[3].name );
			CU_ASSERT_STRING_EQUAL( "$host/example.com<mac>/blob4", rpm->entries[3].url );
			CU_ASSERT_FATAL( 65535 == rpm->entries[3].version );

*/
			/*for(int i = 0; i < (int)rpm->entries_count ; i++)
			{
				printf("rpm->entries[%d].InternalClient %s\n", i, rpm->entries[i].internal_client);
				printf("rpm->entries[%d].ExternalPortEndRange %s\n" , i, rpm->entries[i].external_port_end_range);
				printf("rpm->entries[%d].Enable %s\n", i, rpm->entries[i].enable?"true":"false");
				printf("rpm->entries[%d].Protocol %s\n", i, rpm->entries[i].protocol);
				printf("rpm->entries[%d].Description %s\n", i, rpm->entries[i].description);
				printf("rpm->entries[%d].external_port %s\n", i, rpm->entries[i].external_port);
			}

			portmappingdoc_destroy( rpm );*/
			webcfgparam_destroy( pm );
		}

	}

		
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test_portmappingpack_unpack", test_portmappingpack_unpack);
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
