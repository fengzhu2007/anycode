import subprocess


generators = {
    "win32": "Visual Studio 16 2019",
    "x64": "Visual Studio 16 2019",
}

configs = ["Release", "Debug"]


def cmake(*args):
    subprocess.run(["cmake.exe"] + list(args), check=True)

#cmake -DOPENSSL_ROOT_DIR=/path/to/openssl -DLIBSSH2_ROOT_DIR=/path/to/libssh2 -DZLIB_ROOT=/path/to/zlib /path/to/libcurl/source

# LIBPSL_LIBRARY LIBPSL_INCLUDE_DIR

def configure(source_dir, build_dir, bin_dir, arch):
    print("Configure:"+source_dir)
    cmake(
        "-G%s" % generators[arch],
        "-A %s"% arch,
        "-B%s" % build_dir,
        "-DCMAKE_INSTALL_PREFIX=%s" % bin_dir,
        "-DCURL_USE_OPENSSL=ON -DOPENSSL_LIBRARIES=./bin/%s/lib -DOPENSSL_INCLUDE_DIR=./bin/%s/include -DLIBPSL_LIBRARY=./bin/%s/lib -DLIBPSL_INCLUDE_DIR=./bin/%s/include -DUSE_ZLIB=ON -DZLIB_LIBRARIES=./bin/%s/lib -DZLIB_INCLUDE_DIRS=./bin/%s/include" % (arch,arch,arch,arch,arch,arch),
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
    
def build_library(source_dir, arch):
    print("Build %s:" % source_dir)
    parts = source_dir.split("/")
    output = source_dir
    length = len(parts)
    if length >= 1:
        output = parts[length-1]
        
    build_dir = "build/%s/%s" % (output, arch)
    bin_dir = "bin/%s" % arch
    configure(source_dir=source_dir, bin_dir=bin_dir, build_dir=build_dir, arch=arch,)
    build(build_dir, "Release")
    install(build_dir, "Release")
    
    
if __name__ == "__main__":
    for platform in ["x64"]:
        for library in ["curl"]:
            build_library(library, platform)