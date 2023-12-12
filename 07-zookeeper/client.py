#!/usr/bin/env python2.7
import time, socket, os, uuid, sys, kazoo, logging, signal, utils, random
from election import Election
from utils import MASTER_PATH
from utils import TASKS_PATH
from utils import DATA_PATH
from utils import WORKERS_PATH


task = {
  "fun": "sleep",
  "args": "2.6"
}


class Client:
  def __init__(self,zk):
    self.zk = zk
    self.completed = 0

  def submit_task(self, task):
    # Create an entry 'task_<id>' in '/data' with value set to the task arguments
    data = task['args'].encode('ASCII')
    d = self.zk.create(path=DATA_PATH+"/task_", value=data, sequence=True)
    # Create an entry in '/task', which immediately makes it available for master
    task = self.zk.create(path=TASKS_PATH+d.partition(DATA_PATH)[-1], value="0".encode('ASCII'))
    # Watch for task completion
    zk.get(path=task, watch=self.task_completed)

  # A task is completed when the corresponding entry in /tasks is deleted
  def task_completed(self, event):
    data = DATA_PATH + event.path.partition(TASKS_PATH)[-1]
    # Delete the data (Eventually use it before if the function should return)
    zk.delete(path=data)
    self.completed = self.completed + 1

# Continually submit n tasks and wait for their completion before starting again
  def submit_task_loop(self, task, n):
    while True:
      for i in range(n):
        t = self.submit_task(task)
        while(self.completed != n):
          time.sleep(1)
        self.completed = 0


if __name__ == '__main__':
  zk = utils.init()
  client = Client(zk)

  # Submit tasks by batch of 5
  client.submit_task_loop(task, 5)
  
  client.submit_task(task)
  while True:
    time.sleep(1)

