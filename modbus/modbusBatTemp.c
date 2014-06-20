/*====================================================================================================
modbus.c 1/27/2013 - UNIX C program to read registers via gateway
  compile using
  gcc -o modbus.c -o modbus
  run using
  usage: IPAddress (currently set to 10.0.0.10) Datalogger MB address (currently=1) Port to set high (choices are 0,1,2,3,5,6,7; 1 starts currently starts the program)  booleanValue (0xff to set port high, any other value to set it low)
         EX: type the following to run the compiled program named modbus:./modbus 10.0.0.10  1 3 127
             the above tells the data logger with IPAddr 10.0.0.10 and modbus slave address 1 to set its third port (3) high (127) 
source code:paginas.fe.up.pt/~pfs/recursos/plcs/modbus/sources.html
modbus info:www.scorpioncontrols.com/info/modbustcp.htm
Edited by: MR
====================================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>

int modbusRead(char buffer[],int command, int reg, int s, int i, char obuf[], unsigned short unit, fd_set fds, struct timeval tv, char ibuf[]);

/* global data */
int bAndT = 0;
int stopTram = 0;
int stop = 1;
int battAndTemp = 1;

/*socket related declarations*/    
int sockfd, portno, n2;
struct sockaddr_in serv_addr;
struct hostent *serv;
char buffer[256];

int main(int argc, char **argv)
{
  char *ip_adrs;
  unsigned short unit;
  unsigned short reg_no;
  unsigned char dataVal;
  int s;
  int i;
  struct sockaddr_in server;
  fd_set fds;
  struct timeval tv;
  unsigned char obuf[261];
  char ibuf[261];
  float rbuf[5];
  if (argc<5)
  {
    printf("usage: test2 ip_adrs MB_slave_address reg_no data_Val\n"
    "eg test2 10.0.0.10 1 0 15\n");
    return 1;
  }

  /* confirm arguments */
  ip_adrs = argv[1];
  unit = atoi(argv[2]);
  reg_no = atoi(argv[3]);
  dataVal  = atoi(argv[4]);
//  printf("ip_adrs = %s unit = %d reg_no = %d num_regs = %d\n",
//  ip_adrs, unit, reg_no, dataVal);

  portno = 52001;
  /* Create a socket point */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
  {
   perror("ERROR opening socket");
   exit(1);
  }
  serv = gethostbyname("192.168.2.103");
  if (serv == NULL) {
     fprintf(stderr,"ERROR, no such host\n");
     exit(0);
  }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)serv->h_addr, 
           (char *)&serv_addr.sin_addr.s_addr,
                 serv->h_length);
      serv_addr.sin_port = htons(portno);

     /* Now connect to the server */
   if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
      {
           perror("ERROR connecting");
           exit(1);
      }

   /* establish connection to gateway on ASA standard port 502 */
  s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  server.sin_family = AF_INET;
  server.sin_port = htons(502); /* ASA standard port */
  server.sin_addr.s_addr = inet_addr(ip_adrs);

  i = connect(s, (struct sockaddr *)&server, sizeof(struct sockaddr_in));
  if (i<0)
  {
   printf("connect - error %d\n",i);
    close(s);
    return 1;
  }

  FD_ZERO(&fds);
  tv.tv_sec = 5;
  tv.tv_usec = 0;

  /* check ready to send */
  FD_SET(s, &fds);
  i = select(32, NULL, &fds, NULL, &tv);
  if(0)if (i<=0)
  {
    printf("select - error %d\n",i);
    close(s);
    return 1;
  }
  
  int messageReceived=0;
  buffer[0] = 's';
  char bAndTString[sizeof(float)*6+3];
  float battDec, battFrac, tempDec, tempFrac, stopBase, stopRadiom;
  while(messageReceived==0){
  //battDec = modbusRead(buffer,3, 5, s, i, obuf, unit, fds, tv, ibuf);
  if(buffer[0]!='w'){
  /*Communicate with the datalogger*/
  if(battAndTemp == 1)
  {  
  battDec = modbusRead(buffer,3, 5, s, i, obuf, unit, fds, tv, ibuf); 
  battFrac = modbusRead(buffer,3, 6, s, i, obuf, unit, fds, tv, ibuf);
  tempDec = modbusRead(buffer,3, 7, s, i, obuf, unit, fds, tv, ibuf); 
  tempFrac = modbusRead(buffer,3, 8, s, i, obuf, unit, fds, tv, ibuf);
  battAndTemp = 0;
  }
  if(stop == 1)
  {
  stopBase =  modbusRead(buffer,3, 10, s, i, obuf, unit, fds, tv, ibuf);
  stopRadiom = modbusRead(buffer,3, 11, s, i, obuf, unit, fds, tv, ibuf);
//  stop = 0;
  }
  /* Send message to the server*/ 
  snprintf(bAndTString, sizeof bAndTString*5, "%f %f %f %f %f %f", battDec, battFrac, tempDec, tempFrac, stopBase, stopRadiom);
  n2 = write(sockfd,bAndTString,strlen(bAndTString));
  if (n2 < 0) 
  {
   perror("ERROR writing to socket");
   exit(1);
  }
  /* Now read server response*/ 
  bzero(buffer,256);
  n2 = read(sockfd,buffer,255);
  if (n2 < 0) 
     { 
     perror("ERROR reading from socket");
     exit(1);
     }
//  if(stopTram=5){
//    stop = 1;
//    stopTram = 0;
//  }
  if(bAndT==1000000){
    battAndTemp = 1;
    bAndT = 0;
}
//    stopTram++;
    bAndT++;
}

else
{  
  snprintf(bAndTString, sizeof bAndTString*5, "%c %c %c %c %c %c", 'r','e' ,'a','d','y','?');
  n2 = write(sockfd,bAndTString,strlen(bAndTString));
  if (n2 < 0)
  {
   perror("ERROR writing to socket");
   exit(1);
  }

  /* Now read server response*/
  bzero(buffer,256);
  n2 = read(sockfd,buffer,255);
  if (n2 < 0)
     {
     perror("ERROR reading from socket");
     exit(1);
     }
}

//  sleep(10);
}

return 0;
}

int modbusRead(char buffer[], int command, int reg, int s, int i, char obuf[], unsigned short unit, fd_set fds, struct timeval tv, char ibuf[]){
  /* build MODBUS end measurement request */
  for (i=0;i<5;i++) obuf[i] = 0;
  obuf[5] = 6;
  obuf[6] = unit;
  obuf[7] = command;
  obuf[8] = 0x00;  //>> 8;
  obuf[9] = reg; //& 0xff; // & 0xff;
  obuf[10] = 0;// num_regs >> 8;
  obuf[11] = 1; //num_regs & 0xff;

  /* send request */
  i = send(s, obuf, 12, 0);
  if (i<12)
  {
    printf("failed to send all 12 chars\n");
  }

  /* wait for response */
  FD_SET(s, &fds);
  i = select(32, &fds, NULL, NULL, &tv);
  if (i<=0)
  {
    printf("no TCP response received\n");
    close(s);
    return 1;
  }

  /* receive and analyze response */
  i = recv(s, ibuf, 261, 0);
  if (i<9)
  {
    if (i==0)
    {
      printf("unexpected close of connection at remote end\n");
    }
    else
    {
      printf("response was too short - %d chars\n", i);
    }
  }
  else if (ibuf[7] & 0x80)
  {
    printf("MODBUS exception response - type %d\n", ibuf[8]);
  }
//  else if (i != (9+2*num_regs))
//  {
//    printf("incorrect response size is %d expected %d\n",i,(9+2*num_regs));
//  }
//   else
//   {
//     for (i=0;i<11;i++)
//     {
//       unsigned short  w = ibuf[i];
//       printf("word %d = %d \n", i, w);
//    }
//   }
  return ibuf[10];
}

