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
#include <stdlib.h>
#include <msgpack.h>

#include "helpers.h"
#include "multipart.h"
#include "webcfgparam.h"
#include "webcfg_log.h"
#include "webcfg_db.h"
#include "webcfg_generic.h"
#include "webcfgpack.h"
#include "macbindingdoc.h"
#include "portmappingdoc.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
enum {
    WD_OK                       = HELPERS_OK,
    WD_OUT_OF_MEMORY            = HELPERS_OUT_OF_MEMORY,
    WD_INVALID_FIRST_ELEMENT    = HELPERS_INVALID_FIRST_ELEMENT,
    WD_INVALID_DATATYPE,
    WD_INVALID_WD_OBJECT,
};

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
static webconfig_tmp_data_t * g_head = NULL;
static int numOfMpDocs = 0;
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
int process_webcfgdbparams( webconfig_db_data_t *e, msgpack_object_map *map );
int process_webcfgdb( webconfig_db_t *pm, msgpack_object *obj );

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

int initDB(char * db_file_path )
{
     FILE *fp;
     char *data;
     size_t len;
     int ch_count=0;
     size_t k;
     WebConfigLog("DB file path is %s\n", db_file_path);
     fp = fopen(db_file_path,"rb");

     if (fp == NULL)
     {
	WebConfigLog("Failed to open file %s\n", db_file_path);
	return 0;
     }
     
     fseek(fp, 0, SEEK_END);
     ch_count = ftell(fp);
     fseek(fp, 0, SEEK_SET);
     data = (char *) malloc(sizeof(char) * (ch_count + 1));
     fread(data, 1, ch_count-1,fp);
     len = ch_count;
     (data)[ch_count] ='\0';
     fclose(fp);

     wd = ( webconfig_db_t * ) malloc( sizeof( webconfig_db_t ) );
     wd = decodeData((void *)data, len);
     WebConfigLog("Size of webcfgdb %ld\n", (size_t)wd);
     if(wd != NULL)
     {   
         webcfgdb_data = NULL;
         
         for(k = 0;k< wd->entries_count ; k++)
         {   
             webconfig_db_data_t * webcfgdb = NULL;
             webcfgdb = (webconfig_db_data_t *) malloc (sizeof(webconfig_db_data_t));
            
             webcfgdb->name = wd->entries[k].name;
             webcfgdb->version = wd->entries[k].version;
             webcfgdb->next = NULL;

             addToDBList(webcfgdb);
         }

         /*For test purpose */
          size_t j = wd->entries_count;
          webconfig_db_data_t* temp1 = webcfgdb_data;
          while(temp1 )
          {   
              WebConfigLog("db->entries[%lu].name %s\n", (wd->entries_count)-j, temp1->name);
	      WebConfigLog("db->entries[%lu].version %lu\n" ,(wd->entries_count)-j,  (long)temp1->version); 
              j--; 
            
              temp1 = temp1->next;
          }
        
     }
       
     return 1;
     
}

int addNewDocEntry(webconfig_db_t *subdoc)
{    
     size_t webcfgdbPackSize = -1;
     void* data = NULL;
 
     WebConfigLog("size of subdoc %ld\n", (size_t)subdoc);
     webcfgdbPackSize = webcfgdb_pack(subdoc, &data);
     WebConfigLog("size of webcfgdbPackSize %ld\n", webcfgdbPackSize);
     WebConfigLog("writeToDBFile %s\n", WEBCFG_DB_FILE);
     writeToDBFile(WEBCFG_DB_FILE,(char *)data);
  
     return 0;
}

int writeToDBFile(char *db_file_path, char *data)
{
	FILE *fp;
	fp = fopen(db_file_path , "w+");
	if (fp == NULL)
	{
		WebConfigLog("Failed to open file in db %s\n", db_file_path );
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
		WebConfigLog("WriteToJson failed, Data is NULL\n");
		return 0;
	}
}

webconfig_db_t* decodeData(const void * buf, size_t len)
{
     return helper_convert( buf, len, sizeof(webconfig_db_t),"webcfgdb",
                           MSGPACK_OBJECT_ARRAY, true,
                           (process_fn_t) process_webcfgdb,
                           (destroy_fn_t) webcfgdb_destroy );
}

void webcfgdb_destroy( webconfig_db_t *wd )
{
    if( NULL != wd ) {
        size_t i;
        for( i = 0; i < wd->entries_count; i++ ) {
            if( NULL != wd->entries[i].name ) {
                free( wd->entries[i].name );
            }
	    
        }
        if( NULL != wd->entries ) {
            free( wd->entries );
        }
        free( wd );
    }
}

const char* webcfgdbparam_strerror( int errnum )
{
    struct error_map {
        int v;
        const char *txt;
    } map[] = {
        { .v = WD_OK,                               .txt = "No errors." },
        { .v = WD_OUT_OF_MEMORY,                    .txt = "Out of memory." },
        { .v = WD_INVALID_FIRST_ELEMENT,            .txt = "Invalid first element." },
        { .v = WD_INVALID_DATATYPE,                 .txt = "Invalid 'datatype' value." },
        { .v = WD_INVALID_WD_OBJECT,                .txt = "Invalid 'parameters' array." },
        { .v = 0, .txt = NULL }
    };
    int i = 0;

    while( (map[i].v != errnum) && (NULL != map[i].txt) ) { i++; }

    if( NULL == map[i].txt ) {
        return "Unknown error.";
    }

    return map[i].txt;
}

webconfig_tmp_data_t * get_global_node(void)
{
    return g_head;
}

int get_numOfMpDocs()
{
    return numOfMpDocs;
}

//new_node indicates the docs which need to be added to list
int addToTmpList( multipart_t *mp)
{
	int m = 0;
	int retStatus = 0;

	WebConfigLog("mp->entries_count is %zu\n", mp->entries_count);
	for(m = 0 ; m<((int)mp->entries_count); m++)
	{
		webconfig_tmp_data_t *new_node;
		new_node=(webconfig_tmp_data_t *)malloc(sizeof(webconfig_tmp_data_t));
		if(new_node)
		{
			memset( new_node, 0, sizeof( webconfig_tmp_data_t ) );

			if(numOfMpDocs == 0)
			{
				new_node->name = strdup("root");
				new_node->version = 0;
				new_node->status = strdup("pending");
			}
			else
			{
				new_node->name = strdup(mp->entries[m-1].name_space);
				new_node->version = mp->entries[m-1].etag;
				new_node->status = strdup("pending");
			}

			WebConfigLog("new_node->name is %s\n", new_node->name);
			WebConfigLog("new_node->version is %lu\n", (long)new_node->version);
			WebConfigLog("new_node->status is %s\n", new_node->status);


			new_node->next=NULL;

			if (g_head == NULL)
			{
				WebConfigLog("Adding first doc to list\n");
				g_head = new_node;
			}
			else
			{
				webconfig_tmp_data_t *temp = NULL;
				WebConfigLog("Adding docs to list\n");
				temp = g_head;

				while(temp->next !=NULL)
				{
					temp=temp->next;
				}

				temp->next=new_node;
			}

			WebConfigLog("--->>doc %s with version %lu is added to list\n", new_node->name, (long)new_node->version);
			numOfMpDocs = numOfMpDocs + 1;
		}
		WebConfigLog("numOfMpDocs %d\n", numOfMpDocs);

		if((int)mp->entries_count == numOfMpDocs)
		{
			WebConfigLog("addToTmpList success\n");
			retStatus = 1;
		}
	}
    WebConfigLog("addToList return %d\n", retStatus);
    return retStatus;
}

//update version, status for each doc
int updateTmpList(char *docname, uint32_t version, char *status)
{
	webconfig_tmp_data_t *temp = NULL;
	temp = get_global_node();

	//Traverse through doc list & update required doc
	while (NULL != temp)
	{
		WebConfigLog("node is pointing to temp->name %s \n",temp->name);
		if( strcmp(docname, temp->name) == 0)
		{
			temp->version = version;
			temp->status = strdup(status);
			WebConfigLog("-->>doc %s is updated to version %lu status %s\n", docname, (long)temp->version, temp->status);
			return 1;
		}
		temp= temp->next;
	}
	return 0;
}


//delete doc from webcfg Tmp list
int deleteFromTmpList(char* doc_name)
{
	webconfig_tmp_data_t *prev_node = NULL, *curr_node = NULL;

	if( NULL == doc_name )
	{
		WebConfigLog("Invalid value for doc\n");
		return -1;
	}
	WebConfigLog("doc to be deleted: %s\n", doc_name);

	prev_node = NULL;
	curr_node = g_head ;

	// Traverse to get the doc to be deleted
	while( NULL != curr_node )
	{
		if(strcmp(curr_node->name, doc_name) == 0)
		{
			WebConfigLog("Found the node to delete\n");
			if( NULL == prev_node )
			{
				WebConfigLog("need to delete first doc\n");
				g_head = curr_node->next;
			}
			else
			{
				WebConfigLog("Traversing to find node\n");
				prev_node->next = curr_node->next;
			}

			WebConfigLog("Deleting the node\n");
			free( curr_node );
			curr_node = NULL;
			WebConfigLog("Deleted successfully and returning..\n");
			numOfMpDocs =numOfMpDocs - 1;
			WebConfigLog("numOfMpDocs after delete is %d\n", numOfMpDocs);
			return 0;
		}

		prev_node = curr_node;
		curr_node = curr_node->next;
	}
	WebConfigLog("Could not find the entry to delete from list\n");
	return -1;
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

/**
 *  Convert the msgpack map into the webconfig_db_data_t structure.
 *
 *  @param e    the entry pointer
 *  @param map  the msgpack map pointer
 *
 *  @return 0 on success, error otherwise
 */
int process_webcfgdbparams( webconfig_db_data_t *e, msgpack_object_map *map )
{
    int left = map->size;
    uint8_t objects_left = 0x02;
    msgpack_object_kv *p;

    p = map->ptr;
    while( (0 < objects_left) && (0 < left--) )
    {
        if( MSGPACK_OBJECT_STR == p->key.type )
        {
            if( MSGPACK_OBJECT_POSITIVE_INTEGER == p->val.type )
            {
                if( 0 == match(p, "version") )
                {
                    if( UINT32_MAX < p->val.via.u64 )
                    {
			//printf("e->type is %d\n", e->type);
                        errno = WD_INVALID_DATATYPE;
                        return -1;
                    }
                    else
                    {
                        e->version = (uint32_t) p->val.via.u64;
			WebConfigLog("e->version is %lu\n", (long)e->version);
                    }
                    objects_left &= ~(1 << 1);
		    //printf("objects_left after datatype %d\n", objects_left);
                }
                
            }
            else if( MSGPACK_OBJECT_STR == p->val.type )
            {
                if( 0 == match(p, "name") )
                {
                    e->name = strndup( p->val.via.str.ptr, p->val.via.str.size );
		    WebConfigLog("e->name is %s\n", e->name);
                    objects_left &= ~(1 << 0);
		    //printf("objects_left after name %d\n", objects_left);
                }
            }
        }
        p++;
    }

    if( 1 & objects_left )
    {
    }
    else
    {
        errno = WD_OK;
    }

    return (0 == objects_left) ? 0 : -1;
}

int process_webcfgdb( webconfig_db_t *wd, msgpack_object *obj )
{
    //printf(" process_webcfgdbparam \n");
    msgpack_object_array *array = &obj->via.array;
    if( 0 < array->size )
    {
        size_t i;

        wd->entries_count = array->size;
        wd->entries = (webconfig_db_data_t *) malloc( sizeof(webconfig_db_data_t) * wd->entries_count );
        if( NULL == wd->entries )
        {
            wd->entries_count = 0;
            return -1;
        }
        
        //printf("wd->entries_count %lu\n",wd->entries_count);
        memset( wd->entries, 0, sizeof(webconfig_db_data_t) * wd->entries_count );
        for( i = 0; i < wd->entries_count; i++ )
        {
            if( MSGPACK_OBJECT_MAP != array->ptr[i].type )
            {
                errno = WD_INVALID_WD_OBJECT;
                return -1;
            }
            if( 0 != process_webcfgdbparams(&wd->entries[i], &array->ptr[i].via.map) )
            {
		//printf("process_webcfgdbparam failed\n");
                return -1;
            }
        }
    }

    return 0;
}