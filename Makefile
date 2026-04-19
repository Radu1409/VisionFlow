SERVICE_BUILD := service/build

.PHONY: all run clean

all:
	$(MAKE) -C $(SERVICE_BUILD)

run:
	$(MAKE) -C $(SERVICE_BUILD) run

clean:
	$(MAKE) -C $(SERVICE_BUILD) clean

