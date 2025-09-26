BUILD_FILE = ../build_number.txt
TIME_FILE  = ../last_build_time.txt
BUILD_NUM  = $(shell cat $(BUILD_FILE))

.PHONY: re

all: kernel

kernel:
	@echo "Starting kernel build..."
	@# Increment build number
	@count=$$(cat $(BUILD_FILE)); \
	count=$$((count + 1)); \
	echo $$count > $(BUILD_FILE); \
	echo "Build number: $$count"
	@date +%s > $(TIME_FILE)
	@echo "Kernel build finished successfully."

re:
	@echo "Checking last build and updating status..."
	@if [ ! -f $(TIME_FILE) ]; then \
	    echo "No previous build timestamp found. Skipping involuntary rebuild check."; \
	else \
	    last=$$(cat $(TIME_FILE)); \
	    now=$$(date +%s); \
	    diff=$$((now - last)); \
	    if [ $$diff -ge 600 ]; then \
	        echo "Involuntary rebuild detected! Last successful build was more than 10 minutes ago."; \
	    else \
	        echo "Build is within normal interval."; \
	    fi \
	fi
	@echo "Rolling back build number due to reported error..."
	@count=$$(cat $(BUILD_FILE)); \
	count=$$((count - 1)); \
	[ $$count -lt 0 ] && count=0; \
	echo $$count > $(BUILD_FILE); \
	echo "Build number after rollback: $$count"