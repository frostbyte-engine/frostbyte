import sys
sys.path.insert(0, "../..")

from buildutils import *

if __name__ == "__main__":
    change_directory("curl")

    ensure_directory("cmake")
    change_directory("cmake")

    run_command(["cmake", ".."])
    run_command(["cmake", "--build", ".", "--config", "Release"])
