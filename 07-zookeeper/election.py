#!/usr/bin/env python2.7
import time, socket, os, uuid, sys, kazoo, logging, signal, inspect
from kazoo.client import KazooClient
from kazoo.client import KazooState
from kazoo.exceptions import KazooException
from utils import MASTER_PATH

ADR  = '127.0.0.1'
PORT = '2181'

class Election:
  def __init__(self, zk, path, func, args):
    self.election_path = path
    self.zk = zk
    self.is_leader = False
    self.args = args
    # Record callback function
    if not (inspect.isfunction(func)) and not(inspect.ismethod(func)):
      logging.debug("not a function "+str(func))
      raise SystemError
    self.func = func
    
    # Ensure the parent path, create if necessary
    self.zk.ensure_path(path)
    self.path = path

    # Start election
    self.node = zk.create(path+"/election", ephemeral=True, sequence=True)

    self.check_leader(None)


  def check_leader(self, event):
    ## Providing a watch function here would induce a herd effect
    children = self.zk.get_children(path=MASTER_PATH)
    my_idx = self.node.split('/')[-1]
    candidates = [c for c in children if c < my_idx]
    # Check if leader
    if len(candidates) == 0:
      self.become_leader()
    else:
      # Attach watchpoint on the node with highest index lower than self
      self.zk.get(path=MASTER_PATH+"/"+max(candidates), watch=self.check_leader)


  def is_leading(self):
    return self.is_leader

  def become_leader(self):
    self.is_leader = True
    self.func(self.args)

      
def hello(args):
  print("Becomming the election")


if __name__ == '__main__':
  zkhost = ADR + ':' + PORT #default ZK host
  logging.basicConfig(format='%(asctime)s %(message)s',level=logging.DEBUG)
  if len(sys.argv) == 2:
    zkhost=sys.argv[1]
    print("Using ZK at %s"%(zkhost))

  zk = KazooClient(zkhost)
  zk.start()
  election = Election(zk,MASTER_PATH,hello,[])
      
  while True:
    time.sleep(1)
