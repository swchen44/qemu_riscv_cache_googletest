#!/usr/bin/env bash
# Default simulates. --install requires a locally authorized Ubuntu administrator.
set -euo pipefail
B=""; MODE=--simulate
while (($#)); do
 case "$1" in
  --bundle-root) [[ $# -ge 2 ]] || exit 2; B="$2"; shift 2 ;;
  --simulate|--install) MODE="$1";shift ;;
  *) echo 'Usage: install-ubuntu-offline.sh --bundle-root DIRECTORY [--simulate|--install]' >&2;exit 2 ;;
 esac
done
[[ -n "$B" ]] || { echo '--bundle-root is required' >&2;exit 2; }
B="$(cd "$B" && pwd)"
source /etc/os-release
[[ "$ID" == ubuntu && "$VERSION_ID" == 24.04 && "$(dpkg --print-architecture)" == amd64 ]] || { echo 'Requires Ubuntu 24.04 amd64' >&2;exit 2; }
python3 "$(dirname "$0")/verify-bundle.py" --bundle-root "$B"
# Install only verified lock entries, never every file found in a directory.
debs_text="$(python3 - "$B" <<'PY'
import json,pathlib,sys
b=pathlib.Path(sys.argv[1])
for d in json.loads((b/'ubuntu-packages.lock.json').read_text())['packages']:
 p=(b/d['path']).resolve();assert p.is_relative_to(b)
 assert '\n' not in str(p) and '\r' not in str(p)
 print(p)
PY
)"
[[ -n "$debs_text" ]]
mapfile -t debs <<< "$debs_text"
[[ ${#debs[@]} -gt 0 ]]
case "$MODE" in
 --simulate) apt-get -o Dir::Cache::archives="$B/apt/archives" --simulate --no-download --no-remove --no-install-recommends install "${debs[@]}" ;;
 --install)
  [[ $EUID == 0 ]] || { echo 'Installation requires target administrator privileges.' >&2;exit 2; }
  DEBIAN_FRONTEND=noninteractive apt-get -o Dir::Cache::archives="$B/apt/archives" --yes --no-download --no-remove --no-install-recommends install "${debs[@]}"
  ;;
esac
