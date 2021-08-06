CC=gcc
CFLAGS=-I. -lpthread
DEPS = cbuf.h s2ds-buffer.h
EXE = S2DS.out
OBJS = cbuf.o s2ds-buffer.o
OBJS_DIR = .objs

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)
	
$(OBJS_DIR)/%.o: %.c | $(OBJS_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)
		
$(EXE): $(OBJS:%.o=$(OBJS_DIR)/%.o)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	-rm -rf .objs $(EXE)
