#!/bin/bash
# OPTIONS:
#   $1 install directory

if [[ $# -ne 1 ]]; then
  echo "This script requires one argument: "
  echo "OPTIONS: "
  echo "  $1 install directory"
  exit
fi

cmake --build .. --target all
sudo cmake --build .. --target install

if [ ! -f root_v6.11.01_Linux-ubuntu16-x86_64-gcc5.4_263508429d.tar.gz ]; then
  wget -O root_v6.11.01_Linux-ubuntu16-x86_64-gcc5.4_263508429d.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=root_v6.11.01_Linux-ubuntu16-x86_64-gcc5.4_263508429d.tar.gz"
fi

if [ ! -f Qt5.9.1_ubuntu16_gcc5.4.tar.gz ]; then
  wget -O Qt5.9.1_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=Qt5.9.1_ubuntu16_gcc5.4.tar.gz"
fi

if [ ! -f paraview-5.4.1_ubuntu16_gcc5.4.tar.gz ]; then
  wget -O paraview-5.4.1_ubuntu16_gcc5.4.tar.gz "https://cernbox.cern.ch/index.php/s/1ekMudArMqFhcXt/download?path=%2F&files=paraview-5.4.1_ubuntu16_gcc5.4.tar.gz"
fi

# create run command
mkdir -p bin
echo '#!/bin/bash' > bin/run
echo '# execute command given as first parameter' >> bin/run
echo '"$@"' >> bin/run
chmod +x bin/run

sudo docker pull snapcore/snapcraft
echo "Start building snap package"
sudo docker run --net=host -v $PWD:$PWD -v $1:$1 -e SNAPCRAFT_SETUP_CORE=1 -w $PWD snapcore/snapcraft snapcraft -d
