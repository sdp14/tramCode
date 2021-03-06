# StateMachine/tram/TramAction.py

class TramAction:
    def __init__(self, action):
        self.action = action
    def __str__(self): return self.action
    def __cmp__(self, other):
        return cmp(self.action, other.action)
    # Necessary when __cmp__ or __eq__ is defined
    # in order to make this class usable as a
    # dictionary key:
    def __hash__(self):
        return hash(self.action)

# Static fields; an enumeration of instances:
TramAction.wait = TramAction("wait")
TramAction.measure = TramAction("measure")
TramAction.picture = TramAction("videoStream")
TramAction.emergency = TramAction("emergency")
TramAction.shutdown = TramAction("shutdown")
