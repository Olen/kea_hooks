# kea_hooks

## Build
g++ -std=c++11 -I /usr/include/kea -L /usr/lib64 -fpic -shared -o options_to_options.so load_unload.cc pkt4_send.cc version.cc -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions


## Example config

```
	"option-def": [
	{
		"name": "config-file-name",
		"code": 1,
		"space": "vendor-encapsulated-options-space",
		"type": "string",
		"array": false,
		"record-types": "",
		"encapsulate": ""
	},

(...)
  
	"option-data": [
	{
		"name": "vendor-encapsulated-options",
		"csv-format": false
	},
	{
		"name": "config-file-name",
		"code": 1,
		"space": "vendor-encapsulated-options-space",
		"csv-format": true,
		"data": "path/to/config-@OPTION82_2@"
	},

```
