#!/usr/bin/env python2.7
from kazoo.client import KazooClient
from kazoo.client import KazooState

import logging
logging.basicConfig(format='%(asctime)s %(message)s', level=logging.DEBUG)

import time
import socket
import os

#logging.basicConfig()

ADR  = '127.0.0.1'
PORT = '2181'
zk = KazooClient(hosts=ADR+':'+PORT)

def my_listener(state):
    if state == KazooState.LOST:
        # Register somewhere that the session was lost
        logging.info('Session lost')
    elif state == KazooState.SUSPENDED:
        # Handle being disconnected from Zookeeper
        logging.info('Disconnected from ZK')
    else:
        # Handle being connected/reconnected to Zookeeper
        logging.info('Connected to ZK')

zk.add_listener(my_listener)

zk.start()
logging.info("I am '%s-%d'" %(socket.gethostname(),os.getpid()))
zk.create("/leader",ephemeral=True, sequence=True)
