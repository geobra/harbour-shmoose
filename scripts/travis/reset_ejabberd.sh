#! /bin/bash
set -x

# remove old db and use new config
sudo systemctl stop ejabberd
sudo cp ${GITHUB_WORKSPACE}/test/config/ejabberd.yml /etc/ejabberd/
sudo rm -rf /var/lib/ejabberd/*
sudo systemctl start ejabberd

# add all the users and rooms for the tests
sudo ejabberdctl register user1 localhost user1
sudo ejabberdctl register user2 localhost user2
sudo ejabberdctl register user3 localhost user3
sudo ejabberdctl create_room testroom conference.localhost localhost


