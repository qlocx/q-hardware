#!/bin/bash

echo "Start programming IoT card with 32 ports..."

# program_result=$(nrfjprog --program ./build/zephyr/merged.hex --verify 2>&1 | grep "ERROR")

# if echo "$program_result" | grep "ERROR"; then
#     echo -e "\e[31mError detected during flashing. Exiting script.\e[0m"

#     afplay ./fail.mp3
#     exit 1
# fi

echo "Reading RAM to get device id..."
program_result=$(nrfjprog --readram ram.hex 2>&1 | grep "ERROR")

# TODO: MAYBE ADD SLEEP SO THAT ENDPOINT NAME CAN HAVE SOME TIME TO BE REASSIGNED MASED ON IMEI

if echo "$program_result" | grep "ERROR"; then
    echo -e "\e[31mError detected when reading RAM. Exiting script.\e[0m"
    afplay ./fail.mp3
    exit 1
fi

memory_match=$(grep -A 1 "5133496F542D" ram.hex | head -n 2)

line_number=1

deviceId=""
# Iterate through each line and print
while IFS= read -r line; do
    parsed_data=$(echo -e "$line" | xxd -r -p | iconv -f latin1)
    if [ "$line_number" -eq 1 ]; then
       deviceId=$(echo "$parsed_data" | sed 's/.*Q3IoT-/Q3IoT-/;s/[^A-Za-z0-9-]//g')
    else
     for ((i=0; i<${#parsed_data}; i++)); do
         char="${parsed_data:$i:1}"
         
         if [[ "$char" =~ [0-9] ]]; then
             found_numeric=true
         fi

         if [ "$found_numeric" = true ]; then
             deviceId+="$char"

             if [ ${#deviceId} -eq 21 ]; then
                 break
             fi
         fi
     done
    fi

    line_number=$((line_number + 1))
done <<< "$memory_match"

echo "Device id: $deviceId"

if [ -z "$deviceId" ]; then
    echo -e "\e[31mDevice id empty\e[0m"

dotenv_file=".env"
if [ -f "$dotenv_file" ]; then
    export $(grep -v '^#' "$dotenv_file" | xargs)
else
    echo -e "\e[31mQlocx sync: status $status\e[0m"
    afplay ./fail.mp3
    exit 1
fi

body="{\"endpoint\":\"$deviceId\", \"board\":\"q3iot-32\"}"

status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/json" -H "Authorization: $JWT_TOKEN" -d "$body" "$URL")

if [ "$status" -ne 200 ]; then
    echo -e "\e[31mQlocx sync: status $status\e[0m"
    afplay ./fail.mp3
    exit 1
else
    sleep 1
    echo "$(nrfjprog --reset)"
    echo -e "\n\e[32m===== Device registration successful =====\e[0m"
    afplay ./success.mp3
fi

exit 0
