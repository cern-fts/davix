#!/usr/bin/env python2

################################################################################
## Script to generate version numbers from git tags.                          ##
## Author: Georgios Bitzes - CERN                                             ##
## https://gitlab.cern.ch/gbitzes/build-tools                                 ##
##                                                                            ##
## Copyright (C) 2017 CERN/Switzerland                                        ##
## This program is free software: you can redistribute it and/or modify       ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation, either version 3 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful,            ##
## but WITHOUT ANY WARRANTY; without even the implied warranty of             ##
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              ##
## GNU General Public License for more details.                               ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program.  If not, see <http://www.gnu.org/licenses/>.      ##
################################################################################

import os, subprocess, sys, argparse

def removePrefix(s, prefix):
    if s.startswith(prefix):
        return s[len(prefix):]
    return s

def makeVersionTriplet(fragments):
    if len(fragments) == 2:
        return (int(fragments[0]), int(fragments[1]), 0)
    if len(fragments) == 3:
        return (int(fragments[0]), int(fragments[1]), int(fragments[2]))

    raise Exception("Invalid length of version fragments: {0}".format(fragments))

class SoftwareVersion:
    def __init__(self, major, minor, patch, miniPatch=None):
        self.major = major
        self.minor = minor
        self.patch = patch
        self.miniPatch = miniPatch

        if self.patch == None: assert self.miniPatch == None

    def toString(self):
        ret = "{0}.{1}".format(self.major, self.minor)

        if self.patch is not None:
            ret += ".{0}".format(self.patch)

        if self.miniPatch is not None:
            ret += ".{0}".format(self.miniPatch)

        return ret

class GitDescribe:
    def __init__(self, description):
        self.description = description
        self.parse()

    def parse(self):
        parts = self.description

        # Is the working dir dirty?
        self.dirty = self.description.endswith("-dirty")
        if self.dirty:
            parts = parts[0:len(parts)-len("-dirty")]

        # Trim any preceeding "v" or "R" in the version, if any
        parts = removePrefix(parts, "v")
        parts = removePrefix(parts, "R_")

        # Is there a git hash?
        self.commitHash = None

        potentialHash = parts.split("-")
        if potentialHash and potentialHash[-1].startswith("g"):
            self.commitHash = potentialHash[-1][1:]
            parts = parts[0:(len(parts) - len(self.commitHash) - 2 )]

        # Is there a number of commits since tag? Can only exist if hash has been
        # found already.
        self.numberOfCommits = None
        if self.commitHash:
            tmp = parts.split("-")
            self.numberOfCommits = int(tmp[-1])
            parts = parts[0:(len(parts) - len(tmp[-1]) - 1)]

        # Are we using "_", ".", or "-" as delimiter?
        self.versionFragments = None
        for delim in ["_", ".", "-"]:
            if delim in parts:
                self.versionFragments = parts.split(delim)
                break

        if not self.versionFragments:
            raise Exception("Unable to parse vresion fragments of {0}".format(self.description))

        if len(self.versionFragments) != 2 and len(self.versionFragments) != 3:
            raise Exception("Unable to understand version fragments ({0}) of {1}".format(self.versionFragments, self.description))

        self.versionTriplet = makeVersionTriplet(self.versionFragments)
        self.buildMiniPatch()
        self.buildVersion()

    def buildVersion(self):
        self.version = SoftwareVersion(
          self.versionTriplet[0],
          self.versionTriplet[1],
          self.versionTriplet[2],
          self.miniPatch
        )

    def buildMiniPatch(self):
        self.miniPatch = None

        if self.commitHash:
            self.miniPatch = "{0}.{1}".format(self.numberOfCommits, self.commitHash)

        if self.isDirty():
            if not self.miniPatch:
                self.miniPatch = "dirty"
            else:
                self.miniPatch += ".dirty"

    def toString(self):
        return self.description

    def isDirty(self):
        return self.dirty

    def getVersion(self):
        return self.version

    def getCommitHash(self):
        return self.commitHash

    def getNumberOfCommits(self):
        return self.numberOfCommits

    def getVersionTriplet(self):
        return self.versionTriplet

    def getMiniPatch(self):
        return self.miniPatch

def applyTemplate(templateContent, replacements):
    newContent = templateContent
    for replacement in replacements:
        if replacement[1] is not None:
            replacement[1] = str(replacement[1])
            newContent = newContent.replace(replacement[0], replacement[1])

    return newContent

def sh(cmd):
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True, stderr=subprocess.STDOUT)
    output, unused_err = process.communicate()
    retcode = process.poll()

    if retcode:
        raise Exception("Command {0} exited with code {1}".format(cmd, retcode))

    return output.decode("utf-8")

def getFile(filename):
    try:
        with open(filename) as f:
            content = "".join(f.readlines())
    except:
        return ""

    return content

def replaceFile(output, outfile, fullversion):
    oldContent = getFile(outfile)

    if oldContent == output:
        print("{0} up-to-date.".format(outfile))
    else:
        with open(outfile, "w") as f:
            f.write(output)

        print("{0} updated. ({1})".format(outfile, fullversion))

def giveOutput(output, outfile, fullversion):
    if outfile:
        replaceFile(output, outfile, fullversion)
    else:
        print(output)

def declare_incompatible_options(parser, option, group):
    if option not in sys.argv: return

    for item in group:
        if item in sys.argv:
            parser.error("argument {0} is incompatible with argument {1}".format(option, item))

def ensureRunningInGitRepo(args):
    try:
        root_dir = sh("git rev-parse --show-toplevel").strip()
        os.chdir(root_dir)
    except:
        # not a failure - simply means we're building from a release tarball
        print("Cannot regenerate {0} from git".format(args.out))
        sys.exit(0)

    # Ensure the release tarball isn't being built from inside a different git repository
    path_to_genversion = os.path.abspath(os.path.dirname(__file__))
    if path_to_genversion != root_dir:
        print("Cannot regenerate {0} from git".format(args.out))
        sys.exit(0)

def main():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     description="Configure files that contain version numbers.\n")
    parser.add_argument('--template', type=str, help="The template input file.")
    parser.add_argument('--out', type=str, help="The file to output - if not specified, stdout will be used instead.")
    parser.add_argument('--template-string', type=str, help="The template string.")
    parser.add_argument('--custom-version', type=str, help="Don't run git - extract version from the given string")
    args = parser.parse_args()

    if (not args.template and not args.template_string):
        parser.error("no input specified; use either --template or --template-string")

    declare_incompatible_options(parser, "--template-string", ["--template"])

    if not args.custom_version:
      ensureRunningInGitRepo(args)
      gitDescribe = GitDescribe(sh("git describe --dirty").strip())
    else:
      gitDescribe = GitDescribe(args.custom_version)

    softwareVersion = gitDescribe.getVersion()

    replacements = [
      ["@GIT_DESCRIBE@", gitDescribe.toString()],
      ["@VERSION_MAJOR@", softwareVersion.major],
      ["@VERSION_MINOR@", softwareVersion.minor],
      ["@VERSION_PATCH@", softwareVersion.patch],
      ["@VERSION_MINIPATCH@", softwareVersion.miniPatch],
      ["@VERSION_FULL@", softwareVersion.toString()]
    ]

    inputContents = args.template_string
    if not inputContents:
        inputContents = getFile(args.template)

    output = applyTemplate(inputContents, replacements)

    giveOutput(output, args.out, softwareVersion.toString())


if __name__ == '__main__':
    main()
