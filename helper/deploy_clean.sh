#!/bin/bash

function replace_gitlab_ssh_key () {
# download ssh private key for download GVDS source code
curl ftp://10.134.150.155/GVDS/id_rsa_gitlab_buaaica -o ~/.ssh/id_rsa_gitlab_buaaica --user "ftpuser:ftpuser"
chmod 400 ~/.ssh/id_rsa_gitlab_buaaica

cat <<EOF > ~/.ssh/config
host gitlab.com
HostName gitlab.com
IdentityFile ~/.ssh/id_rsa_gitlab_buaaica
User git

EOF
}

# DANGEROUS!! It Would modify your ~/.ssh/ssh_config!
while true; do
    read -p "Do you wish to append gitlab private key to your ssh config file?[y/n]" yn
    case $yn in
        [Yy]* ) replace_gitlab_ssh_key; break;;
        [Nn]* ) break;;
        * ) echo "Please answer Y/y or N/n.";;
    esac
done

sudo mkdir -p /opt/hvs
sudo chmod -R `whoami`:`whoami` /opt/hvs
git clone git@gitlab.com:buaaica/hvs-one.git /opt/hvs
