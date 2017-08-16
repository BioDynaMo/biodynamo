#!/bin/bash

#install git to fetch repository
apt-get update
apt-get install -y git

# create non priviledged user
useradd -m -c "Testuser" testuser
passwd --delete testuser
apt-get install -y sudo
adduser testuser sudo
