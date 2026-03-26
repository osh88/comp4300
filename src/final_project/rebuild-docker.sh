#!/bin/sh

sudo docker build -t comp4300_final_project . \
    && sudo docker run --rm -it -v .:/final_project --name comp4300_final_project comp4300_final_project /final_project/rebuild.sh
sudo chown -R $USER:$USER .
