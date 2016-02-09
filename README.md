# kea_hooks

## Options to Options
This hook takes the value from different options in a DHCPREQUEST and inserts them into (other) options in a DHCPREPLY

Example usage is to take the value from an Option 82 string, and insert it in an Option 43 sub option, to direct the dhcp client to the correct config file in an auto provisioning scenario.
The config below will take the string from sub option 002 in an option 82 request, and add it as part of the path/filename in sub option 001 in the Option43 reply.

The DHCPREQUEST comes in with an Option 82:
<pre>
(...)
  type=082, len=044:,
options:
    type=001, len=019: "Router01:ge-0/0/4:1" (string)
    type=002, len=017: "<span style='color: green;'>customer-id-10000</span>" (string)
</pre>

And the DHCPREPLY will contain

<pre>
(...)
  type=043, len=056:,
options:
    type=001, len=032: "path/to/<span style='color: green;'>config-customer-id-10000</span>" (string)
</pre>


## Build
```
g++ -std=c++11 -I /usr/include/kea -L /usr/lib64 -fpic -shared -o options_to_options.so load_unload.cc pkt4_send.cc version.cc -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions
```


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
		"data": "path/to/config-@OPTION_82_2@"
	},

```
