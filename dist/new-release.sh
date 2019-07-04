#!/usr/bin/env bash
set -e

function cleanup {
  rm -f $release_notes
  rm -rf $tempbuild
  rm -f $merged
}

function confirm {
  read -p "$@ [y/N] " choice
  case "$choice" in
    y|Y ) ;;
    * ) echo "Aborting. "; exit 1
  esac
}

function get_version_number {
  read -p "Give me the new version number of your release - format x.y.z: " version_number

  major=$(echo $version_number | cut -d "." -f 1)
  minor=$(echo $version_number | cut -d "." -f 2)
  patch=$(echo $version_number | cut -d "." -f 3)

  confirm "Releasing davix $major.$minor.$patch - is this OK?"
}

function get_author {
  author_name=$(git config user.name)
  author_email="$(echo $(git config user.email) | sed 's/@/ at /g')"

  confirm "Author's name and email: $author_name <$author_email> - is this OK?"
}

function edit_rpm_spec {
  echo "Patching specfile.."
  line1="* $(LC_ALL=POSIX date '+%a %b %d %Y') $author_name <$author_email> - $major.$minor.$patch-1"
  line2=" - davix $major.$minor.$patch release, see RELEASE-NOTES.md for changes"
  sed -i "s/%changelog/%changelog\n$line1\n$line2\n/g" packaging/davix.spec.in

  # now run cmake, necessary to re-generate davix.spec from davix.spec.in
  tempbuild=$(mktemp -d)
  src=$PWD

  pushd $tempbuild
  cmake $src
  popd
}

function edit_deb_changelog {
  echo "Patching debian package changelog.."
  line1="davix ($major.$minor.$patch-1) unstable; urgency=low"
  line2="\n  * Update to version $major.$minor.$patch"
  line3="\n -- $author_name <$author_email>  $(date -R)\n"
  sed -i "1i $line1\n$line2\n$line3" packaging/debian/changelog
}

function update_release_cmakefile {
  ./genversion.py --template version.cmake.in --out release.cmake --custom-version "R_${major}_${minor}_${patch}"
}

function git_commit {
  echo "Creating commit.."
  git add .
  git add --force release.cmake
  git commit -e -m "RELEASE: $major.$minor.$patch"
}

function git_tag {
  echo "Creating tag.."
  git tag -a R_${major}_${minor}_${patch} -m "Tag for version $major.$minor.$patch"
}

function ensure_git_root {
  gitroot=$(git rev-parse --show-toplevel)
  if [[ $PWD != $gitroot ]]; then
    echo "Error: This script can only be run when you're in the root git directory: $gitroot"
    echo "Please go there and try again."
    exit 1
  fi
}

function ensure_git_clean {
  if [[ -n $(git status --porcelain) ]]; then
    echo "WARNING: git directory not clean! ALL uncommited changes (including untracked files) will be commited if you continue."
    confirm "Are you sure you want to continue? (NOT RECOMMENDED) "
  fi
}

function patch_release_notes {
  if ! grep "## Unreleased" RELEASE-NOTES.md > /dev/null; then
    echo "RELEASE-NOTES.md does not contain an ## Unreleased entry! Add it, populate it with the changes contained in this release, and re-run this script."
    exit 1
  fi

  TODAY_DATE=$(date +%Y-%m-%d)
  sed -i "s/## Unreleased/## $major.$minor.$patch ($TODAY_DATE)/" RELEASE-NOTES.md
}

ensure_git_root
ensure_git_clean

trap cleanup EXIT
get_version_number
patch_release_notes
get_author
edit_rpm_spec
edit_deb_changelog

update_release_cmakefile
git_commit
git_tag

printf "All done! You still need to take the following actions in this order:\n\n"
printf "1. Push the newly created commit, as well as the tag: git push --follow-tags\n"
printf "2. Mark this version on jira as released, create new one\n"
printf "3. Push the package to EPEL\n"
printf "4. After the package is available on EPEL stable, create the binary tarballs, copy to AFS\n"
printf "5. Create a release announcement on http://dmc.web.cern.ch/\n"
