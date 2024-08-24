# Makefile for Model Transform Project

# Variables
APPLICATION_NAME = OpenGLViewer
BATCH_SCRIPT = script/build.bat

# Targets
all: build

build:
	@echo "Building $(APPLICATION_NAME)..."
	@cmd "script/build.bat"