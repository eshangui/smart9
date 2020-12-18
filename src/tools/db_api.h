#ifndef DB_API_H
#define DB_API_H
#include "type.h"
#include "ctype.h"
#define DEBUG
#include "dbg.h"

typedef struct{
	char *name;
	uint32_t offset;
	uint32_t len;
}struct_info_t;

typedef struct {
	char *field;
	char *type;
} db_fields_t;

typedef struct {
	char *tbl_name;
	db_fields_t fields[128];
} db_tbl_info_t;

typedef struct {
	uint8_t *addr;
	uint32_t len;
	uint8_t is_bcd;
} db_val_t;

typedef struct {
	uint8_t *addr;
	uint32_t *len;
	uint8_t is_char;
} db_parse_t;

typedef int ( *get_table_cb)( char **res, int row, int col, void *arg );

typedef struct {
    char tbl_name[64];
    char order_field_list[64];
    int offset;
    int limit;
    int sort;
    get_table_cb cb;
    void *arg;
} offset_inq_info_t;

typedef struct {
    char filed[32];
    char val[32];
} db_inqure_filter_t;

typedef struct {
    db_inqure_filter_t filters[32];
    int num;
} db_filter_info_t;


typedef struct {
    char field[32];
    int sort;
} db_inquire_order_t;

typedef struct {
    db_inquire_order_t ordes[32];
    int num;
} db_order_info_t;

/*
 * select xxxx(fields) from xxx(tbl_name) order by xxx(order) where xxx(cond) limit xxx offset xxx
 *
 * */
typedef struct {
    char *fields;
    char *tbl_name;
    char *cond;
    char *order;
    int limit;
    int offset;
    get_table_cb cb;
    void *param;
} db_inquire_t;

typedef struct {
    char *tbl_name;
    char *fields;
    char **vals;
    int val_num;
} db_insert_t;

typedef struct {
    char *tbl_name;
    char *cond;
    int limit;
    int offset;
} db_del_t;

enum {
    SORT_TYPE_ASC,
    SORT_TYPE_DESC,
};


#define DB_PATH "./offline_data.db"


typedef int (*db_cb_t)( void *param, char **res, int row, int cloumn );

int db_table_create( char *tbl_name, char *field_list, uint32_t field_len );
int db_table_insert( char *tbl_name, char *updatge_filed_list, uint32_t update_field_len,  char *val, uint32_t val_len );
int db_tbl_inquire( char *tbl_name, char *inquire_filed_list, uint32_t inquire_field_len, char (*out)[256], uint32_t *outlen, int id );
int db_tbl_inquire2( char *tbl_name,
		char *inquire_filed_list,
		uint32_t inquire_field_len,
		char *cond,
		uint32_t cond_len,
		char *cond_val,
		uint32_t cond_val_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param );
int db_tbl_inquire3( char *tbl_name,
		char *inquire_filed_list,
		uint32_t inquire_field_len,
		char *cond,
		uint32_t cond_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param );
int db_tbl_all_fields_inquire3( char *tbl_name,
		char *cond,
		uint32_t cond_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param);
int db_tbl_update(char *tbl_name,
		uint32_t tbl_name_len,
		char *update_field,
		uint32_t update_field_len,
		char *update_val,
		uint32_t update_val_len ,
		uint32_t id);
int db_value_convert( char *val, uint32_t len, uint32_t *outlen );
int db_value_buff_setup( uint8_t *val, uint32_t len, char *out, uint32_t *outlen, uint8_t is_bcd, uint8_t is_last );
int db_data_parse( char *in, uint32_t inlen, uint8_t *out, uint32_t *outlen, uint8_t is_char );
//int db_tbl_common_insert( db_val_t *val, int32 val_amount, char *tbl_name, char *insert_field );
int db_data_common_parse(db_parse_t *in, uint32_t inlen, char (*buff)[256] );
int db_tbl_clr( char *tbl_name );
int db_tbl_comon_field_update( char *tbl_name,
		uint32_t tbl_name_len,
		char *update_field,
		uint32_t update_field_len,
		uint8_t *val,
		uint32_t len,
		uint32_t id);
int db_record_amount_get( char *tbl_name, int *outlen );
int db_record_amount_get_by_cond( char *tbl_name,
		int *outlen ,
		char *cond,
		uint32_t cond_len,
		char *cond_val,
		uint32_t cond_val_len);
int db_tbl_insert_fields_get( db_fields_t *fields, char *out, int *outlen );
int db_tbl_update2(char *tbl_name,
		uint32_t tbl_name_len,
		char *update_val,
		uint32_t update_val_len,
		uint32_t id);

int db_tbl_common_inquire( db_parse_t *info, char *tbl_name, char *fields, int id);
int db_record_del(char *tbl_name, uint32_t id );
int db_tbl_all_fields_inquire( char *tbl_name,
		char *cond,
		uint32_t cond_len,
		char *cond_val,
		uint32_t cond_val_len,
		uint32_t *out_count,
		db_cb_t cb,
		void *param);
int db_tbl_common_create( db_tbl_info_t *info );
//int db_tbl_common_insert2(void *data, db_tbl_info_t *info, struct_info_t *struct_info, int struct_info_sz );
int db_tbl_common_insert2(void *data, db_tbl_info_t *info, struct_info_t *struct_info, int struct_info_sz, int id );
int db_tbl_common_insert( db_val_t *val, int32_t val_amount, char *tbl_name, char *insert_field, int id );
int db_record_del2(char *tbl_name, char *field, char *field_val );
int db_tbl_update3(char *tbl_name,
		char *update_field,
		char *update_val,
		char *cond,
		char *cond_val);
int db_tbl_inquire5( char *tbl_name, char *field, char *cond, char *cond_val, int arr_len, char *out, uint32_t *outlen );
int db_tbl_common_inquire3( db_parse_t *info, char *tbl_name, char *fields, char *cond, char *cond_val );
int db_tbl_common_inquire2( void *data, db_tbl_info_t *info, struct_info_t *struct_info, int struct_info_sz, int id );
int db_record_del3(char *tbl_name, uint32_t id );
int db_tbl_traval( char *tbl_name, char *fields, db_cb_t cb, void *param, uint32_t *out_count );
int db_tbl_inquire_part_field_by_cond( char *tbl_name,
		char *field,
		char *cond,
		uint32_t *len,
		db_cb_t cb,
		void *param );

int db_exec( char *cmd );
int db_get_table( char *cmd, get_table_cb cb, void *arg );
int db_tbl_inquire_by_offset( offset_inq_info_t *info );
int db_inquire( db_inquire_t *info );
int db_insert( db_insert_t *info );
int db_del( db_del_t *info );
int dbg_db_get_table( char *cmd );
int db_field_add( char *tbl_name, char *field, char *type );

#endif  /*DB_API_H*/
