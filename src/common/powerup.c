#include "global.h"
#include "powerup.h"
#include "cJSON.h"
//#include "hafs.h"
#include <string.h>

char server_ip[64] = {0};

uint32_t powerup(void)
{
    usleep(100000);
    mprintf(0,"powerup: device init ok \n");
    usleep(100000);
    mprintf(0,"powerup: no ukey connected \n");

    var_init();
    return D9_OK;
}

uint32_t var_init(void)
{
    uint32_t ret = 0;
    ret = config_init();

    return D9_OK;
}

uint32_t config_init(void)
{
    int i;
    FILE *h_file = NULL;
    long len = 0;
    char *content = NULL;
    cJSON *json, *json_ip;

    h_file = fopen("./smart9_scheme.json", "rb");
    fseek(h_file, 0, SEEK_END);
    len = ftell(h_file);
    content = (char*)malloc(len + 1);
    fseek(h_file, 0, SEEK_SET);
    fread(content, 1, len, h_file);
    fclose(h_file);

    printf("addr = %p, str = %s\n", content, content);

    json = cJSON_Parse(content);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }
    char *str = cJSON_Print(json);

    json_ip = cJSON_GetObjectItem(json, "config");
    json_ip = cJSON_GetObjectItem(json_ip, "server info");
    json_ip = cJSON_GetObjectItem(json_ip, "ip");
    json_ip = cJSON_GetArrayItem(json_ip, 0);
    printf("ip = %s\n", json_ip->valuestring);
    strcpy(server_ip, json_ip->valuestring);
    free(content);
    return D9_OK;
}
