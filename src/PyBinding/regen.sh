#!/bin/sh
python3 setup.py bdist_wheel
pip install dist/*.whl --force-reinstall
