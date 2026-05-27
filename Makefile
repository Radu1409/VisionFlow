SERVICE_BUILD := service/build

.PHONY: all run clean \
        run-rgb_to_yuv_concat run-rgb_to_yuv_split \
        run-yuv_to_rgb_concat run-yuv_to_rgb_split \
        run-raw_to_rgb_concat run-raw_to_rgb_split \
        run-raw_to_yuv_concat run-raw_to_yuv_split \
        clean-rgb_to_yuv_concat clean-rgb_to_yuv_split \
        clean-yuv_to_rgb_concat clean-yuv_to_rgb_split \
        clean-raw_to_rgb_concat clean-raw_to_rgb_split \
        clean-raw_to_yuv_concat clean-raw_to_yuv_split \
        clean-all-scenarios

all:
	$(MAKE) -C $(SERVICE_BUILD)

run:
	$(MAKE) -C $(SERVICE_BUILD) run

clean:
	$(MAKE) -C $(SERVICE_BUILD) clean

run-rgb_to_yuv_concat:
	$(MAKE) -C $(SERVICE_BUILD) run-rgb_to_yuv_concat

run-rgb_to_yuv_split:
	$(MAKE) -C $(SERVICE_BUILD) run-rgb_to_yuv_split

run-yuv_to_rgb_concat:
	$(MAKE) -C $(SERVICE_BUILD) run-yuv_to_rgb_concat

run-yuv_to_rgb_split:
	$(MAKE) -C $(SERVICE_BUILD) run-yuv_to_rgb_split

run-raw_to_rgb_concat:
	$(MAKE) -C $(SERVICE_BUILD) run-raw_to_rgb_concat

run-raw_to_rgb_split:
	$(MAKE) -C $(SERVICE_BUILD) run-raw_to_rgb_split

run-raw_to_yuv_concat:
	$(MAKE) -C $(SERVICE_BUILD) run-raw_to_yuv_concat

run-raw_to_yuv_split:
	$(MAKE) -C $(SERVICE_BUILD) run-raw_to_yuv_split

clean-camera-scenario:
	$(MAKE) -C $(SERVICE_BUILD) clean-camera-scenario

clean-rgb_to_yuv_concat:
	$(MAKE) -C $(SERVICE_BUILD) clean-rgb_to_yuv_concat

clean-rgb_to_yuv_split:
	$(MAKE) -C $(SERVICE_BUILD) clean-rgb_to_yuv_split

clean-yuv_to_rgb_concat:
	$(MAKE) -C $(SERVICE_BUILD) clean-yuv_to_rgb_concat

clean-yuv_to_rgb_split:
	$(MAKE) -C $(SERVICE_BUILD) clean-yuv_to_rgb_split

clean-raw_to_rgb_concat:
	$(MAKE) -C $(SERVICE_BUILD) clean-raw_to_rgb_concat

clean-raw_to_rgb_split:
	$(MAKE) -C $(SERVICE_BUILD) clean-raw_to_rgb_split

clean-raw_to_yuv_concat:
	$(MAKE) -C $(SERVICE_BUILD) clean-raw_to_yuv_concat

clean-raw_to_yuv_split:
	$(MAKE) -C $(SERVICE_BUILD) clean-raw_to_yuv_split

clean-all-scenarios:
	$(MAKE) -C $(SERVICE_BUILD) clean-all-scenarios

