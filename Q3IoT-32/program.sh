#!/bin/bash

echo "Start programming IoT card with 32 ports..."

# TODO: RUN HEX SCRIPT!

# IF SCRIPT FAILS, EXIT WITH SOUND

echo "Reading RAM to get device id..."
nrfjprog --readram ram.hex

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
    # Play an exit sound using afplay (adjust the sound file path accordingly)
    afplay ./fail.mp3
    exit 1
fi

URL="https://webhook.site/1d1cb398-85ec-44fb-a604-c17489de62ce"
# TODO: read token from .env file!
JWT="your_jwt_token"
body="{\"endpoint\":\"$deviceId\"}"

# Perform the POST request with curl
status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/json" -H "Authorization: $JWT" -d "$body" "$URL")

if [ "$status" -ne 200 ]; then
    echo "Qlocx sync: status $status"
    afplay ./fail.mp3
else
    sleep 1
    echo "$(nrfjprog --reset)"
    echo -e "\n===== Device registration sucessful ====="
    afplay ./success.mp3
fi

# # Check the status code
# status=$(echo "$response" | jq -r '.status')
# message=$(echo "$jsonResult" | jq -r '.message')

# if [ "$status" -ne 200 ]; then
#     echo "Qlocx sync: status $status, $message"
# else
#     sleep 1
#     echo "$(nrfjprog --reset)"
#     echo -e "\n===== Stoppa nu i batteriet i kretskortet ====="
#     echo -e "\n===== Kretskortet har fått ID: ${config.public_key:0:8} ===\n"
# fi