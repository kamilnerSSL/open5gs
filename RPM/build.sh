#!/bin/bash
# build.sh - Build open5g RPMs with automatic branch-based versioning.
#
# Feature-branch builds get a pre-release Release field (0.1.branchname.ringer)
# that sorts LOWER than the final merged build (1.ringer), so installing the
# merged package always upgrades cleanly over any feature-branch build.
#
# Usage:
#   ./RPM/build.sh            # auto-detect branch
#   ./RPM/build.sh --release  # force a release build (no branch prefix)
#   ./RPM/build.sh --no-check # skip %check (tests)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
SPEC_FILE="${SCRIPT_DIR}/open5gs.spec"

# --- Parse arguments ---
FORCE_RELEASE=false
EXTRA_RPMBUILD_ARGS=()
for arg in "$@"; do
    case "${arg}" in
        --release)   FORCE_RELEASE=true ;;
        --no-check)  EXTRA_RPMBUILD_ARGS+=(--nocheck) ;;
        *) echo "Unknown argument: ${arg}"; exit 1 ;;
    esac
done

# --- Read spec metadata ---
PKGNAME=$(awk '/^Name:/{print $2}' "${SPEC_FILE}")
VERSION=$(awk '/^Version:/{print $2}' "${SPEC_FILE}")

# --- Determine branch ---
BRANCH=$(git -C "${REPO_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")

# Sanitize branch name: keep only alphanumeric and underscores
BRANCH_SANITIZED=$(echo "${BRANCH}" | tr '/' '_' | tr '-' '_' | sed 's/[^a-zA-Z0-9_]//g' | tr '[:upper:]' '[:lower:]')

# Main branches get a standard release build
MAIN_BRANCHES=("main" "master")
IS_FEATURE_BRANCH=true
for mb in "${MAIN_BRANCHES[@]}"; do
    if [[ "${BRANCH}" == "${mb}" ]]; then
        IS_FEATURE_BRANCH=false
        break
    fi
done
[[ "${FORCE_RELEASE}" == "true" ]] && IS_FEATURE_BRANCH=false

# --- Set up rpmbuild tree ---
RPMBUILD_DIR="${HOME}/rpmbuild"
mkdir -p "${RPMBUILD_DIR}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# --- Create source tarball via git archive ---
# The prefix must match %{name}-%{version} so %autosetup finds the right dir.
TARBALL_NAME="open5gs-v${VERSION}.tar.gz"
TARBALL_PATH="${RPMBUILD_DIR}/SOURCES/${TARBALL_NAME}"

echo "==> Creating source tarball from git HEAD (committed files only)..."
echo "    Branch : ${BRANCH}"
echo "    Version: ${VERSION}"
git -C "${REPO_ROOT}" archive \
    --format=tar.gz \
    --prefix="${PKGNAME}-${VERSION}/" \
    HEAD \
    > "${TARBALL_PATH}"
echo "    Tarball: ${TARBALL_PATH}"

# --- Copy spec ---
cp "${SPEC_FILE}" "${RPMBUILD_DIR}/SPECS/"

# --- Assemble rpmbuild arguments ---
RPMBUILD_ARGS=(
    -ba
    "${EXTRA_RPMBUILD_ARGS[@]+"${EXTRA_RPMBUILD_ARGS[@]}"}"
    "${RPMBUILD_DIR}/SPECS/open5gs.spec"
)

if [[ "${IS_FEATURE_BRANCH}" == "true" ]]; then
    echo "==> Feature branch build"
    echo "    Release will be: 0.1.${BRANCH_SANITIZED}.ringer.<dist>"
    RPMBUILD_ARGS+=(--define "branchsuffix .${BRANCH_SANITIZED}")
else
    echo "==> Release build (no branch prefix)"
    echo "    Release will be: 1.ringer.<dist>"
fi

echo ""
echo "==> Running rpmbuild..."
rpmbuild "${RPMBUILD_ARGS[@]}"

echo ""
echo "==> Done. RPMs are in: ${RPMBUILD_DIR}/RPMS/"
find "${RPMBUILD_DIR}/RPMS" -name "${PKGNAME}*.rpm" -newer "${TARBALL_PATH}" | sort
