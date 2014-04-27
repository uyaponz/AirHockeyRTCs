all:
	g++ `pkg-config --libs --cflags opencv` -o camera_calib camera_calib.c
	g++ -lgpg5300 `pkg-config --libs --cflags opencv` -o capture_file capture_file.c
