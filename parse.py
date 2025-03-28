import struct
import argparse
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.dates as mdates


file_prefix = "pinatrace.out."



def draw_timeline_graph(file_num):
    names = []
    sdates = []
    edates = []
    levels = []
    for i in range(file_num):
        with open(file_prefix + f"{i}", "rb") as file:
            names.append(f"{i}")
            buf = file.read(8)
            if buf == b'':
                continue
            sdates.append(struct.unpack('l', buf)[0])
            file.seek(-18, 2)
            edates.append(struct.unpack('l', file.read(8))[0])
            levels.append(i+1)

    # Create figure and plot a stem plot with the date
    fig, ax = plt.subplots(figsize=(12, 12), layout="constrained")
    ax.set(title="thread start time and end time")

    ax.hlines(levels, sdates, edates, color="tab:red")  # The vertical stems.
    for d, l, r in zip(sdates, levels, names):
        ax.annotate(r, xy=(d, l),
                    xytext=(-3, np.sign(l)*3), textcoords="offset points",
                    horizontalalignment="right",
                    verticalalignment="bottom" if l > 0 else "top")


    # remove y-axis and spines
    ax.yaxis.set_visible(False)
    ax.spines[["left", "top", "right"]].set_visible(False)

    ax.margins(y=0.1)
    plt.savefig("dummy_name.png")

def get_start_end_time(file_num): 
    
    start_time = -1
    start_time_tmp = -1
    end_time = 0
    end_time_tmp = 0
    for i in range(file_num):
        
        with open(file_prefix + f"{i}", "rb") as file:
            buf = file.read(8)
            if buf == b'':
                continue
            start_time_tmp =  struct.unpack('L', buf)[0]
            if start_time > start_time_tmp or start_time == -1:
                start_time = start_time_tmp
            file.seek(-18, 2)
            end_time_tmp =  struct.unpack('L', file.read(8))[0]
            if end_time < end_time_tmp:
                end_time = end_time_tmp

    return start_time,end_time


def scale_trace(file_num, time, scale=1):
    for i in range(file_num):
        with open(file_prefix + f"{i}", "rb") as read_file:
            with open(file_prefix + f"{i}"+ ".scale", "wb+")as write_file:
                while (True):
                    time_buf = read_file.read(8)
                    rest_buf = read_file.read(10)
                    if time_buf == b'' or rest_buf == b'':
                        break
                    write_file.write(struct.pack('L',int((struct.unpack('L', time_buf)[0]-time) / scale )))
                    write_file.write(rest_buf)
            



def main() -> None:
    parser = argparse.ArgumentParser(description='parser and modifier of')
    parser.add_argument('-t', '--thread', type=int, 
                    help='thread num (file num)')
    parser.add_argument('-s', '--scale', type=int, 
                    help='the scale of real workload compare to pin trace workload')
    parser.add_argument('-d', '--draw', type=bool, 
                    help='True or False draw a graph or not')
    

    args = parser.parse_args()
    if (not args.draw or not args.thread )and ( not args.thread or not args.scale):
        print("-h for help")
        exit(0)
    

    if args.draw == True:
        draw_timeline_graph(args.thread)

    else:
        start,end = get_start_end_time(args.thread)
        print(f"total interval time is {end - start}")
        scale_trace(args.thread, start, args.scale)

if __name__ == "__main__":
    main()