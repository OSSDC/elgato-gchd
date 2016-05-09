/**
 * Copyright (c) 2014 - 2015 Tolga Cakir <tolga@cevel.net>
 *
 * This source file is part of Game Capture HD Linux driver and is distributed
 * under the MIT License. For more information, see LICENSE file.
 */

#include <atomic>
#include <csignal>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <libusb-1.0/libusb.h>

#include <core/gchd.hpp>
#include <process.hpp>

int main(int argc, char *argv[]) {
	// object for managing runtime information
	Process process;

	// object for storing device settings
	Settings settings;

	// commandline-specific settings
	std::string pidPath = "/var/run/gchd.pid";
	std::string outputPath = "/tmp/gchd.ts";
	bool useFifo = true;

	// handling command-line options
	int opt;

	// TODO show help information
	while ((opt = getopt(argc, argv, ":di:o:p:r:")) != -1) {
		switch (opt) {
			case 'd':
				useFifo = false;
				break;
			case 'i':
				if (std::string(optarg) == "Composite") {
					settings.setInputSource(InputSource::Composite);
					break;
				} else if (std::string(optarg) == "SVideo") {
					settings.setInputSource(InputSource::SVideo);
					break;
				} else if (std::string(optarg) == "Component") {
					settings.setInputSource(InputSource::Component);
					break;
				} else if (std::string(optarg) == "HDMI") {
					settings.setInputSource(InputSource::HDMI);
					break;
				}

				std::cerr << "Unrecognized Input Source." << std::endl;
				return EXIT_FAILURE;
			case 'o':
				outputPath = std::string(optarg);
				break;
			case 'p':
				pidPath = std::string(optarg);
				break;
			case 'r':
				if (std::string(optarg) == "SD") {
					settings.setResolution(Resolution::Standard);
					break;
				} else if (std::string(optarg) == "720") {
					settings.setResolution(Resolution::HD720);
					break;
				} else if (std::string(optarg) == "1080") {
					settings.setResolution(Resolution::HD1080);
					break;
				}

				std::cerr << "Unsupported resolution." << std::endl;
				return EXIT_FAILURE;
			case ':':
				fprintf(stderr, "Missing argument.\n");
				return EXIT_FAILURE;
			case '?':
				fprintf(stderr, "Unrecognized option.\n");
				return EXIT_FAILURE;
			default:
				fprintf(stderr, "Unexpected error.\n");
				return EXIT_FAILURE;
		}
	}

	// TODO not ready for primetime yet, program needs to be restarted too
	// often at the moment
	// create PID file for single instance mechanism
//	if (process.createPid(pidPath)) {
//		return EXIT_FAILURE;
//	}

	GCHD gchd(&settings);

	// device initialization
	if(gchd.init()) {
		return EXIT_FAILURE;
	}

	if (useFifo) {
		if (process.createFifo(outputPath)) {
			return EXIT_FAILURE;
		}

		// when FIFO file has been opened
		process.streamToFifo(&gchd);
	} else {
		process.streamToDisk(&gchd, outputPath);
	}

	fprintf(stderr, "Terminating.\n");

	return EXIT_SUCCESS;
}
