#!/bin/bash
program_result=$(nrfjprog --readram ram.hex 2>&1 | grep "ERROR")

if echo "$program_result" | grep "ERROR"; then
    echo -e "Error detected when reading RAM. Exiting script."
    mpg123 ./fail.mp3 > /dev/null 2>&1
    exit 1
fi

memory_match=$(grep -A 1 "5133496F542D" ram.hex | head -n 2 | tr -d '[:space:]')

nrfjprog --reset

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


label_value=$(echo "$deviceId" | rev | cut -c 1-8 | rev)

echo "📷 Generating QR code..."
convert -size 1590x210 xc:white -pointsize 180 -fill black -gravity center -annotate 0 "Qlocx Id: $label_value" -extent 1590x350 text_image.png

text_width=$(identify -format %w text_image.png)
qrencode -s 26 -m 3 -o qr_code.png "$label_value" && convert qr_code.png -gravity North -background white -borderColor white -border 150x150 -extent +0-40 qr_code_resized.png && convert qr_code_resized.png -resize ${text_width}x qr_code_resized.png

composite -gravity North text_image.png qr_code_resized.png - | convert - -resize 2048x409 -rotate 90 combined_image_resized.png

echo "🖨️ Printing label $label_value..."

# todo: configure printer once here: http://localhost:8000
# create two label prints, for top and bottom
print_result=$(ghostscript-printer-app combined_image_resized.png)
print_result=$(ghostscript-printer-app combined_image_resized.png)
