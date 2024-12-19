import subprocess
import sys

generators = {
    "x64": "Unix Makefiles",
}

configs = ["Release", "Debug"]


def cmake(*args):
    subprocess.run(["cmake"] + list(args), check=True)


def configure(source_dir, build_dir, bin_dir, arch):
    print("Configure:"+source_dir)
    generator = generators[arch]


    cmake(
        "-G%s" % generator,
        "-B%s" % build_dir,
        "-DCMAKE_INSTALL_PREFIX=%s" % bin_dir,
        "-Wno-dev",
        source_dir,
    )


def build(build_dir, config):
    assert config in configs
    print("Build '%s':" % config)
    cmake("--build", build_dir, "--config", config)


def install(build_dir, config):
    assert config in configs
    print("Install '%s':" % config)
    cmake("--build", build_dir, "--config", config, "--target", "install")
