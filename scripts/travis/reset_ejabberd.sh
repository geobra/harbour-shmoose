#! /bin/bash
set -x

# remove old db and use new config
sudo apt-get remove --purge ejabberd
sudo mv /bin/hostname /bin/hostname.no
sudo apt-get install ejabberd
sudo mv /bin/hostname.no /bin/hostname
sudo systemctl stop ejabberd
sudo cp ${GITHUB_WORKSPACE}/test/config/ejabberd.yml /etc/ejabberd/
sudo systemctl start ejabberd

# add all the users and rooms for the tests
sudo ejabberdctl register user1 localhost user1
sudo ejabberdctl register user2 localhost user2
sudo ejabberdctl register user3 localhost user3
sudo ejabberdctl create_room testroom conference.localhost localhost


