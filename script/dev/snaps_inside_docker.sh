sudo docker run \
    --net=host \
    --name=mytest \
    --privileged \
    -ti \
    --tmpfs /run \
    --tmpfs /run/lock \
    --tmpfs /tmp \
    --cap-add SYS_ADMIN \
    --device=/dev/fuse \
    --security-opt apparmor:unconfined \
    --security-opt seccomp:unconfined \
    -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
    -v /lib/modules:/lib/modules:ro \
    -d ubuntu:16.04 /sbin/init

# inside docker run
apt-get update &&\
 DEBIAN_FRONTEND=noninteractive\
 apt-get install -y fuse snapd snap-confine squashfuse sudo &&\
 apt-get clean &&\
 dpkg-divert --local --rename --add /sbin/udevadm &&\
 ln -s /bin/true /sbin/udevadm
systemctl enable snapd
# wait 60s for snapd to startup

# https://bugs.launchpad.net/ubuntu/+source/snapd/+bug/1631514
systemctl restart snapd.service


# sudo docker run \
#     --net=host \
#     --name=travis-snaps \
#     -ti \
#     --tmpfs /run \
#     --tmpfs /run/lock \
#     --tmpfs /tmp \
#     --cap-add SYS_ADMIN \
#     --device=/dev/fuse \
#     --security-opt apparmor:unconfined \
#     --security-opt seccomp:unconfined \
#     -v /sys/fs/cgroup:/sys/fs/cgroup:ro \
#     -v /lib/modules:/lib/modules:ro \
#     -d travisci/ci-garnet:packer-1496954857 /sbin/init
