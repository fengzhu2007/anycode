import cmake
from glob import glob
import re


def build_library(source_dir, arch):
    print("Build %s:" % source_dir)
    parts = source_dir.split("/")
    output = source_dir
    length = len(parts)
    if length >= 1:
        output = parts[length-1]
        
    build_dir = "build/%s/%s" % (output, arch)
    bin_dir = "bin/%s" % arch
    cmake.configure(source_dir=source_dir, bin_dir=bin_dir, build_dir=build_dir, arch=arch,)
    cmake.build(build_dir, "Release")
    cmake.install(build_dir, "Release")

def detect_library(prefix):
    dirs = glob(prefix)
    for dir in dirs:
            return dir

    assert False, "Library with prefix '%s' not found!" % prefix

def build_apr_util(source_dir, arch):
    print("Build %s:" % source_dir)
    build_dir = "build/%s/%s" % (source_dir, arch)
    bin_dir = "bin/%s" % arch
    cmake.configure(source_dir=source_dir, bin_dir=bin_dir, build_dir=build_dir, arch=arch,deps="-DAPR_INCLUDE_DIR=../apr/include -DAPR_LIBRARIES=../build/apr/%/Release")
    cmake.build(build_dir, "Release")


if __name__ == "__main__":
    for platform in ["win32","x64"]:
        for library_prefix in ["zlib"]:
            library = detect_library(library_prefix)
            build_library(library, platform)
            
        #build_apr_util("apr-util",platform)
