DATE="$(date '+%Y-%m-%d_%H-%M-%S')"
g++ *.cpp Engine/*.cpp -L. -lSDL2 -lSDL2main -lSDL2_image -o Builds/build-"$DATE"
./Builds/build-"$DATE"
