//
// arduino-serial-lib -- simple library for reading/writing serial ports
//
// 2006-2013, Tod E. Kurt, http://todbot.com/blog/
//

#include "arduino-serial-lib.h"

#include <stdio.h>    // Standard input/output definitions 
#include <unistd.h>   // UNIX standard function definitions 
#include <fcntl.h>    // File control definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <string.h>   // String function definitions 
#include <sys/ioctl.h>
#include <stdlib.h>
// uncomment this to debug reads
//#define SERIALPORTDEBUG 

// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(serialport, O_RDWR | O_NONBLOCK );
    
    if (fd == -1)  {
        perror("serialport_init: Unable to open port ");
        return -1;
    }
    
    //int iflags = TIOCM_DTR;
    //ioctl(fd, TIOCMBIS, &iflags);     // turn on DTR
    //ioctl(fd, TIOCMBIC, &iflags);    // turn off DTR

    if (tcgetattr(fd, &toptions) < 0) {
        perror("serialport_init: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
#ifdef B14400
    case 14400:  brate=B14400;  break;
#endif
    case 19200:  brate=B19200;  break;
#ifdef B28800
    case 28800:  brate=B28800;  break;
#endif
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    //toptions.c_cflag &= ~HUPCL; // disable hang-up-on-close to avoid reset

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 0;
    //toptions.c_cc[VTIME] = 20;
    
    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

    return fd;
}

//
int serialport_close( int fd )
{
    return close( fd );
}

//
int serialport_writebyte( int fd, uint8_t b)
{
    int n = write(fd,&b,1);
    if( n!=1)
        return -1;
    return 0;
}

//
int serialport_write(int fd, const char* str)
{
    int len = strlen(str);
    int n = write(fd, str, len);
    if( n!=len ) {
        perror("serialport_write: couldn't write whole string\n");
        return -1;
    }
    return 0;
}

//
int serialport_read_until(int fd, char* buf)
{
    unsigned char b[1],until;
    uint8_t i=1;
    int j=0;
    int k=0;
    int l=1;
    int m=0; 
    int r=0;
    buf[0]=buf[6]=buf[12]='A';
    do { 
        int n = read(fd, b, 1);  // read a char at a time
        if( n < 0) 
	  {
	    while (( n < 0 && errno == EAGAIN))
	      {
		usleep (10000);  // wait a little bit
		n = read(fd, b, 1);  // read a char at a time
	      }
	    if( n < 0 && errno != EAGAIN) 
	      {
		return -1;    // couldn't read
	      }
	  }
        if( n==0 ) {
	  usleep( 1 * 1000 ); // wait 10 msec try again
	  continue;
        }
         if((b[0]=='A'||j==1)&&(b[0]!='\n')&&(b[0]!=' ')&&(b[0]!=0)){
         j=1;
//         printf("here");
         if( ((b[0]=='1'||k==1)&&l==1) || ((b[0]=='2'||k==1)&&m==1) || ((b[0]=='3'||k==1)&&r==1) ){
         buf[i]=b[0]; i++; k=1;}
         else{if(b[0]!='A'){j=0;}}
         if(i%6==0){//printf("%s\n %d\n",buf,i);
         if(l==1){m=1;l=0;j=0;k=0;i++;}
         else{if(m==1){r=1;m=0;j=0;k=0;i++;}
         else{if(r==1){until='\n';r=0;j=0;k=0;}}}
         }
        }
    } while( until != '\n');

//    buf[i] = 0;  // null terminate the string
   // i = 0;
    j = 0;
    k = 0;
//    scanf("%*[ \n\t]%s", buf);
//    for(j=0; j<=i;j=j+2){printf("%c",buf[j]);}
//    printf("%s\n",buf);
  //  exit(0);
    return i;
}
//
int serialport_flush(int fd)
{
    sleep(2); //required to make flush work, for some reason
    return tcflush(fd, TCIOFLUSH);
}
