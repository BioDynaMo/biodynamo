#!/bin/bash
# Runs the install_prerequesites_ubuntu_16.04.sh script and performs a build
# afterwards to verify if everything is setup correctly
# Arguments:
#  $1 Branch name that should be used

if [[ $# -ne 1 ]]; then
  echo "Wrong number of arguments.
Usage:
Runs the install_prerequesites_ubuntu_16.04.sh script and performs a build
afterwards to verify if everything is setup correctly
Arguments:
  $1 Branch name that should be used"
  exit 1
fi

BRANCH=$1

# enables GUI apps
xhost +local:root

sudo docker stop ubuntu-1604
sudo docker rm ubuntu-1604

sudo docker run --name ubuntu-1604 --net=host --env="DISPLAY" -dit ubuntu:16.04 /sbin/init

# get path of this script
pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

sudo docker cp $SCRIPTPATH/inside_docker_setup.sh ubuntu-1604:/
sudo docker exec -it ubuntu-1604 chmod +x /inside_docker_setup.sh
sudo docker exec -it ubuntu-1604 ./inside_docker_setup.sh

sudo docker cp $SCRIPTPATH/inside_docker_install_verify.sh ubuntu-1604:/home/testuser/
sudo docker exec -it ubuntu-1604 chmod +x /home/testuser/inside_docker_install_verify.sh
sudo docker exec -it --user testuser ubuntu-1604 /home/testuser/inside_docker_install_verify.sh $BRANCH

RETURN_VAL=$?
if [ "$RETURN_VAL" == "0" ]; then
  echo "Install test successful"
  exit $RETURN_VAL
else
  echo "Install test FAILED"
fi
