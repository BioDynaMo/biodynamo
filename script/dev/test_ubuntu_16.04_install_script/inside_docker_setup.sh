#!/bin/bash

#install git to fetch repository
apt-get update
apt-get install -y git
apt-get install -y sudo
apt-get install -y man  # required by ROOT

# create non priviledged user
useradd -m -c "Testuser" testuser
passwd --delete testuser
# adduser testuser sudo
echo "testuser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
