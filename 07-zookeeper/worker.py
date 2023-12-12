#!/usr/bin/env python2.7
import time, socket, os, uuid, sys, kazoo, logging, signal, utils
from election import Election
from utils import MASTER_PATH, TASKS_PATH, DATA_PATH, WORKERS_PATH, task

zk = None

class Worker:
  def __init__(self,zk):
    self.zk = gzk = zk
    self.uuid = uuid.uuid4()
    # Create and watch /worker/uuid
    w = self.zk.create(path=WORKERS_PATH+'/'+str(self.uuid), ephemeral=False)
    # self.zk.DataWatch(path=w, func=self.assignment_change)
    zk.get_children(path=w, watch=self.assignment_change)


  # Do something upon the change on assignment
  # def assignment_change(self, data, stat, event):
  def assignment_change(self, event):
    # Ignore trigger on setting the watchpoint
    # if event is None:
    #   return
    # print("")
    # print(">>>>>>>>>> %s <<<<<<<<<<" % str(data))
    # print("")
    # return
    
    # Get the task
    t = zk.get_children(path=event.path)[0]
    data = DATA_PATH + '/' + t
    print("\n\n"+data)
    d = float(zk.get(data)[0])
    print(d)
    # Execute the task
    task(d)
    # Delete the asignment
    zk.delete(path=event.path + '/' + t)
    # Set back watchpoint
    zk.get_children(path=event.path, watch=self.assignment_change)


if __name__ == '__main__':
  zk = utils.init()
  worker = Worker(zk)
  while True:
    time.sleep(1)
