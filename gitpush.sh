#!/bin/sh

git status
echo
read -p "Enter commit message: " mess
mess=${mess:-`date +'%d%m:%H%M%S'`}

git add .
git commit -m "$mess"
git push origin master
echo
read -p "Clear?: " clr
clr=${clr:-y}
if [ "$clr" = "y" ]; then
   clear;
fi

