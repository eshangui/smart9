
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "sqlite3.h"
#include "db_api.h"

#define DEBUG
#include "dbg.h"


int db_table_create( char *tbl_name, char *field_list, uint32_t field_len )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;

	char cmd_tbl_create[1024] = "create table if not exists ";

	//VAL_S(DB_PATH);
	sq_ret = sqlite3_open( DB_PATH, &db );
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db) );
		return -1;
	}
	strncat( cmd_tbl_create, tbl_name, strlen(tbl_name) );
	strncat( cmd_tbl_create, " ( ", 3 );
	strncat( cmd_tbl_create, field_list, field_len );
	strncat( cmd_tbl_create, " ) ", 3 );

	printf("create table --> %s\n", cmd_tbl_create);
	sq_ret = sqlite3_exec( db, cmd_tbl_create, NULL, NULL, &errmsg );
	if( sq_ret != SQLITE_OK ) {
		printf( "create table fail: %s\n",errmsg );
		sqlite3_free( errmsg );
		sqlite3_close( db );
		return -1;
	}

	sqlite3_free( errmsg );
	sqlite3_close( db );

	return 0;
}

int db_table_insert( char *tbl_name, char *updatge_filed_list, uint32_t update_field_len,  char *val, uint32_t val_len )
{
	char *errmsg = 0;
	int sq_ret = 0;
	char **dbresult;
	int nrow, ncolumn;
	sqlite3 *db = 0;

	char cmd_tbl_select[1024] = "select ";
	char cmd_tbl_insert[1024] = "insert into ";

	sq_ret = sqlite3_open( DB_PATH, &db );
	if( sq_ret != SQLITE_OK ) {
		printf( "Cannot open db: %s\n", sqlite3_errmsg(db) );
		goto err;
	}

	strncat( cmd_tbl_select, updatge_filed_list, update_field_len);
	strncat( cmd_tbl_select, " from ", 6 );
	strncat( cmd_tbl_select, tbl_name, strlen(tbl_name) );
	printf("select --> %s\n", cmd_tbl_select );
	sq_ret = sqlite3_get_table( db, cmd_tbl_select, &dbresult, &nrow, &ncolumn, &errmsg );
	if( sq_ret != SQLITE_OK ) {
		printf("select record from table fail: %s\n",errmsg);
		goto err;
	} else {
		strncat( cmd_tbl_insert, tbl_name, strlen(tbl_name) );
		strncat( cmd_tbl_insert, " ( ", 3 );
		strncat( cmd_tbl_insert, updatge_filed_list, update_field_len );
		strncat( cmd_tbl_insert, " ) values ( ", 12 );
		strncat( cmd_tbl_insert, val, val_len );
		strncat( cmd_tbl_insert, " ) ", 3 );
		printf("inset --> %s\n", cmd_tbl_insert );
		sq_ret = sqlite3_exec( db, cmd_tbl_insert, NULL, NULL, &errmsg );
		if( sq_ret != SQLITE_OK ) {
			fprintf( stderr, "insert record to table fail: %s\n", errmsg );
			goto err;
		}
	}

	sqlite3_free_table( dbresult );
	sqlite3_free( errmsg );
	sqlite3_close( db );
	return 0;
err:
	sqlite3_free_table( dbresult );
	sqlite3_free( errmsg );
	sqlite3_close( db );
	return -1;
}

int db_tbl_inquire( char *tbl_name, char *inquire_filed_list, uint32_t inquire_field_len, char (*out)[256], uint32_t *outlen, int id )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char **dbresult;
	int i=0,nrow,ncolumn, index = 0;
	char cmd_tbl_select[1024] = "select ";

	sq_ret = sqlite3_open(DB_PATH,&db);
	if( sq_ret != SQLITE_OK ) {
		ERR("Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}

	strncat( cmd_tbl_select, inquire_filed_list, inquire_field_len );
	strncat( cmd_tbl_select, " from ", 6 );
	strncat( cmd_tbl_select, tbl_name, strlen(tbl_name) );
	strncat( cmd_tbl_select, " where id = ", 12 );
	sprintf( cmd_tbl_select + strlen(cmd_tbl_select), "%d", id );

    printf("inquire --> %s\n", cmd_tbl_select );
	sq_ret = sqlite3_get_table( db, cmd_tbl_select, &dbresult, &nrow, &ncolumn, &errmsg );
    printf("nrow = %d\n",nrow );
    printf("ncolumn = %d\n",ncolumn );

	if ( nrow == 0 ) { 
		sqlite3_free_table(dbresult);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		printf("ncolumn = 0\n");
		return -2;
	}

	if( sq_ret == SQLITE_OK ) {
		/*printf("there is %i records in table.\n",nrow);*/
		index = ncolumn + 1;// skip id
		if ( ncolumn == 1 ) {
            if ( dbresult[1] && out[i] )  {
                memcpy( out[i], dbresult[1], strlen(dbresult[1]));
                *outlen = 1;
            } else {
                ERR("no data in the table\n");
                return -2;
            }
		} else {
			for( i=0; i<ncolumn-1; i++ ) {
				/*printf("dbresult[%d] = %s\n", i, dbresult[index]);*/
                if ( dbresult[index] ) {
                    memcpy(out[i], dbresult[index], strlen(dbresult[index]) );
                    index ++;
                }
			}
			*outlen = ncolumn - 1;
		}
	} else {
		sqlite3_free_table(dbresult);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		ERR("get table: %s\n",errmsg);
		return -1;
	}

	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
}

int db_tbl_inquire2( char *tbl_name,
		char *inquire_filed_list,
		uint32_t inquire_field_len,
		char *cond,
		uint32_t cond_len,
		char *cond_val,
		uint32_t cond_val_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char **dbresult;
	int nrow,ncolumn;
	char cmd_tbl_select[1024] = "select ";

	sq_ret = sqlite3_open(DB_PATH,&db);
	if( sq_ret != SQLITE_OK ) {
		fprintf(stderr,"Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}

	strncat( cmd_tbl_select, inquire_filed_list, inquire_field_len );
	strncat( cmd_tbl_select, " from ", 6 );
	strncat( cmd_tbl_select, tbl_name, strlen(tbl_name) );
	strncat( cmd_tbl_select, " where ", 7 );
	strncat( cmd_tbl_select, cond, cond_len );
	strncat( cmd_tbl_select, " = ", 3 );
	strncat( cmd_tbl_select, cond_val, cond_val_len );

	printf("inquire --> %s\n", cmd_tbl_select );
	sq_ret = sqlite3_get_table( db, cmd_tbl_select, &dbresult, &nrow, &ncolumn, &errmsg );
	printf("nrow = %d\n",nrow );
	printf("ncolumn = %d\n",ncolumn );

	if( sq_ret == SQLITE_OK ) {
		/*printf("there is %i records in table.\n",nrow*ncolumn);*/
		*out_count = nrow;
		printf("*out_count = %d\n", *out_count);
		/*index = ncolumn + 1;// skip id*/
		/*if ( ncolumn == 1 ) {*/
		/*printf("dbresult[%d] = %s\n", i, dbresult[1]);*/
		/*memcpy( out[i], dbresult[1], strlen(dbresult[1]));*/
		/**outlen = 1;*/
		/*} else {*/
		/*for( i=0; i<ncolumn-1; i++ ) {*/
		/*printf("dbresult[%d] = %s\n", i, dbresult[index]);*/
		/*memcpy(out[i], dbresult[index], strlen(dbresult[index]) );*/
		/*index ++;*/
		/*}*/
		/**outlen = ncolumn - 1;*/
		/*}*/
		cb( param, dbresult, nrow, ncolumn );
	} else {
		sqlite3_free_table(dbresult);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		printf("get table: %s\n",errmsg);
		return -1;
	}

	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
}

int db_tbl_inquire3( char *tbl_name,
		char *inquire_filed_list,
		uint32_t inquire_field_len,
		char *cond,
		uint32_t cond_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char **dbresult;
	int nrow,ncolumn;
	char cmd_tbl_select[1024] = "select ";

	sq_ret = sqlite3_open(DB_PATH,&db);
	if( sq_ret != SQLITE_OK ) {
		fprintf(stderr,"Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}

	strncat( cmd_tbl_select, inquire_filed_list, inquire_field_len );
	strncat( cmd_tbl_select, " from ", 6 );
	strncat( cmd_tbl_select, tbl_name, strlen(tbl_name) );
	strncat( cmd_tbl_select, " where ", 7 );
	strncat( cmd_tbl_select, cond, cond_len );

	printf("inquire --> %s\n", cmd_tbl_select );
	sq_ret = sqlite3_get_table( db, cmd_tbl_select, &dbresult, &nrow, &ncolumn, &errmsg );
	printf("nrow = %d\n",nrow );
	printf("ncolumn = %d\n",ncolumn );

	if ( ncolumn == 0 ) {
		sqlite3_free_table(dbresult);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		ERR("ncolumn = 0\n");
		return -2;
	}

	if( sq_ret == SQLITE_OK ) {
		/*printf("there is %i records in table.\n",nrow*ncolumn);*/
		*out_count = nrow;
		printf("*out_count = %d\n", *out_count);
		if ( cb )
			cb( param, dbresult, nrow, ncolumn );
	} else {
		sqlite3_free_table(dbresult);
		sqlite3_free(errmsg);
		sqlite3_close(db);
		printf("get table: %s\n",errmsg);
		return -1;
	}

	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
}

int db_tbl_all_fields_inquire( char *tbl_name,
		char *cond,
		uint32_t cond_len,
		char *cond_val,
		uint32_t cond_val_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param)
{
	return ( db_tbl_inquire2( tbl_name,
				"*",
				1,
				cond,
				cond_len,
				cond_val,
				cond_val_len,
				out_count,
				cb,
				param) );
}

int db_tbl_all_fields_inquire3( char *tbl_name,
		char *cond,
		uint32_t cond_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param)
{
	return ( db_tbl_inquire3( tbl_name,
				"*",
				1,
				cond,
				cond_len,
				out_count,
				cb,
				param) );
}

int db_tbl_update(char *tbl_name,
		uint32_t tbl_name_len,
		char *update_field,
		uint32_t update_field_len,
		char *update_val,
		uint32_t update_val_len ,
		uint32_t id)
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_update[1024] = "update ";
	char buff_id[32] = { 0 };

	sprintf( buff_id, "%d", id);
	printf("buff_id = %s\n", buff_id);

	strncat( cmd_update, tbl_name, tbl_name_len );
	strncat( cmd_update, " set ", 5 );
	strncat( cmd_update, update_field, update_field_len );
	strncat( cmd_update, " = ", 3 );
	strncat( cmd_update, update_val, update_val_len );
	strncat( cmd_update, " where id = ", 12);
	strncat( cmd_update, buff_id, strlen(buff_id)) ;
	printf("cmd_update = %s\n", cmd_update );

	sq_ret = sqlite3_open(DB_PATH,&db);
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		goto err;
	}

	sq_ret = sqlite3_exec( db, cmd_update, NULL, NULL, &errmsg);
	if(sq_ret != SQLITE_OK) {
		ERR("sqlite3_exec error: %s\n",errmsg);
		goto err;
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
err:

	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;
}

int db_tbl_update2(char *tbl_name,
		uint32_t tbl_name_len,
		char *update_val,
		uint32_t update_val_len,
		uint32_t id)
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_update[1024] = "update ";
	char buff_id[32] = { 0 };

	sprintf( buff_id, "%d", id);
	printf("buff_id = %s\n", buff_id);

	strncat( cmd_update, tbl_name, tbl_name_len );
	strncat( cmd_update, " set ", 5 );
	strncat( cmd_update, update_val, update_val_len );
	strncat( cmd_update, " where id = ", 12);
	strncat( cmd_update, buff_id, strlen(buff_id)) ;
	printf("cmd_update = %s\n", cmd_update );

	sq_ret = sqlite3_open(DB_PATH,&db);
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		goto err;
	}

	sq_ret = sqlite3_exec( db, cmd_update, NULL, NULL, &errmsg);
	if(sq_ret != SQLITE_OK) {
		printf("create table fail: %s\n",errmsg);
		goto err;
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
err:

	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;
}

int db_tbl_update3(char *tbl_name,
		char *update_field,
		char *update_val,
		char *cond,
		char *cond_val)
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_update[1024] = "update ";
	LINE();

	VAL( strlen(cond_val) );
	strncat( cmd_update, tbl_name, strlen(tbl_name) );
	strncat( cmd_update, " set ", 5 );
	strncat( cmd_update, update_field, strlen(update_field) );
	strncat( cmd_update, " = ", 3 );
	strncat( cmd_update, update_val, strlen(update_val) );
	strncat( cmd_update, " where ", 7);
	strncat( cmd_update, cond, strlen(cond) );
	strncat( cmd_update, " = ", 3 );
	VAL( strlen(cond_val) );
	strncat( cmd_update, cond_val, strlen(cond_val) );
	printf("cmd_update = %s\n", cmd_update );

	sq_ret = sqlite3_open(DB_PATH,&db);
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		goto err;
	}

	sq_ret = sqlite3_exec( db, cmd_update, NULL, NULL, &errmsg);
	if(sq_ret != SQLITE_OK) {
		ERR("sqlite3_exec error: %s\n",errmsg);
		goto err;
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;
err:

	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;
}

/*
 *
 * add '' to a string
 * ex:
 *   char buff[] = "hello"; --> char buff[] = "'hello'"
 *
 *
 * */
int db_value_convert( char *val, uint32_t len, uint32_t *outlen )
{
	char tmp[256] = { 0 }, *p = val;

	memcpy( tmp, val, len );
	*p++ = '\'';
	memcpy( p, tmp, len );
	p += len;
	*p++ = '\'';
    *p = '\0';

	/*printf( "p = %s\n", p);*/
	*outlen = len + 2;
	return 0;
}

int db_value_buff_setup( uint8_t *val, uint32_t len, char *out, uint32_t *outlen, uint8_t is_bcd, uint8_t is_last )
{
	char buff[64] = { 0 };
	uint32_t datalen = 0;
	char res[64] = { 0 };

	if ( !val || !out || !outlen ) {
		printf("param check error\n");
		return -1;
	}

	/*DBG_BUFF_DUMP( val, len, "val");*/

	memcpy( buff, val, len );

	if ( is_bcd ) {
		hex2str( (uint8_t *)buff, len, res, &datalen );
		memcpy( buff, res, datalen );
	} else {
		datalen = len;
	}
	/*printf("buff = %s\n", buff);*/
	db_value_convert( buff, datalen, &datalen );
	/*printf("buff = %s\n", buff);*/
	/*printf("datalen = %d\n", datalen);*/
	if ( is_last ) {
		buff[datalen] = ' ';
		buff[datalen + 1] = '\0';

		memcpy( out, buff, datalen+1 );
		/*printf("buff = %s\n", buff);*/
		*outlen = datalen + 1;
	} else {
		buff[datalen] = ',';
		/*printf("buff = %s\n", buff);*/
		buff[datalen + 1] = ' ';
		buff[datalen + 2] = '\0';

		memcpy( out, buff, datalen+2 );
		/*printf("buff = %s\n", buff);*/
		*outlen = datalen + 2;
	}

	return 0;
}

int db_data_parse( char *in, uint32_t inlen, uint8_t *out, uint32_t *outlen, uint8_t is_char )
{
	if (!in || !out || !outlen ) {
		printf("param check error\n");
		return -1;
	}

	if ( is_char ) {
		memcpy( out, in, inlen );
	} else {
		str2hex( in, inlen, out, outlen );
	}

	return 0;
}

/*
 *
 *

 typedef struct {
 uint8_t *addr;
 uint32_t len;
 uint8_t is_bcd;
 } db_val_t;

 char buff1[] = "hello";
 int i = 10;
ex:
db_val_t demo[] =
{
{
buff1,// variable address
strlen(buff1), // variable length
1, // bcd or not
},
{
&i,
4,
1
}
};
val_amount : ARR_SZ(demo);

 * */
int db_tbl_common_insert( db_val_t *val, int32_t val_amount, char *tbl_name, char *insert_field, int id )
{
	char buff[1024] = { 0 }, *pbuff = buff;
	uint32_t len = 0;
	db_val_t *p = val;
	int i=0;

	if ( !val || !tbl_name || !insert_field ) {
		printf("param check error\n");
		return -1;
	}


	sprintf( pbuff, "%d", id );
	strcat( pbuff, ", ");
	/*printf("pbuff = %s\n", pbuff );*/
	pbuff += strlen(pbuff);

	for ( i=0; i<val_amount; i++) {
		if ( i == val_amount - 1)
			db_value_buff_setup( p->addr, p->len, pbuff, &len, p->is_bcd, 1 );
		else
			db_value_buff_setup( p->addr, p->len, pbuff, &len, p->is_bcd, 0 );

		pbuff += len;
		/*printf("buff = %s\n", buff);*/
		p++;
	}
	printf("buff = %s\n", buff);

	if ( db_table_insert( tbl_name,
				insert_field,
				strlen(insert_field),
				buff,
				strlen(buff) ) < 0 ) {
		return -1;
	}
	return 0;
}

int db_data_common_parse(db_parse_t *in, uint32_t inlen, char (*buff)[256] )
{
	int i = 0;
	db_parse_t *p = in;

	for ( i=0; i<inlen; i++ ) {
		printf("%s\n", buff[i]);
		db_data_parse( buff[i], strlen(buff[i]), p->addr, p->len, p->is_char );
		p++;
	}
	return 0;
}

int db_tbl_clr( char *tbl_name )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;

	char cmd_tbl_del[64] = " delete from ";
	/*char cmd_update[128] = "update sqlite_sequence SET seq = 0 where name = ";*/

	sq_ret = sqlite3_open( DB_PATH, &db );
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db) );
		return -1;
	}
	strncat( cmd_tbl_del, tbl_name, strlen(tbl_name) );

	printf("clear table --> %s\n", cmd_tbl_del);
	sq_ret = sqlite3_exec( db, cmd_tbl_del, NULL, NULL, &errmsg );
	if( sq_ret != SQLITE_OK ) {
		printf( "create table fail: %s\n",errmsg );
		goto err;
	}

	/*strncat( cmd_update, tbl_name, strlen(tbl_name) );*/
	/*printf("update table --> %s\n", cmd_update);*/
	/*sq_ret = sqlite3_exec( db, cmd_update, NULL, NULL, &errmsg );*/
	/*if( sq_ret != SQLITE_OK ) {*/
	/*printf( "create table fail: %s\n",errmsg );*/
	/*return -1;*/
	/*}*/

	sqlite3_free( errmsg );
	sqlite3_close( db );

	return 0;
err:
	sqlite3_free( errmsg );
	sqlite3_close( db );
	return -1;
}

int db_tbl_comon_field_update( char *tbl_name,
		uint32_t tbl_name_len,
		char *update_field,
		uint32_t update_field_len,
		uint8_t *val,
		uint32_t len,
		uint32_t id)
{
	char buff[64] = { 0 };
	uint32_t datalen = 0;
	char res[64] = { 0 };
	int ret = 0;

	if ( !tbl_name || !update_field || !val ) {
		printf("param check error\n");
		return -1;
	}

	hex2str( val, len, res, &datalen );
	printf("res = %s\n", res);
	printf("datalen = %d\n", datalen );
	memcpy( buff, res, datalen );
	db_value_convert( buff, datalen, &datalen );
	printf("buff = %s\n", buff );
	printf("datalen = %d\n", datalen );
	ret = db_tbl_update(tbl_name,
			tbl_name_len,
			update_field,
			update_field_len,
			buff,
			datalen,
			id);
	if ( ret < 0 ) {
		printf("db_tbl_update error\n");
		return -1;
	}

	return 0;
}

int db_record_amount_get( char *tbl_name, int *outlen )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_select[256] = "select count(*) from ";
	char **dbresult;
	int nrow, ncolumn;

	if ( !outlen || !tbl_name  ) {
		printf("check param error\n");
		return -1;
	}

	strcat( cmd_select, tbl_name );
	strcat( cmd_select, " ;");

	/*printf("select --> %s\n", cmd_select );*/
	sq_ret = sqlite3_open(DB_PATH,&db);
	if( sq_ret != SQLITE_OK ) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}
	sq_ret = sqlite3_get_table( db, cmd_select, &dbresult, &nrow, &ncolumn, &errmsg );
	if( sq_ret == SQLITE_OK ) {
		printf("nrow = %d\n", nrow);
		/*printf("ncolumn = %d\n", ncolumn);*/
		/*printf("dbresult[1] = %s\n", dbresult[1]);*/
        if ( dbresult[1] )
            *outlen = atoi( dbresult[1] );
	} else {
		printf("get record error\n");
		goto err;
	}

	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;

err:
	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;
}

int db_record_amount_get_by_cond( char *tbl_name,
		int *outlen ,
		char *cond,
		uint32_t cond_len,
		char *cond_val,
		uint32_t cond_val_len)
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_select[256] = "select count(*) from ";
	char **dbresult;
	int nrow, ncolumn;

	if ( !outlen || !tbl_name  ) {
		printf("check param error\n");
		return -1;
	}

	strncat( cmd_select, tbl_name, strlen(tbl_name) );
	strncat( cmd_select, " where ", 7);
	strncat( cmd_select, cond, cond_len );
	strncat( cmd_select, " = ", 3);
	strncat( cmd_select, cond_val, cond_val_len );
	strncat( cmd_select, " ; ", 3);

	printf("select --> %s\n", cmd_select );
	sq_ret = sqlite3_open(DB_PATH,&db);
	if( sq_ret != SQLITE_OK ) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}
	sq_ret = sqlite3_get_table( db, cmd_select, &dbresult, &nrow, &ncolumn, &errmsg );
	if( sq_ret == SQLITE_OK ) {
		printf("nrow = %d\n", nrow);
		printf("ncolumn = %d\n", ncolumn);
        if ( dbresult[1] ) {
            printf("dbresult[1] = %s\n", dbresult[1]);
            *outlen = atoi( dbresult[1] );
        }
	} else {
		printf("get record error\n");
		goto err;
	}

	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return 0;

err:
	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;
}

int db_tbl_insert_fields_get( db_fields_t *fields, char *out, int *outlen )
{
	db_fields_t *p = fields;
	char buff[512] = { 0 };

	if ( !fields || !out ) {
		ERR("param check error\n");
		return -1;
	}

	while( p->field ) {
		strncat( buff, p->field, strlen(p->field) );
		strncat( buff, ", ", 2 );
		p++;
	}

	buff[strlen(buff)-2] = '\0';
	memcpy( out, buff, strlen(buff));
	if ( outlen )
		*outlen = strlen(buff);

	return 0;
}

int db_tbl_common_inquire( db_parse_t *info, char *tbl_name, char *fields, int id)
{
	char buff[64][256];
	uint32_t len = 0;
	int i = 0, ret = 0;
	db_parse_t *p = info;

	memset( buff, 0, sizeof(buff) );

	ret = db_tbl_inquire( tbl_name,
			fields,
			strlen(fields),
			buff,
			&len,
			id);
	if ( ret < 0 ) {
		if ( ret != -2 ) {
			ERR("db_tbl_inquire() error\n");
		}
		else {
			printf("there is 0 record in db\n");
		}
		return ret;
	}
	/*printf("len = %d\n", len); */

	for ( i=0; i<len; i++ ) {
		/*printf("%s\n", buff[i]);*/
		/*printf("p->addr = 0x%p\n", p->addr);*/
		db_data_parse( buff[i], strlen(buff[i]), p->addr, p->len, p->is_char );
		p++;
	}
	return 0;
}

int db_record_del(char *tbl_name, uint32_t id )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_del[50] = "delete from account where id = ";
	char buff_id[64] = { 0 };

	sprintf( buff_id, "%d", id );
	strncat( cmd_del, buff_id, strlen(buff_id));
	printf("sq::%s\n", cmd_del);
	sq_ret = sqlite3_open( DB_PATH, &db );
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		goto err;
	}
	sq_ret = sqlite3_exec(db, cmd_del, NULL, NULL, &errmsg);
	if(sq_ret != SQLITE_OK) {
		printf("create table fail: %s\n",errmsg);
		goto err;
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);
	return 0;
err:
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;

}

int db_record_del2(char *tbl_name, char *field, char *field_val )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_del[50] = "delete from ";

	strncat( cmd_del, tbl_name, strlen(tbl_name) );
	strncat( cmd_del, " where ", strlen(" where ") );
	strncat( cmd_del, field, strlen(field) );
	strncat( cmd_del, " = ", 3 );
	strncat( cmd_del, field_val, strlen(field_val) );

	printf("sq::%s\n", cmd_del);
	sq_ret = sqlite3_open( DB_PATH, &db );
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		goto err;
	}
	sq_ret = sqlite3_exec(db, cmd_del, NULL, NULL, &errmsg);
	if(sq_ret != SQLITE_OK) {
		printf("create table fail: %s\n",errmsg);
		goto err;
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);
	return 0;
err:
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;

}

int db_record_del3(char *tbl_name, uint32_t id )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char cmd_del[50] = "delete from ";
	char buff_id[64] = { 0 };

	sprintf( buff_id, "%d", id );
	strncat( cmd_del, tbl_name, strlen(tbl_name) );
	strncat( cmd_del, " where id = ", strlen(" where id = ") );
	strncat( cmd_del, buff_id, strlen(buff_id));
	printf("sq::%s\n", cmd_del);
	sq_ret = sqlite3_open( DB_PATH, &db );
	if(sq_ret != SQLITE_OK) {
		printf("Cannot open db: %s\n",sqlite3_errmsg(db));
		goto err;
	}
	sq_ret = sqlite3_exec(db, cmd_del, NULL, NULL, &errmsg);
	if(sq_ret != SQLITE_OK) {
		printf("create table fail: %s\n",errmsg);
		goto err;
	}

	sqlite3_free(errmsg);
	sqlite3_close(db);
	return 0;
err:
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return -1;

}

int db_tbl_common_create( db_tbl_info_t *info )
{
	char fields[512] = { 0 };
	db_fields_t *p = info->fields;

	while( p->field ) {
		strncat( fields, p->field, strlen(p->field) );
		strncat( fields, " ", 1);
		strncat( fields, p->type, strlen(p->type) );
		strncat( fields, ", ", 2);
		p++;
	}

	VAL((int)strlen(fields));
	fields[strlen(fields)-2] = '\0';
	VAL((int)strlen(fields));

	//VAL_S( fields );

	return ( db_table_create( info->tbl_name, fields, strlen(fields) ) );
}

int db_tbl_common_insert2(void *data, db_tbl_info_t *info, struct_info_t *struct_info, int struct_info_sz, int id )
{
	int ret = 0;
	char fields[512] = { 0 };
	int i = 0, sz = struct_info_sz;
	db_val_t val_info[32], *p = val_info;
	struct_info_t *pinfo = struct_info;

	ret = db_tbl_insert_fields_get( info->fields, fields, NULL );
	if ( ret < 0 ) {
		ERR("db_tbl_insert_fields_get() error\n");
		return -1;
	}

	for ( i=0; i<sz; i++ ) {
		p->addr = (uint8_t *)data + pinfo->offset;
		p->len =  pinfo->len;
		p->is_bcd = 1;
		p++;
		pinfo++;
	}

	//VAL_S( fields );
	return ( db_tbl_common_insert( val_info, sz, info->tbl_name, fields, id ) );
}

int db_tbl_common_inquire2( void *data, db_tbl_info_t *info, struct_info_t *struct_info, int struct_info_sz, int id )
{
	uint32_t len = 0;
	int i = 0, sz = struct_info_sz, ret = 0;
	db_parse_t res[32], *p = res;
	struct_info_t *pinfo = struct_info;
	char fields[512] = { 0 };

	for ( i=0; i<sz; i++ ) {
		p->addr = (uint8_t *)data + pinfo->offset;
		p->len =  &len;
		p->is_char = 0;
		p++;
		pinfo++;
	}

	p->addr = NULL;

	ret = db_tbl_insert_fields_get( info->fields, fields, NULL );
	if ( ret < 0 ) {
		ERR("db_tbl_insert_fields_get() error\n");
		return -1;
	}
	return ( db_tbl_common_inquire( res, info->tbl_name, fields, id) );
}

int db_tbl_inquire5( char *tbl_name, char *field, char *cond, char *cond_val, int arr_len, char *out, uint32_t *outlen )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char **dbresult;
	int i=0, nrow, ncolumn;
	char cmd_tbl_select[1024] = "select ";
	int ret = 0;

	sq_ret = sqlite3_open( DB_PATH, &db );
	if( sq_ret != SQLITE_OK ) {
		fprintf(stderr,"Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}

	strncat( cmd_tbl_select, field, strlen(field) );
	strncat( cmd_tbl_select, " from ", 6 );
	strncat( cmd_tbl_select, tbl_name, strlen(tbl_name) );
	strncat( cmd_tbl_select, " where ", 7 );
	strncat( cmd_tbl_select, cond, strlen(cond) );
	strncat( cmd_tbl_select, " = ", 3);
	strncat( cmd_tbl_select, cond_val, strlen(cond_val) );

	printf("inquire --> %s\n", cmd_tbl_select );
	sq_ret = sqlite3_get_table( db, cmd_tbl_select, &dbresult, &nrow, &ncolumn, &errmsg );
	printf("nrow = %d\n",nrow );
	printf("ncolumn = %d\n",ncolumn );

	if ( sq_ret != SQLITE_OK ) {
		ret = -1;
		printf("get table: %s\n",errmsg);
		goto err;
	}

	if ( nrow == 0 ) {
		ERR("ncolumn = 0\n");
		ret = -2;
		goto err;
	}

	/*printf("there is %i records in table.\n",nrow);*/
	for( i=0; i<ncolumn; i++ ) {
        if ( dbresult[nrow+i] ) {
            printf("dbresult[%d] = %s\n", i, dbresult[nrow+i]);// nrow - skip 1st row
            memcpy( out, dbresult[nrow+i], strlen(dbresult[nrow+i]) );
            out += arr_len;
        }
	}
	*outlen = ncolumn;

err:
	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);

	return ret;
}

int db_tbl_common_inquire3( db_parse_t *info, char *tbl_name, char *fields, char *cond, char *cond_val )
{
	char buff[64][256];
	uint32_t len = 0;
	int i = 0, ret = 0;
	db_parse_t *p = info;

	memset( buff, 0, sizeof(buff) );

	ret = db_tbl_inquire5( tbl_name, fields,
			cond, cond_val,
			256, *buff, &len);
	if ( ret < 0 ) {
		ERR("db_tbl_inquire() error\n");
		return ret;
	}
	/*printf("len = %d\n", len); */

	for ( i=0; i<len; i++ ) {
		/*printf("%s\n", buff[i]);*/
		/*printf("p->addr = 0x%p\n", p->addr);*/
		db_data_parse( buff[i], strlen(buff[i]), p->addr, p->len, p->is_char );
		p++;
	}
	return 0;
}

int db_tbl_traval( char *tbl_name, char *fields, db_cb_t cb, void *param, uint32_t *out_count )
{
	char *errmsg = 0;
	int sq_ret = 0;
	sqlite3 *db = 0;
	char **dbresult;
	int nrow,ncolumn;
	char cmd_tbl_select[1024] = "select ";
	int ret = 0;

	sq_ret = sqlite3_open(DB_PATH,&db);
	if( sq_ret != SQLITE_OK ) {
		ERR("Cannot open db: %s\n",sqlite3_errmsg(db));
		return -1;
	}

	strncat( cmd_tbl_select, fields, strlen(fields) );
	strncat( cmd_tbl_select, " from ", 6 );
	strncat( cmd_tbl_select, tbl_name, strlen(tbl_name) );

	printf("inquire --> %s\n", cmd_tbl_select );
	sq_ret = sqlite3_get_table( db, cmd_tbl_select, &dbresult, &nrow, &ncolumn, &errmsg );
	printf("nrow = %d\n",nrow );
	printf("ncolumn = %d\n",ncolumn );

	if( sq_ret == SQLITE_OK ) {
		/*printf("there is %i records in table.\n",nrow*ncolumn);*/
		*out_count = nrow;
		printf("*out_count = %d\n", *out_count);
		cb( param, dbresult, nrow, ncolumn );
	} else {
		printf("get table: %s\n",errmsg);
		ret = -1;;
		goto err;
	}

err:
	sqlite3_free_table(dbresult);
	sqlite3_free(errmsg);
	sqlite3_close(db);
	return ret;
}

/*
 * select aaa,bbb from tbl_ccc where cond_ddd = val_eee
 *
 * */
int db_tbl_inquire_part_field_by_cond( char *tbl_name,
		char *field,
		char *cond,
		uint32_t *len,
		db_cb_t cb,
		void *param )
{

	return ( db_tbl_inquire3( tbl_name,
				field,
				strlen(field),
				cond,
				strlen(cond),
				len,
				cb,
				param ));
}

int db_exec( char *cmd )
{
	char *errmsg = 0;
	int sq_ret = 0, ret = 0;
	sqlite3 *db = 0;

	sq_ret = sqlite3_open( DB_PATH, &db );
	if( sq_ret != SQLITE_OK ) {
		ERR("Cannot open db: %s\n",sqlite3_errmsg(db) );
		ret = -1;
		goto err1;
	}

	sq_ret = sqlite3_exec( db, cmd, NULL, NULL, &errmsg );
	if( sq_ret != SQLITE_OK ) {
		ERR( "sqlite3_exec() fail, %s\ncmd = %s\n",errmsg, cmd );
		ret = -1;
		goto err;
	}
	printf("cmd %s cmd exec ok\n", cmd);
	ret = 0;

err:
	sqlite3_free( errmsg );
err1:
	sqlite3_close( db );
	return ret;
}

int dbg_db_get_table( char *cmd )
{
    sqlite3 *db = 0;
    char *errmsg = 0, **dbresult;
    int sq_ret = 0, i = 0, j = 0, ret = 0, nrow, ncolumn;

    sq_ret = sqlite3_open( DB_PATH, &db );
    if( sq_ret != SQLITE_OK ) {
        ERR("Cannot open db: %s\n", sqlite3_errmsg(db) );
        ret = -1;
        goto err1;
    }

    sq_ret = sqlite3_get_table( db, cmd, &dbresult, &nrow, &ncolumn, &errmsg );
    VAL( nrow );
    VAL( ncolumn );

    if( sq_ret == SQLITE_OK ) {
        for ( i=1; i<=nrow; i++ ) {
            printf("[%02d] ", i - 1);
            for ( j=0; j<ncolumn; j++ ) {
                if ( dbresult[i*ncolumn+j] ) {
                    printf("%2s ", dbresult[i*ncolumn+j] );
                }
            }
            printf("\n");
        }
    } else {
        ERR("get table: %s\n",errmsg);
        ret = -1;
        goto err;
    }

    if ( nrow == 0 ) {
        printf("there is no data in table\n");
    }

    ret = 0;
err:
    sqlite3_free_table( dbresult );
    sqlite3_free( errmsg);
err1:
    sqlite3_close( db );
    return ret;
}

int db_get_table( char *cmd, get_table_cb cb, void *arg )
{
    sqlite3 *db = 0;
    char *errmsg = 0, **dbresult;
    int sq_ret = 0, nrow, ncolumn, ret = 0;

    sq_ret = sqlite3_open( DB_PATH, &db );
    if( sq_ret != SQLITE_OK ) {
        ERR("Cannot open db: %s\n", sqlite3_errmsg(db) );
        ret = -1;
        goto err1;
    }

    sq_ret = sqlite3_get_table( db, cmd, &dbresult, &nrow, &ncolumn, &errmsg );
    /*VAL_S( cmd );*/
    /*VAL( nrow );*/
    /*VAL( ncolumn );*/

    if( sq_ret == SQLITE_OK ) {
        if ( nrow == 0 ) {
            printf("there is no data in table\n");
            ret = 0;
        } else {
            ret = cb( dbresult, nrow, ncolumn, arg );
            if ( ret < 0 ) {
                ret = -1;
                goto err;
            }
        }
    } else {
        ERR("get table: %s, cmd = %s\n",errmsg, cmd);
        ret = -1;
        goto err;
    }


    ret = nrow;
err:
    sqlite3_free_table( dbresult );
    sqlite3_free( errmsg);
err1:
    sqlite3_close( db );
    return ret;
}

int db_tbl_inquire_by_offset( offset_inq_info_t *info )
{
    char cmd[1024] = "select * from ";
    char str_limit[] = " limit ";
    char str_offset[]= " offset ";
    char str_order[] = " order by ";
    offset_inq_info_t *pinfo = info;

    strncat( cmd, pinfo->tbl_name, strlen(pinfo->tbl_name) );
    strncat( cmd, str_order, strlen(str_order) );
    if ( strlen(pinfo->order_field_list) > 0 ) {
        strncat( cmd, pinfo->order_field_list, strlen(pinfo->order_field_list) );
    }
    /*if ( pinfo->sort == SORT_TYPE_ASC ) {*/
        /*strncat( cmd, str_asc, strlen(str_asc) );*/
    /*} else {*/
        /*strncat( cmd, str_desc, strlen(str_desc) );*/
    /*}*/
    strncat( cmd, str_limit, strlen(str_limit) );
    sprintf( cmd+strlen(cmd), "%d ", pinfo->limit );
    strncat( cmd, str_offset, strlen(str_offset) ) ;
    sprintf( cmd+strlen(cmd), "%d ", pinfo->offset );

    return( db_get_table(cmd, pinfo->cb, pinfo->arg) );
}

int db_inquire( db_inquire_t *info )
{
    int ret = 0;
    db_inquire_t *pinfo = info;
    char cmd[256] = { 0 };
    char str_list[][32] =
    {
        " select ",
        " from ",
        " order by ",
        " where ",
        " limit ",
        " offset ",
        " * "
    };

    if ( !pinfo ) {
        ERR("param check error\n");
        ret = -1;
        goto err;
    }

    if ( !pinfo->tbl_name ) {
        ERR("param check error\n");
        ret = -1;
        goto err;
    }

    strncat( cmd, str_list[0], strlen(str_list[0]));
    if ( pinfo->fields )
        strncat( cmd, pinfo->fields, strlen(pinfo->fields) );
    else
        strncat( cmd, str_list[6], strlen(str_list[6]) );
    strncat( cmd, str_list[1], strlen(str_list[1]) );
    strncat( cmd, pinfo->tbl_name, strlen(pinfo->tbl_name) );
    if ( pinfo->order ) {
        strncat( cmd, str_list[2], strlen(str_list[2]) );
        strncat( cmd, pinfo->order, strlen(pinfo->order) );
    }
    if ( pinfo->cond ) {
        strncat( cmd, str_list[3], strlen(str_list[3]) );
        strncat( cmd, pinfo->cond, strlen(pinfo->cond) );
    }
    if ( pinfo->limit ) {
        strncat( cmd, str_list[4], strlen(str_list[4]) );
        sprintf( cmd+strlen(cmd), "%d", pinfo->limit );
    }

    if ( pinfo->offset ) {
        strncat( cmd, str_list[5], strlen(str_list[5]) );
        sprintf( cmd+strlen(cmd), "%d", pinfo->offset );
    }

    ret = db_get_table( cmd, pinfo->cb, pinfo->param );

err:
    return ret;
}

int db_insert( db_insert_t *info )
{
    int i = 0, num = 0;
    char cmd[1024] = { 0 };
    char str_insert[] = "insert into ";
    char str_left_bracket[] = " ( ";
    char str_right_bracket[] = " ) ";
    char str_values[] = " values ";
    char str_comma[] = ", ";
    db_insert_t *pinfo = info;
    uint32_t len = 0;

    if ( !pinfo->vals ) {
        ERR("param check error\n");
        return -1;
    }

    strncat( cmd, str_insert, strlen(str_insert) );
    strncat( cmd, pinfo->tbl_name, strlen(pinfo->tbl_name) );
    strncat( cmd, str_left_bracket, strlen(str_left_bracket) );
    strncat( cmd, pinfo->fields, strlen(pinfo->fields) );
    strncat( cmd, str_right_bracket, strlen(str_right_bracket) ) ;
    strncat( cmd, str_values, strlen(str_values) );
    strncat( cmd, str_left_bracket, strlen(str_left_bracket) );
    num = (int)pinfo->vals[0][0];//vals[0]
    for ( i=1; i<num+1; i++ ) {// the 1st value of array is the length of the array
        VAL( i );
        if ( pinfo->vals[i] ) {
            db_value_convert( pinfo->vals[i], strlen(pinfo->vals[i]), &len );
            strncat( cmd, pinfo->vals[i], strlen(pinfo->vals[i]) );
            if ( i != num )
                strncat( cmd, str_comma, strlen(str_comma) );
        }
    }
    strncat( cmd, str_right_bracket, strlen(str_right_bracket) ) ;
    return ( db_exec( cmd ) );
}

int db_del( db_del_t *info )
{
    char cmd[512] = "delete from ";
    char str_where[] = " where ";
    char str_rowid[] = " rowid in ( select rowid from ";
    db_del_t *pinfo = info;

    strncat( cmd, pinfo->tbl_name, strlen(pinfo->tbl_name) );
    strncat( cmd, str_where, strlen(str_where) );
    if ( pinfo->cond ) {
        strncat( cmd, pinfo->cond, strlen(pinfo->cond) );
    }

    if ( pinfo->limit ) {
        strncat( cmd, str_rowid, strlen(str_rowid) );
        strncat( cmd, pinfo->tbl_name, strlen(pinfo->tbl_name));
        strncat( cmd, " limit ", strlen(" limit ") );
        sprintf( cmd+strlen(cmd), "%d ", pinfo->limit );
        strncat( cmd, " offset ", strlen(" offset ") );
        sprintf( cmd+strlen(cmd), "%d ", pinfo->offset);
        strncat( cmd, " ) ", 3 );
    }

    return ( db_exec( cmd ) );
}

int db_field_add( char *tbl_name, char *field, char *type )
{
    char cmd[1024] = "alter table ";
    char str_add[] = " add ";
    char space[] = " ";

    strncat( cmd, tbl_name, strlen(tbl_name) );
    strncat( cmd, str_add, strlen(str_add) );
    strncat( cmd, field, strlen(field) );
    strncat( cmd, space, strlen(space) );
    strncat( cmd, type, strlen(type) );

    return ( db_exec( cmd ) );
}

#if 0
int db_field_del( char *tbl_name, char *field )
{

}

#endif

