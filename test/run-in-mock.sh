#!/bin/bash
# Must be run from the top-level directory of the repository
# ie ~/dev/davix, NOT ~/dev/davix/test

# Defaults
MOCK_CONFIG_DIR="/etc/mock"
MOCK_ROOT="epel-6-x86_64"
SKIP_INIT=0
OUTPUT_DIR="$PWD/test-results"
CORES=$(grep -c ^processor /proc/cpuinfo)

# Get parameters
while [ -n "$1" ]; do
    case "$1" in 
        "--configdir")
            shift
            MOCK_CONFIG_DIR=$1
        ;;
        "--root")
            shift
            MOCK_ROOT=$1
        ;;
        "--skip-init")
            SKIP_INIT=1
        ;;
        "--output")
            shift
            OUTPUT_DIR=$1
        ;;
        *)
            echo "Unknown parameter $1"
            exit 1
        ;;
    esac
    shift
done

# Feedback
echo "# Mock config dir:  ${MOCK_CONFIG_DIR}"
echo "# Mock root:        ${MOCK_ROOT}"

# Run inside mock
function mock_cmd()
{
    /usr/bin/mock -v --configdir "${MOCK_CONFIG_DIR}" -r "${MOCK_ROOT}" --disable-plugin=tmpfs "$@"
}

# Prepare environment
if [ $SKIP_INIT == 0 ]; then
    mock_cmd clean
    mock_cmd init
else
    echo "Skipping initialization"
fi

# Install deps
mock_cmd install cmake git yum-utils abi-compliance-checker boost-devel doxygen gsoap-devel gtest-devel libxml2-devel openssl-devel
MOCK_HOME="/builddir"

# Copy davix codebase to chroot
mock_cmd --copyin "$PWD" "$MOCK_HOME/davix"

# Compile
mock_cmd chroot "cd $MOCK_HOME/davix &&
                 rm -rf build &&
                 mkdir build &&
                 cd build &&
                 cmake .. -DFUNCTIONAL_TESTS=TRUE &&
                 make -j $CORES &&
                 (ctest --no-compress-output -T Test || true)"

# Recover logs
mkdir -p "${OUTPUT_DIR}"
rm -rf "${OUTPUT_DIR}/Testing"
mock_cmd --copyout "$MOCK_HOME/davix/build/Testing" "${OUTPUT_DIR}/Testing"
