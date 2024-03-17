#!/usr/bin/env bash
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
cd "$SCRIPT_DIR"

python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install --upgrade pip
pip install west
pip install -r ./zephyr/scripts/requirements.txt
pip install paho-mqtt
cd -
