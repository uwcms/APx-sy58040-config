#include <errno.h>
#include <fcntl.h>
#include <gpiod.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define XPT_DEBUG

typedef union {
	int line_int_data[6];
	struct {
		int config;
		int load;
		int sin[2];
		int sout[2];
	};
} s_line_data;

typedef union {
	unsigned int line_int_data[6];
	struct {
		unsigned int config;
		unsigned int load;
		unsigned int sin[2];
		unsigned int sout[2];
	};
} u_line_data;

void configure_xpt(struct gpiod_line_bulk *lines, int in, int out);

int main(int argc, char *argv[]) {
	if (argc != 6) {
		printf("Program Crosspoint\n");
		printf("%s $gpiodev $out0src $out1src $out2src $out3src\n", argv[0]);
		printf("\n");
		printf("Example: %s /dev/gpio.axi_gpio_clk_xpt_config 0 0 0 0 # source all outputs from input 0.\n", argv[0]);
		return 1;
	}

	int xptmap[4]; // xptmap[output] = input
	for (int i = 0; i < 4; i++) {
		if (strcmp(argv[2 + i], "0") == 0)
			xptmap[i] = 0;
		else if (strcmp(argv[2 + i], "1") == 0)
			xptmap[i] = 1;
		else if (strcmp(argv[2 + i], "2") == 0)
			xptmap[i] = 2;
		else if (strcmp(argv[2 + i], "3") == 0)
			xptmap[i] = 3;
		else {
			printf("Invalid source selection: %s\n", argv[1 + i]);
			return 1;
		}
	}

	char *gpiochip_path = realpath(argv[1], NULL);
	if (gpiochip_path == NULL) {
		perror("unable to resolve gpiochip path");
		return 1;
	}
	struct gpiod_chip *gpiochip = gpiod_chip_open(gpiochip_path);
	free(gpiochip_path);
	if (gpiochip == NULL) {
		perror("error opening gpio chip");
		return 1;
	}

	u_line_data line_offsets;
	line_offsets.config = 0;
	line_offsets.load = 1;
	line_offsets.sin[0] = 2;
	line_offsets.sin[1] = 3;
	line_offsets.sout[0] = 4;
	line_offsets.sout[1] = 5;

	s_line_data line_defaults;
	for (int i = 0; i < 6; ++i)
		line_defaults.line_int_data[i] = 1;
	line_defaults.config = 0;
	line_defaults.load = 0;

	struct gpiod_line_bulk lines;
	if (0 != gpiod_chip_get_lines(gpiochip, line_offsets.line_int_data, 6, &lines)) {
		perror("error accessing gpiochip lines");
		return 1;
	}
	if (0 != gpiod_line_request_bulk_output(&lines, "sy58040-config", const_cast<const int *>(line_defaults.line_int_data))) {
		perror("error reserving gpiochip lines for output");
		return 1;
	}

	for (int i = 0; i < 4; i++)
		configure_xpt(&lines, xptmap[i], i);

	return 0;
}

void configure_xpt(struct gpiod_line_bulk *lines, int in, int out) {

#ifdef XPT_DEBUG
	printf("XPT: Programming in %d -> out %d\n", in, out);
#endif

	s_line_data outputs;
	outputs.config = 0;
	outputs.load = 0;
	outputs.sout[0] = (out & 1 ? 1 : 0);
	outputs.sout[1] = (out & 2 ? 1 : 0);
	outputs.sin[0] = (in & 1 ? 1 : 0);
	outputs.sin[1] = (in & 2 ? 1 : 0);

	gpiod_line_set_value_bulk(lines, const_cast<const int *>(outputs.line_int_data));
	usleep(100);

	outputs.load = 1;
	gpiod_line_set_value_bulk(lines, const_cast<const int *>(outputs.line_int_data));
	usleep(100);

	outputs.load = 0;
	gpiod_line_set_value_bulk(lines, const_cast<const int *>(outputs.line_int_data));
	usleep(100);

	outputs.config = 1;
	gpiod_line_set_value_bulk(lines, const_cast<const int *>(outputs.line_int_data));
	usleep(100);

	outputs.config = 0;
	gpiod_line_set_value_bulk(lines, const_cast<const int *>(outputs.line_int_data));
	usleep(100);
}
