#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build/agent}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
ENABLE_EBPF_VALUE="${ENABLE_EBPF:-OFF}"

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "agent only supports Linux." >&2
  exit 1
fi

if command -v nproc >/dev/null 2>&1; then
  JOBS="$(nproc)"
else
  JOBS=4
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DBUILD_MANAGER=OFF \
  -DBUILD_AGENT=ON \
  -DENABLE_EBPF="${ENABLE_EBPF_VALUE}"

cmake --build "${BUILD_DIR}" --target agent --parallel "${JOBS}"

echo "agent build done: ${BUILD_DIR}/agent"
