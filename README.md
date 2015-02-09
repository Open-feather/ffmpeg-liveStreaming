ffmpeg-liveStreaming
====================

To output live stream you need install nginx server from source


Download
========
wget http://nginx.org/download/nginx-1.7.9.tar.gz

Unzip
=============
tar -zxvf nginx-1.7.9.tar.gz


Download RTMP module
=====================
wget https://github.com/arut/nginx-rtmp-module/archive/master.zip


Compile NGINX
===============
./configure --with-http_ssl_module --add-module=../nginx-rtmp-module-master
make
sudo make install

Start Server
=============
By default it installs to /usr/local/nginx, so to start the server run the following command:
sudo /usr/local/nginx/sbin/nginx
