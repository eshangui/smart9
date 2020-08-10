uint32_t init(const char* port);
void poll(uint32_t i_time);
uint32_t net_handler(struct mg_connection *nc, int ev, void *p);