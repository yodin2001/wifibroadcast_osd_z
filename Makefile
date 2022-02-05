CPPFLAGS+= -I/opt/vc/include/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -DMAVLINK
LDFLAGS+= -lfreetype -lz
LDFLAGS+=-L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -lm -lshapes

all: osd

build/%.o: %.c
	gcc -c -o $@ $< $(CPPFLAGS)

osd: build/main.o build/render.o build/telemetry.o build/frsky.o build/ltm.o build/mavlink.o build/smartport.o build/vot.o
	gcc -o build/$@ $^ $(LDFLAGS)

clean:
	rm -f build/osd build/*.o build/*~
