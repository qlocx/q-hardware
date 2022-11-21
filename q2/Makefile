OUTPUT_DIRECTORY=build
SDK_ROOT=nRF5_SDK_15.0.0_a53641a
BOOTLOADER_DIRECTORY=bootloader
APPLICATION_DIRECTORY=app

flash: erase flash_softdevice flash_bootloader flash_application flash_config flash_bootloader_setting reset upload_config protect_from_readback

m: erase flash_softdevice flash_bootloader flash_application flash_config flash_bootloader_setting reset upload_config

clean:
	rm -rf $(OUTPUT_DIRECTORY)
	cd app && make clean
	cd bootloader && make clean

reset:
	nrfjprog --reset -f nrf52

protect_from_readback:
	nrfjprog --rbp all

###############
# APPLICATION #
###############

application: $(APPLICATION_DIRECTORY)/build/nrf52832_xxaa.hex

flash_application: $(APPLICATION_DIRECTORY)/build/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $<

.PHONY: $(APPLICATION_DIRECTORY)/build/nrf52832_xxaa.hex

$(APPLICATION_DIRECTORY)/build/nrf52832_xxaa.hex:
	cd app && make

##############
# BOOTLOADER #
##############

bootloader: $(BOOTLOADER_DIRECTORY)/build/nrf52832_xxaa.hex

flash_bootloader: $(BOOTLOADER_DIRECTORY)/build/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $<

.PHONY: $(BOOTLOADER_DIRECTORY)/build/nrf52832_xxaa.hex

$(BOOTLOADER_DIRECTORY)/build/nrf52832_xxaa.hex:
	cd bootloader && make

######################
# BOOTLOADER_SETTING #
######################

bootloader_setting: $(OUTPUT_DIRECTORY)/bootloader_setting.hex

flash_bootloader_setting: $(OUTPUT_DIRECTORY)/bootloader_setting.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $<

$(OUTPUT_DIRECTORY)/bootloader_setting.hex: app/build/nrf52832_xxaa.hex
	mkdir -p build
	nrfutil settings generate --family NRF52 --application $< --application-version 0 --bootloader-version 0 --bl-settings-version 1 $@

################
# CONFIG (HEX) #
################

flash_config: $(OUTPUT_DIRECTORY)/config.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $<

$(OUTPUT_DIRECTORY)/config.hex: scripts/config_to_hex.js $(OUTPUT_DIRECTORY)/config.json node_modules
	mkdir -p $(OUTPUT_DIRECTORY)
	gcc -E -P - < $< > $(OUTPUT_DIRECTORY)/config_to_hex.js
	node $(OUTPUT_DIRECTORY)/config_to_hex.js > $@

#################
# CONFIG (JSON) #
#################

upload_config: $(OUTPUT_DIRECTORY)/config.json scripts/sync_config_to_database.js
	gcc -E -P - < scripts/sync_config_to_database.js > $(OUTPUT_DIRECTORY)/sync_config_to_database.js
	node $(OUTPUT_DIRECTORY)/sync_config_to_database.js


$(OUTPUT_DIRECTORY)/config.json: scripts/create_json_config.js node_modules
	mkdir -p $(OUTPUT_DIRECTORY)
	gcc -E -P - < $< > $(OUTPUT_DIRECTORY)/create_json_config.js
	node $(OUTPUT_DIRECTORY)/create_json_config.js > $@

##############
# SOFTDEVICE #
##############

flash_softdevice:
	@echo Flashing: s132_nrf52_6.0.0_softdevice.hex
	nrfjprog -f nrf52 --program $(SDK_ROOT)/components/softdevice/s132/hex/s132_nrf52_6.0.0_softdevice.hex

#########
# OTHER #
#########

erase:
	nrfjprog -f nrf52 --eraseall

$(OUTPUT_DIRECTORY)/package.zip: $(OUTPUT_DIRECTORY)/application.hex
	mkdir -p build
	nrfutil pkg generate \
	--hw-version 52 \
	--application-version 2 \
	--application $< \
	--sd-req 0xA8 \
	--key-file private_key.txt \
	$@
	nrfutil pkg display $@
