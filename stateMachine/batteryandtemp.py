from subprocess import PIPE, Popen


class batteryandtemp:
       def __init__(self):
	   self.Batt = [0.0]
           self.Temp = [0.0]
           self.stopBase = [5.0]
           self.stopRadiom = [5.0]
	   p = Popen(["./modbusBatTemp", "192.168.3.115", "1","0","15"],stdout=PIPE,cwd='/home/tram/tramCode/modbus')
         
