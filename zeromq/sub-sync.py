import time, zmq, sys
import logging, hashlib
from threading import Thread
from optparse import OptionParser

logging.basicConfig(level=logging.DEBUG)

# Parse command line options and dump results
def parseOptions():
    "Parse command line options"
    parser = OptionParser()
    parser.add_option('--remote-host', dest='host', default="127.0.0.1", help='Remote host IP address')
    parser.add_option('--remote-port', dest='port', default="2222", help='Remote TCP port')
    parser.add_option('--sync', dest='sync', default="2223", help='Synchronization TCP port')
    parser.add_option('--log-file', dest='fname', default="streaming_res.log", help='Log file name')
    (options, args) = parser.parse_args()

    return options, args

class Poller(Thread):
  def __init__(self, id, topic):
      super().__init__()
      self.id = id
      self.topic = topic

  def run(self):
      opts, args = parseOptions()
      # Initialize log files
      results_log = open(opts.fname, 'a+')

      sync_context = zmq.Context()
      logging.info("SYNCing with Publisher...")
      sync_socket = sync_context.socket(zmq.REQ)
      sync_socket.connect("tcp://"+opts.host+":"+opts.sync)
      sync_socket.send_string("SYNC")
      resp = sync_socket.recv_string()
      logging.info("Received reply: %s" % resp)

      self.start = time.time()
      logging.debug('start poller {} with topic {}'.format(self.id, self.topic))

      subscriber = context.socket(zmq.SUB)
      subscriber.connect("tcp://"+opts.host+":"+opts.port)
      subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic)
      self.loop = True
      count = 0
      while self.loop:
          message = subscriber.recv()
          size = sys.getsizeof(message)
          if size >= 4194304:
              count += 1
          #logging.debug('MSG {} @ local time {}'.format(self.id, time.strftime('%H:%M:%S')))
          logging.debug('poller {}th {}: {} bytes @ local time {} {}'.format(count, self.id, size, time.strftime('%H:%M:%S'), hashlib.md5(message).hexdigest()))
          if message == 'SciStream:STOP':
              t = time.time() - self.start
              results_log.write("%s,%s\n" % (t, count))
              results_log.flush()
              self.loop = False
              results_log.close()
              break

  def stop(self):
      self.loop = False

if __name__ == '__main__':
    context = zmq.Context()

    poller1 = Poller(1, 'SciStream')
    poller1.start()

    sys.exit(0)
