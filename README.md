# torrent-stats

torrent-stats is a high performance program to query UDP trackers and retrieve extensive peer lists with IP addresses of those who are downloading / uploading a particular torrent. It can also retrieve stats pertaining to Seeds, Completed downloads, and Leechs. 
It makes use of C sockets and C++11 threads to send raw hex UDP packets to the tracker, ensuring maximum performance.

## Basic Usage

```
./torrent-stats [40 character infohash]
```

Example:
```
./torrent-stats afdc692052a1344f4b3669507f51b202b25dbc58
Seeds: 2
Downloads: 28
Leechs: 1
From 1 servers.
```

## Detailed usage

torrent-stats uses the local file `trackers.txt`, which contains a list of trackers in `ip:port` format. These are all UDP trackers (the program doesn't support HTTP) and hence protocol isn't specified. 

There is also an included script to convert lists of trackers (such as the one which can be [found here](https://github.com/ngosang/trackerslist/blob/master/trackers_all_udp.txt) ) and convert it to an `ip:port` form, details at the bottom

### Command line options

| Flag | Description |
| --- | --- |
| `-s` | Scrape (default) (instead of announce). Returns Seeds, Downloads and Leechs, by default from first tracker only and single request to the server|
| `-a` | Announce (cannot be used with s). Sends an announce be default to all trackers in the file, and prints a list of all the IPs currently downloading / uploading |
| `-f` | Force scrape request to be sent to all trackers **OR** announce request to be sent to just the first tracker (Seeds / Downloads / Leechs returned bu each tracker are summed up, and may lead to double counting. |
| `-r [number]` | Specify the number of requests to send to each tracker (default 1). Can get more IPs when avg peers > 200. |
| `-t [filename]` | Specify a filename to load trackers from (default `trackers.txt`). Must be UDP in `ip:port` format. |

### Options when announcing

If you want to announce to just the first tracker instead of all, you can specify the `-f` flag.

By default, when a UDP announce request is sent, most trackers send a maximum of 200 peers in the response. For torrents with over 200 peers, multiple requests return different peers in the response. Two requests dont return 400 unique peers, more around 300 (in my minimal testing). You can send multiple requests to the same tracker on seprate threads, using the `-r` parameter. For example:

```
./torrent-stats [hash] -r 10
```

This would send 10 requests to each tracker in the file, and potentially get more unique IPs. Of course, if there are exactly 200 connected peers, then you could set this parameter to 100 and still not see additional IPs. You may benchmark its results and adjust it as per your needs.

Generally this parameter should not be used for torrents with less than 200 peers.

## Script to generate tracker list

The file domain2ip.py is a Python script that can take an input file of the format `udp://domain.com:port/anyqueryparams` and create a file of the format `ip:port`. Useful if you have a list of trackers, such as the one linked above. 

This script automatically ignores http / https trackers, so it's okay if the input is mixed. 
The default input file is `domains.txt` and the default output file is `ips.txt`, though these can be explicitly set as seen below:

#### Script examples

General usage (`domains.txt`) contains trackers with domain name, `ip:port` output stored in `ips.txt`
```
python domain2ip.py
```

Specifying the domains in `abc.txt` and the target file as `outfile.txt`:
```
python domain2ip.py abc.txt outfile.txt
```
