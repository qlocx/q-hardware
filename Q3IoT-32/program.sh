#!/bin/bash

echo "Start programming IoT card with 32 ports..."
echo "Make sure to:"
echo "1. Connect the debugger to the board and the computer"
echo "2. Connect power cord to board"
echo "3. Connect printer to computer"
echo "4. Connect printer to power"

echo "Checking if printer is connected..."

if ! lsusb | grep -q "Brother"; then
    echo -e "Printer not connected or not powered on"

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

nrfjprog --eraseall > /dev/null 2>&1
nrfjprog --reset > /dev/null 2>&1
nrfjprog --recover > /dev/null 2>&1
echo "Programming mfw file..."
program_result=$(nrfjprog --program ./mfw_nrf9160_1.3.5.zip --verify 2>&1)

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected during flashing. Exiting script."

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi


echo "Programming hex file..."
program_result=$(nrfjprog --program ./releases/v.1.12.0-mfw-1.3.5-ncs-2.6.0-32-ports/merged.hex --verify 2>&1)

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected during flashing. Exiting script."

    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

retry_deviceId_count=0
max_deviceId_retries=9

while [ $retry_deviceId_count -lt $max_deviceId_retries ]; do
    echo "Attempt $((retry_deviceId_count + 1)): Reading RAM to get device id..."
    
    nrfjprog --reset > /dev/null 2>&1
    sleep 10
    program_result=$(nrfjprog --readram ram.bin 2>&1)

    if echo "$program_result" | grep "ERROR"; then
        echo -e "Error detected when reading RAM. Exiting script."
        mpg123 ./fail.mp3 > /dev/null 2>&1
        exit 1
    fi
    
    # Extract device ID from RAM
    parse_device_id_result=$(strings ram.bin | grep "Q3IoT" | sed 's/.*\(Q3IoT\)/\1/' | head -1)
    
    if [ -n "$parse_device_id_result" ]; then
        # Validate length of deviceId
        if [ ${#parse_device_id_result} -ne 21 ]; then
            echo "Error: Length of deviceId is not equal to 21 characters." >&2
            ((retry_deviceId_count++))
            echo "Retrying read operation in 10 seconds..."
        else
            echo "Device ID successfully parsed: $parse_device_id_result"
            break  # Exit the loop if device ID is found
        fi
    else
        echo "Device ID not found in RAM. Retrying parsing operation in 10 seconds..."
        ((retry_deviceId_count++))
    fi
done

# Check if all retry attempts failed
if [ $retry_deviceId_count -eq $max_deviceId_retries ]; then
    echo "All retry attempts failed. Exiting script."
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
   signal_strength=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/4/0/2?timeout=60&format=TLV" | jq -r '.content.value')

    if ! [[ "$signal_strength" =~ ^-[0-9]+$ ]]; then
        echo "🕒 Attempt $attempt: Device is not online yet. Retrying in $wait_time seconds..."
        sleep "$wait_time"
        attempt=$((attempt + 1))
    else
        echo "🟢 Device is online"
        echo "📡 Device signal strength: $signal_strength dBm"
        SLACK_URL="https://slack.com/api/chat.postMessage"
        CHANNEL="qiot-test-logs"

        message="🟢 New QIoT-32 device online: $deviceId, signal strength: 📡  $signal_strength dBm"
        curl -X POST "$SLACK_URL" -H "Authorization: Bearer $SLACK_TOKEN" -H "Content-Type: application/json; charset=utf-8" --data "{\"text\":\"$message\",\"channel\":\"$CHANNEL\"}" > /dev/null 2>&1
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

all_ports_status=$(curl -s "$TEST_SUITE_URL/clients/$deviceId/26242/0/1?timeout=60&format=TLV" | jq -r '.content.value')

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
rm ram.bin > /dev/null 2>&1

mpg123 ./success.mp3 > /dev/null 2>&1


GREEN="\e[32m"
BLACK='\033[1;90m'

echo -e "${GREEN}"
echo '========== DEVICE PROGRAMMING SUITE SUCCESSFUL! =========='
echo -e "${BLACK}"

exit 0
