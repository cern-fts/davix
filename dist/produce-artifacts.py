#!/usr/bin/env python3
import argparse
import subprocess
import os
import shutil
import sys
import re

args = None

def sh(command):
    subprocess.call(command, shell=True)

def parseargs():
    parser = argparse.ArgumentParser(description="Creates all necessary artifacts for a davix release.\n")
    parser.add_argument('--name', default='davix', help="The program name")
    parser.add_argument('--tag', required=True, help="The tag number for which to create a release. (example: R_0_6_3)")
    parser.add_argument('--tmpdir', default='/tmp/davix-tmp', help="Temporary location where to store files")
    parser.add_argument('--koji', default="https://kojipkgs.fedoraproject.org/packages", help="The koji build repository to use")
    parser.add_argument('--git', default="https://github.com/cern-fts/davix", help="The git repository to use (can even be local)")
    parser.add_argument('--build', type=int, default=1, help="The build number to use")
    parser.add_argument('--archs', type=str, nargs="+", default=["el6.x86_64", "el7.x86_64", "el8.x86_64"])
    parser.add_argument('--packages', type=str, nargs="+", default=["default", "devel", "libs", "doc.noarch"])
    args = parser.parse_args()

    if os.path.exists(args.tmpdir):
        parser.error("tmpdir '{}' already exists".format(args.tmpdir))

    args.release = args.tag
    m = re.match(r"R_(\d+)_(\d+)_(\d+)", args.tag)
    if m.groups():
        args.release = ".".join(m.groups())

    return args

def extract_rpms(packages, fullname):
    for url in packages:
        sh("wget {}".format(url))

    os.mkdir(fullname)
    os.chdir(fullname)

    for url in packages:
        filename = url.split("/")[-1]
        sh("rpm2cpio ../{} | cpio -idmv".format(filename))

        sh("""sed -i "s|prefix=/usr|prefix=/|" usr/lib64/pkgconfig/*.pc""")

    os.chdir("..")
    sh("tar -cvzf {0}.tar.gz {0} --transform 's|{0}/usr|{0}|'".format(fullname))
    shutil.copy("{0}.tar.gz".format(fullname), args.release)

def create_binary_releases():
    for arch in args.archs:
        sarch = arch.split(".")
        base = "{0}/{1}/{2}/{3}.{4}".format(args.koji, args.name, args.release, args.build, sarch[0])

        packages = []
        for pkg in args.packages:
            if pkg == "default": pkg = ""
            myarch = arch.split(".")[1]

            if pkg.endswith(".noarch"):
                pkg = pkg.split(".")[0]
                myarch = "noarch"

            if pkg != "": pkg = "-" + pkg

            packages.append("{0}/{1}/{2}{3}-{4}-{5}.{6}.{7}.rpm".format(base, myarch, args.name, pkg, args.release, args.build, sarch[0], myarch))

        extract_rpms(packages, "{name}-{version}-{arch}".format(name=args.name, version=args.release, arch=arch))

def create_source_releases():
    foldername = args.git.split("/")[-1]
    sh("git clone {}".format(args.git))
    os.chdir(foldername)
    sh("git checkout {}".format(args.tag))
    sh("git submodule update --init")
    sh("./packaging/make-dist.sh")
    os.chdir("..")
    shutil.copy("davix/build/{0}-{1}.tar.gz".format(args.name, args.release), args.release)

def main():
    global args
    args = parseargs()

    os.mkdir(args.tmpdir)
    os.chdir(args.tmpdir)
    os.mkdir(args.release)

    create_binary_releases()
    create_source_releases()

    print("\n\nDone! You can find all artifacts under {0}/{1}".format(args.tmpdir, args.release))

if __name__ == '__main__':
    main()
