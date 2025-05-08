#!/bin/bash
# This script fixes yum install on centos 7 machines so that we can install new packages.
# changes remote server for yum to pull updates from
# packages are definitely not on the latest version, but atleast its something.
set -ex
# PACKAGES="gdb"

sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-*.repo
sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*.repo
yum clean all; yum makecache
yum update -y && yum install gdb -y
