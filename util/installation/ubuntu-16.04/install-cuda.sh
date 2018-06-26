#!/bin/bash

sudo -v
wget https://developer.nvidia.com/compute/cuda/9.2/Prod/local_installers/cuda-repo-ubuntu1604-9-2-local_9.2.88-1_amd64-deb
sudo dpkg -i cuda-repo-ubuntu1604-9-2-local_9.2.88-1_amd64-deb
sudo apt-key add /var/cuda-repo-<version>/7fa2af80.pub
sudo apt-get update
sudo apt-get install cuda
