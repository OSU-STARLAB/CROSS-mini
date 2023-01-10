SRC_DIR=hls
THREADS=`nproc`

.PHONY: all
all:
	cd $(SRC_DIR) && $(MAKE) -j $(THREADS)

.PHONY: clean
clean:
	cd $(SRC_DIR) && $(MAKE) clean