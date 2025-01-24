#!/bin/bash

program_name="SwitchPCWebServer"

service_file=$program_name".service"
service_path="/etc/systemd/system/"

printf "Uninstalling "$program_name"\n\n"

printf "Disable service "$service_file"\n"
# disable service
systemctl disable --now $service_file

# delete file service
rm -f $service_path$service_file

# restart daemons
systemctl daemon-reload
printf "\nUninstall complete\n"