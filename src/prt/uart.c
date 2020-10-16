#include <stdio.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */
#include <errno.h>   /* ERROR Number Definitions           */
#include "uart.h"

int g_dev = 0;

int uart_init (int *dev){
    int fd;
    fd = open ("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        printf ("\n Open Error \n");
        return 0;
    }
    /*RX init*/
    struct termios SerialPortSettings; /* Create the structure                          */
    tcgetattr (fd, &SerialPortSettings); /* Get the current attributes of the Serial port */
    /* Setting the Baud rate */
    cfsetispeed (&SerialPortSettings, B19200);                     
    cfsetospeed (&SerialPortSettings, B19200); 
    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB; /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB; /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;  /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;     /* Set the data bits = 8                                 */

    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    //SerialPortSettings.c_cflag |= CRTSCTS;       /* No Hardware flow Control                         */

    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */

    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);         /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */

    SerialPortSettings.c_oflag &= ~OPOST; /*No Output Processing   raw  format  output*/

    SerialPortSettings.c_iflag &= ~ICRNL;

    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 0;  /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 1; /* Wait indefinetly   */

    if ((tcsetattr (fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        printf ("\n  ERROR ! in Setting attributes");
    tcflush (fd, TCIFLUSH); /* Discards old data in the rx buffer            */
    *dev = fd;
    return 1;
}

int uart_write (unsigned char *data, int len)
{
    int rv = 0;
    int i = 0;
    unsigned char leave = 0;
    unsigned char time = 0;
    if (g_dev == 0 || g_dev < 0){
        rv = uart_init (&g_dev);
        if (rv == 0){
            return 0;
        }
    }

    time = len / 512;
    leave = len % 512; 

    for(i = 0; i < time; i++)
    {
        rv = write (g_dev, &data[i * 512], 512);
            usleep(200000);
    }

    rv = write (g_dev, &data[time * 512], leave);
    return rv;
}

int uart_read (unsigned char *data, int len){
    int rv = 0;

    int len_get = 0;
    int counter = 0;
    if (g_dev == 0 || g_dev < 0){
        rv = uart_init (&g_dev);
        if (rv == 0){
            printf ("---------init- fail-g_dev=%d------\n", g_dev);
            return 0;
        }
    }
    rv = read (g_dev, data, len);
    if (rv < 0)
        return rv;
    return rv;
}

int uart_close (){
    close (g_dev);
    return 0;
}


