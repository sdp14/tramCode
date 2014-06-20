from subprocess import PIPE, Popen


class measure:
       def __init__(self):
	   self.done = 0
           self.busy = 0
           self.streamVid=0
           self.Toler=10
           self.Retry=0
           self.initAccel=1 
           self.a = None
           self.b = None
           self.c = None
           self.d = None
           self.e = None

       def takeMeasurements(self,loggerYes,pictureYes,videoYes,RTMPstreamYes,RTMPstreamNo):
           if loggerYes == 1 :
              self.a = Popen(["./modbus", "192.168.3.115","1","0","15"],cwd='/home/tram/tramCode/modbus')
	   if pictureYes == 1 and self.streamVid==0:
	      self.b = Popen(["./grabber"],cwd='/home/tram/tramCode/boneCV/')
	   if videoYes == 1 and self.streamVid==0 :	   
	      self.c = Popen(["./video.sh"],cwd='/home/tram/tramCode/boneCV/')
           if RTMPstreamYes == 1 and self.streamVid==0 :
	      self.d = Popen(["./startStream.sh"],cwd='/home/tram/tramCode/boneCV/')
	   if RTMPstreamNo == 1 and self.streamVid==1:
              self.e = Popen(["./stopStream.sh"],cwd='/home/tram/tramCode/boneCV/')
