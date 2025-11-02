#!/bin/sh
rm -rf build/lib.linux-x86_64-cpython-313/
python3 setup.py bdist_wheel
pip install dist/pokebattle-1.0-cp313-cp313-linux_x86_64.whl --force-reinstall
