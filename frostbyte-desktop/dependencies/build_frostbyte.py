import sys
sys.path.insert(0, "./frostbyte/dependencies/scripts")

from utils import *

from threading import Thread

if __name__ == "__main__":
    change_directory("frostbyte/dependencies")

    # threads = []

    # threads.append(Thread(target=run_command, args=(["python3", "scripts/build_curl.py"],)))
    # threads.append(Thread(target=run_command, args=(["python3", "scripts/build_luau.py"],)))

    # for t in threads:
    #     t.start()

    # for t in threads:
    #     t.join()

    # TODO: figure out why the threads don't work (basically the program exits before luau is finished compiling)
    run_command(["python3", "scripts/build_curl.py"])
    run_command(["python3", "scripts/build_luau.py"])

    change_directory("..")

    run_command(["gcc", "-o", "mate", "mate.c"])
    run_command(["./mate"])
