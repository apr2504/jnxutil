## jnxutil
Garmin jnx file manipulation utility for Linux. 
### Features: 
* Read content of the jnx file. 
* Edit header and map, level description blocks. 
* Export tiles to GeoTIFF format files. (requires GDAL for build). 
### Building:
* cmake -DWITHOUT_GDAL=1 -DCMAKE_INSTALL_PREFIX="/usr" ./ # export tiles disabled
<br />OR<br />
* cmake -DCMAKE_INSTALL_PREFIX="/usr" ./  # export tiles enabled
* make
* sudo make install
<br />OR<br />
* open jnxutil.cbp in CodeBlocks
