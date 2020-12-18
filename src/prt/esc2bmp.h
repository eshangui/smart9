#ifndef __LIB_ESC_TO_BMP_H
#define __LIB_ESC_TO_BMP_H

#ifdef __cplusplus
extern "C" {
#endif
#define JCIAI //JUST CALL IT AFTER INITIALIZATION

typedef enum {
    HPRT_LIB_EVENT_STATUS,
    HPRT_LIB_EVENT_CUT,
    HPRT_LIB_EVENT_DRAWER,
    HPRT_LIB_EVENT_ID_RET,
} hprt_lib_event_t;

extern void lib_event_callback(hprt_lib_event_t e, const void *arg, unsigned int size);


typedef struct esc2bmp_handle{
    char            font_path[64];  //字库路径 无设置时，默认 "/opt/fonts_gb18030.bin"
    unsigned int    paper_width;    //纸张宽度 为0 时默认576 大于640时取640

    unsigned int    option;         //图片输出条件 0-超时，其他值时 （超时或切刀、定位、设置浓度速度）。默认0
    unsigned int    timeout_ms;     //图片输出超时时间，单位毫秒，默认100ms（设置0时会默认至100）
    char            bmp_path[64];   //字库路径 无设置默认为 /tmp/esc2bmp.bmp
    void            (*bmp_callback)(const char *bmp_path); //图片输出回调
    JCIAI int       (*bmp_print)(const char *bmp_path);//图片打印 仅支持2值图像 1bpp/8bpp, id为该图片的任务ID， 为0时不起效

    JCIAI unsigned char (*get_printer_status)(void);    //无需赋值，初始化后可用
    /* 
        —— ————————————————————  ———————————————— —— ————————————————  ———————————— —— ——
        |7 |6                   |5               |4 |3                |2           |1 |0 |
        —— ————————————————————  ———————————————— —— ————————————————  ———————————— —— ——
        |1 |打印机状态 正常0 异常1 |纸张  有纸0 无纸1 |1 |走纸键 按住1 松开0 |上盖 开-1 关0 |1 |0 |  
        —— ————————————————————  ———————————————— —— ————————————————  ———————————— —— —— 
    */
    JCIAI void      (*printer_reboot)(void);    //无需赋值，初始化后可用 重启打印机
    JCIAI void      (*printer_cut)(unsigned char len);       //切刀
    JCIAI void      (*printer_drawer)(unsigned char idx);    //钱箱操作，无需赋值，初始化后可用 idx 为序号 0或1
    JCIAI void      (*push_process_id)(unsigned int idx);    //process id 压栈

    unsigned int            (*usb_data_cb)(void *buff, unsigned int size);  //usb数据接收回调，buff为数据地址，size为数据长度，为null时，默认指向esc_2_lib
    JCIAI unsigned int      (*esc_2_prt)(const void *buff, unsigned int size);//发送esc数据。无需赋值，初始化后可用
    JCIAI unsigned int      (*esc_2_lib)(const void *buff, unsigned int size);    //往解析器发送数据,无需赋值，初始化后可用
}esc2bmp_handle_t;

extern int esc2bmp_init(struct esc2bmp_handle *handle);//handle要保留，不能释放
extern void esc2bmp_version_print(void);
extern void set_bmp_abandon_flag(void);//丢弃 bmp_print 尚未传输给 打印机的数据部分
extern void set_esc_abandon_flag(void);//丢弃esc_2_prt 尚未传输给打印机的esc数据部分

#ifdef __cplusplus
}
#endif
#endif