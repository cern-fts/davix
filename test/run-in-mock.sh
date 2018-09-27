#!/usr/bin/env bash
# Must be run from the top-level directory of the repository
# ie ~/dev/davix, NOT ~/dev/davix/test

# Defaults
MOCK_CONFIG_DIR="/etc/mock"
MOCK_ROOT="epel-6-x86_64"
SKIP_INIT=0
OUTPUT_DIR="$PWD/test-results"

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
mock_cmd install cmake git yum-utils abi-compliance-checker doxygen gsoap-devel libxml2-devel openssl-devel voms-clients libuuid-devel
MOCK_HOME="/builddir"

# copy grid-security and vomses
mock_cmd chroot "rm -rf /etc/grid-security /etc/vomses"
mock_cmd --copyin "/etc/grid-security" "/etc/grid-security"
mock_cmd --copyin "/etc/vomses" "/etc/vomses"

# Copy davix codebase to chroot
mock_cmd chroot "rm -rf $MOCK_HOME/davix"
mock_cmd --copyin "$PWD" "$MOCK_HOME/davix"

# run tests
mock_cmd chroot "cd $MOCK_HOME/davix && test/run-tests.sh"

# Recover logs
mkdir -p "${OUTPUT_DIR}"
rm -rf "${OUTPUT_DIR}/Testing"
mock_cmd --copyout "$MOCK_HOME/davix/build/Testing" "${OUTPUT_DIR}/Testing"
