#!/bin/bash

path="$(dirname "$(readlink -f "$0")")/"
program_name="SwitchPCWebServer"

# Upload sketch with arduino-cli
arduino_serial_port="/dev/ttyUSB0"
arduino_fqbn="esp32:esp32:esp32"
arduino_sketch=$path"sketch/blink/blink.ino"
arduino_build=$path"sketch/build"

printf "Installing "$program_name"\n\n"

printf "Upload sketch for ESP32 with arduino-cli on port: "$arduino_serial_port"\n\n"
sudo -u $SUDO_USER arduino-cli compile --fqbn $arduino_fqbn --build-path $arduino_build $arduino_sketch
chown $USER $arduino_serial_port
sudo -u $SUDO_USER arduino-cli upload -p $arduino_serial_port --fqbn $arduino_fqbn --build-path $arduino_build $arduino_sketch

# Systemd file service
script_path=$path"script/"
script_file=$program_name".py"

python="venv/bin/python3"

service_file=$program_name".service"
service_path="/etc/systemd/system/"

printf "\nCreating service...\n"
cat <<EOL > $service_path$service_file
[Unit]
Description=$service_file send status connect service
After=network-online.target
Wants=network-online.target

[Service]
Type=idle
WorkingDirectory=$script_path

# on startup send 1=connected at the WebServer
ExecStart=$script_path$python $script_path$script_file 1
# on poweroff send 0=disconnected at the WebServer
ExecStop=$script_path$python $script_path$script_file 0

Restart=on-failure
TimeoutStopSec=10

StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOL

# restart daemons
systemctl daemon-reload

printf "\nEnable service "$service_file"\n"
# enable service
systemctl enable --now $service_file
printf "\nInstallation complete\n"
