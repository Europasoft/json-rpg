### JSON without overhead
#### This compact library provides functions for loading, manipulating, and saving [JSON](https://www.json.org/json-en.html) data at maximum throughput.
High performance and reliability achieved by:
- applying modern C++ design practices for top efficiency.
- not using any third-party libraries, eliminating integration complexity.
- keeping the code concise and readable, to ease in debugging and optimize compile times.

It maintains **[RFC 8259](https://datatracker.ietf.org/doc/html/rfc8259)**-compliance (or as close to it as possible, while remaining practical).

Integration with a **[glTF](https://github.com/KhronosGroup/glTF)** loader-writer library is planned.

### Usage example
Loading JSON from a file `test.json`
```c
JSON::Object obj;
JSON::Result result = JSON::loadFromFile("C:/test.json", obj);
```
The `JSON::Object`-class holds a tree of objects, arrays, and values corresponding to the document structure. 
In the above example, `obj` becomes the document root. If the input file is standard JSON, `obj` will contain a nested JSON object or array, or a single value. 
I.e., `obj[0]` is the first structure in the JSON text, and its children are the nested data.

The `JSON::Result` enum has multiple possible error codes, but `OK` is always returned on success.
```c
if (result == JSON::Result::OK)
{
  // successfully parsed JSON document
}
```

### Serialize to text
To dump a `JSON::Object` as a string, simply call the function. This will reformat the content as human-readable text (or alternatively `toString(false)`, for compact JSON without whitespace).
```c
std::string jstr = obj.toString();
```


UTF-16 and UTF-32 compatibility is planned, but currently only UTF-8 is supported. 
The IETF standard recommends using UTF-8, but it does not explicitly disallow the other encodings in all circumstances, thus, lacking support could be considered mildly non-compliant.
