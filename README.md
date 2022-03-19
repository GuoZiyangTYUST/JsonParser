# JsonParser
json paser with single .h file and c++17


## example

```cpp
#include "JsonUtils.h"
int main()
{
	auto JsonRoot=CJsonUtils::ParseFromFile("D:\\test.json");

	CJsonUtils::SaveTo(JsonRoot.get(), "D:\\test2.json");
}
```
