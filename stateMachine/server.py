#!/usr/bin/python

# Import all from module socket
import socket 
from socket import error as SocketError

class server:
        def __init__(self,hostName,portNum):
            self.HOST = hostName             # Symbolic name meaning the local host
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
            self.distress = 1
            self.prevState = [" "]
            self.wait = 0
 
	def listun(self, typ, accel, meas, bAndT):
             self.connInfo[0], self.connInfo[1] = self.s.accept()
	     while True:
                  try:
                      self.dat = self.connInfo[0].recv(1024)
#                      if(typ == 2):
                      print(self.dat)
                  except SocketError as e:
                          if(typ==0):
                            self.dat = "wait 0 0 0 0 0 10 180 20"
                          print "typ==0 true: self.dat set to 'wait 0 0 0 0 0 10 180 20'"     
                  if len(self.dat) != 0 :
                     try:
                        if(typ==2):
                           if(self.wait == 1):
                             self.connInfo[0].send('w')
                           else:
                             if(self.wait == 0):
                                self.connInfo[0].send('s')
                        else:
                            self.connInfo[0].send('Recieved the last command:'+str(self.dat))
                     except SocketError as e:
                            pass

                  if len(self.dat) == 0 :
                     print "Disconnected!" 
                     self.flag = [1, 1, 1, 1, 1]
                     self.connInfo[0], self.connInfo[1] = self.s.accept()

                  if typ == 0 and self.dat!='':
                     self.dataS[0] = self.dat
                     self.dataS=self.dataS[0].split(" ")
                     if(self.dataS[0] == str("wait") or self.dataS[0] == str("measure") or self.dataS[0] == str("emergency") or self.dataS[0] == str("shutdown") or self.dataS[0] == str("videoStream")) :  
    	                self.dataT = self.dataS[0:6]
                        meas.Toler = int(self.dataS[6])
                        accel.Sens[0] = float( ( float(self.dataS[7]) /100 ) )
                        accel.Toler[0] = int(self.dataS[8])
 
                  if typ == 1 and self.dat!='': 
 	   	     self.dataT[0] = self.dat
                     self.dataT = self.dataT[0].split(" ")
   		     accel.Data[0] = float(self.dataT[0])
	             accel.Data[1] = float(self.dataT[1])
	             accel.Data[2] = float(self.dataT[2])
                     accel.Stats[0] = float(self.dataT[3])
	             accel.Stats[1] = float(self.dataT[4])
	             accel.Stats[2] = float(self.dataT[5])
	             accel.Stats[3] = float(self.dataT[6])
		     accel.Stats[4] = float(self.dataT[7])
		     accel.Stats[5] = float(self.dataT[8])
                     
                  if typ == 2 and self.dat!='':
                     self.dataT[0] = self.dat
                     self.dataT = self.dataT[0].split(" ")
                     if(str(self.dataT[0]) != str('r')):
                        bAndT.Temp[0] = float(self.dataT[0])+ (float(self.dataT[1])/100)
                        bAndT.Batt[0] = float(self.dataT[2])+(float(self.dataT[3])/100)
                        bAndT.stopBase[0] = float(self.dataT[4])
                        bAndT.stopRadiom[0] = float(self.dataT[5])
