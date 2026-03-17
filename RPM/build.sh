#!/bin/bash
# build.sh - Build open5g RPMs.
#
# Usage:
#   ./RPM/build.sh            # build RPMs
#   ./RPM/build.sh --no-check # skip %check (tests)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SPEC_FILE="${SCRIPT_DIR}/open5gs.spec"

# --- Parse arguments ---
EXTRA_RPMBUILD_ARGS=()
for arg in "$@"; do
    case "${arg}" in
        --no-check)  EXTRA_RPMBUILD_ARGS+=(--nocheck) ;;
        *) echo "Unknown argument: ${arg}"; exit 1 ;;
    esac
done

# --- Read spec metadata ---
PKGNAME=$(awk '/^Name:/{print $2}' "${SPEC_FILE}")
VERSION=$(awk '/^Version:/{print $2}' "${SPEC_FILE}")

# --- Set up rpmbuild tree ---
RPMBUILD_DIR="${HOME}/rpmbuild"
mkdir -p "${RPMBUILD_DIR}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# --- Create source tarball via git archive ---
# The prefix must match %{name}-%{version} so %autosetup finds the right dir.
TARBALL_NAME="open5gs-v${VERSION}.tar.gz"
TARBALL_PATH="${RPMBUILD_DIR}/SOURCES/${TARBALL_NAME}"

echo "==> Creating source tarball from git HEAD (committed files only)..."
echo "    Version: ${VERSION}"
git -C "${REPO_ROOT}" archive \
    --format=tar.gz \
    --prefix="${PKGNAME}-${VERSION}/" \
    HEAD \
    > "${TARBALL_PATH}"
echo "    Tarball: ${TARBALL_PATH}"

# --- Copy spec ---
cp "${SPEC_FILE}" "${RPMBUILD_DIR}/SPECS/"

# --- Run rpmbuild ---
echo ""
echo "==> Running rpmbuild..."
rpmbuild -ba \
    "${EXTRA_RPMBUILD_ARGS[@]+"${EXTRA_RPMBUILD_ARGS[@]}"}" \
    "${RPMBUILD_DIR}/SPECS/open5gs.spec"

echo ""
echo "==> Done. RPMs are in: ${RPMBUILD_DIR}/RPMS/"
find "${RPMBUILD_DIR}/RPMS" -name "${PKGNAME}*.rpm" -newer "${TARBALL_PATH}" | sort
