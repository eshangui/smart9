void mprintf(int32_t level,const char *cmd, ...);
void dump_data(char* path, uint8_t *data, int32_t len);
void fbmp(uint8_t * img, int32_t w, int32_t h, char* name);
long long getSystemTime();
void fbmp_print(uint8_t *img, int32_t w, int32_t h, char *name, const char *cmd, ...);
void print_array(unsigned char* buf, int len);
void load_data(char *path, uint8_t *data, int32_t *len);

extern long long app_start_time;
