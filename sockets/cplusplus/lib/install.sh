#!/bin/bash 

set -e      # exit when any command fails

exit_on_error() {
    exit_code=$1
    last_command=${@:2}
    if [ $exit_code -ne 0 ]; then
        >&2 echo "\"${last_command}\" command failed with exit code ${exit_code}."
        exit exit_code
    fi
}

# include directory location
includeDir="/usr/local/include/mysocket"

# build files and suppress make output
make -s
exit_on_error $? !!

### create the directory if it does not exist ###
if [ ! -d $includeDir ]
then
    sudo mkdir $includeDir
fi

# install
sudo make install 

# configure
sudo ldconfig -n /usr/local/lib || exit_on_error "ldconfig failed"

# clean
rm -f *.o *.so.1.0 

# add to path
echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc

# restart bash
source ~/.bashrc
 
