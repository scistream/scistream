# -*- coding: UTF-8 -*-

import sys
import zmq
import time
from optparse import OptionParser

# Parse command line options and dump results
def parseOptions():
    "Parse command line options"
    parser = OptionParser()
    parser.add_option('--port', dest='port', default="50000", help='Publisher TCP port')
    (options, args) = parser.parse_args()

    return options, args

opts, args = parseOptions()

context = zmq.Context()

socket = context.socket(zmq.PUB)

socket.bind("tcp://127.0.0.1:" + opts.port)
# socket.bind("tcp://*:3333")

for index in range(3600):
    time.sleep(0.05)
    _msg = 'NASDA:' + '%04dth message from publisher @ %s' % (index, time.strftime('%H:%M:%S'))
    socket.send_string( _msg )
    print("published MSG: %s" % _msg)

socket.send_string('NASDA:STOP')

sys.exit(0)
