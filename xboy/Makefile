TARGET  = gpsp
CC      = /home/hsinyuwang/v3s/buildroot-2019.08/output/host/bin/arm-linux-gnueabihf-gcc
VPATH   = .. ../arm
OBJS    = main.o cpu.o memory.o video.o input.o sound.o cheats.o zip.o  arm_stub.o  warm.o cpu_threaded.o video_blend.o sha1.o imageio.o gui_xboy.o #gui.o
CFLAGS  = -DARM_ARCH -DPC_BUILD -march=armv7-a -mtune=cortex-a7 -mfloat-abi=hard -mfpu=neon -ffast-math -Ofast `/home/hsinyuwang/v3s/buildroot-2019.08/output/host/arm-buildroot-linux-gnueabihf/sysroot/usr/bin/sdl-config --cflags`
LIBS    = -lSDL -lSDL_ttf -lz -lpng

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(TARGET)

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(TARGET)
