#!/usr/bin/env python2.7
import time, socket, os, uuid, sys, kazoo, logging, signal, utils
from election import Election
from utils import MASTER_PATH
from utils import TASKS_PATH
from utils import DATA_PATH
from utils import WORKERS_PATH

class Master:
  #initialize the master
  def __init__(self,zk):
    self.zk = zk
    # Only the effective master will watch for /tasks and /workers
    # to prevent herd effect
    # Elect a master (the elected master will call 'watch')
    self.election = Election(self.zk, MASTER_PATH, self.watch, None)


  # Watch for changes on tasks and workers
  def watch(self, args):
    self.zk.get_children(path=TASKS_PATH, watch=self.workload_change)
    self.zk.get_children(path=WORKERS_PATH, watch=self.workload_change)


  # Called if there is a change in the tasks, workers or worker assignments
  # Try to assign all non assigned tasks to idle workers
  def workload_change(self, event):
    tasks = self.zk.get_children(path=TASKS_PATH)
    workers = self.zk.get_children(path=WORKERS_PATH)
    
    # Assign one non already assigned task to a free worker
    if len(tasks):
      for worker in workers:
        task = zk.get_children(path=WORKERS_PATH + '/' + worker)
        # Assign a task is worker is free
        if len(task) == 0:
          print("\n\n" + WORKERS_PATH + '/' + worker + '/' +  tasks[0])
          task = self.zk.create(path=WORKERS_PATH + '/' + worker + '/' +  tasks[0])
          tasks.pop(0)
          # Watch if the worker finishes his assignment
          zk.get(path=task, watch=self.task_completed)
          # Stop if there is no more task to assign
          if len(tasks) == 0:
            break

    # Reset the watch on changes on tasks or workers
    self.zk.get_children(path=TASKS_PATH, watch=self.workload_change)
    self.zk.get_children(path=WORKERS_PATH, watch=self.workload_change)


  # Triggered when a worker finished its assignment
  def task_completed(self, event):
    # Delete the asignment
    task = zk.get_children(path=event.path)[0]
    zk.delete(path=TASKS_PATH + '/' + t)
    # Call workload_changes as a worker just finished its job
    self.workload_change(None)

  
  # Assign tasks
  def assign(self,children):
    pass


if __name__ == '__main__':
  zk = utils.init()
  master = Master(zk)
  while True:
    time.sleep(1)
