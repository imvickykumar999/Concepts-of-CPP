The error indicates that the compiler cannot locate the `httpserver.hpp` file. This likely means the library is not installed or not correctly configured. Below is how to resolve this issue step by step:

---

### Steps to Use `CPP-HTTP-LIB`:

#### 1. Download the Library
You can download the library from its repository:

```bash
git clone https://github.com/etr/libhttpserver.git
cd libhttpserver
```

#### 2. Install the Required Dependencies
The library requires `libmicrohttpd` to function. Install it using your package manager:

- **For Windows with MSYS2:**
  ```bash
  pacman -S mingw-w64-x86_64-libmicrohttpd
  ```
- **For Ubuntu:**
  ```bash
  sudo apt-get install libmicrohttpd-dev
  ```

#### 3. Build and Install `libhttpserver`
Inside the `libhttpserver` directory:

```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

For Windows, use `mingw32-make` instead of `make`.

#### 4. Verify Installation
Ensure the header file is accessible in your include path. The default is usually `/usr/local/include/`.

---

### Compiling the Code

Ensure you link against the library correctly. Assuming `libhttpserver` is installed in the standard location:

```bash
g++ -std=c++11 server.cpp -o server -lhttpserver -lmicrohttpd
```

- `-lhttpserver`: Links the HTTP server library.
- `-lmicrohttpd`: Links the microhttpd library.

---

### If Using a Local Copy
If you cannot or do not want to install the library globally, specify the path explicitly during compilation:

1. Include the path to `httpserver.hpp` using `-I`:
   ```bash
   g++ -std=c++11 -I/path/to/libhttpserver/include server.cpp -o server -L/path/to/libhttpserver/lib -lhttpserver -lmicrohttpd
   ```

2. Replace `/path/to/libhttpserver/` with the actual directory where the library resides.


```bash
surface@LAPTOP-716KFR11 UCRT64 /c/Users/surface/Documents/GitHub/Concepts-of-CPP/CPP-HTTP-LIB
$ g++ -std=c++11 server.cpp -o server -I./libhttpserver/src -lhttpserver
In file included from server.cpp:1:
./libhttpserver/src/httpserver.hpp:25:4: error: #error ("libhttpserver requires C++17 or later.")
   25 | #  error("libhttpserver requires C++17 or later.")
      |    ^~~~~
In file included from ./libhttpserver/src/httpserver/basic_auth_fail_response.hpp:29,
                 from ./libhttpserver/src/httpserver.hpp:3 :
./libhttpserver/src/httpserver/http_utils.hpp:45:10: fatal error: microhttpd.h: No such file or directory
   45 | #include <microhttpd.h>
      |          ^~~~~~~~~~~~~~
compilation terminated.

surface@LAPTOP-716KFR11 UCRT64 /c/Users/surface/Documents/GitHub/Concepts-of-CPP/CPP-HTTP-LIB
$
```
