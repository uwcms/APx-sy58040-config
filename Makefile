APP = xptconfig

all: sy58040-config

sy58040-config: sy58040-config.cpp
	$(CXX) -std=c++11 -o $@ $^ -lgpiod

.PHONY: clean
clean:
	rm -f sy58040-config *.rpm
