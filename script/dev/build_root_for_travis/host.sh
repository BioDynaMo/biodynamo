#!/bin/bash

# enables GUI apps
xhost +local:root

sudo docker stop travis-14
sudo docker rm travis-14

sudo docker run --name travis-14 --net=host --env="DISPLAY" -dit travisci/ci-garnet:packer-1496954857 /sbin/init

# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

sudo docker cp $SCRIPTPATH/inside_docker.sh travis-14:/
sudo docker exec -it travis-14 chmod +x /inside_docker.sh
sudo docker exec -it travis-14 ./inside_docker.sh

sudo docker cp travis-14:/root/install/root.tar.gz ~/Downloads/root.tar.gz

echo "Build copied to ~/Downloads/root.tar.gz"
