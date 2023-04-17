SRC_DIR=hls
THREADS=`nproc`
SIM_EXE=accel_sim

$(SIM_EXE): all
	cp $(SRC_DIR)/$@ $@

.PHONY: all
all:
	cd $(SRC_DIR) && $(MAKE) -j $(THREADS)

.PHONY: clean
clean:
	rm -f $(SIM_EXE)
	cd $(SRC_DIR) && $(MAKE) clean
