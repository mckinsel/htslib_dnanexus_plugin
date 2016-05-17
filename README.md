[![Build
Status](https://travis-ci.org/mckinsel/htslib_dnanexus_plugin.svg?branch=master)](https://travis-ci.org/mckinsel/htslib_dnanexus_plugin)

DNAnexus HTSlib Plugin
======================

A plugin that allows tools that use HTSlib to interface directly with the
DNAnexus platform.

For example, the plugin enables the following samtools command:

```
samtools view dx:my_remote_alignments.bam 10:2000000-3000000
```

This command will behave just as it would for a local BAM file even though
the BAM file is hosted remotely on DNAnexus. It will first find and download
an index for the BAM file using the usual samtools naming conventions. Then,
using the index, it will request and download only the alignments that
overlap the specified genomic region.

This scheme allows users to work with subsets of a very large BAM file
locally without needing to download the entire file.

Installation
============

The best resource for how to install is probably the `.travis.yml` file in this
repo.

1. Install dependencies for building dx-toolkit. On Ubuntu 14.04, this is

```
sudo apt-get install make python-setuptools python-pip python-virtualenv python-dev g++ cmake libboost1.55-all-dev libcurl4-openssl-dev zlib1g-dev libbz2-dev flex bison autoconf
```

2. Fetch and source dx-toolkit. For Ubuntu 14.04:
```
wget https://wiki.dnanexus.com/images/files/dx-toolkit-current-ubuntu-14.04-amd64.tar.gz
tar xf dx-toolkit-current-ubuntu-14.04-amd64.tar.gz
source dx-toolkit/environment
```

3. Build dxcpp
```
make -C dx-toolkit/share/dnanexus/src/cpp/
```

4. Fetch htslib and set HTSLIB\_ROOT:
```
wget https://github.com/samtools/htslib/releases/download/1.3.1/htslib-1.3.1.tar.bz2
tar xf htslib-1.3.1.tar.bz2
export HTSLIB_ROOT=$(pwd)/htslib-1.3.1
```

5. Build and install htslib. One line needs to be commented out, or downloads
will be very slow:
```
sed -e '/capacity > 32768/ s/^/\/\//' -i htslib-1.3.1/hfile.c
cd htslib-1.3.1 && ./configure --enable-plugins --enable-libcurl && make && sudo make install && cd ..
```

6. Build this plugin and copy it to the htslib plugin directory:
```
make
sudo cp hfile_dnanexus.so /usr/local/libexec/htslib
```

7. Install samtools using the htslib we just built:
```
tar xf samtools-1.3.1.tar.bz2
cd samtools-1.3.1 && ./configure --with-htslib=system --without-curses && make && sudo make install && cd ..
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```


Using the Plugin
================

Once the plugin is in the search path for htslib, then samtools can be used
just as you would with local files. Prefix DNAnexus filenames with "dx:" so
samtools knows to use the DNAnexus htslib plugin when accessing them.
