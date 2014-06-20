/*====================================================================================================
modbus.c 1/27/2013 - UNIX C program to read registers via gateway
  compile using
  gcc modbus.c -o modbus
  run using
  usage: IPAddress (currently set to 192.168.3.115) Datalogger MB address (currently=1) Port to set high (choices are 0,1,2,3,5,6,7; 1 starts currently starts the program)  booleanValue (0xff to set port high, any other value to set it low)
         EX: type the following to run the compiled program named modbus:./modbus 192.168.3.115  1 3 127
             the above tells the data logger with IPAddr 192.168.3.115 and modbus slave address 1 to set its third port (3) high (127) 
source code:paginas.fe.up.pt/~pfs/recursos/plcs/modbus/sources.html
modbus info:www.scorpioncontrols.com/info/modbustcp.htm
Edited by: MR
====================================================================================================*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>

/* global data */
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
  unsigned char ibuf[261];

  if (argc<5)
  {
    printf("usage: test2 ip_adrs MB_slave_address reg_no data_Val\n"
    "eg test2 192.168.3.115 1 0 15\n");
    return 1;
  }

  /* confirm arguments */
  ip_adrs = argv[1];
  unit = atoi(argv[2]);
  reg_no = atoi(argv[3]);
  dataVal  = atoi(argv[4]);
  printf("ip_adrs = %s unit = %d reg_no = %d num_regs = %d\n",
  ip_adrs, unit, reg_no, dataVal);

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

  /* build MODBUS take measurement request */
  unsigned char wordNum=1;
  for (i=0;i<5;i++) obuf[i] = 0;
  obuf[5]=7+(2*wordNum);
  obuf[6]=1;
  obuf[7]=15;
  obuf[8]=0 >> 8;
  obuf[9]=0 & 0xff;
  obuf[10]=wordNum >> 8;
  obuf[11]=wordNum & 0xff;
  obuf[12]=2*wordNum;
  obuf[13]=dataVal;
  obuf[14]=dataVal;
  obuf[15]=dataVal;
  obuf[16]=dataVal;
  
  /* send request */
  i = send(s, obuf, 15, 0);

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
  else if (i != 12)
 {   printf("incorrect response size is %d expected %d\n",i,12);
 }
//  else
//  {
//   for (i=0;i<12;i++)
//  {
//      unsigned short w = ibuf[i];
//      printf("word %d = %d\n", i, w);//MB slave sends a report back and this checks if received data is correct; mainly for debugging purposes/
//    }
//  }
  int messageReceived=0;
  while(messageReceived==0){
    /* build MODBUS end measurement request */
  for (i=0;i<5;i++) obuf[i] = 0;
  obuf[5] = 6;
  obuf[6] = unit;
  obuf[7] = 3;
  obuf[8] = 0 >> 8;
  obuf[9] = 2 & 0xff;
  obuf[10] =0;// num_regs >> 8;
  obuf[11] =1; //num_regs & 0xff;

  /* send request */
  i = send(s, obuf, 12, 0);
  if (i<12)
  {
    printf("failed to send all 12 chars\n");
  }

  /* wait for response */
  FD_SET(s, &fds);
  i = select(45, &fds, NULL, NULL, &tv);
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
//    else
//    {
//     for (i=0;i<11;i++)
 //   {
//      unsigned short w = ibuf[i];
//      printf("word0 %d = %d\n", i, w);
//    }
// }
  if(ibuf[10]==1){
  messageReceived=1;
  }
  sleep(1);
}
  /* build MODBUS acknowledge data received request request */
//  unsigned char wordNum1=1;
  for (i=0;i<5;i++) obuf[i] = 0;
  obuf[5] = 6;
  obuf[6] = unit;
  obuf[7] = 6;
  obuf[8] = 0 >> 8;
  obuf[9] = 2 & 0xff;
  obuf[10] =0;// num_regs >> 8;
  obuf[11] =0; //num_regs & 0xff;

//  for (i=0;i<5;i++) obuf[i] = 0;
//  obuf[5]=6;//7+(2*wordNum1);
//  obuf[6]=1;
//  obuf[7]=16;
//  obuf[8]=0 >> 8;
//  obuf[9]= 2 & 0xff;
 // obuf[10]=wordNum1 >> 8;
 // obuf[11]=wordNum1 & 0xff;
 // obuf[12]=2*wordNum1;
//  obuf[10]=0;
//  obuf[11]=1;
  //obuf[15]=0;
  //obuf[16]=0;
//for multiple registers, but probably doesn't work
  
  /* send request */
  i = send(s, obuf, 12, 0);
//was 15
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
//  else if (i != 12)
// {   printf("incorrect response size is %d expected %d\n",i,12);
// }
// else
//  {
//   for (i=0;i<12;i++)
//  {
//     unsigned short w = ibuf[i];
 //    printf("word1 %d = %d\n", i, w);//MB slave sends a report back and this checks if received data $
//    }
//  }


  /* close down connection */
  close(s);

return ibuf[9];
}
