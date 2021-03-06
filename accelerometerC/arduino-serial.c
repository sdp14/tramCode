/*
 * arduino-serial
 * --------------
 *
 * A simple command-line example program showing how a computer can
 * communicate with an Arduino board. Works on any POSIX system (Mac/Unix/PC)
 *
 *
 * Compile with something like:
 *   gcc -o arduino-serial arduino-serial-lib.c arduino-serial.c -lm
 * or use the included Makefile
 *
 * Mac: make sure you have Xcode installed
 * Windows: try MinGW to get GCC
 *
 *
 * Originally created 5 December 2006
 * Copyleft (c) 2006, Tod E. Kurt, tod@todbot.com
 * http://todbot.com/blog/
 *
 *
 * Updated 8 December 2006:
 *  Justin McBride discovered B14400 & B28800 aren't in Linux's termios.h.
 *  I've included his patch, but commented out for now.  One really needs a
 *  real make system when doing cross-platform C and I wanted to avoid that
 *  for this little program. Those baudrates aren't used much anyway. :)
 *
 * Updated 26 December 2007:
 *  Added ability to specify a delay (so you can wait for Arduino Diecimila)
 *  Added ability to send a binary byte number
 *
 * Update 31 August 2008:
 *  Added patch to clean up odd baudrates from Andy at hexapodia.org
 *
 * Update 6 April 2012:
 *  Split into a library and app parts, put on github
 *
 * Update 20 Apr 2013:
 *  Small updates to deal with flushing and read backs
 *  Fixed re-opens
 *  Added --flush option
 *  Added --sendline option
 *  Added --eolchar option
 *  Added --timeout option
 *  Added -q/-quiet option
 *
 */

#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>

#include "arduino-serial-lib.h"


//
void usage(void)
{
    printf("Usage: arduino-serial -b <bps> -p <serialport> [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -h, --help                 Print this help message\n"
    "  -b, --baud=baudrate        Baudrate (bps) of Arduino (default 9600)\n"
    "  -p, --port=serialport      Serial port Arduino is connected to\n"
    "  -s, --send=string          Send string to Arduino\n"
    "  -S, --setup                Setup socket to write accelerometer data \n"
    "  -r, --receive              Receive string from Arduino & print it out\n"
    "  -n  --num=num              Send a number as a single byte\n"
    "  -F  --flush                Flush serial port buffers for fresh reading\n"
    "  -d  --delay=millis         Delay for specified milliseconds\n"
    "  -e  --eolchar=char         Specify EOL char for reads (default '\\n')\n"
    "  -t  --timeout=millis       Timeout for reads in millisecs (default 5000)\n"
    "  -q  --quiet                Don't print out as much info\n"
    "\n"
    "Note: Order is important. Set '-b' baudrate before opening port'-p'. \n"
    "      Used to make series of actions: '-d 2000 -s hello -d 100 -r' \n"
    "      means 'wait 2secs, send 'hello', wait 100msec, get reply'\n"
    "\n");
    exit(EXIT_SUCCESS);
}

//
void error(char* msg)
{
    fprintf(stderr, "%s\n",msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    /*accelerometer related declarations*/
    const int buf_max = 2048;

    int fd = -1;
    char serialport[buf_max];
    int baudrate = 9600;  // default
    char quiet=0;
    char eolchar = ' ';
    int timeout = 5000;
    char buf[buf_max];
    int rc,n,p;
    float xDat[103],yDat[103],zDat[103];
    float sumX=0,sumY=0,sumZ=0;
    float sum1=0,sum2=0,sum3=0;
    float stdX=0,stdY=0,stdZ=0,avgX=0,avgY=0,avgZ=0,varX=0,varY=0,varZ=0,recal=0;
    int total=1;
    int calibrate=0;
    /*---------------------------------*/

//    char stdbuffer[10];
    /*socket related declarations*/    
    int sockfd, portno, n2;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    int sendMessage=1;
   /*----------------------------*/

    if (argc==1) {
        usage();
    }

    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",       no_argument,       0, 'h'},
        {"port",       required_argument, 0, 'p'},
        {"baud",       required_argument, 0, 'b'},
        {"send",       required_argument, 0, 's'},
        {"setup",      no_argument,       0, 'S'},
        {"receive",    no_argument,       0, 'r'},
        {"flush",      no_argument,       0, 'F'},
        {"num",        required_argument, 0, 'n'},
        {"delay",      required_argument, 0, 'd'},
        {"eolchar",    required_argument, 0, 'e'},
        {"timeout",    required_argument, 0, 't'},
        {"quiet",      no_argument,       0, 'q'},
        {NULL,         0,                 0, 0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hp:b:s:rSFn:d:qe:t:",
                           loptions, &option_index);
        if (opt==-1) break;
        switch (opt) {
        case '0': break;
        case 'q':
            quiet = 1;
            break;
        case 'e':
            eolchar = optarg[0];
            if(!quiet) printf("eolchar set to '%c'\n",eolchar);
            break;
        case 't':
            timeout = strtol(optarg,NULL,10);
            if( !quiet ) printf("timeout set to %d millisecs\n",timeout);
            break;
        case 'd':
            n = strtol(optarg,NULL,10);
            if( !quiet ) printf("sleep %d millisecs\n",n);
            usleep(n * 1000 ); // sleep milliseconds
            break;
        case 'h':
            usage();
            break;
        case 'b':
            baudrate = strtol(optarg,NULL,10);
            break;
        case 'p':
            if( fd!=-1 ) {
                serialport_close(fd);
                if(!quiet) printf("closed port %s\n",serialport);
            }
            strcpy(serialport,optarg);
            fd = serialport_init(optarg, baudrate);
            if( fd==-1 ) error("couldn't open port");
            if(!quiet) printf("opened port %s\n",serialport);
            serialport_flush(fd);
            break;
        case 'n':
            if( fd == -1 ) error("serial port not opened");
            n = strtol(optarg, NULL, 10); // convert string to number
            rc = serialport_writebyte(fd, (uint8_t)n);
            if(rc==-1) error("error writing");
            break;
        case 'S':
	     portno = 52002;//atoi(argv[2]);
             /* Create a socket point*/ 
             sockfd = socket(AF_INET, SOCK_STREAM, 0);
             if (sockfd < 0) 
             {
                perror("ERROR opening socket");
                exit(1);
             }
             server = gethostbyname("192.168.2.103");//gethostbyname(argv[1]);
             if (server == NULL) {
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
             }

             bzero((char *) &serv_addr, sizeof(serv_addr));
             serv_addr.sin_family = AF_INET;
             bcopy((char *)server->h_addr, 
                   (char *)&serv_addr.sin_addr.s_addr,
                         server->h_length);
             serv_addr.sin_port = htons(portno);

             /* Now connect to the server */
             if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
             {
                  perror("ERROR connecting");
                  exit(1);
              }
              while(1){
              int r;
              int a1, a2, a3;
	      char a4[sizeof(float)*3+3];
              memset (buf, 0, 256);
              r = serialport_read_until(fd, buf);
              //fprintf (stderr, "VAL: %s -->", buf);
              if (r >= 0)
              {
               sscanf (buf, "A1:%d\tA2:%d\tA3:%d", &a1, &a2, &a3);
              // fprintf (stderr, "(%d)(%d)(%d)\n", a1, a2, a3);
               if(calibrate==0){
                  sumX= sumX + a1; sumY= sumY + a2; sumZ= sumZ + a3;
                  xDat[total]=a1;yDat[total]=a2;zDat[total]=a3;
                  total++;
              if (total==100){ 
		  calibrate=1;
                  avgX = sumX/(float)total; avgY = sumY/(float)total; avgZ = sumZ/(float)total;
                  for(p = 1; p<total;p++){
                  sum1 = sum1 + pow((xDat[p]-avgX),2); sum2 = sum2 + pow((yDat[p]-avgY),2); sum3 = sum3 + pow((zDat[p]-avgZ),2);
                  }
		   varX= sum1/(float)total; varY=sum2/(float)total; varZ=sum3/(float)total;
                   stdX=sqrt(varX); stdY=sqrt(varY); stdZ=sqrt(varZ);
               }
	       }
               if(calibrate==1){snprintf(a4, sizeof a4*7, "%f %f %f %f %f %f %f %f %f", (float)a1, (float)a2,(float)a3,avgX,avgY,avgZ,stdX,stdY,stdZ);}
               while(sendMessage==1&&calibrate==1){
                /* Send message to the server*/ 
                //printf("here");
		n2 = write(sockfd,a4,strlen(a4));
                if (n2 < 0) 
                {
                        perror("ERROR writing to socket");
                        exit(1);
                }
                /* Now read server response */
                bzero(buffer,256);
                n2 = read(sockfd,buffer,255);
                    if (n2 < 0) 
                {
                        perror("ERROR reading from socket");
                        exit(1);
                }
//                printf("Received message: %s\n",buffer);
                 sendMessage=0;
                }
             sendMessage=1; 
	     }
              sendMessage=1;
              fflush (0);
              //sleep (5);
           }    
           break;
        case 's':
            if( fd == -1 ) error("serial port not opened");

            sprintf(buf, (opt=='S' ? "%s\n" : "%s"), optarg);

            if( !quiet ) printf("send string:%s\n", buf);
            rc = serialport_write(fd, buf);
            if(rc==-1) error("error writing");
            break;
        case 'r':
	  while (1)
	    {
	      int r;
	      int a1, a2, a3;

	      memset (buf, 0, 256);
	      r = serialport_read_until(fd, buf);
              fprintf (stderr, "VAL: %s -->", buf);
//              printf("%d",r);
	      if (r >= 0)
		{
		 sscanf (buf, "A1:%d\tA2:%d\tA3:%d", &a1, &a2, &a3);
		 fprintf (stderr, "(%d)(%d)(%d)\n", a1, a2, a3);
                 //fgets(stdbuffer,10,stdin);
	         //if(strcmp(stdbuffer,"getData")==0){
		 // printf("%d",a1);
                 // printf("%d",a2);
                 // printf("%d",a3);}
                //}
        /*       while(sendMessage==1){
    		/* Send message to the server 
    		n2 = write(sockfd,"measure",strlen("measure"));
    		if (n2 < 0) 
    		{
         		perror("ERROR writing to socket");
         		exit(1);
    		}
    		/* Now read server response 
    		bzero(buffer,256);
    		n2 = read(sockfd,buffer,255);
        	    if (n2 < 0) 
    		{
         		perror("ERROR reading from socket");
         		exit(1);
    		}
    		printf("Received message: %s\n",buffer);
                sendMessage=0;
   		}*/
              }
	      fflush (0);
	      usleep (10000);
	    }
            break;
        case 'F':
            if( fd == -1 ) error("serial port not opened");
            if( !quiet ) printf("flushing receive buffer\n");
            serialport_flush(fd);
            break;

        }
    }

    exit(EXIT_SUCCESS);
} // end main
