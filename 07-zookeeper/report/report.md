---
title: "Lab 7: Coordination in practice with Apache ZooKeeper"
output: pdf
author:
 - "MAKSIMOVIC Aleksandar"
 - "PANIC Dimitrije"
 - "TOLJAGA Jana"
 - "Timothée ZERBIB"
header-includes: |
    \usepackage [margin=25mm, foot=15mm] {geometry}

    \usepackage{xcolor}
    \usepackage[onelanguage, ruled, lined]{algorithm2e}
    \newcommand\comment[1]{\footnotesize\ttfamily\textcolor{green}{#1}}
date: "\\today"
---


# Coordination in practice with Apache ZooKeeper

## Introduction

This practical shows how to build a dependable master/worker architecture
that execute tasks dispatched by clients using the Apache Zookeeper coordination
service.

This lab has taken us more time than expected as our initial design was not
compatible with the Zookeeper API. It is unfinished, yet we present
the current advancement of our architecture.

This report and all the source code are also available
on the [github](https://github.com/jacikot/cloud_labs) of this course.


## Setup

1. Once installed, the Zookeeper service is started with its start script:
   ```bash
   ./zkServer.sh start
   ```

2. Using zk-shell, we created the permanent paths ``/master``, ``/tasks``,
   ``/data`` Sand ``/workers``.
  
```bash
# start the zk-shell
zk-shell localhost:<port>
# Once in the zk-shell, create the folders
/> create master 'master'
/> create tasks 'tasks'
/> create data 'data'
/> create workers 'workers'
```

## Leader election

All agent competing to become leader will create an instance
of the é``Election`` class that creates a file with ``ephemeral``
and ``sequence`` options.
Then each agent checks if the created file has minimum name
(using the alphanumerical order). The option ``sequence`` guarantee that
each name is different thus giving a way to distinguish agents.
Finally, a watchpoint can be set on the master file.
The ``ephemeral`` option delete the file of an agent if it fails
(crashes or lags), making this failure visible to other agents that can start
the Election process again (by just comparing the remaining files names again).


Having a watchpoint directly on the ``/master`` node makes the system
sensible to **herd effect**.
In order to prevent this, in our implementation, the watchpoint was moved
to the node with the biggest index smaller than the one owned by the agent.
Thus, if an agent $i$ is disconnected, the running agent with the lowest id $j$
such that $i < j$ was the only one to watch on it and will recalculate
the list of running agents with $\mathit{id} < j$ to determine
the new watchpoint to set (i.e. on the agent previously watched by $i$).
If this list is empty, it means that it is the new leader.

The leader election process is implemented in ``election.py``.

## A dependable master/worker architecture

The client/worker architecture is implemented using files.

A task is represented by a file $f$ in ``/tasks`` and its data are located
in ``/data/f``. A task completion happens when the file in ``/tasks``
is deleted ; the client is in charge of deleting the corresponding data
(which could contain the return of the function for instance).

The assignment of task $t$ to worker $w$ is achieved by creating a file
``/worker/w/t``.

The elected master watch for new tasks, workers and task completion.
It loops for unfinished tasks and unassigned workers and makes the assignation.

In our initial design, the task assignment of a worker is represented
by a child to the worker node.
However, as Zookeeper does not allow for ``ephemeral`` nodes to have children,
it was not possible to make the worker node ``ephemeral`` and thus to support
worker failures. We should thus change our design and use the data
of the worker nodes and the ``DataWatch`` function instead
of our initial design.
