#!/usr/bin/env bash
set -e

function cleanup {
  rm -rf $tempbuild
}

function confirm {
  read -p "$@ [y/N] " choice
  case "$choice" in
    y|Y ) ;;
    * ) echo "Aborting. "; exit 1
  esac
}

function get_version_number {
  read -p "Give me the new version number of your release - format x.y.z[-r]: " version_number

  release=$(echo $version_number | cut -d "-" -f2 -s)
  version=$(echo $version_number | cut -d "-" -f1 | cut -d "." -f 1,2,3)

  if [[ -z $release ]]; then
    release=1
  fi

  # "version_with_release" always contains the release
  # "version_canonical" contains the release number only if release != 1
  version_with_release="$version-$release"
  version_canonical=$version

  if [[ $release -ne 1 ]]; then
    release_regex="^(rc)?[1-9][0-9]*$"
    if [[ $release =~ $release_regex ]]; then
      version_canonical="$version-$release"
    else
      echo "Release '${release}' must follow format: [rc](number > 0)"
      exit 1
    fi
  fi

  confirm "Releasing davix $version_canonical - is this OK?"
}

function get_author {
  author_name=$(git config user.name)
  author_email="$(echo $(git config user.email) | sed 's/@/ at /g')"

  confirm "Author's name and email: $author_name <$author_email> - is this OK?"
}

function edit_rpm_spec {
  echo "Patching specfile.."
  line1="* $(LC_ALL=POSIX date '+%a %b %d %Y') $author_name <$author_email> - $version_with_release"
  line2=" - davix $version_canonical release, see RELEASE-NOTES.md for changes"
  sed -i "s/%changelog/%changelog\n$line1\n$line2\n/g" packaging/davix.spec.in

  # now run cmake, necessary to re-generate davix.spec from davix.spec.in
  tempbuild=$(mktemp -d)
  src=$PWD

  pushd $tempbuild
  cmake3 $src -Wno-dev
  popd
}

function edit_deb_changelog {
  echo "Patching debian package changelog.."
  line1="davix ($version_with_release) unstable; urgency=low"
  line2="\n  * Update to version $version_canonical"
  line3="\n -- $author_name <$author_email>  $(date -R)\n"
  sed -i "1i $line1\n$line2\n$line3" packaging/debian/changelog
}

function update_release_cmakefile {
  tag="R_${version_canonical//./_}"
  ./genversion.py --template version.cmake.in --out release.cmake --custom-version ${tag}
}

function git_commit {
  echo "Creating commit.."
  git add .
  git add --force release.cmake
  git commit -e -m "RELEASE: $version_canonical"
}

function git_tag {
  echo "Creating tag ${tag}.."
  git tag -a ${tag} -m "Tag for version $version_canonical"
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
    echo "WARNING: git directory not clean! ALL uncommitted changes (including untracked files) will be committed if you continue."
    confirm "Are you sure you want to continue? (NOT RECOMMENDED) "
  fi
}

function patch_release_notes {
  if ! grep -i "## Unreleased" RELEASE-NOTES.md > /dev/null; then
    echo "RELEASE-NOTES.md does not contain an ## Unreleased entry! Add it, populate it with the changes contained in this release, and re-run this script."
    exit 1
  fi

  TODAY_DATE=$(date +%Y-%m-%d)
  sed -i "s/## Unreleased/## $version_canonical ($TODAY_DATE)/I" RELEASE-NOTES.md
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
printf "1. Merge the 'devel' branch into 'master': git checkout master ; git merge --ff-only devel\n"
printf "2. Push the newly created commit and the tag: git push --atomic origin devel master ${tag}\n"
printf "3. Mark this version on JIRA as released\n"
printf "4. Create a release announcement on https://github.com/cern-fts/davix/releases\n"
printf "5. Publish correct source and binary tarball to the Github Release page\n"
printf "5a. Source and binary tarballs are built in the CI"
printf "6. When ready, push the package to DMC production and EPEL repositories\n"
printf "\nRelease Guide available on https://cern.ch/dmc-docs\n"
