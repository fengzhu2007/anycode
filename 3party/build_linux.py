import cmake_linux
import subprocess
from glob import glob
import re



import subprocess


configs = ["Release", "Debug"]


def cmake_curl(*args):
    subprocess.run(["cmake"] + list(args), check=True)



def configure_curl(source_dir, build_dir, bin_dir, arch):
    print("Configure:"+source_dir)
    cmake_curl(
        "-G%s" % "Unix Makefiles",
        "-B%s" % build_dir,
        "-DCMAKE_INSTALL_PREFIX=%s" % bin_dir,
        "-DCURL_USE_OPENSSL=ON -DOPENSSL_LIBRARIES=./bin/%s/lib -DOPENSSL_INCLUDE_DIR=./bin/%s/include -DLIBPSL_LIBRARY=./bin/%s/lib -DLIBPSL_INCLUDE_DIR=./bin/%s/include -DUSE_ZLIB=ON -DZLIB_LIBRARIES=./bin/%s/lib -DZLIB_INCLUDE_DIRS=./bin/%s/include" % (arch,arch,arch,arch,arch,arch),
        "-Wno-dev",
        source_dir,
    )


def build_curl(build_dir, config):
    assert config in configs
    print("Build '%s':" % config)
    cmake_curl("--build", build_dir, "--config", config)


def install_curl(build_dir, config):
    assert config in configs
    print("Install '%s':" % config)
    cmake_curl("--build", build_dir, "--config", config, "--target", "install")

def build_curl_library(source_dir, arch):
    print("Build %s:" % source_dir)
    parts = source_dir.split("/")
    output = source_dir
    length = len(parts)
    if length >= 1:
        output = parts[length-1]

    build_dir = "build/%s/%s" % (output, arch)
    bin_dir = "bin/%s" % arch
    configure_curl(source_dir=source_dir, bin_dir=bin_dir, build_dir=build_dir, arch=arch,)
    build_curl(build_dir, "Release")
    install_curl(build_dir, "Release")




def build_library(source_dir, arch):
    print("Build %s:" % source_dir)
    parts = source_dir.split("/")
    output = source_dir
    length = len(parts)
    if length >= 1:
        output = parts[length-1]
        
    build_dir = "build/%s/%s" % (output, arch)
    bin_dir = "bin/%s" % arch
    cmake_linux.configure(source_dir=source_dir, bin_dir=bin_dir, build_dir=build_dir, arch=arch)
    cmake_linux.build(build_dir, "Release")
    cmake_linux.install(build_dir, "Release")

def detect_library(prefix):
    dirs = glob(prefix)
    for dir in dirs:
            return dir

    assert False, "Library with prefix '%s' not found!" % prefix

def build_apr_util(source_dir, arch):
    print("Build %s:" % source_dir)
    build_dir = "build/%s/%s" % (source_dir, arch)
    bin_dir = "bin/%s" % arch
    cmake_linux.configure(source_dir=source_dir, bin_dir=bin_dir, build_dir=build_dir, arch=arch,deps="-DAPR_INCLUDE_DIR=../apr/include -DAPR_LIBRARIES=../build/apr/%/Release")
    cmake_linux.build(build_dir, "Release")


if __name__ == "__main__":
    #for platform in ["win32","x64"]:
    for platform in ["x64"]:
        for library_prefix in ["libgit2"]:
            library = detect_library(library_prefix)
            build_library(library, platform)
