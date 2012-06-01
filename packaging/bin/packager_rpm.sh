#!/bin/bash
## help tool for generic packaging
# @author: Devresse Adrien

## vars
BASE_TMP=/tmp
FINAL_DIR="RPMS/"

TARBALL_FILTER="--exclude='.git' --exclude='.svn' --exclude='RPMS' --exclude='*.rpm'"
FOO="HELLO"

set -e


create_tmp_dir() {
	export TMP_DIR="$BASE_TMP/$RANDOM$RANDOM$RANDOM_$$"
	mkdir -p $TMP_DIR
}

get_attrs_spec(){
	export SPEC_CONTENT="$(rpm -E "`cat $1`")"
	export PKG_VERSION="$( echo "$SPEC_CONTENT" | grep "Version:"  | sed 's@Version:\(.*\)@\1@g' | sed -e 's/^[ \t]*//')"
	export PKG_RELEASE="$( echo "$SPEC_CONTENT" | grep "Version:"  | sed 's@Release:\(.*\)@\1@g' | sed -e 's/^[ \t]*//')"	
	export PKG_NAME="$(echo "$SPEC_CONTENT" | grep "Name:" | sed 's@Name:\(.*\)@\1@g' | sed -e 's/^[ \t]*//')"
	export SPEC_CONTENT="$(echo "$SPEC_CONTENT" | sed "s/%{name}/$PKG_NAME/g" | sed "s/%{version}/$PKG_VERSION/g" | sed "s/%{Release}/$PKG_RELEASE/g")"
	export PKG_SOURCE="$( echo "$SPEC_CONTENT" | grep "Source0:"  | sed 's@Source0:\(.*\)@\1@g' )"
	export PKG_SOURCE="$( echo $PKG_SOURCE | awk -F/ '{print $NF'})"
	export SRC_NAME="$PKG_SOURCE"
	echo "res : $SRC_NAME $PKG_VERSION $PKG_NAME $PKG_SOURCE"
}

# src_dir, tarbal_filepath
create_tarball(){
	create_tmp_dir
	SRC_FOLDER="/$TMP_DIR/$PKG_NAME-$PKG_VERSION"
	mkdir -p "$SRC_FOLDER"
	echo "copy files..."
	cp -r $1/* $SRC_FOLDER/
	CURRENT_DIR=$PWD
	cd $TMP_DIR
	echo "copy files..."
	eval "tar -cvzf $2 $TARBALL_FILTER $PKG_NAME-$PKG_VERSION"
	echo "tarball result : $2 $TARBALL_FILTER "
	cd $CURRENT_DIR
	rm -rf $TMP_DIR
}

# specfiles_dir 
create_rpmbuild_env(){
	create_tmp_dir
	export RPM_BUILD_DIR="$TMP_DIR"	
	mkdir -p $RPM_BUILD_DIR/RPMS $RPM_BUILD_DIR/SOURCES $RPM_BUILD_DIR/BUILD $RPM_BUILD_DIR/SRPMS $RPM_BUILD_DIR/SPECS $RPM_BUILD_DIR/tmp
	cp $1/* $RPM_BUILD_DIR/SPECS/
	
}

# specfiles_dir 
delete_rpmbuild_env(){
	rm -rf $RPM_BUILD_DIR/
	
}


# specfile
rpm_build_src_package(){
	echo "Begin the rpmbuild source call for spec file $1 ...."
	local OLD_DIR=$PWD
	local MACRO_TOPDIR="s  \"_topdir $RPM_BUILD_DIR\""
	cd $RPM_BUILD_DIR
	ls $PWD/SOURCES/
	rpmbuild -bs --nodeps --define "_topdir $RPM_BUILD_DIR" SPECS/$1
	cd $OLD_DIR
	echo "End the rpmbuild source call...."	
}



## main
if [[ "$1" == "" || "$2" == "" ]]; then
	echo "Usage $0 [spec_dir] [src_dir] "
	exit -1
fi

create_rpmbuild_env $1
mkdir -p SRPMS

# list spec file
for i in $1/*.spec 
do
	echo " create RPMS for spec file : $i"
	get_attrs_spec $i
	echo "Source : $SRC_NAME"
	echo "Version : $PKG_VERSION"
	echo "Name: $PKG_NAME"	
	echo "create source tarball..."
	create_tarball $2 "$RPM_BUILD_DIR/SOURCES/$SRC_NAME"
	echo "TARBALL: $RPM_BUILD_DIR/SOURCES/$SRC_NAME"
	rpm_build_src_package `basename $i` 
done
mkdir -p  $FINAL_DIR
cp $RPM_BUILD_DIR/SRPMS/* $FINAL_DIR
## clean everything
delete_rpmbuild_env
	
