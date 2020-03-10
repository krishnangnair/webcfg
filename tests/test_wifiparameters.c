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
#include "../src/webcfgdoc.h"
#include "../src/webcfgparam.h"
#include "../src/multipart.h"
#include "../src/helpers.h"
#include "../src/macbindingdoc.h"
#include "../src/portmappingdoc.h"
#include "../src/portmappingencoder.h"
#include <msgpack.h>
#include <curl/curl.h>
#define FILE_URL "/tmp/webcfg_url"

char *url = NULL;
char *interface = NULL;

//To strip all spaces , new line & carriage return characters from header output
void stripLine(char *str, int len, char **final_str)
{
	int i=0, j=0;

	for(i=0;i <len;++i)
	{
		if(str[i]!='\n')
		{
			str[j++]=str[i];
		}
	}
	str[j]='\0';
	*final_str = str;
	printf("j is %d\n", j);
	printf("*final_str is %s\n", *final_str);
}

void test_multipart()
{
	int r_count=0;
	int configRet = -1;
        //webcfgparam_t *pm;
	char *webConfigData = NULL;
	long res_code;
        //int err;
        int len =0;
	//void* subdbuff;
	char *subfileData = NULL;
	
	if(url == NULL)
	{
		printf("\nProvide config URL as argument\n");
		return;
	}
	configRet = webcfg_http_request(url, &webConfigData, r_count, &res_code, interface, &subfileData, &len);
	if(configRet == 0)
	{
		printf("config ret success\n");
	}	
	else
	{
		printf("webcfg_http_request failed\n");
	}
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
	if( CUE_SUCCESS == CU_initialize_registry() )
        {
	   add_suites( &suite );

	   if( NULL != suite )
           {
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

