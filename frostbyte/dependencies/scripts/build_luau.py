from utils import *
from pathlib import Path

if __name__ == "__main__":
    change_directory("luau")

    ensure_directory("cmake")
    change_directory("cmake")

    run_command(["cmake", "..", "-DCMAKE_BUILD_TYPE=Release"])

    run_command(["cmake", "--build", ".", "--target", "Luau.Analysis", "--config", "Release"])
    run_command(["cmake", "--build", ".", "--target", "Luau.Ast", "--config", "Release"])
    run_command(["cmake", "--build", ".", "--target", "Luau.Compiler", "--config", "Release"])
    run_command(["cmake", "--build", ".", "--target", "Luau.Config", "--config", "Release"])
    run_command(["cmake", "--build", ".", "--target", "Luau.VM", "--config", "Release"])

    if os_windows():
        fail_exit("Windows build is currently not supported for Luau")

    run_command(["ar", "-x", "./libLuau.Analysis.a"])
    run_command(["ar", "-x", "./libLuau.Ast.a"])
    run_command(["ar", "-x", "./libLuau.Compiler.a"])
    run_command(["ar", "-x", "./libLuau.Config.a"])
    run_command(["ar", "-x", "./libLuau.VM.a"])
    run_command(["ar", "rcs", "libLuau.a"] + [str(p) for p in Path(".").glob("*.o")])
