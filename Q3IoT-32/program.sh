#!/bin/bash

echo "Start programming IoT card with 32 ports..."
echo "Make sure to:"
echo "1. Connect the debugger to the board and the computer"
echo "2. Connect power cord to board"
echo "3. Connect printer to computer"
echo "4. Connect printer to power"

echo "Checking if printer is connected..."

if ! lsusb | grep -q "Brother Industries, Ltd"; then
    echo -e "Printer not connected or not powered on"

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "Programming mfw file..."
program_result=$(nrfjprog --program ./mfw_nrf9160_1.3.5.zip --verify)

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected during flashing. Exiting script."

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi


echo "Programming hex file..."
nrfjprog --eraseall
nrfjprog --reset
program_result=$(nrfjprog --program ./releases/v.1.11-mfw-1.3.5-ncs-2.2.0-32-ports/merged.hex --verify 2>&1 | grep "ERROR")

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected during flashing. Exiting script."

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "Reading RAM to get device id..."

sleep 10

program_result=$(nrfjprog --readram ram.hex 2>&1 | grep "ERROR")

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected when reading RAM. Exiting script."
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

memory_match=$(grep -A 1 "5133496F542D" ram.hex | head -n 2 | tr -d '[:space:]')

nrfjprog --reset

line_number=1

parsed_data=$(echo -e "$memory_match" | xxd -r -p | iconv -f latin1 | sed 's/Q3IoT-/-/g')
only_nums=$(echo "$parsed_data" | tr -cd '0-9' | cut -c -16)
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

if [ "$registration_status" -ne 200 ]; then
    echo -e "Qlocx sync: registration_status $registration_status"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "🕒 Waiting for device to connect..."

sleep 10

max_retries=20

wait_time=10

attempt=1

while [ "$attempt" -le "$max_retries" ]; do
   signal_strength=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/4/0/2?timeout=30&format=TLV" | jq -r '.content.value')

    if ! [[ "$signal_strength" =~ ^-[0-9]+$ ]]; then
        echo "🕒 Attempt $attempt: Device is not online yet. Retrying in $wait_time seconds..."
        sleep "$wait_time"
        attempt=$((attempt + 1))
    else
        echo "🟢 Device is online"
        echo "📡 Device signal strength: $signal_strength dBm"
        break
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

voltage=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/3316/0/5700?timeout=30&format=TLV" | jq -r '.content.value')

if ! [[ "$voltage" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    echo "❌ Error: Unexpected value for voltage $voltage"
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

echo "🔋 Device voltage: $voltage V"

label_value=$(echo "$deviceId" | rev | cut -c 1-8 | rev)

echo "📷 Generating QR code..."
convert -size 1590x210 xc:white -pointsize 180 -fill black -gravity center -annotate 0 "Qlocx Id: $label_value" -extent 1590x350 text_image.png

text_width=$(identify -format %w text_image.png)
qrencode -s 26 -m 3 -o qr_code.png "$label_value" && convert qr_code.png -gravity North -background white -borderColor white -border 150x150 -extent +0-40 qr_code_resized.png && convert qr_code_resized.png -resize ${text_width}x qr_code_resized.png

composite -gravity North text_image.png qr_code_resized.png - | convert - -resize 2048x409 combined_image_resized.png

echo "🖨️ Printing label $label_value..."

# todo: configure printer once here: http://localhost:8000
# create two label prints, for top and bottom
print_result=$(ghostscript-printer-app combined_image_resized.png)
print_result=$(ghostscript-printer-app combined_image_resized.png)

echo "Second print job id: $print_result"

echo "🧹 Cleanup..." 
rm *.png > /dev/null 2>&1
rm ram.hex > /dev/null 2>&1

mpg123 ./success.mp3 > /dev/null 2>&1


GREEN="\e[32m"
BLACK='\033[1;90m'

echo -e "${GREEN}"
echo '========== DEVICE PROGRAMMING SUITE SUCCESSFUL! =========='
echo -e "${BLACK}"

exit 0
