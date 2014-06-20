#!/usr/bin/python

# Import all from module socket
import socket 
from socket import error as SocketError

class server:
        def __init__(self,hostName,portNum):
            self.HOST = hostName                 # Symbolic name meaning the local host
	    self.PORT = portNum              # Arbitrary non-privileged port
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	    self.s.bind((self.HOST, self.PORT))
	    self.s.listen(5)
	    self.dataT = []
            self.dataS = []
            self.dataT.append(str("wait"))
            self.dataT.append(str("wait"))
            self.dataS.append(str(" "))
            self.connInfo = [None, None]
            self.flag = [1, 1, 1, 1, 1]
            self.stream = [1]            		
 
	def listun(self, typ, accel):
             self.connInfo[0], self.connInfo[1] = self.s.accept()
	     while True:
                  #print(connInfo[0] + connInfo[1])
                  #self.connInfo[0], self.connInfo[1] = self.s.accept()
	    #print 'Connected by', self.addr
                  try:
                      self.dat = self.connInfo[0].recv(1024)
                     # print(self.dat)
                  except SocketError as e:
                         self.dat = "wait 0 0 0 0 0"
                  if len(self.dat) != 0 :
                     try:
                         self.connInfo[0].send('Recieved the last command:'+str(self.dat))
                     except SocketError as e:
                         self.dat = "wait 0 0 0 0 0"
                  if len(self.dat) == 0 :
                     print "Disconnected!" #consider using a function call
                     #self.connInfo[0].close()
                     #self.s.close()
                     #self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                     #self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	             #self.s.bind((self.HOST, self.PORT))
                     #self.s.listen(2)
                     self.flag = [1, 1, 1, 1, 1]
                     self.connInfo[0], self.connInfo[1] = self.s.accept()

                  if typ == 0 and self.dat!='' :
                     self.dataS[0] = self.dat
                     self.dataS=self.dataS[0].split(" ")
                     print(self.dataS)
                     #print("here")
                     if(self.dataS[0] == str("wait") or self.dataS[0] == str("measure") or self.dataS[0] == str("emergency") or self.dataS[0] == str("shutdown") or self.dataS[0] == str("videoStream")) :  
    	                self.dataT=self.dataS
                        #print("taco1")

                  if typ == 1 and self.dat!='': 
 	   	     self.dataT[0]=self.dat
                     self.dataT=self.dataT[0].split(" ")
                    # print("here")
   		     accel.Data[0]=float(self.dataT[0])
	             accel.Data[1]=float(self.dataT[1])
	             accel.Data[2]=float(self.dataT[2])
                     accel.Stats[0] = float(self.dataT[3])
	             accel.Stats[1] = float(self.dataT[4])
	             accel.Stats[2] = float(self.dataT[5])
	             accel.Stats[3] = float(self.dataT[6])
		     accel.Stats[4] = float(self.dataT[7])
		     accel.Stats[5] = float(self.dataT[8])
		   #self.data = self.data

