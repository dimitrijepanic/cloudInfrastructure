import os
import sys
from statistics import mean
import matplotlib.pyplot as plt

def parse_file(lines, t):
  data = []
  for l in lines:
    l = l.replace(",", ".")
    data.append(float(l[7:12]))
  t.append(data)


def extract(t, f):
  res = []
  for i in range(len(t[0])):
    res.append(f(t,i))
  return res


def plot(x, ys, size, labels, savepath):
  for y,l in zip(ys,labels):
    plt.plot(x, y, label=l)

  plt.xlim((1, size))
  plt.ylim(0)
  plt.xlabel("Requests")
  plt.ylabel("Time (in s)")

  plt.fill_between(x, ys[0], ys[1], color='orange', alpha=0.5, interpolate=True)
  plt.legend(loc='lower center', fancybox=True, shadow=True)
  plt.tight_layout()

  plt.savefig(savepath)
  # plt.show()


def create_plot(datas, size, t, savepath):
  res = []
  labels = []
  
  # Create interesting arrays
  for k in t:
    res.append(extract(datas, k["fun"]))
    labels.append(k["label"])

  # Plot
  x = [i for i in range(1, size+1)]
  plot(x, res, size, labels, savepath)


def main():
    if len(sys.argv) < 4:
        print("usage: {} <request_number> <path> <dest>".format(sys.argv[0]))
        exit(1)
    
    # Array of values
    t = []

    for filename in os.listdir(sys.argv[2]):
      f = os.path.join(sys.argv[2], filename)
      # checking if it is a file
      if os.path.isfile(f):
        # Gathering values
        f = open(f, "r")
        lines = f.readlines()
        parse_file(lines, t)
        f.close()
      
    # Create interesting files
    results = [
      {
        "fun": lambda tab,idx : [min(i) for i in zip(*tab)][idx],
        "label": "Minimum latency"
      },
      {
        "fun": lambda tab,idx : [max(i) for i in zip(*tab)][idx],
        "label": "Maximum latency"
      },
      {
        "fun": lambda tab,idx : [mean(i) for i in zip(*tab)][idx],
        "label": "Average latency"
      }
    ]

    create_plot(t, int(sys.argv[1]), results, sys.argv[3])


if __name__ == "__main__":
    sys.exit(main())
