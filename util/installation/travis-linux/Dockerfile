# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

FROM travisci/ci-garnet:packer-1496954857

# preserve the environment variable BDM_LOCAL_LFS when calling scripts with
# sudo
RUN echo "Defaults env_keep += \"BDM_LOCAL_LFS\"" | sudo tee -a /etc/sudoers

ENV TRAVIS=1
ENV TRAVIS_OS_NAME="linux"

RUN git config --system user.name "Test User" && \
    git config --system user.email user@test.com

# update user id and group id such that mapped volumes can be accessed with the
# same rights as on the host. Files created by the container can also be
# accessed on the host without chowning.
ARG HOST_UID
ARG HOST_GID

RUN DOCKER_USER=travis && \
    OLD_UID=2000 && \
    OLD_GID=2000 && \
    usermod -u $HOST_UID $DOCKER_USER && \
    groupmod -g $HOST_UID $DOCKER_USER && \
    sudo rm -rf /home/$DOCKER_USER && \
    mkhomedir_helper $DOCKER_USER && \
    chown travis /home/$DOCKER_USER && \
    chgrp travis /home/$DOCKER_USER

USER travis
