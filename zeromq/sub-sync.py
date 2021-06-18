import time, zmq, sys
import logging
from threading import Thread
from optparse import OptionParser

logging.basicConfig(level=logging.INFO)

# Parse command line options and dump results
def parseOptions():
    "Parse command line options"
    parser = OptionParser()
    parser.add_option('--remote-host', dest='host', default="127.0.0.1", help='Remote host IP address')
    parser.add_option('--remote-port', dest='port', default="50000", help='Remote TCP port')
    parser.add_option('--sync', dest='sync', default="51000", help='Synchronization TCP port')
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

      sync_context = zmq.Context(io_threads=2)
      logging.info("SYNCing with Publisher...")
      sync_socket = sync_context.socket(zmq.REQ)
      sync_socket.connect("tcp://"+opts.host+":"+opts.sync)
      sync_socket.send_string("SYNC")
      resp = sync_socket.recv_string()
      logging.info("Received reply: %s" % resp)

      logging.info('start poller {} with topic {}'.format(self.id, self.topic))

      context = zmq.Context()
      subscriber = context.socket(zmq.SUB)
      subscriber.connect("tcp://"+opts.host+":"+opts.port)
      subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic)
      self.loop = True
      count = 0
      self.start = time.time()
      while self.loop:
          message = subscriber.recv_string()
          now = time.time()
          #size = sys.getsizeof(message)
          #if size >= 4194304:
          count += 1
          if not (count%10):
              logging.debug('{}: MSG {} @ local time {}'.format(count, message[:11], time.strftime('%H:%M:%S')))
          #logging.debug('poller {}th {}: {} bytes @ local time {} {}'.format(count, self.id, size, time.strftime('%H:%M:%S'), hashlib.md5(message).hexdigest()))
          if message == 'SciStream:STOP':
              t = t_last_msg - self.start
              logging.info("Experiment time: %s seconds, %s samples received." % (t, count-1))
              results_log.write("%s,%s\n" % (t, count-1))
              results_log.flush()
              self.loop = False
              results_log.close()
              sync_socket.send_string("FIN")
              resp = sync_socket.recv_string()
              logging.info("Received reply: %s" % resp)
              break
          else:
              t_last_msg = now

  def stop(self):
      self.loop = False

if __name__ == '__main__':
    poller1 = Poller(1, 'SciStream')
    poller1.start()

    sys.exit(0)
