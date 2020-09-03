# define CPPFLAGS=-I... for other (system) includes
# define LDFLAGS=-L... for other (system) libs to link

CC = g++ -g -Wfloat-conversion -Wreturn-type -Wunused-function -Wreorder -Wunused-variable

CC_DEBUG = @$(CC) -std=c++11
CC_RELEASE = @$(CC) -std=c++11 -O3 -DNDEBUG

G_SRC = src/*.cpp *.cpp

G_INC = -Iinclude -Iapps $(CPPFLAGS)

G_LINK = $(LDFLAGS)

all: image

image : $(G_SRC) apps/image.cpp apps/image_recs.cpp
	$(CC_DEBUG) $(G_INC) $(G_SRC) apps/image.cpp apps/image_recs.cpp -o image

DRAW_SRC = apps/draw.cpp apps/GWindow.cpp apps/GTime.cpp
draw: $(DRAW_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(G_LINK) $(DRAW_SRC) -lSDL2 -o draw

VIEWER_SRC = apps/viewer.cpp apps/GWindow.cpp apps/GTime.cpp apps/image_recs.cpp
viewer: $(VIEWER_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(G_LINK) $(VIEWER_SRC) -lSDL2 -o viewer


BOUNCE_SRC = apps/bounce.cpp apps/GWindow.cpp apps/GTime.cpp
bounce: $(BOUNCE_SRC) $(G_SRC)
	$(CC_RELEASE) $(G_INC) $(G_SRC) $(G_LINK) $(BOUNCE_SRC) -lSDL2 -o bounce


clean:
	@rm -rf image draw viewer bounce *.png *.dSYM

