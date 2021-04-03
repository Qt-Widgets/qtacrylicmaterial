# QtAcrylicMaterial

Cross-platform blur effect inspired by [Microsoft's Acrylic blur](https://docs.microsoft.com/en-us/windows/uwp/design/style/acrylic).

## Features

- Cross-platform: support Windows (7 ~ 10), Linux and macOS.
- Support both Qt Widgets and Qt Quick.
- Very easy to build and use.

## Screenshots

## Notice

Rendering acrylic material surfaces is highly GPU-intensive, which can slow down the application, increase the power consumption on the devices on which the application is running.

## Build

```bash
cmake .
cmake --build .
cmake --install .
```

## Use

See [examples](/examples).

## License

```text
MIT License

Copyright (C) 2021 wangwenx190 (Yuhang Zhao)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
