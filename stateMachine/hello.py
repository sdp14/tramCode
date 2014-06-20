def connect():
    global irc
    irc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    irc.connect((server, port))
    #and nick, pass, and join stuffs
connect()
while True:
    data = irc.recv(4096)
    if len(data) == 0:
        print "Disconnected!"
        connect()
