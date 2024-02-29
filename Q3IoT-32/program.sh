#!/bin/bash

echo "Start programming IoT card with 32 ports..."

# program_result=$(nrfjprog --program ./build/zephyr/merged.hex --verify 2>&1 | grep "ERROR")

# if echo "$program_result" | grep "ERROR"; then
#     echo -e "Error detected during flashing. Exiting script."

#     afplay ./fail.mp3
#     exit 1
# fi

echo "Reading RAM to get device id..."
program_result=$(nrfjprog --readram ram.hex 2>&1 | grep "ERROR")

# TODO: MAYBE ADD SLEEP SO THAT ENDPOINT NAME CAN HAVE SOME TIME TO BE REASSIGNED BASED ON IMEI

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected when reading RAM. Exiting script."
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
    echo -e "Device id empty"
    afplay ./fail.mp3
    exit 1
fi

dotenv_file=".env"
if [ -f "$dotenv_file" ]; then
    export $(grep -v '^#' "$dotenv_file" | xargs)
else
    echo -e "Qlocx Missing dotenv file"
    afplay ./fail.mp3
    exit 1
fi

body="{\"endpoint\":\"$deviceId\", \"board\":\"q3iot-32\"}"

registration_status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/json" -H "Authorization: $JWT_TOKEN" -d "$body" "$REGISTRATION_URL")

if [ "$registration_status" -ne 200 ]; then
    echo -e "Qlocx sync: registration_status $registration_status"
    afplay ./fail.mp3
    exit 1
else
    sleep 1
    echo "$(nrfjprog --reset)"
fi

max_retries=5

wait_time=5

attempt=1

while [ "$attempt" -le "$max_retries" ]; do
    connected_status=$(curl -s -o /dev/null -w "%{http_code}" "$TEST_SUITE_URL/clients/$deviceId")

    if [ "$connected_status" -eq 200 ]; then
        echo "🟢 Device is online"
        break
    else
        echo "Attempt $attempt: Device is not online yet. Retrying in $wait_time seconds..."
        sleep "$wait_time"
        attempt=$((attempt + 1))
    fi
done


if [ "$attempt" -gt "$max_retries" ]; then
    echo "❌ Maximum retries reached. Device did not come online."
    afplay ./fail.mp3
    exit 1
fi

echo "🧪 Running test suite..."

port_idx=0

while [ "$port_idx" -le 31 ]; do
    open_port_request_body='{"id":100,"value":"0,500"}'
    open_port_response=$(curl -s -X PUT -d "$open_port_request_body" -H "Content-Type: application/json" "$TEST_SUITE_URL/clients/$deviceId/10351/$port_idx/100?timeout=30&format=TLV")

    if [ "$(echo "$open_port_response" | jq -r '.success')" = "true" ]; then
        echo "✅ Port $port_idx opened "
        port_idx=$((port_idx + 1))
        sleep 1
    else
        echo "❌ Port $port_idx opening FAILED. Response: $open_port_response"
        afplay ./fail.mp3
        exit 1
    fi
done

# PRINT LABELS USING PYTHON SCRIPT
# OTHERWISE, BJörn will be angry

all_ports_status=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/26242/0/1?timeout=30&format=TLV" | jq -r '.content.value')

if [[ "$all_ports_status" -ne 4294967295 ]]; then
    echo "❌ Error: Unexpected value in port status: $all_ports_status"
    afplay ./fail.mp3
    exit 1
fi

signal_strength=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/4/0/2?timeout=30&format=TLV" | jq -r '.content.value')

if ! [[ "$signal_strength" =~ ^-[0-9]+$ ]]; then
    echo "❌ Error: Unexpected value for signal strength: $signal_strength"
    afplay ./fail.mp3
    exit 1
fi

echo "📡 Device signal strength: $signal_strength dBm"

voltage=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/3316/0/5700?timeout=30&format=TLV" | jq -r '.content.value')

if ! [[ "$voltage" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    echo "❌ Error: Unexpected value for voltage $voltage"
    afplay ./fail.mp3
    exit 1
fi

echo "🔋 Device voltage: $voltage V"

# run python script that uses deviceId to print using label printer with Brother printer application
# or better yes, maybe inspect commands inside brother printer to see if we can reverse engineer it

COLOR_REST="$(tput sgr0)"
COLOR_GREEN="$(tput setaf 2)"
printf '%s%s%s\n' $COLOR_GREEN ' ========== DEVICE PROGRAMMING SUITE SUCCESSFUL! ========== ' $COLOR_REST

afplay ./success.mp3

exit 0
