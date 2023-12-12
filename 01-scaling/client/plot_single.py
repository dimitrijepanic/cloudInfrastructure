import os
import sys
import matplotlib.pyplot as plt

# transform strings in data files (conatinig ',') to numbers (with '.')
def ensure_floats(data, result):
    for line in data:
        line = line.replace(",", ".")
        result.append(float(line[7:12]))


def plot(x, y, size, savepath):
    plt.plot(x, y)
    plt.xlim((1, size))
    plt.ylim(0)
    plt.xlabel("Requests")
    plt.ylabel("Time (in s)")
    plt.savefig(savepath)
    # plt.show()


def findavg(data):
    print('Average time taken to answer is: ', sum(data)/len(data))


def main():
    if len(sys.argv) < 3:
        print("usage: {} <request_number> <datafile> [<dest>]".format(sys.argv[0]))
        exit(1)

    f = open(sys.argv[2], "r")
    lines = f.readlines()
    f.close()
    size = int(sys.argv[1])
    x = [i for i in range(1,size+1)]
    y = []
    ensure_floats(lines, y)

    if len(sys.argv) > 3:
        savepath = sys.argv[3]
    else:
        savepath = os.path.splitext(sys.argv[2])[0]

    findavg(y)
    plot(x, y, size, savepath)


if __name__ == "__main__":
    sys.exit(main())
