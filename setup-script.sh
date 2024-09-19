#!/bin/bash

echo "This script requires platformio to be installed and setup (CLI included)"
echo "To install the CLI run: sudo pip install -U platformio"
echo "Always remember to run this script within the project directory"

if [ "$#" -ne 3 ]; then
  echo "Error: This script requires exactly 3 arguments. The DEVICE_NUMBER, PACKAGE_ID and WIFI_PASSWORD"
  echo "Example: ./setup-script.sh 3 2daY6 UNIBO1234"
  exit 1
fi

DEVICE_NUMBER=$1
PACKAGE_ID=$2
WIFI_PASSWORD=$3

WIFI_NAME="Rete $PACKAGE_ID dei MicroMondi"

cp src/main.cpp src/.main.cpp.backup

sed -i '' "s/BASH_DEVICE_NUMBER/$DEVICE_NUMBER/g" src/main.cpp
sed -i '' "s/BASH_WIFI_NAME/$WIFI_NAME/g" src/main.cpp
sed -i '' "s/BASH_WIFI_PASSWORD/$WIFI_PASSWORD/g" src/main.cpp

echo "Building project and uploading to device"

platformio run --target upload

rm src/main.cpp
mv src/.main.cpp.backup src/main.cpp
