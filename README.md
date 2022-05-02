# kea_hooks

## Options to Options

***UPDATE***
MOST of the functionality in this hook has since been replaced by a built in hook:
https://kea.readthedocs.io/en/latest/arm/hooks.html#flex-option-flexible-option-for-option-value-settings




This hook takes the value from different options in a DHCPREQUEST and inserts them into (other) options in a DHCPREPLY

Example usage is to take the value from an Option 82 string, and insert it in an Option 43 sub option, to direct the dhcp client to the correct config file in an auto provisioning scenario.
The config below will take the string from sub option 002 in an option 82 request, and add it as part of the path/filename in sub option 001 in the Option43 reply.

## Example config

```
	"hooks-libraries": [{
		"library": "/full/path/to/options_to_options.so"
	}],

(...)
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
		"name": "boot-file-name",
		"data": "path/to/config-@HWADDR_CISCO@"
	},
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

### Results in

The DHCPREQUEST comes in with an Option 82:
<pre>
(...)
  type=082, len=044:,
options:
    type=001, len=019: "Router01:ge-0/0/4:1" (string)
    type=002, len=017: "<b>customer-id-10000</b>" (string)
</pre>

And the DHCPREPLY will contain

<pre>
(...)
  type=043, len=056:,
options:
    type=001, len=032: "path/to/config-<b>customer-id-10000</b>" (string)
</pre>
and
<pre>
  type=067, len=029: "path/to/config-<b>ccc1.1234.2d41</b>" (string)
</pre>

### Placeholder / variables
The following placeholders or "variables" are currently defined

* @GIADDR@ => giaddr.  <em>172.16.1.1</em> (see special note below)
* @HWADDR@ => hardware address of client in "normal" (colon separated) format: <em>cc:c1:12:34:2d:41</em>
* @HWADDR_CISCO@ => hardware address in "cisco format": <em>ccc1.1234.2d41</em>
* @HWADDR_WINDOWS@ => hardware address in "windows format" (dash separated): <em>cc-c1-12-34-2d-41</em>
* @IPADDR@ => IP address assigned to client: <em>172.16.1.100</em>
* @IPADDR_HEX@ = IP address assigned to client in hex: <em>ac100164</em>
* @OPTION_82_1@ = Value of sub option 1 in Option 82 ("Agent Circuit ID") (sanitized): <em>Router01_ge-0_0_4_1</em>
* @OPTION_82_2@ = Value of sub option 2 in Option 82 ("Agent Remote ID") (sanitized): <em>customer-id-100000</em>
* @OPTION_82_6@ = Value of sub option 6 in Option 82 ("Subscriber-ID") (sanitized): <em>customer-id-100000</em>

### Note about giaddr
The hook also supports a special case where you can automatically insert the "giaddr" as default router in the DHCP-reply.  Simply set Option 3 ("routers") to "0.0.0.0" in the kea-config, and that value will be replaced by the data from "giaddr" in the request (if it is set).

This might sound like a strange thing to do, but it allows you to greatly simplify the config for certain cases.  You can have *one* common subnet defined for several locations (behind different routers, acting as dhcp-relays), and each device will get the correct default gateway.  This might be feasible in situations where the clients are not supposed to connect to each other - like provisioning of CPEs or other devices.


## Build
install kea-devel boost-devel gcc-c++ packages

Then run
```
$ libtoolize
$ aclocal
$ autoconf 
$ automake --add-missing
$ ./configure
$ make

# cp .lib/options_to_options.so /path/to/kea/plugins/
```


