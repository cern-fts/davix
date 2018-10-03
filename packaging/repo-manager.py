#!/usr/bin/env python
# Author: Georgios Bitzes <georgios.bitzes@cern.ch>

import os, subprocess, sys, inspect, argparse, re, shutil, errno

DRY_RUN = False
NO_CREATE_REPO = False

def sh(cmd):
    # poor man's subprocess.check_output, not supported on SL6
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, stderr=subprocess.STDOUT)
    output, unused_err = process.communicate()
    retcode = process.poll()

    if retcode:
        raise Exception("Command {0} exited with code {1}. Output: {2}".format(cmd, retcode, output))

    return output

def ensure_valid_choice(parser, choice, text, available):
    if choice:
        if type(choice) == str: choice = [choice]

        for item in choice:
            if item not in available:
                parser.error("unrecognized {0}: '{1}'. Available choices: {2}".format(text, item, available))

def add_dependency(parser, when_present, dependency):
    if hasattr(parser, when_present):
        if not hasattr(parser, dependency):
            parser.error("argument --{0} is required when --{1} is present".format(dependency, when_present))

def declare_required(parser, args, choice):
    if not hasattr(args, choice):
        parser.error("argument --{0} is required".format(choice))

def bailout(msg):
    raise ValueError(msg)

# poor man's enum
class PackageType:
    Binary, NoArch, Source = range(1, 4)

class Package(object):
    def __init__(self, path):
        self.path = path
        if not os.path.isfile(self.path):
            bailout("Not a file: {0}".format(self.path))

        self.filename = os.path.basename(self.path)

        if self.filename.endswith(".src.rpm"):
            self.type = PackageType.Source
            tmp = self.filename[0:-8]
        elif self.filename.endswith(".noarch.rpm"):
            self.type = PackageType.NoArch
            tmp = self.filename[0:-11]
        elif self.filename.endswith(".rpm"):
            self.type = PackageType.Binary
            tmp = self.filename[0:-4]
        else:
            bailout("Unable to parse RPM type for {0}".format(self.path))

        if self.type == PackageType.Source or self.type == PackageType.NoArch:
            self.arch = None
        elif tmp.endswith(".x86_64"):
            tmp = tmp[0:-7]
            self.arch = "x86_64"
        elif tmp.endswith(".i386"):
            tmp = tmp[0:-5]
            self.arch = "i386"
        else:
            bailout("Unable to determine architecture for {0}".format(self.path))

        self.platform = tmp.split(".")[-1]
        tmp = tmp[0:-len(self.platform)-1]

        if self.platform == "cern":
            self.platform = tmp.split(".")[-1]
            tmp = tmp[0:-len(self.platform)-1]

        self.packagename = tmp

def construct_location(platform, arch, filename):
    return "{0}/{1}/{2}".format(platform, arch, filename)

def is_tag(ref):
    return (re.compile("""^(v?)(\d+)\.(\d+)\.(\d+)$""").match(ref) != None or
           re.compile("""^(v?)(\d+)\.(\d+)$""").match(ref) != None)

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def createrepo(repo):
    print("-- Running createrepo on {0}".format(repo))
    if NO_CREATE_REPO: return
    sh("rm -rf {0}".format(repo + "/.olddata"))
    sh("createrepo -q {0}".format(repo))

def copy_to_repo(source, repo):
    print("-- Copying {0} to {1}".format(source, repo))
    if DRY_RUN: return

    mkdir_p(repo)
    shutil.copyfile(source, "{0}/{1}".format(repo, os.path.basename(source)))

class Repository(object):
    def __init__(self, base):
        self.base = base
        if not os.path.isdir(self.base):
            bailout("Not a directory: {0}".format(self.base))

    def store(self, ref, packages):
        platforms = set([x.platform for x in packages])
        if len(platforms) != 1:
            raise ValueError("Cannot mix packages of different platforms in the same invocation: {0}".format(list(platforms)))

        archs = set([x.arch for x in packages])
        archs.remove(None)
        if len(archs) != 1:
            raise ValueError("Cannot mix packages of different architectures in the same invocation: {0}".format(list(archs)))

        tag = is_tag(ref)

        base = "{0}/{1}".format(self.base, ref)
        if tag: base = "{0}/tag".format(self.base)
        base += "/" + list(platforms)[0]

        reposToCreate = set()
        for package in packages:
            repo = "{0}/{1}".format(base, list(archs)[0])
            if package.type == PackageType.Source:
                repo = "{0}/SRPMS".format(base)

            copy_to_repo(package.path, repo)
            reposToCreate.add(repo)

        for repo in reposToCreate:
            createrepo(repo)

def declare_incompatible_options(parser, option, group):
    if option not in sys.argv: return

    for item in group:
        if item in sys.argv:
            parser.error("argument {0} is incompatible with argument {1}".format(option, item))

def parseargs():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     description="An opinionated yum repository manager.\n")

    parser.add_argument('--base', type=str, required=True, help="The base directory for your project.")
    parser.add_argument('--action', type=str, required=True, help="The action to perform. Choices: ['add', 'cleanup']")
    parser.add_argument('--dry-run', action="store_true", help="If set, don't actually change any files, just show what would happen if ran.")
    parser.add_argument('--no-create-repo', action="store_true", help="If set, don't run createrepo at the end.")
    parser.set_defaults(dry_run=False, no_create_repo=False)

    group = parser.add_argument_group('add options')
    group.add_argument('--ref', type=str, help="The branch or tag that is being built. Tag names must match 'x.y' or 'x.y.z' (may be prepended by 'v')")
    group.add_argument('--packages', type=str, nargs='+', help="The list of packages to add")

    group = parser.add_argument_group('cleanup options')
    group.add_argument('--keep-last-days', type=int, help="How many days worth of RPMs to keep. (only affects branches)")

    args = parser.parse_args()

    ensure_valid_choice(parser, args.action, "action", ["add", "cleanup"])
    declare_incompatible_options(parser, "--no-create-repo", ["--dry-run"])

    if args.action == "add":
        declare_required(parser, args, "ref")
        declare_required(parser, args, "packages")

    if args.action == "cleanup":
        declare_required(parser, args, "keep-last-days")
        bailout("NYI")

    if args.ref == "tags" or args.ref == "tag":
        bailout("A branch named '{0}'? Really?".format(args.ref))

    global DRY_RUN
    global NO_CREATE_REPO
    if args.dry_run:
        DRY_RUN = True
        NO_CREATE_REPO = True

    if args.no_create_repo:
        NO_CREATE_REPO = True

    return args

def main():
    args = parseargs()

    repository = Repository(args.base)
    packages = [Package(x) for x in args.packages]

    repository.store(args.ref, packages)

if __name__ == '__main__':
    main()
