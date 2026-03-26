#!/bin/sh

xhost +local: \
    && sudo docker build -t comp4300_final_project . \
    && sudo docker run --rm -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:ro -v .:/final_project \
        --name comp4300_final_project comp4300_final_project /final_project/run.sh
sudo chown -R $USER:$USER .
