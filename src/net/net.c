#include <stdio.h>
#include "global.h"
#include "mongoose.h"
#include "db.h"
#include "net.h"

static const char *s_user_name = "admin";
static const char *s_password = "password";
static const char *pub_topic_heartbeat = "/smart9/up/heartbeat/";
static const char *sub_topic_heartbeat = "/smart9/down/heartbeat/1122334455667788";
static const char *pub_topic_upload = "/smart9/up/upload/1122334455667788";
static const char *sub_topic_upload = "/smart9/down/upload/1122334455667788";
static struct mg_mqtt_topic_expression s_topic_expr = {NULL, 0};

uint32_t tcp_init(const char *port)
{
    pn_data.len = 0;
    pn_data.is_handle = 0;
    mg_mgr_init(&m_tcp, NULL);
    mg_bind(&m_tcp, port, tcp_handler);
    printf("Starting tcp mgr on port %s\n", port);
    return D9_OK;
}

void tcp_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mbuf *io = &nc->recv_mbuf;
    (void)p;

    switch (ev)
    {
    case MG_EV_RECV:
        //mg_send(nc, io->buf, io->len); // Echo message back
        memcpy(pn_data.data, io->buf, io->len);
        printf("get through socket len = %d\n",io->len);
        mbuf_remove(io, io->len); // Discard message from recv buffer
        break;
    default:
        break;
    }
}

void tcp_poll(uint32_t i_time)
{
    mg_mgr_poll(&m_tcp, i_time);
}

uint32_t mqtt_state = 0;

uint32_t mqtt_init(const char* address)
{
    mg_mgr_init(&m_mqtt, NULL);
    if (mg_connect(&m_mqtt, address, mqtt_handler) == NULL) {
        fprintf(stderr, "mg_connect(%s) failed\n", address);
        exit(EXIT_FAILURE);
    }
    mqtt_state = 0;

}

void mqtt_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *)p;
    (void)nc;

    if (ev != MG_EV_POLL) printf("mqtt GOT event %d\n", ev);

    switch (ev) {
    case MG_EV_CONNECT: {
        struct mg_send_mqtt_handshake_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.user_name = s_user_name;
        opts.password = s_password;

        mg_set_protocol_mqtt(nc);
        mg_send_mqtt_handshake_opt(nc, "dummy", opts);
        break;
    }
    case MG_EV_MQTT_CONNACK:
        if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) {
            printf("Got mqtt connection error: %d\n", msg->connack_ret_code);
            exit(1);
        }
        s_topic_expr.topic = sub_topic_heartbeat;
        printf("Subscribing to '%s'\n", sub_topic_heartbeat);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
        s_topic_expr.topic = sub_topic_upload;
        printf("Subscribing to '%s'\n", sub_topic_upload);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
        break;
    case MG_EV_MQTT_PUBACK:
        printf("Message publishing acknowledged (msg_id: %d)\n", msg->message_id);
        break;
    case MG_EV_MQTT_SUBACK:
        printf("Subscription acknowledged\n");
        break;
    case MG_EV_MQTT_PUBLISH: {
        printf("Got incoming message %.*s: %.*s\n", (int)msg->topic.len,
            msg->topic.p, (int)msg->payload.len, msg->payload.p);
        mqtt_state = 1;
        break;
    }
    case MG_EV_CLOSE:
        printf("Connection closed\n");
        exit(1);
    }
}

void mqtt_poll(uint32_t i_time)
{
    mg_mgr_poll(&m_mqtt, i_time);
}

uint32_t mqtt_publish_sync(uint32_t topic, char* data, uint32_t *len)
{
    char sn[16] = { 0 };
    get_prt_sn(sn);
    switch(topic)
    {
        case MQTT_TOPIC_HEARTBEAT:
            mg_mqtt_publish(m_mqtt.active_connections, pub_topic_heartbeat, 65, MG_MQTT_QOS(0),sn,strlen(sn));
            break;
        case MQTT_TOPIC_UPLOAD:
            mg_mqtt_publish(m_mqtt.active_connections, pub_topic_upload, 65, MG_MQTT_QOS(0),sn,strlen(sn));
            break;
    }
    
    while(mqtt_state==0)
        mqtt_poll(100);
    mqtt_state = 0;
    return D9_OK;
}