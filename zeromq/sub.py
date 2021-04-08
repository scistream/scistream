#!/Users/zliu/usr/miniconda3/bin/python
# -*- coding: UTF-8 -*-
import time, zmq, sys
from threading import Thread


class Poller(Thread):
  def __init__(self, id, topic):
      super().__init__()
      self.id = id
      self.topic = topic

  def run(self):
      print('start poller {} with topic {}'.format(self.id, self.topic))
      subscriber = context.socket(zmq.SUB)
      subscriber.connect("tcp://127.0.0.1:2222")
      subscriber.setsockopt_string(zmq.SUBSCRIBE, self.topic)
      self.loop = True
      while self.loop:
        message = subscriber.recv()
        print('poller {}: {} @ local time {}'.format(self.id, message, time.strftime('%H:%M:%S')))

  def stop(self):
      self.loop = False

context = zmq.Context()

poller1 = Poller(1, 'NASDA')
poller1.start()

# poller2 = Poller(2, 'NASDAQ')
# poller2.start()

time.sleep(60*5) # terminate after 5 minutes

poller1.stop()
# poller2.stop()

sys.exit(0)
