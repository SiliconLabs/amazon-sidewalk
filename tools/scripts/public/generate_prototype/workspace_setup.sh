#!/bin/bash
SCRIPT_DIR=$(dirname "$0")
echo "*** setting up workspace ***"

PYTHON3_VENV_DIR=${SCRIPT_DIR}/venv
if [ ! -f $PYTHON3_VENV_DIR/bin/activate ]; then
echo "creating virtual environment"
python3 -m venv $PYTHON3_VENV_DIR

$PYTHON3_VENV_DIR/bin/pip3 install wheel pyyaml
fi

echo "installing requirements"
. $PYTHON3_VENV_DIR/bin/activate
python3 -m pip install --upgrade pip
python3 -m pip install -r ./amazon_dependencies/requirements.txt