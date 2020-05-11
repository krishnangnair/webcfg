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
#include "../src/webcfg_param.h"
#include "../src/webcfg.h"
#include "../src/webcfg_multipart.h"
#include "../src/webcfg_helpers.h"
#include "../src/webcfg_db.h"
#include "../src/webcfg_notify.h"
#include <msgpack.h>
#include <curl/curl.h>
#include <base64.h>
#include "../src/webcfg_generic.h"
#include "../src/portmappingdoc.h"
#define FILE_URL "/tmp/webcfg_url"

#define UNUSED(x) (void )(x)

char *url = NULL;
char *interface = NULL;
char device_mac[32] = {'\0'};

char* get_deviceMAC()
{
	strcpy(device_mac, "b42xxxxxxxxx");
	return device_mac;
}
void setValues(const param_t paramVal[], const unsigned int paramCount, const int setType, char *transactionId, money_trace_spans *timeSpan, WDMP_STATUS *retStatus, int *ccspStatus)
{
	UNUSED(paramVal);
	UNUSED(paramCount);
	UNUSED(setType);
	UNUSED(transactionId);
	UNUSED(timeSpan);
	UNUSED(retStatus);
	UNUSED(ccspStatus);
	return;
}

int getForceSync(char** pString, char **transactionId)
{
	UNUSED(pString);
	UNUSED(transactionId);
	return 0;
}
int setForceSync(char* pString, char *transactionId,int *session_status)
{
	UNUSED(pString);
	UNUSED(transactionId);
	UNUSED(session_status);
	return 0;
}

char * getParameterValue(char *paramName)
{
	UNUSED(paramName);
	return NULL;
}

char * getSerialNumber()
{
	char *sNum = strdup("1234");
	return sNum;
}

char * getDeviceBootTime()
{
	char *bTime = strdup("152200345");
	return bTime;
}

char * getProductClass()
{
	char *pClass = strdup("Product");
	return pClass;
}

char * getModelName()
{
	char *mName = strdup("Model");
	return mName;
}

char * getFirmwareVersion()
{
	char *fName = strdup("Firmware.bin");
	return fName;
}

void sendNotification(char *payload, char *source, char *destination)
{
	WEBCFG_FREE(payload);
	WEBCFG_FREE(source);
	UNUSED(destination);
	return;
}

char *get_global_systemReadyTime()
{
	char *sTime = strdup("158000123");
	return sTime;
}

int Get_Webconfig_URL( char *pString)
{
	char *webConfigURL =NULL;
	loadInitURLFromFile(&webConfigURL);
	pString = webConfigURL;
        printf("The value of pString is %s\n",pString);
	return 0;
}

int Set_Webconfig_URL( char *pString)
{
	printf("Set_Webconfig_URL pString %s\n", pString);
	return 0;
}

/*void test_multipart()
{
	unsigned long status = 0;

	if(url == NULL)
	{
		printf("\nProvide config URL as argument\n");
		return;
	}
	initWebConfigNotifyTask();
	processWebconfgSync(status);
}*/
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

void test_multipart()
{
	        
	//char *subfileData = NULL;
	char* b64buffer =  NULL;
	size_t encodeSize = 0;
	size_t subLen=0;
	char * decodeMsg =NULL;
	size_t decodeMsgSize =0;
	size_t size =0;
	int err;
  	portmappingdoc_t *rpm;
	//decode inner blob
	int len =0;//, i =0;
        char *binfileData = NULL;
        char subdocfile[] = "/home/comcast/portforwarding_test.bin";
        msgpack_zone mempool;
	msgpack_object deserialized;
	msgpack_unpack_return unpack_ret;

	printf("\n--------------decode blob-------------\n");
        readFromFile1(subdocfile, &binfileData , &len);
	printf("----Start of msgpack decoding1----\n");
	
		subLen = (size_t) len;
		//subdbuff = ( void*)subfileData;
		printf("subLen is %ld\n", subLen);

		/*********** base64 encode *****************/
		printf("-----------Start of Base64 Encode ------------\n");
		encodeSize = b64_get_encoded_buffer_size( len );
		printf("b64_get_encoded_buffer_size %ld\n",encodeSize);
		printf("encodeSize is %ld\n", encodeSize);
		b64buffer = malloc(encodeSize+1);
		memset( b64buffer, 0, sizeof( encodeSize ) );
		b64_encode((const uint8_t *)binfileData, subLen, (uint8_t *)b64buffer);
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
	printf("\n----End of msgpack decoding----\n");
        printf("\n\n");
                rpm = portmappingdoc_convert( binfileData, len) ;
		err = errno;
		printf( "errno: %s\n", portmappingdoc_strerror(err) );
		//CU_ASSERT_FATAL( NULL != rpm );
		CU_ASSERT_FATAL( NULL != rpm->entries );

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
    CU_add_test( *suite, "Full", test_multipart);
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
	unsigned rv = 1;
	CU_pSuite suite = NULL;
	// int len=0;
	printf("argc %d \n", argc );
	if(argv[1] !=NULL)
	{
		url = strdup(argv[1]);
	}
	// Read url from file
	//readFromFile(FILE_URL, &url, &len );
	if(url !=NULL && strlen(url)==0)
	{
		printf("<url> is NULL.. add url in /tmp/webcfg_url file\n");
		return 0;
	}
	printf("url fetched %s\n", url);
	if(argv[2] !=NULL)
	{
		interface = strdup(argv[2]);
	}
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

