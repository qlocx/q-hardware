#!/bin/bash

echo "Start programming IoT card with 32 ports..."
echo "Make sure to:"
echo "1. Connect the debugger to the board and the computer"
echo "2. Connect power cord to board"
echo "3. Connect printer to computer"

echo "Programming hex file..."
program_result=$(nrfjprog --program ./merged.hex --verify 2>&1 | grep "ERROR")

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected during flashing. Exiting script."

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "Reading RAM to get device id..."

sleep 5

program_result=$(nrfjprog --readram ram.hex 2>&1 | grep "ERROR")

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected when reading RAM. Exiting script."
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

memory_match=$(grep -A 1 "5133496F542D" ram.hex | head -n 2 | tr -d '[:space:]')

line_number=1

parsed_data=$(echo -e "$memory_match" | xxd -r -p | iconv -f latin1)
only_nums=$(echo "$parsed_data" | tr -cd '0-9' | cut -c 2-16)
deviceId="Q3IoT-$only_nums"

echo "Device id: $deviceId"

if [ -z "$deviceId" ]; then
    echo -e "Device id empty"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

dotenv_file=".env"
if [ -f "$dotenv_file" ]; then
    export $(grep -v '^#' "$dotenv_file" | xargs)
else
    echo -e "Qlocx Missing dotenv file"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "📝 Registering device in system..."
body="{\"endpoint\":\"$deviceId\", \"board\":\"q3iot-32\"}"

registration_status=$(curl -s -o /dev/null -w "%{http_code}" -X POST -H "Content-Type: application/json" -H "Authorization: $JWT_TOKEN" -d "$body" "$REGISTRATION_URL")

# TODO: also check if sleeping in response body, exit if sleeping is true
if [ "$registration_status" -ne 200 ]; then
    echo -e "Qlocx sync: registration_status $registration_status"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

max_retries=5

wait_time=5

attempt=1

while [ "$attempt" -le "$max_retries" ]; do
    connected_response=$(curl -s -w "%{http_code}\n%{response_body}" "$TEST_SUITE_URL/clients/$deviceId")
    connected_status=$(echo "$connected_response" | head -n 1)
    device_sleeping=$(echo "$connected_response" | tail -n 1 | jq -r '.sleeping')

if [ "$connected_status" -eq 200 ] && [ "$device_sleeping" == "false" ]; then
        echo "🟢 Device is online"
        break
    else
        echo "🕒 Attempt $attempt: Device is not online yet. Retrying in $wait_time seconds..."
        sleep "$wait_time"
        attempt=$((attempt + 1))
    fi
done


if [ "$attempt" -gt "$max_retries" ]; then
    echo "❌ Maximum retries reached. Device did not come online."
    mpg123 ./fail.mp3 > /dev/null 2>&1
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
        mpg123 ./fail.mp3 > /dev/null 2>&1
        exit 1
    fi
done

all_ports_status=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/26242/0/1?timeout=30&format=TLV" | jq -r '.content.value')

if [[ "$all_ports_status" -ne 4294967295 ]]; then
    echo "❌ Error: Unexpected value in port status: $all_ports_status"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

signal_strength=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/4/0/2?timeout=30&format=TLV" | jq -r '.content.value')

if ! [[ "$signal_strength" =~ ^-[0-9]+$ ]]; then
    echo "❌ Error: Unexpected value for signal strength: $signal_strength"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "📡 Device signal strength: $signal_strength dBm"

voltage=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/3316/0/5700?timeout=30&format=TLV" | jq -r '.content.value')

if ! [[ "$voltage" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    echo "❌ Error: Unexpected value for voltage $voltage"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "🔋 Device voltage: $voltage V"

label_value=$(echo "$deviceId" | rev | cut -c 1-8 | rev)

echo "📷 Generating QR code..."
convert -size 1590x210 xc:white -pointsize 180 -fill black -gravity center -annotate 0 "Qlocx Id: $label_value" text_image.png

text_width=$(identify -format %w text_image.png)
qrencode -s 26 -m 3 -o qr_code.png "$label_value" && convert qr_code.png -gravity North -background white -extent +0-40 qr_code_resized.png && convert qr_code_resized.png -resize $((text_width / 3))x qr_code_resized.png

composite -gravity North text_image.png qr_code_resized.png - | convert - -resize 2048x409 combined_image_resized.png

echo "🖨️ Printing label $label_value..."

# todo: configure printer once here: http://localhost:8000
print_result=$(ghostscript-printer-app combined_image_resized.png)

# TODO: INSTALL FOR PROGRAMMING COMPUTER
# TODO: MAKE SUCCESSFUL PRINT, AND CHECK LOGS. THOSE LOGS SHOULD WE DO AN IF FOR AND RETURN ERROR IF SOMETHING GOES WRONG

echo "$print_result"

echo "🧹 Cleanup..." 
rm *.png > /dev/null 2>&1

COLOR_REST="$(tput sgr0)"
COLOR_GREEN="$(tput setaf 2)"
printf '%s%s%s\n' $COLOR_GREEN ' ========== DEVICE PROGRAMMING SUITE SUCCESSFUL! ========== ' $COLOR_REST

mpg123 ./success.mp3 > /dev/null 2>&1

exit 0
