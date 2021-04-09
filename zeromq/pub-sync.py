import sys
import zmq
import time, hashlib
import logging, string
from optparse import OptionParser

logging.basicConfig(level=logging.DEBUG)

# Parse command line options and dump results
def parseOptions():
    "Parse command line options"
    parser = OptionParser()
    parser.add_option('--port', dest='port', default="3333", help='Publisher TCP port')
    parser.add_option('--sync', dest='sync', default="3334", help='Publisher synchronization TCP port')
    parser.add_option('--ds', dest='ds', default=4, help='Data size in MB')
    parser.add_option('--st', dest='st', default=0.1, help='Sample time in seconds')
    parser.add_option('--ns', dest='ns', default=1000, help='Number of samples')
    (options, args) = parser.parse_args()

    return options, args

if __name__ == '__main__':
    opts, args = parseOptions()
    size = int(opts.ds)*(2**20) - 10
    sample_time = float(opts.st)
    n = int(opts.ns)

    sync_context = zmq.Context()
    sync_socket = sync_context.socket(zmq.REP)
    sync_socket.bind("tcp://*:%s" % opts.sync)
    logging.info("SYNCing on port %s" % opts.sync)
    message = sync_socket.recv_string()
    logging.info("Received: %s" % message)
    sync_socket.send_string("SYNC_REP")
    logging.info("SYNCed")

    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.bind("tcp://*:"+opts.port)

    logging.info("Starting Publisher...")
    for index in range(n):
        time.sleep(sample_time)
        _msg = 'SciStream:'+('a' * size)
        socket.send_string( _msg )
        logging.debug("published MSG %s of size %s, %s" % (index, sys.getsizeof( _msg ), hashlib.md5(_msg.encode()).hexdigest()))

    socket.send_string('SciStream:STOP')
    logging.info("Streaming ended, exiting...")
    sys.exit(0)
