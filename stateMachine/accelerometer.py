from subprocess import PIPE, Popen


class accelerometer:
       def __init__(self):
	   self.Data = [0.0, 0.0, 0.0]
           self.Stats = [0, 0, 0, 0, 0, 0]
           self.Danger = [0, 0]
           self.Sens = [float(2.5), float(0.0)]
           self.Toler = [20]
           p = Popen(["./startAcceler.sh"],stdout=PIPE,cwd='/home/tram/tramCode/accelerometerC/')         

                  
