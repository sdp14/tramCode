# StateMachine/StateMachine.py

from TramAction import TramAction
import string
from server import server
from thread import *
import threading
from accelerometer import accelerometer
from measure import measure
from subprocess import PIPE, Popen
import sys
from batteryandtemp import batteryandtemp

class StateMachine:
    def __init__(self, initialState):
        self.currentState = initialState
        

    def runAll(self):      
        count = 1
        serv = server('192.168.2.103',52000)
        serv1 = server('192.168.2.103',52002)
        serv2 = server('192.168.2.103',52001)
        accel = accelerometer()
        meas = measure()
        bAndT = batteryandtemp()
        thread1 = threading.Thread(target=serv.listun,args=(0, accel,meas,bAndT))
        thread2 = threading.Thread(target=serv1.listun,args=(1,accel,meas,bAndT))
        thread3 = threading.Thread(target=serv2.listun,args=(2,accel,meas,bAndT))
        thread1.start()
        thread2.start()
        thread3.start()
        while count == 1 :
              serv.data = map(TramAction, serv.dataT)
	      self.currentState.run(serv,serv2,accel,meas,bAndT)

              if((bAndT.Batt[0] != 0.0 and bAndT.Batt[0] < 11.75) or (bAndT.Temp[0] != 0.0 and bAndT.Temp[0] > 40) or int(bAndT.stopBase[0]) == int(1.0) or int(bAndT.stopRadiom[0]) == int(1.0)) :
                 serv.dataT[0] = (str("shutdown"))
                 serv.data = map(TramAction, serv.dataT)
                 
              if str(serv.data[0]) == str('measure'):
                 serv2.wait=1


	      self.currentState = self.currentState.next(serv.data[0])

	      if(thread1.isAlive() == False) :
                 thread1 = threading.Thread(target=serv.listun,args=(0, accel,meas,bAndT))
                 thread1.start()

	      if(thread2.isAlive() == False) :
                 thread2 = threading.Thread(target=serv1.listun,args=(1,accel,meas,bAndT))
                 thread2.start()

              if(thread3.isAlive() == False) :
                 thread3 = threading.Thread(target=serv2.listun,args=(2,accel,meas,bAndT))
                 thread3.start()

