BUILD			 = build
OBJ    		 = $(patsubst %.c,%.o,$(wildcard *.cpp))
TESTS  		 = $(wildcard t/*.lisp)
LLVM   		 = $(shell llvm-config  --cxxflags --ldflags --libs core)

# Size of the expression table
EXPRSZ 		 ?= 1024

# Size of the root expression table
ROOTEXPRSZ ?= 1024

# Size of the token table
TOKSZ  		 ?= 1024

MAXFILESZ  ?= 4096

CFLAGS 		 = -DEXPRSZ=$(EXPRSZ) -DROOTEXPRSZ=$(ROOTEXPRSZ) -DTOKSZ=$(TOKSZ) \
						 -DMAXFILESZ=$(MAXFILESZ) -g

LDFLAGS 	 = $(LLVM)
CXFLAGS 	 = $(LLVM) -std=c++20

all: $(OBJ)
	$(CXX) $(OBJ) $(CFLAGS) $(LDFLAGS) $(CXFLAGS) -o lc

%.o: %.cpp
	$(CXX) $< $(CFLAGS) $(CXFLAGS) -c -o $@

%.lisp: all
	./lc $@

check: $(TESTS)

dirs:
	[ -d $(BUILD) ] || mkdir -p $(BUILD)

clean:
	[ -f lc ] && rm lc
