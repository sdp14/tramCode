#-------------------------------------------------------------------------------
# Name:        ServerSTM
# Purpose:     Defines the behavior of the server
# Author:      The Internet
# Editor:      M
# Created:     01/26/2014
#-------------------------------------------------------------------------------
# StateMachine/TramControl.py
# Autonomous and directed control of tram operation
import pudb; pu.db
import string, sys, time

import subprocess 
sys.path += [".\lib"]
from ctypes import *
from State import State
from StateMachine import StateMachine
from TramAction import TramAction
from socket import error as SocketError
import time
#import pdb

#pdb.set_trace()

class StateT(State):
    def __init__(self):
        self.transitions = None
    def next(self, input):
        if(input in self.transitions):
            return self.transitions[input]
        else:
            print ("Input %s not supported for current state" % input)
            return self

class Wait(StateT):
    def run(self, serv,serv2,accel,meas,bAndT):
        if(serv.flag[0] == 1) :
          print("Tram Waiting for Command from Base Station")
          serv.flag[0] = 0
          serv.flag[1] = 1
          serv.flag[2] = 1
          serv.flag[3] = 1
          serv.flag[4] = 1

        if accel.Data[0] < (accel.Stats[0] - (accel.Sens[0]*accel.Stats[3])) and accel.Data[0] > (accel.Stats[0] + (accel.Sens[0]*accel.Stats[3])) or accel.Data[1] < (accel.Stats[1] - (accel.Sens[0]*accel.Stats[4])) or accel.Data[1] > (accel.Stats[1] + (accel.Sens[0]*accel.Stats[4])) or accel.Data[2] < (accel.Stats[2] - (accel.Sens[0]*accel.Stats[5])) or accel.Data[2] > (accel.Stats[2] + (accel.Sens[0]*accel.Stats[5])) :
           serv.prevState[0] = (str("wait"))
	   serv.dataT[0] = (str("emergency"))
           serv.data = map(TramAction, serv.dataT)

    def next(self,input):
        # Supported transition initialization:
        if not self.transitions:
            self.transitions = {
                TramAction.wait : TramControl.wait,
                TramAction.measure : TramControl.measure,
                TramAction.emergency : TramControl.emergency,
                TramAction.picture : TramControl.picture,
                TramAction.shutdown : TramControl.shutdown
           }
        return StateT.next(self, input)

class Measure(StateT):
    def run(self,serv,serv2,accel,meas,bAndT):
        if(serv.flag[1] == 1) : 
          print("Tram Measuring")
          serv.flag[0] = 1
          serv.flag[1] = 0
          serv.flag[2] = 1
          serv.flag[3] = 1
          serv.flag[4] = 1
          if(meas.initAccel == 1):
             print "accel1"
             accel.Danger[0] = 0
             accel.Danger[1] = 0
             meas.initAccel = 0
             print "accel2"
       
        #print(accel.Data)
	#print(accel.Stats)
	#print(accel.Sens)
        if accel.Data[0] > (accel.Stats[0] - (accel.Sens[0]*accel.Stats[3])) or accel.Data[0] < (accel.Stats[0] + (accel.Sens[0]*accel.Stats[3])) or accel.Data[1] > (accel.Stats[1] - (accel.Sens[0]*accel.Stats[4])) or accel.Data[1] < (accel.Stats[1] + (accel.Sens[0]*accel.Stats[4])) or accel.Data[2] > (accel.Stats[2] - (accel.Sens[0]*accel.Stats[5])) or accel.Data[2] < (accel.Stats[2] + (accel.Sens[0]*accel.Stats[5])) :
           #print "accel ok"
           if meas.busy==0 and (meas.a == None or meas.a.poll() != None) and (meas.b == None or meas.b.poll() != None) and (meas.c == None or meas.c.poll() != None) :
              #print(serv.dataT)
              if meas.streamVid==0:
                 meas.takeMeasurements(int(serv.dataT[1]),int(serv.dataT[2]),0,0,0)
                 meas.busy=1
              else :
                 if meas.busy==0 and (meas.a == None or meas.a.poll() != None) and (meas.b == None or meas.b.poll() != None) (meas.c == None or meas.c.poll() != None): 
                    meas.takeMeasurements(int(serv.dataT[1]),0,0,0,0)
                    meas.busy=1
    
	   if meas.streamVid==0 and (meas.a == None or meas.a.poll() != None) and (meas.b == None or meas.b.poll() != None) and (meas.c == None or meas.c.poll() != None) and meas.busy == 1 :
              meas.takeMeasurements(0,0,int(serv.dataT[3]),0,0)
              meas.busy=2
           else :
              if (meas.a == None or meas.a.poll() != None) and (meas.b == None or meas.b.poll() != None) and (meas.c == None or meas.c.poll() != None) :
                 meas.busy=2

           if meas.busy == 2 and (meas.a == None or meas.a.poll() != None) and (meas.b == None or meas.b.poll() != None) and (meas.c == None or meas.c.poll() != None) :
              print("busy = 2: done with measurement?")
              meas.busy=0
              try:
                  if(int((accel.Danger[0] + (100*accel.Danger[1])))<int(meas.Toler)):
                     print("trying to send done")
                     if(meas.Retry == 0):
                       print("trying to send done retry=0")
                       print "CASE 1"
                       print(connInfo[0].getpeername)
                       print(connInfo[0].getsockname)
                       serv.connInfo[0].send('done')
                       serv.connInfo[0].send('0')
                       serv2.wait=0
                     if(meas.Retry > 0):
                       print("trying to send done retry!=0")
                       print "CASE 2"
                       print(connInfo[0].getpeername)
                       print(connInfo[0].getsockname)
                       serv.connInfo[0].send('done')
                       serv.connInfo[0].send('1')
                       serv2.wait=0
                     meas.initAccel = 1
                     meas.Retry = 0
              except SocketError as e:
                     print("socket error!!")
                     pass
              if(int((accel.Danger[0] + (100*accel.Danger[1])))<int(meas.Toler)):
                 print("going into wait")
	         serv.dataT[0]=(str("wait"))
                 serv.data = map(TramAction, serv.dataT)
                 accel.Danger[0] = 0
                 accel.Danger[1] = 0
              if(int((accel.Danger[0] + (100*accel.Danger[1])))>int(meas.Toler)):
                 serv.flag[1] = 1
                 meas.Retry = meas.Retry + 1
                 accel.Danger[0] = 0
                 accel.Danger[1] = 0
                 if(meas.Retry > 10):
                    serv.dataT[0]=(str("shutdown"))
                    serv.data = map(TramAction, serv.dataT)
                    meas.Retry = 0

           if accel.Data[0] < (accel.Stats[0] - (accel.Sens[0]*accel.Stats[3])) or accel.Data[0] > (accel.Stats[0] + (accel.Sens[0]*accel.Stats[3])) or accel.Data[1] < (accel.Stats[1] - (accel.Sens[0]*accel.Stats[4])) or accel.Data[1] > (accel.Stats[1] + (accel.Sens[0]*accel.Stats[4])) or accel.Data[2] < (accel.Stats[2] - (accel.Sens[0]*accel.Stats[5])) or accel.Data[2] > (accel.Stats[2] + (accel.Sens[0]*accel.Stats[5])) :
	       serv.prevState[0] = (str("measure"))
	       serv.dataT[0] = (str("emergency"))
               serv.data = map(TramAction, serv.dataT)
	else:
            print "accel bad"

    def next(self, input):
        # Supported transition initialization:
        if not self.transitions:
            self.transitions = {
                TramAction.wait : TramControl.wait,
                TramAction.measure : TramControl.measure,
                TramAction.emergency : TramControl.emergency,
                TramAction.picture : TramControl.picture,
                TramAction.shutdown : TramControl.shutdown
            }
        return StateT.next(self, input)

class Picture(StateT):
    def run(self,serv,serv2,accel,meas,bAndT):
        if(serv.flag[2] == 1):
          print("Tram Streaming Video")
          serv.flag[0] = 1
          serv.flag[1] = 1
          serv.flag[2] = 0                              
          serv.flag[3] = 1
          serv.flag[4] = 1
   
        if meas.busy == 0 :
           meas.takeMeasurements(0,0,0,int(serv.dataT[4]),int(serv.dataT[5]))
           meas.streamVid=1
           meas.busy=1

	if meas.busy == 1 and (meas.d == None or meas.d.poll() != None) :
           meas.takeMeasurements(0,0,0,int(serv.dataT[4]),int(serv.dataT[5]))
           meas.busy=0
           meas.streamVid=0
           try:
              serv.connInfo[0].send('done')
              serv.connInfo[0].send('0')
           except SocketError as e:
              pass
	   serv.dataT[0]=(str("wait"))
           serv.data = map(TramAction, serv.dataT)
       
        if accel.Data[0] < (accel.Stats[0] - (accel.Sens[0]*accel.Stats[3])) or accel.Data[0] > (accel.Stats[0] + (accel.Sens[0]*accel.Stats[3])) or accel.Data[1] < (accel.Stats[1] - (accel.Sens[0]*accel.Stats[4])) or accel.Data[1] > (accel.Stats[1] + (accel.Sens[0]*accel.Stats[4])) or accel.Data[2] < (accel.Stats[2] - (accel.Sens[0]*accel.Stats[5])) or accel.Data[2] > (accel.Stats[2] + (accel.Sens[0]*accel.Stats[5])) :
	   serv.prevState[0] = (str("streamVideo"))
	   serv.dataT[0] = (str("emergency"))
           serv.data = map(TramAction, serv.dataT)

    def next(self, input):
        # Supported transition initialization:
        if not self.transitions:
            self.transitions = {
                TramAction.wait : TramControl.wait,
                TramAction.measure : TramControl.measure,
                TramAction.emergency : TramControl.emergency,
                TramAction.picture : TramControl.picture,
                TramAction.shutdown : TramControl.shutdown
            }
        return StateT.next(self, input)

class Emergency(StateT):
    def run(self,serv,serv2,accel,meas,bAndT):
        if(serv.flag[3] == 1):
          print("Tram in Emergency State")
          serv.flag[0] = 1
          serv.flag[1] = 1
          serv.flag[2] = 1                              
          serv.flag[3] = 0
          serv.flag[4] = 1

        if accel.Data[0] < (accel.Stats[0] - (accel.Sens[0]*accel.Stats[3])) or accel.Data[0] > (accel.Stats[0] + (accel.Sens[0]*accel.Stats[3])) or accel.Data[1] < (accel.Stats[1] - (accel.Sens[0]*accel.Stats[4])) or accel.Data[1] > (accel.Stats[1] + (accel.Sens[0]*accel.Stats[4])) or accel.Data[2] < (accel.Stats[2] - (accel.Sens[0]*accel.Stats[5])) or accel.Data[2] > (accel.Stats[2] + (accel.Sens[0]*accel.Stats[5])) :
            accel.Danger[0] = accel.Danger[0] + 1
            print(accel.Danger[0])
            print(accel.Danger[1])
            if accel.Danger[0] == 100 :
               accel.Danger[0] = 0
               accel.Danger[1] = accel.Danger[1] + 1
        
        if (int(accel.Danger[1]) > int(accel.Toler[0])) or (accel.Data[0] < (accel.Stats[0] - ((int(accel.Sens[0])*20)*accel.Stats[3])) or accel.Data[0] > (accel.Stats[0] + ((int(accel.Sens[0])*20)*accel.Stats[3])) or accel.Data[1] < (accel.Stats[1] - ((int(accel.Sens[0])*20)*accel.Stats[4])) or accel.Data[1] > (accel.Stats[1] + ((int(accel.Sens[0])*20)*accel.Stats[4])) or accel.Data[2] < (accel.Stats[2] - ((int(accel.Sens[0])*20)*accel.Stats[5])) or accel.Data[2] > (accel.Stats[2] + ((int(accel.Sens[0])*20)*accel.Stats[5]))) :
	   serv.dataT[0]=(str("shutdown"))
           serv.data = map(TramAction, serv.dataT)
        else:
            if accel.Data[0] > (accel.Stats[0] - (accel.Sens[0]*accel.Stats[3])) and accel.Data[0] < (accel.Stats[0] + (accel.Sens[0]*accel.Stats[3])) and accel.Data[1] > (accel.Stats[1] - (accel.Sens[0]*accel.Stats[4])) and accel.Data[1] < (accel.Stats[1] + (accel.Sens[0]*accel.Stats[4])) and accel.Data[2] > (accel.Stats[2] - (accel.Sens[0]*accel.Stats[5])) and accel.Data[2] < (accel.Stats[2] + (accel.Sens[0]*accel.Stats[5])) :
               serv.dataT[0] = serv.prevState[0]
               serv.data = map(TramAction, serv.dataT)

    def next(self, input):
        # Supported transition initialization:
        if not self.transitions:
            self.transitions = {
                TramAction.wait : TramControl.wait,
                TramAction.measure : TramControl.measure,
                TramAction.emergency : TramControl.emergency,
                TramAction.picture : TramControl.picture,
                TramAction.shutdown : TramControl.shutdown
             }
        return StateT.next(self, input)

class Shutdown(StateT):
    def run(self,serv,serv2,accel,meas,bAndT):
        if(serv.flag[4] == 1):
          print("Tram Shutting Down")
          serv.flag[0] = 1
          serv.flag[1] = 1
          serv.flag[2] = 1                              
          serv.flag[3] = 1
          serv.flag[4] = 0
          serv2.wait=0
        
        shutdownReport = ""
        print bAndT.Batt[0]
        print bAndT.Temp[0]
        print bAndT.stopBase[0]
        print bAndT.stopRadiom[0]
        if 1 == 1:
       # if serv.connInfo[0] != None:
           try:
              if(serv.distress == 1):  
                # serv.connInfo[0].send('done')
                 serv.distress = 0
              if(int(bAndT.stopBase[0]) == 1 or int(bAndT.stopRadiom[0]) == 1):
                 shutdownReport += str(5)
	      else : 
                  shutdownReport += str(6) 
              if(bAndT.Batt[0] < 11.75) :
                 shutdownReport += str(4) 
              else:
                  if(bAndT.Batt[0] >= 12.00) :
                     shutdownReport += str(7)
              if(bAndT.Temp[0] > 40) :
                 shutdownReport += str(3) 
              else :
                  if(bAndT.Temp[0] < 40) :
                     shutdownReport  += str(8)
              if(accel.Danger[1] >  int(accel.Toler[0]) ): 
                 shutdownReport += str(2)
                 accel.Danger[0] = 0
                 accel.Danger[1] = 0
              else :
                 shutdownReport += str(0)
              meas.initAccel = 1
             #  serv.distress = 0
              print shutdownReport
             # serv.connInfo[0].send(shutdownReport)
            #  time.sleep(0.1)
           except SocketError as e:
                 pass

    def next(self, input):
	# Supported transition initialization:
        if not self.transitions:
            self.transitions = {
                TramAction.wait : TramControl.wait,
                TramAction.emergency : TramControl.emergency,
                TramAction.shutdown : TramControl.shutdown
            }
        return StateT.next(self, input)
    
class TramControl(StateMachine):
    def __init__(self):
        # Initial state
        StateMachine.__init__(self, TramControl.wait)

def main():
    pass


if __name__ == '__main__':

    main()
    TramControl.wait = Wait()
    
    TramControl.measure = Measure()
    TramControl.picture = Picture()
    TramControl.emergency= Emergency()
    TramControl.shutdown= Shutdown()
  
    
   
    TramControl().runAll()   



