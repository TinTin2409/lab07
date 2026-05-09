#!/usr/bin/env bash
# One-shot WSL bootstrap: APT deps, sync/copy into ~/labs/Lab07-CPUFuzzing, smoke check.
#
# Usage:
#   chmod +x scripts/setup-lab07-cpufuzzing-wsl.sh
#   ./scripts/setup-lab07-cpufuzzing-wsl.sh              # from repo clone; detects repo root as source
#   ./scripts/setup-lab07-cpufuzzing-wsl.sh /path/to/Lab07-CPUFuzzing   # explicit source (e.g. /mnt/c/Users/...)
#
# Source of truth: work only under ~/labs/Lab07-CPUFuzzing after this runs; avoid dual-editing via /mnt/c.
set -euo pipefail

PROJECT_SLUG="${PROJECT_SLUG:-Lab07-CPUFuzzing}"
DEST="${HOME}/labs/${PROJECT_SLUG}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ "${1:-}" ]]; then
  SRC="$(cd "$1" && pwd)"
else
  SRC="$(cd "${SCRIPT_DIR}/.." && pwd)"
fi

echo ">>> Source repo: ${SRC}"
echo ">>> Destination: ${DEST}"

if [[ "${SRC}" == "${DEST}" ]]; then
  echo ">>> Already inside ~/labs target; skipping rsync."
else
  mkdir -p "${HOME}/labs"
  if command -v rsync >/dev/null 2>&1; then
    rsync -a \
      "${SRC}/" "${DEST}/"
  else
    echo "Installing rsync (recommended for syncing from /mnt/c)..."
    sudo apt-get install -y rsync
    rsync -a \
      "${SRC}/" "${DEST}/"
  fi
fi

echo ">>> APT: toolchain (riscv64-unknown-elf, verilator optional for sim rebuild, python)"
sudo apt-get update -y
sudo apt-get install -y \
  build-essential \
  make \
  python3 \
  python3-pip \
  python3-venv \
  git \
  rsync \
  gcc-riscv64-unknown-elf \
  verilator \
  gdb-multiarch

echo ">>> Verifying tools..."
command -v riscv64-unknown-elf-gcc
command -v make
command -v python3

BOOT="${DEST}/part1/Makefile"
if [[ -f "${BOOT}" ]]; then
  echo ">>> Smoke: make clean && make (part1)"
  (
    cd "${DEST}/part1"
    make clean || true
    make
  )
else
  echo ">>> WARN: ${BOOT} missing; skip make smoke."
fi

cat << EOF

Done. Prefer opening this folder ONLY in Remote – WSL:
  ${DEST}

Verify parts (examples):
  cd ${DEST}/part1  && make
  cd ${DEST}/part2  && make
  bash ${DEST}/run.sh part1          # requires sim/obj_dir binaries + csr_file.mem present

Fallback if \`wsl\` hangs in the terminal UI: \\\\wsl\$\\Ubuntu\\home\\$(whoami)\\labs\\${PROJECT_SLUG}

Or from WSL: cursor "${DEST}"
EOF
