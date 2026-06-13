import sys
sys.path.insert(0, "./frostbyte/dependencies/scripts")

from utils import *

if __name__ == "__main__":
    change_directory("rlImGui")

    replace_in_file("premake5.lua", "\"IMGUI_DISABLE_OBSOLETE_FUNCTIONS\",", "")

    if os_windows():
        run_command(["premake5.exe", "gmake"])
    else:
        run_command(["chmod", "+x", "./premake5"])
        run_command(["./premake5", "gmake"])

    run_command(["make", "config=release_x64"])
