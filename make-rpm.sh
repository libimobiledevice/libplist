#!/bin/sh
# export the tarball

# update the .spec file with the version number

# upload both files
echo "[general]" > ~/.oscrc
echo "apiurl = https://api.opensuse.org" >> ~/.oscrc
echo "[https://api.opensuse.org]" >> ~/.oscrc
echo "user = qmfrederik" >> ~/.oscrc
echo "pass = $GPG_PASSPHRASE" >> ~/.oscrc

mkdir osc
cd osc
osc checkout home:qmfrederik
cd home\:qmfrederik/libplist
osc rm *
cd ../../../
git archive --format tar.gz -o osc/home\:qmfrederik/libplist/libplist-${LIBPLIST_VERSION_PREFIX}.$TRAVIS_BUILD_NUMBER.tar.gz --prefix libplist-${LIBPLIST_VERSION_PREFIX}.$TRAVIS_BUILD_NUMBER/ HEAD
cd osc/home\:qmfrederik/libplist
cp ../../../libplist.spec .
sed -i "s/${LIBPLIST_VERSION_PREFIX}.build/${LIBPLIST_VERSION_PREFIX}.${TRAVIS_BUILD_NUMBER}/" libplist.spec
osc add *
osc commit -m "Update for Travis CI build $TRAVIS_BUILD_NUMBER"

