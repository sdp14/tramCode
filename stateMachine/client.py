#-------------------------------------------------------------------------------
# Name:        module1
# Purpose:
#
# Author:      Arsid
#
# Created:     24/01/2014
# Copyright:   (c) Arsid 2014
# Licence:     <your licence>
#-------------------------------------------------------------------------------
#!usr/bin/python
import socket

class client:

    def speak(self, host, port, data_to_send):
        HOST = host             # The remote host
        PORT = port             # The same port as used by the server	      
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)   
	s.connect((HOST, PORT))
        s.send(data_to_send)
        data = s.recv(1024)
        s.close()
        print 'Received', repr(data)
         


        

	





