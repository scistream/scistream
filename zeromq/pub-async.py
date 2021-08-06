import time, hashlib, sys, zmq, logging, string, threading, queue
from optparse import OptionParser

logging.basicConfig(level=logging.DEBUG)

# Parse command line options and dump results
def parseOptions():
    "Parse command line options"
    parser = OptionParser()
    parser.add_option('--port', dest='port', default="3333", help='Publisher TCP port')
    parser.add_option('--sync', dest='sync', default="3334", help='Publisher synchronization TCP port')
    parser.add_option('--ds', dest='ds', type=int, default=4, help='Data size in MB')
    parser.add_option('--st', dest='st', type=float, default=0.1, help='Sample time in seconds')
    parser.add_option('--ns', dest='ns', type=int, default=1000, help='Number of samples')
    (options, args) = parser.parse_args()

    return options, args

opts, args = parseOptions()
size = opts.ds * (2**20) - 10

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

_msg = 'SciStream:'+('a' * size)

logging.info("Starting Publisher...")

def consumer():
    while True:
        item = dq.get()
        # logging.debug("sent one unit of data @ %f" % time.time())
        socket.send_string( _msg )
        dq.task_done()

dq = queue.Queue()

threading.Thread(target=consumer, daemon=True).start()

for data_idx in range(opts.ns):
    time.sleep(opts.st)
    dq.put(data_idx)
    logging.debug("produced one unit of data @ %f" % time.time())

# block until all tasks are done
dq.join()

socket.send_string('SciStream:STOP')
logging.info("Streaming ended, SYNCing the exit...")
message = sync_socket.recv_string()
logging.info("Received: %s" % message)
sync_socket.send_string("FIN_ACK")
logging.info("Bye, bye...")
sys.exit(0)
