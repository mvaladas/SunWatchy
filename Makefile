# Uncomment lines below if you have problems with $PATH
#SHELL := /bin/bash
#PATH := /usr/local/bin:$(PATH)

all:
		pio -f -c vim run --target compiledb
		pio -f -c vim run -j 8

upload: 
		pio -f -c vim run --target upload

compiledb:
		pio -f -c vim run --target compiledb

clean:
		pio -f -c vim run --target clean

program:
		pio -f -c vim run --target program

uploadfs:
		pio -f -c vim run --target uploadfs

update:
		pio -f -c vim update
