#include <stdio.h>
#include "mongoose.h"

#define PRT_NET_MAX_BUFF    1024*1024*8 //8MB
typedef struct _prt_net_data{
    uint8_t data[PRT_NET_MAX_BUFF];
    uint32_t len;
    uint8_t is_handle;
}prt_net_data;


struct mg_mgr mgr;
prt_net_data pn_data;

uint32_t net_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mbuf *io = &nc->recv_mbuf;
    (void)p;

    switch (ev)
    {
    case MG_EV_RECV:
        //mg_send(nc, io->buf, io->len); // Echo message back
        memcpy(pn_data.data,io->buf,io->len);
        printf("get through socket");
        mbuf_remove(io, io->len); // Discard message from recv buffer
        break;
    default:
        break;
    }
}

uint32_t init(const char *port)
{
    const char *port1 = "1234", *port2 = "127.0.0.1:17000";
    pn_data.len = 0;
    pn_data.is_handle = 0;
    mg_mgr_init(&mgr, NULL);
    mg_bind(&mgr, port, net_handler);
    printf("Starting echo mgr on port %s\n", port);
}

void poll(uint32_t i_time)
{
    mg_mgr_poll(&mgr, i_time);
}
