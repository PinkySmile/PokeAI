#!/bin/sh
rm -rf dist/*.whl
python3 setup.py bdist_wheel
pip install dist/*.whl --force-reinstall
