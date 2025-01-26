# Concepts-of-CPP

### 1. **Install g++ (MinGW)**:
#### Step 1: Download MinGW
- Visit the [MinGW website](https://sourceforge.net/projects/mingw/) and download the installer.

#### Step 2: Install MinGW
- Run the installer.
- During installation, select the `gcc-g++` component for C++.

#### Step 3: Add MinGW to PATH
- Find where MinGW is installed (e.g., `C:\MinGW\bin`).
- Add it to your PATH:
  1. Press `Win + R`, type `sysdm.cpl`, and press Enter.
  2. Go to the **Advanced** tab and click **Environment Variables**.
  3. In the **System Variables**, find `Path`, and click **Edit**.
  4. Add the path to the MinGW `bin` folder (e.g., `C:\MinGW\bin`).

#### Step 4: Verify Installation
- Open a new terminal and type:
  ```cmd
  g++ --version
  ```
- If installed correctly, you will see the version information for `g++`.

---

### 2. **Alternative: Use a Different Compiler**
If installing MinGW isn't feasible, you can use other tools:
- **Visual Studio**: Download and install the [Microsoft Visual Studio](https://visualstudio.microsoft.com/) Community Edition.
- **Online Compiler**: Use an online tool like [OnlineGDB](https://www.onlinegdb.com/).

---

### 3. **Recompile and Run**
Once `g++` is installed:
1. Navigate to your project directory in the terminal:
   ```cmd
   cd C:\Users\surface\Documents\GitHub\Concepts-of-CPP\Hello World
   ```
2. Compile:
   ```cmd
   g++ hello.cpp -o hello
   ```
3. Run:
   ```cmd
   hello
   ```
