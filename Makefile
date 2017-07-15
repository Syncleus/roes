CODE_DIR = src

.PHONY: all

all:
	$(MAKE) -C $(CODE_DIR)

clean:
	$(MAKE) -C $(CODE_DIR) clean
