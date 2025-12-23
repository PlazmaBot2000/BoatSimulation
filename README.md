# Dependencies:
- SDL2
- toml++

# How to build:
```g++ *.cpp Engine/*.cpp -L. -lSDL2 -lSDL2main -lSDL2_image -o Builds/build-$(date '+%Y-%m-%d_%H-%M-%S')``` \
or run ```build-and-run.sh```. Also if you want just run your build open folder ```Builds``` and run your build.
