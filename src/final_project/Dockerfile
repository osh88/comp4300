# Run:
# xhost +local: && sudo docker build -t comp4300_final_project . && sudo docker run --rm -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:ro -v .:/final_project --name comp4300_final_project comp4300_final_project && sudo chown -R $USER:$USER .
#
# Build:
# sudo docker build -t comp4300_final_project . && sudo docker run --rm -it -v .:/final_project --name comp4300_final_project comp4300_final_project && sudo chown -R $USER:$USER .

FROM xianpengshen/clang-tools:22
ENV CXX=/usr/bin/clang-22

RUN apt-get update
RUN apt-get install -y cmake libgl1-mesa-dev mesa-common-dev liblzma-dev libvorbis-dev libfreetype6-dev libxrandr2 libxcursor1 libxi6

WORKDIR /final_project

#CMD ["/final_project/rebuild.sh"]
#CMD ["/final_project/run.sh"]
CMD ["/usr/bin/bash"]
