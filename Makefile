SRC_DIR=hls
THREADS=`nproc`

.PHONY: all
all:
	cd $(SRC_DIR) && $(MAKE) -j $(THREADS)

.PHONY: clean
clean:
	rm -f *.log
	rm -rf test_outputs
	cd $(SRC_DIR) && $(MAKE) clean
