import subprocess

generators = {
    "win32": "Visual Studio 16 2019",
    "x64": "Visual Studio 16 2019",
}

configs = ["Release", "Debug"]


def cmake(*args):
    subprocess.run(["cmake.exe"] + list(args), check=True)


def configure(source_dir, build_dir, bin_dir, arch):
    print("Configure:"+source_dir)
    cmake(
        "-G%s" % generators[arch],
        "-A %s"% arch,
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
