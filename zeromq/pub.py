#!/Users/zliu/usr/miniconda3/bin/python
# -*- coding: UTF-8 -*-

import sys
import zmq
import time

context = zmq.Context()

socket = context.socket(zmq.PUB)
socket.bind("tcp://*:50000")

for index in range(3600):
    time.sleep(2)
    _msg = 'NASDA:' + '%04dth message from publisher @ %s' % (index, time.strftime('%H:%M:%S'))
    socket.send_string( _msg )
    print("published MSG: %s" % _msg)

socket.send_string('NASDA:STOP')

sys.exit(0)
