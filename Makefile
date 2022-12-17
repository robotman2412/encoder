
.PHONY: build flash clean

build:
	mkdir -p build
	cd build; cmake ..
	make -j$(shell nproc) -C build

flash: build
	picotool load -f build/encoder.uf2
	picotool reboot

clean:
	rm -rf build/*
