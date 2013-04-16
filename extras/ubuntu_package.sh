#!/bin/bash
###############################################################################
#
# file:     ubuntu_package.sh
#
# Purpose:  Prepares and builds DEB packages.
#
###############################################################################
#
# Copyright 2013 Bruno Braga
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.
#
###############################################################################

base_path="`pwd`/`dirname $0`/.."
cur_dir=`pwd`
package_name="logcat-colorize"

# raring quantal precise oneiric natty maverick lucid karmic jaunty hardy
# invalid (dead) releases: intrepid 
if [ ! "$1" == "" ]; then
    ubuntu_release=$1
else
    ubuntu_release=quantal
fi

if [ ! "$2" = "" ]; then
    retry_suffix=$2
else
    retry_suffix=""
fi

ubuntu_suffix=u1

# get package version
temp=`grep 'VERSION =' logcat-colorize.cpp`
temp=`echo ${temp#*\"}`
package_version=`echo  ${temp%\";}``echo ${ubuntu_release:0:1}`".$retry_suffix"


# Fix changelog for Ubuntu release (make sure DEBFULLNAME and DEBEMAIL are set)
dch -v ${package_version}-1$ubuntu_suffix -b -D $ubuntu_release -u low -M "Packaging for $ubuntu_release release."

temp_dir=/tmp/packaging/
package_dir_name=${package_name}_${package_version}

echo "Removing old garbage..."
rm -rfv $temp_dir
echo "Done"

echo "Creating temporary directory..."
mkdir -p $temp_dir
echo "Done"

echo "Copying project..."
cp -rfv $base_path $temp_dir/$package_dir_name
echo "Done"

cd  $temp_dir/$package_dir_name

echo "Cleaning up..."
rm -rfv .hg*
echo "Done"

echo "Fixing makefile..."
sed -i 's/BINDIR=\/usr\/local/BINDIR=\/usr/' $temp_dir/$package_dir_name/Makefile

echo "Done"

echo "Making original tarball"
mv debian ../
tar -czvf ../$package_dir_name.orig.tar.gz ../$package_dir_name
mv ../debian .
echo "Done"

#
# Fix stuff for Ubuntu
#

# remove quilt
#rm -rfv debian/source/format
#echo "Done"

echo "Building package..."
#dpkg-buildpackage -rfakeroot
debuild -S # can not upload deb to launchpad, only sources
echo "Done"

cd $cur_dir

# Done!
echo "Finished packaging $package_dir_name"

# upload to Launchpad PPA
echo "Type anything to upload to LaunchPad... (will receive a confirmation by email)"
read -n 1
dput -f logcat-colorize /tmp/packaging/${package_dir_name}-1${ubuntu_suffix}_source.changes

