PIO_BOARD := $(shell grep '^board *= *' platformio.ini | sed 's/^board *= *//')
BOARD := $(PIO_BOARD)
PRJNAME := $(PIO_BOARD)-ex

default: west-build

west-build:
	@west build -b $(BOARD) -p auto zephyr -- -DPROJ_NAME=$(PRJNAME)

clean:
	@rm -rf build

.PHONY: default west-build clean
