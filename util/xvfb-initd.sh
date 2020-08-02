#!/bin/bash
# Script taken from docker image: travisci/ci-garnet:packer-1496954857

XVFB=/usr/bin/Xvfb
XVFBARGS=":99 -ac -screen 0 2560x1440x24"
PIDFILE=/tmp/cucumber_xvfb_99.pid
OS_ID=$(grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"')
case "$1" in
  start)
    echo -n "Starting virtual X frame buffer: Xvfb"
    if [ ${OS_ID} != "centos" ]; then
	     /sbin/start-stop-daemon --start --quiet --pidfile $PIDFILE --make-pidfile --background --exec $XVFB -- $XVFBARGS
    elif [ "$(ps -ef | grep "$XVFB" | wc -l)" != "2" ]; then
      $XVFB $XVFBARGS &
    fi
    echo "."
    ;;
  stop)
    echo -n "Stopping virtual X frame buffer: Xvfb"
    if [ ${OS_ID} != "centos" ]; then
      /sbin/start-stop-daemon --stop --quiet --pidfile $PIDFILE
    else
      pkill -f "$XVFB"
      sleep 5
    fi
    rm -f $PIDFILE
    echo "."
    ;;
  restart)
    $0 stop
    $0 start
    ;;
  *)
  echo "Usage: xvfb-initd.sh {start|stop|restart}"
  exit 1
esac
exit 0
