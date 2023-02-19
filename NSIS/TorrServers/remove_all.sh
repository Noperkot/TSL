#!/bin/bash

find . -type f -name "TorrServer-windows-*.exe"  -exec rm -f "{}" +
echo -e "\nAll TorrServer binaries was removed\n"
sleep 2
  