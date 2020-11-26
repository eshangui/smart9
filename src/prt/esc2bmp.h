#ifndef __LIB_ESC_TO_BMP_H
#define __LIB_ESC_TO_BMP_H

#ifdef __cplusplus
extern "C" {
#endif
#define JCIAI //JUST CALL IT AFTER INITIALIZATION

typedef struct esc2bmp_handle{
    char            font_path[64];  //字库路径 无设置时，默认 "/opt/fonts_gb18030.bin"
    unsigned int    paper_width;    //纸张宽度 为0 时默认576 大于640时取640

    unsigned int    option;         //图片输出条件 0-超时，1 超时或切刀。默认0
    unsigned int    timeout_ms;     //图片输出超时时间，单位毫秒，默认100ms（设置0时会默认至100）
    char            bmp_path[64];   //字库路径 无设置默认为 /tmp/esc2bmp.bmp
    void            (*bmp_callback)(const char *bmp_path); //图片输出回调
    JCIAI int       (*bmp_print)(const char *bmp_path);//图片打印 仅支持2值图像 1bpp/8bpp

    JCIAI unsigned char (*get_printer_status)(void);    //无需赋值，初始化后可用
    /* 
        —— ————————————————————  ———————————————— —— ————————————————  ———————————— —— ——
        |7 |6                   |5               |4 |3                |2           |1 |0 |
        —— ————————————————————  ———————————————— —— ————————————————  ———————————— —— ——
        |1 |打印机状态 正常0 异常1 |纸张  有纸0 无纸1 |1 |走纸键 按住1 松开0 |上盖 开-1 关0 |1 |0 |  
        —— ————————————————————  ———————————————— —— ————————————————  ———————————— —— —— 
    */
    JCIAI void      (*printer_reboot)(void);    //无需赋值，初始化后可用 重启打印机
    JCIAI void      (*printer_cut)(void);       //切刀
    JCIAI void      (*printer_drawer)(unsigned char idx);    //钱箱操作，无需赋值，初始化后可用 idx 为序号 0或1
    void            (*drawer_cb)(unsigned char idx); //库收到钱箱回调

    unsigned int            (*usb_data_cb)(void *buff, unsigned int size);  //usb数据接收回调，buff为数据地址，size为数据长度，为null时，默认指向esc_2_lib
    JCIAI unsigned int      (*esc_2_prt)(const void *buff, unsigned int size);//发送esc数据。无需赋值，初始化后可用
    JCIAI unsigned int      (*esc_2_lib)(const void *buff, unsigned int size);    //往解析器发送数据,无需赋值，初始化后可用
}esc2bmp_handle_t;

extern int esc2bmp_init(struct esc2bmp_handle *handle, unsigned int bp);//handle要保留，不能释放
extern void esc2bmp_version_print(void);

#ifdef __cplusplus
}
#endif
#endif