subdirs = $(wildcard */)

all:
	$(foreach test, $(subdirs), $(MAKE) -C $(test) all;)

bin:
	$(foreach test, $(subdirs), $(MAKE) -C $(test) bin;)

clean:
	$(foreach test, $(subdirs), $(MAKE) -C $(test) clean;)
