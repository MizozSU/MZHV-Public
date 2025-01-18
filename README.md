# MZHV - A type II hypervisor with EPT mapping API
This is a compact type II hypervisor solution for Windows/Intel(x86-64) platform that exposes driver level API for manipulating Intel EPT table mappings. It supports assigning a guest physical address to a different host physical addresses (separately for read, write and execute operations). This allows for a more fine-grained control over system memory that wouldn't be possible using only Windows's APIs.

## Solution
This solution consists of 2 projects:
* MZHVDriver - the hypervisor that virtualizes Windows OS, handles EPT mappings and exposes driver level APIs,
* MZHVDemo - a simple demo C program that shows usage examples.

## API (IOCTL Endpoints)
This section describes the two IOCTL endpoints accessible via `DeviceIoControl` for interacting with the driver. These endpoints manage memory page mappings.

### DRIVER_MAP: Change Page Mapping

**IOCTL Code:** `DRIVER_MAP`

##### Description
Modifies the mapping of a specific memory page by associating it with different read/write and code execution pages.

#### Input Parameters
- **Type:** `VOID*[3]`
- **Description:** An array containing three pointers:
  1. `originalAddress`: Pointer to the original page to be changed.
  2. `rwAddress`: Pointer to the page to be mapped for read/write access.
  3. `fetchAddress`: Pointer to the page to be mapped for code execution.

#### Output Parameters
- None.

#### Return Value
- **Type:** `BOOL`
  - **TRUE:** Mapping was changed successfully.
  - **FALSE:** Failed to change the mapping.

#### Example Usage
```
VOID* addresses[3] = { originalAddress, rwAddress, fetchAddress };
BOOL isSuccessful = DeviceIoControl(
  deviceHandle,
  DRIVER_MAP,
  &addresses,
  sizeof(addresses),
  NULL,
  0,
  NULL,
  NULL);
```

### DRIVER_UNMAP: Remove Page Mapping Change

**IOCTL Code:** `DRIVER_UNMAP`

#### Description
Restores a previously modified page mapping to its default state.

#### Input Parameters
- **Type:** `VOID*[1]`
- **Description:** An array containing a single pointer:
  - `originalAddress`: Pointer to the original page to be restored.

#### Output Parameters
- None.

#### Return Value
- **Type:** `BOOL`
  - **TRUE:** Mapping was restored successfully.
  - **FALSE:** Failed to restore the mapping.

#### Example Usage
```
VOID* addresses[1] = { originalAddress };
BOOL isSuccessful = DeviceIoControl(
  deviceHandle,
  DRIVER_UNMAP,
  &addresses,
  sizeof(addresses),
  NULL,
  0,
  NULL,
  NULL);
```


## How to run
### Compiling
To compile the project, you need to download all the dependencies. Open the Visual Studio solution file (MZHV/MZHV.sln). Once the program is open, build the solution (default key F7).

### Driver Installation
Open a command prompt with administrator privileges, and enable test signing:
```
bcdedit.exe -set TESTSIGNING ON
```
After executing the command, the system must be restarted.

To install the driver, execute the command with administrator privileges:
```
sc create MZHV type=kernel binPath="XYZ\MZHV\x64\Debug\MZHVDriver.sys"
```
Where `XYZ` is the absolute path to the Visual Studio solution folder.

### Starting the Driver (System Virtualization)
In a command prompt with administrator privileges, type:
```
sc start MZHV
```

### Running Examples
To run the examples, open the Visual Studio solution file (MZHV/MZHV.sln). Right-click on the `Solution 'MZHV'` in the Solution Explorer window (panel on the left side of the program) and select `Properties`. Then ensure that in the `Common Properties > Startup Project` tab, the `Single startup project` option is selected, and `MZHVDemo` is chosen. Once this option is set, you can run the examples using the shortcut `CTRL+F5`.

To run an example that requires using the debugger, uncomment the example in the `main` function of the `demo.c` file (you might want to comment out the other two examples) and recompile the solution. Then, download the OllyDbg (x64) program, open it with administrator privileges, choose `File > Open`, and specify the path:
```
"XYZ\MZHV\x64\Debug\MZHVDemo.exe"
```
Where `XYZ` is the absolute path to the Visual Studio solution folder.

You can trace the behavior of the example using the `F9` key.

## Uninstalling the Driver
Stop the driver using the command:
```
sc stop MZHV
```

Then, delete it with:
```
sc delete MZHV
```

You might also want to disable test signing:
```
bcdedit.exe -set TESTSIGNING OFF
```

## Dependencies
The solution was built and tested using:
* Windows 10 Home 22H2 19045.3693
* Windows Driver Kit - Windows 10.0.22621.2428
* Windows Software Development Kit - Windows 10.0.22621.2428
* Microsoft Visual C++ 2015-2022 Redistributable (x86 & x64) 14.36.32532
* Visual Studio Community 2022 17.7.6
