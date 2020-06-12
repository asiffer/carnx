# Protocol Documentation
<a name="top"></a>

## Table of Contents

- [api/carnx.proto](#api/carnx.proto)
    - [AttachParameters](#api.AttachParameters)
    - [AttachStatus](#api.AttachStatus)
    - [CounterID](#api.CounterID)
    - [CounterList](#api.CounterList)
    - [CounterName](#api.CounterName)
    - [CounterValue](#api.CounterValue)
    - [Garbage](#api.Garbage)
    - [LoadAttachParameters](#api.LoadAttachParameters)
    - [LoadParameters](#api.LoadParameters)
    - [LoadStatus](#api.LoadStatus)
    - [NbCounters](#api.NbCounters)
    - [ReturnCode](#api.ReturnCode)
    - [Snap](#api.Snap)
    - [Snap.DataEntry](#api.Snap.DataEntry)
  
    - [Carnx](#api.Carnx)
  
- [Scalar Value Types](#scalar-value-types)



<a name="api/carnx.proto"></a>
<p align="right"><a href="#top">Top</a></p>

## api/carnx.proto



<a name="api.AttachParameters"></a>

### AttachParameters



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| interface | [string](#string) |  | Name of the network interface |
| xdp_flags | [uint32](#uint32) |  | XDP attaching flags |






<a name="api.AttachStatus"></a>

### AttachStatus



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| attached | [bool](#bool) |  | Status of the BPF program |






<a name="api.CounterID"></a>

### CounterID



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| id | [uint32](#uint32) |  | The raw ID of a counter |






<a name="api.CounterList"></a>

### CounterList



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| counters | [string](#string) | repeated | A list of counter names |






<a name="api.CounterName"></a>

### CounterName



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| name | [string](#string) |  | The name of the counter |






<a name="api.CounterValue"></a>

### CounterValue



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| value | [uint64](#uint64) |  | The Value of the counter |






<a name="api.Garbage"></a>

### Garbage







<a name="api.LoadAttachParameters"></a>

### LoadAttachParameters



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| interface | [string](#string) |  | Name of the network interface |
| xdp_flags | [uint32](#uint32) |  | XDP attaching flags |
| bpf_program | [string](#string) |  | Path to the eBPF program |






<a name="api.LoadParameters"></a>

### LoadParameters



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| bpf_program | [string](#string) |  | Path of the BPF program |






<a name="api.LoadStatus"></a>

### LoadStatus



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| loaded | [bool](#bool) |  | Status of the BPF program |






<a name="api.NbCounters"></a>

### NbCounters



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| nb_counters | [uint32](#uint32) |  | The number of available counters |






<a name="api.ReturnCode"></a>

### ReturnCode



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| code | [int32](#int32) |  | Internal function return code |






<a name="api.Snap"></a>

### Snap



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| data | [Snap.DataEntry](#api.Snap.DataEntry) | repeated | Counter Name-&gt;Value mapping |






<a name="api.Snap.DataEntry"></a>

### Snap.DataEntry



| Field | Type | Label | Description |
| ----- | ---- | ----- | ----------- |
| key | [string](#string) |  |  |
| value | [uint64](#uint64) |  |  |





 

 

 


<a name="api.Carnx"></a>

### Carnx


| Method Name | Request Type | Response Type | Description |
| ----------- | ------------ | ------------- | ------------|
| GetNbCounters | [Garbage](#api.Garbage) | [NbCounters](#api.NbCounters) | GetNbCounters returns the number of counters |
| Ping | [Garbage](#api.Garbage) | [Garbage](#api.Garbage) | Ping aims to check the connection |
| GetCounter | [CounterID](#api.CounterID) | [CounterValue](#api.CounterValue) | GetCounter returns the value of a counter given its key |
| GetCounterByName | [CounterName](#api.CounterName) | [CounterValue](#api.CounterValue) | GetCounterByName returns the value of a counter given its name |
| GetCounterNames | [Garbage](#api.Garbage) | [CounterList](#api.CounterList) | GetCounterNames returns the list of the counters (in the right order) |
| Snapshot | [Garbage](#api.Garbage) | [Snap](#api.Snap) | Snapshot returns the current values of the counters |
| Load | [LoadParameters](#api.LoadParameters) | [ReturnCode](#api.ReturnCode) | Load an eBPF program into the kernel |
| LoadAndAttach | [LoadAttachParameters](#api.LoadAttachParameters) | [ReturnCode](#api.ReturnCode) | LoadAndAttach aims to init the XDP program. It loads the program into the kernel and attach it to the given interface with the given flags |
| Unload | [Garbage](#api.Garbage) | [ReturnCode](#api.ReturnCode) | Unload the eBPF program from the kernel |
| Attach | [AttachParameters](#api.AttachParameters) | [ReturnCode](#api.ReturnCode) | Attach the XDP program onto the given interface |
| Detach | [Garbage](#api.Garbage) | [ReturnCode](#api.ReturnCode) | Detach the XDP program from the interface previously given |
| IsLoaded | [Garbage](#api.Garbage) | [LoadStatus](#api.LoadStatus) | IsLoaded check if the program is loaded into the kernel |
| IsAttached | [Garbage](#api.Garbage) | [AttachStatus](#api.AttachStatus) | IsAttached check if the program is attached to the interface |

 



## Scalar Value Types

| .proto Type | Notes | C++ | Java | Python | Go | C# | PHP | Ruby |
| ----------- | ----- | --- | ---- | ------ | -- | -- | --- | ---- |
| <a name="double" /> double |  | double | double | float | float64 | double | float | Float |
| <a name="float" /> float |  | float | float | float | float32 | float | float | Float |
| <a name="int32" /> int32 | Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint32 instead. | int32 | int | int | int32 | int | integer | Bignum or Fixnum (as required) |
| <a name="int64" /> int64 | Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint64 instead. | int64 | long | int/long | int64 | long | integer/string | Bignum |
| <a name="uint32" /> uint32 | Uses variable-length encoding. | uint32 | int | int/long | uint32 | uint | integer | Bignum or Fixnum (as required) |
| <a name="uint64" /> uint64 | Uses variable-length encoding. | uint64 | long | int/long | uint64 | ulong | integer/string | Bignum or Fixnum (as required) |
| <a name="sint32" /> sint32 | Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int32s. | int32 | int | int | int32 | int | integer | Bignum or Fixnum (as required) |
| <a name="sint64" /> sint64 | Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int64s. | int64 | long | int/long | int64 | long | integer/string | Bignum |
| <a name="fixed32" /> fixed32 | Always four bytes. More efficient than uint32 if values are often greater than 2^28. | uint32 | int | int | uint32 | uint | integer | Bignum or Fixnum (as required) |
| <a name="fixed64" /> fixed64 | Always eight bytes. More efficient than uint64 if values are often greater than 2^56. | uint64 | long | int/long | uint64 | ulong | integer/string | Bignum |
| <a name="sfixed32" /> sfixed32 | Always four bytes. | int32 | int | int | int32 | int | integer | Bignum or Fixnum (as required) |
| <a name="sfixed64" /> sfixed64 | Always eight bytes. | int64 | long | int/long | int64 | long | integer/string | Bignum |
| <a name="bool" /> bool |  | bool | boolean | boolean | bool | bool | boolean | TrueClass/FalseClass |
| <a name="string" /> string | A string must always contain UTF-8 encoded or 7-bit ASCII text. | string | String | str/unicode | string | string | string | String (UTF-8) |
| <a name="bytes" /> bytes | May contain any arbitrary sequence of bytes. | string | ByteString | str | []byte | ByteString | string | String (ASCII-8BIT) |

