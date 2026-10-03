#!/usr/bin/env bash
# Run first without arguments to preview. --install changes the target Ubuntu OS.
set -euo pipefail
B="$(cd "$(dirname "$0")/.." && pwd)"
source /etc/os-release
[[ "$ID" == ubuntu && "$VERSION_ID" == 24.04 && "$(dpkg --print-architecture)" == amd64 ]] || { echo 'Requires Ubuntu 24.04 amd64' >&2; exit 2; }
python3 "$B/scripts/verify-bundle.py"
mapfile -t debs < <(find "$B/apt/archives" -maxdepth 1 -name '*.deb' -type f | sort)
[[ ${#debs[@]} -gt 0 ]]
case "${1:---simulate}" in
 --simulate) apt-get -o Dir::Cache::archives="$B/apt/archives" --simulate --no-download --no-remove install "${debs[@]}" ;;
 --install)
  [[ $EUID == 0 ]] || { echo 'Installation requires target administrator privileges.' >&2; exit 2; }
  DEBIAN_FRONTEND=noninteractive apt-get -o Dir::Cache::archives="$B/apt/archives" --yes --no-download --no-remove install "${debs[@]}"
  ;;
 *) echo "Usage: $0 [--simulate|--install]" >&2; exit 2 ;;
esac
